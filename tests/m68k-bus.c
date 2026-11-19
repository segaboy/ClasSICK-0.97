/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/memory-probe/probe.h"
#include "../apps/aline-probe/program.h"
#include "../core/cpu/m68k-guard.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
typedef struct {
    uint8_t code[64],data[64],stack[64];cs_guest_region regions[3];cs_guest_map map;
    cs_trap_gateway gateway;cs_trap_frame frame;cs_m68k_bus_report report;
} fixture;
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->data,0xCC,64);memset(x->stack,0xCC,64);
    for(unsigned i=0;i<32;++i) {x->code[i*2]=0x4E;x->code[i*2+1]=0x71;}
    x->regions[0]=(cs_guest_region){x->code,0,0x401000,64,5};
    x->regions[1]=(cs_guest_region){x->data,x->data,0x1800,64,3};
    x->regions[2]=(cs_guest_region){x->stack,x->stack,0x300000,64,3};
    x->map=(cs_guest_map){x->regions,3};
    cs_trap_config c={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&c)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xABCD0100u+i;x->frame.a[i]=0x1800;}
    x->frame.pc=0x401000;x->frame.a[7]=0x300020;x->frame.sr=0x271F;
}
static void word(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void longword(uint8_t *p,uint32_t v) {word(p,(uint16_t)(v>>16));word(p+2,(uint16_t)v);}
static cs_m68k_result step(fixture *x)
{return cs_m68k_bus_step(&x->map,&x->gateway,&x->frame,&x->report);}
static void unchanged_failure(fixture *x,cs_m68k_result expected)
{
    cs_trap_frame before=x->frame;cs_trap_gateway g=x->gateway;uint8_t data[64],stack[64],code[64];
    memcpy(data,x->data,64);memcpy(stack,x->stack,64);memcpy(code,x->code,64);
    REQUIRE(step(x)==expected && memcmp(&before,&x->frame,sizeof before)==0 &&
        memcmp(&g,&x->gateway,sizeof g)==0 && memcmp(data,x->data,64)==0 &&
        memcmp(stack,x->stack,64)==0 && memcmp(code,x->code,64)==0);
}
static unsigned size_field(unsigned width) {return width==1?1u:width==2?3u:2u;}
static uint16_t move_word(unsigned width,unsigned sm,unsigned sr,unsigned dm,unsigned dr)
{return (uint16_t)(size_field(width)*4096u+dr*512u+dm*64u+sm*8u+sr);}
static void registers(void)
{
    const uint32_t values[]={0,0x7F,0x80,0x7FFF,0x8000,0x89ABCDEF};
    for(unsigned width=1;width<=4;width*=2) for(unsigned src=0;src<8;++src)
    for(unsigned dest=0;dest<8;++dest) for(unsigned n=0;n<6;++n) {
        fixture x;setup(&x);word(x.code,move_word(width,0,src,0,dest));x.frame.d[src]=values[n];
        cs_trap_frame before=x.frame;uint32_t mask=width==1?255u:width==2?65535u:UINT32_MAX;
        uint32_t value=values[n]&mask,sign=width==1?128u:width==2?32768u:0x80000000u;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[dest]==((before.d[dest]&~mask)|value));
        REQUIRE(x.frame.sr==(uint16_t)(0x2710u+(value==0?4u:((value&sign)?8u:0u))));
        REQUIRE(x.frame.pc==before.pc+2);x.frame.pc=before.pc;x.frame.sr=before.sr;x.frame.d[dest]=before.d[dest];
        REQUIRE(memcmp(&x.frame,&before,sizeof before)==0);
    }
    for(unsigned width=2;width<=4;width*=2) for(unsigned src=0;src<8;++src)
    for(unsigned dest=0;dest<8;++dest) {
        fixture x;setup(&x);x.frame.a[src]=0x12348001;word(x.code,move_word(width,1,src,1,dest));
        cs_trap_frame before=x.frame;REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[dest]==(width==2?0xFFFF8001u:0x12348001u));
        REQUIRE(x.frame.sr==before.sr);x.frame.pc=before.pc;x.frame.a[dest]=before.a[dest];
        REQUIRE(memcmp(&x.frame,&before,sizeof before)==0);
    }
    fixture x;setup(&x);word(x.code,move_word(1,1,0,0,0));unchanged_failure(&x,CS_M68K_OPCODE);
    setup(&x);word(x.code,move_word(1,0,0,1,0));unchanged_failure(&x,CS_M68K_OPCODE);
    setup(&x);word(x.code,0x70FF);REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[0]==UINT32_MAX && x.frame.sr==0x2718);
}
static void absolute_short(void)
{
    fixture x;
    const cs_m68k_bus_options data24={CS_M68K_DATA24,NULL,NULL};

    /* Two distinct extension words, source before destination. The last source
     * byte at $7fff is legal for MOVE.B despite the odd effective address. */
    for(unsigned width=1;width<=4;width*=2) {
        setup(&x);
        x.regions[1].base=0x7fc0;
        unsigned offset=64-width;
        x.data[offset]=0x80;
        word(x.code,move_word(width,7,0,7,0));
        word(x.code+2,(uint16_t)(0x7fc0+offset));word(x.code+4,0x7fc4);
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401006 && x.frame.sr==0x2718);
        REQUIRE(!memcmp(x.data+4,x.data+offset,width) && x.data[3]==0xcc && x.data[4+width]==0xcc);
    }

    /* A low positive address with a negative operand exercises the independent
     * sign extension of MOVEA.W. MOVEA.L retains all source bits. */
    for(unsigned width=2;width<=4;width*=2) {
        setup(&x);word(x.code,move_word(width,7,0,1,3));word(x.code+2,0x1804);
        longword(x.data+4,0x80011234);
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[3]==(width==2?0xffff8001u:0x80011234u));
        REQUIRE(x.frame.pc==0x401004 && x.frame.sr==0x271f);
    }

    for(unsigned dest=0;dest<=1;++dest) {
        setup(&x);
        x.regions[1].base=0xff8000;
        x.regions[2].base=0x8000; /* Deliberate decoy for zero extension. */
        word(x.code,dest?move_word(2,0,2,7,0):move_word(2,7,0,0,2));
        word(x.code+2,0x8000);word(x.data,0x1357);word(x.stack,0x99aa);
        unchanged_failure(&x,CS_M68K_ADDRESS);
        REQUIRE(x.report.address==0xffff8000u);
        REQUIRE(cs_m68k_bus_step_options(&x.map,&x.gateway,&x.frame,&x.report,&data24)==CS_M68K_OK);
        REQUIRE(x.frame.pc==0x401004 && x.stack[0]==0x99 && x.stack[1]==0xaa);
        if(dest) REQUIRE(x.data[0]==1 && x.data[1]==2 && x.frame.d[2]==0xabcd0102u);
        else REQUIRE(x.frame.d[2]==0xabcd1357u);
    }

    /* The source postincrement is tentative until the extension and destination
     * checks complete. Capture all registers, gateway state and mapped bytes. */
    for(unsigned failure=0;failure<3;++failure) {
        setup(&x);word(x.code,move_word(2,3,0,7,0));word(x.code+2,failure==1?0x1801:0x1804);
        if(failure==0)x.regions[0].size=2;
        if(failure==2)x.regions[1].flags=CS_GUEST_READ;
        unchanged_failure(&x,CS_M68K_ADDRESS);
    }
    setup(&x);word(x.code,move_word(1,7,0,1,4));word(x.code+2,0x1800);
    unchanged_failure(&x,CS_M68K_OPCODE);
}
typedef struct {cs_m68k_guard_kind kind;uint32_t address;} move_guard_point;
static int permit_except(const void *context,const cs_m68k_guard_event *event)
{
    const move_guard_point *denied=context;
    return denied->kind!=event->kind || denied->address!=event->address;
}
static void displacement_and_index(void)
{
    fixture x;
    setup(&x);x.frame.a[4]=0x1810;x.frame.d[2]=0xcafefffcu;
    x.frame.a[3]=42;x.frame.d[5]=12;
    word(x.code,move_word(2,7,2,5,4));word(x.code+2,46);word(x.code+4,0xfffa);
    word(x.code+6,move_word(2,6,4,1,6));word(x.code+8,0x20fe);
    word(x.code+10,move_word(4,7,3,6,4));word(x.code+12,0xb8fe);word(x.code+14,0x58fc);
    word(x.code+16,move_word(1,5,4,6,4));word(x.code+18,9);word(x.code+20,0xb001);
    word(x.code+48,0xf90d);longword(x.code+52,0x81ac3567);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401006 && x.frame.sr==0x2718);
    REQUIRE(x.data[10]==0xf9 && x.data[11]==0x0d && x.data[9]==0xcc && x.data[12]==0xcc);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[6]==0xfffff90du && x.frame.sr==0x2718);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401010);
    REQUIRE(!memcmp(x.data+24,x.code+52,4) && x.data[23]==0xcc && x.data[28]==0xcc);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401016 && x.data[59]==0xac);

    /* Destination indexing sees the proposed source postincrement. */
    setup(&x);x.frame.d[2]=2;
    word(x.code,move_word(2,3,0,6,0));word(x.code+2,0x2000);
    word(x.data,0xed32);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x1802 && x.data[4]==0xed && x.data[5]==0x32);

    /* Full-width base arithmetic precedes the configured data-address policy. */
    setup(&x);x.frame.a[0]=0xaa001804u;
    word(x.code,move_word(2,5,0,0,0));word(x.code+2,0xfffe);word(x.data+2,0x3a8c);
    unchanged_failure(&x,CS_M68K_ADDRESS);
    const cs_m68k_bus_options data24={CS_M68K_DATA24,NULL,NULL};
    REQUIRE(cs_m68k_bus_step_options(&x.map,&x.gateway,&x.frame,&x.report,&data24)==CS_M68K_OK);
    REQUIRE(x.frame.d[0]==0xabcd3a8cu && x.frame.sr==0x2710);

    /* Negative PC displacement, including a negative longword index. */
    setup(&x);x.frame.pc+=24;x.frame.d[7]=0xfffffff0u;
    word(x.code+24,move_word(4,7,3,1,1));word(x.code+26,0x78fc);
    longword(x.code+6,0x80abcdf1u);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[1]==0x80abcdf1u && x.frame.sr==0x271f);
    for(unsigned reg=2;reg<=3;++reg){
        setup(&x);word(x.code,move_word(2,3,0,7,reg));word(x.code+2,8);
        unchanged_failure(&x,CS_M68K_OPCODE);
    }
    for(unsigned bit=8;bit<=10;++bit){
        setup(&x);word(x.code,move_word(2,3,0,6,0));word(x.code+2,(uint16_t)(1u<<bit));
        unchanged_failure(&x,CS_M68K_OPCODE);
    }
    setup(&x);word(x.code,move_word(4,3,0,5,0));word(x.code+2,1);
    unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);x.regions[0].size=2;word(x.code,move_word(2,7,3,0,1));
    unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);x.frame.a[0]=0xfffffff0u;x.frame.d[0]=0x30;
    word(x.code,move_word(2,6,0,0,1));word(x.code+2,0x08d0);
    unchanged_failure(&x,CS_M68K_ADDRESS);

    const move_guard_point points[]={
        {CS_M68K_GUARD_EXTENSION,0x401002},{CS_M68K_GUARD_READ,0x40100c},
        {CS_M68K_GUARD_EXTENSION,0x401004},{CS_M68K_GUARD_WRITE,0x1804},
        {CS_M68K_GUARD_COMMIT,0x401006}
    };
    const cs_m68k_stack_limits limits={0x300000,64};
    for(unsigned i=0;i<sizeof points/sizeof points[0];++i){
        setup(&x);x.frame.d[1]=2;
        word(x.code,move_word(2,7,2,6,0));word(x.code+2,10);word(x.code+4,0x1002);
        cs_trap_frame before=x.frame;cs_trap_gateway gateway=x.gateway;
        uint8_t data[64];memcpy(data,x.data,sizeof data);
        const cs_m68k_guard guard={permit_except,&points[i],sizeof points[i]};
        REQUIRE(cs_m68k_bus_step_guarded(&x.map,&x.gateway,&x.frame,&x.report,&limits,&guard)==CS_M68K_POLICY);
        REQUIRE(!memcmp(&before,&x.frame,sizeof before) && !memcmp(&gateway,&x.gateway,sizeof gateway) &&
            !memcmp(data,x.data,sizeof data));
    }
}
static void operands(void)
{
    absolute_short();
    displacement_and_index();
    for(unsigned width=1;width<=4;width*=2) for(unsigned mode=2;mode<=4;++mode)
    for(unsigned reg=0;reg<8;++reg) for(unsigned dest=0;dest<2;++dest) {
        fixture x;setup(&x);uint8_t *bytes=reg==7?x.stack:x.data;uint32_t base=reg==7?0x300000u:0x1800u;
        unsigned stride=width==1 && reg==7?2u:width;x.frame.a[reg]=base+(mode==4?stride:0u);
        bytes[0]=0x81;bytes[1]=0x23;bytes[2]=0x45;bytes[3]=0x67;
        uint32_t value=width==1?0x81u:width==2?0x8123u:0x81234567u;
        if(dest) {x.frame.d[0]=0x81234567;word(x.code,move_word(width,0,0,mode,reg));}
        else word(x.code,move_word(width,mode,reg,0,0));
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[reg]==base+(mode==3?stride:0u));
        if(dest) {
            uint32_t out=0;REQUIRE(cs_guest_read(&x.map,base,width,1,&out)==CS_GUEST_OK &&
                out==(width==1?0x67u:width==2?0x4567u:0x81234567u));
        } else REQUIRE(x.frame.d[0]==(width==1?0xABCD0181u:width==2?0xABCD8123u:value));
    }
    fixture x;setup(&x);word(x.code,move_word(4,7,4,7,1));longword(x.code+2,0x89ABCDEF);longword(x.code+6,0x1800);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x40100A && x.data[0]==0x89 && x.data[3]==0xEF);
    setup(&x);word(x.code,move_word(2,7,1,0,3));longword(x.code+2,0x1800);x.data[0]=0xFE;x.data[1]=0xDC;
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[3]==0xABCDFEDC && x.frame.pc==0x401006);
    setup(&x);word(x.code,move_word(1,7,4,0,0));word(x.code+2,0xAB00);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[0]==0xABCD0100 && x.frame.sr==0x2714);
    setup(&x);word(x.code,move_word(2,7,4,1,4));word(x.code+2,0x8001);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[4]==0xFFFF8001 && x.frame.sr==0x271F);
    /* Source postincrement happens before destination evaluation of the same An. */
    setup(&x);word(x.code,move_word(2,3,0,3,0));x.data[0]=0x11;x.data[1]=0x22;
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x1804 && x.data[2]==0x11 && x.data[3]==0x22);
    setup(&x);word(x.code,move_word(4,3,0,1,0));longword(x.data,0xFEDCBA98);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0xFEDCBA98 && x.frame.sr==0x271F);
}
static void branches(void)
{
    /* Independent enumerated truth masks indexed by NZVC, not CPU formulas. */
    const uint16_t masks[16]={0xFFFF,0xFFFF,0x0505,0xFAFA,0x5555,0xAAAA,0x0F0F,0xF0F0,
        0x3333,0xCCCC,0x00FF,0xFF00,0xCC33,0x33CC,0x0C03,0xF3FC};
    for(unsigned c=0;c<16;++c) for(unsigned bits=0;bits<16;++bits) for(unsigned ext=0;ext<2;++ext) {
        fixture x;setup(&x);word(x.code,(uint16_t)(0x6000u+c*256u+(ext?0u:6u)));word(x.code+2,6);
        x.frame.sr=(uint16_t)(0x2710u+bits);int taken=(masks[c]&(1u<<bits))!=0;
        cs_trap_frame before=x.frame;REQUIRE(step(&x)==CS_M68K_OK && x.frame.sr==before.sr);
        REQUIRE(x.frame.pc==(taken?0x401008u:ext?0x401004u:0x401002u));
        REQUIRE(x.report.branch_taken==(unsigned)taken && x.report.subroutine==(unsigned)(c==1));
        REQUIRE(x.frame.a[7]==before.a[7]-(c==1?4u:0u));
        if(c==1) REQUIRE(x.stack[28]==0 && x.stack[29]==0x40 && x.stack[30]==0x10 && x.stack[31]==(ext?4:2));
    }
    fixture x;setup(&x);x.frame.pc+=8;word(x.code+8,0x60F6);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401000);
    setup(&x);x.frame.pc+=8;word(x.code+8,0x6000);word(x.code+10,0xFFF6);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401000);
    setup(&x);word(x.code,0x60FF);unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);word(x.code,0x6000);word(x.code+2,0);REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401002);
    setup(&x);word(x.code,0x6106);x.frame.a[7]=0x300002;unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);word(x.code,0x6106);x.regions[2].flags=1;unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);word(x.code,0x6000);word(x.code+2,0x1800);unchanged_failure(&x,CS_M68K_ADDRESS);
    setup(&x);word(x.code,0x4E75);longword(x.stack+32,0x401002);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x401002 && x.frame.a[7]==0x300024 && x.frame.sr==0x271F);
    setup(&x);word(x.code,0x4E75);longword(x.stack+32,0x1800);unchanged_failure(&x,CS_M68K_ADDRESS);
}
static void failures(void)
{
    fixture x;
    for(unsigned n=0;n<16;++n) {
        setup(&x);word(x.code,move_word(4,3,0,7,1));longword(x.code+2,0x401010);
        switch(n) {
            case 0:break;case 1:x.frame.a[0]++;break;
            case 2:x.frame.a[0]=0x183E;break;case 3:x.frame.a[0]=0x1001800;break;
            case 4:x.regions[0].size=4;break;case 5:x.regions[0].flags=1;break;
            case 6:x.regions[2].base=0x1800;break;case 7:x.frame.sr=0x071F;break;
            case 8:x.frame.sr=0xA71F;break;case 9:x.frame.pc++;break;
            case 10:word(x.code,move_word(4,7,5,0,0));break;
            case 11:word(x.code,move_word(4,0,0,7,4));break;
            case 12:word(x.code,0x4AFC);break;
            case 13:word(x.code,move_word(2,4,0,0,0));x.frame.a[0]=0;break;
            case 14:x.frame.pc=0x1000000;break;
            default:x.regions[0].base=0xFFFFF8;x.regions[0].size=8;x.frame.pc=0xFFFFF8;
                word(x.code,move_word(4,7,4,7,1));longword(x.code+2,0x11223344);break;
        }
        cs_m68k_result expected=(n==7 || n==8)?CS_M68K_MODE:(n>=10 && n<=12)?CS_M68K_OPCODE:CS_M68K_ADDRESS;
        unchanged_failure(&x,expected);
    }
    setup(&x);REQUIRE(cs_m68k_bus_step(NULL,&x.gateway,&x.frame,&x.report)==CS_M68K_ARGUMENT);
    REQUIRE(cs_m68k_bus_step(&x.map,NULL,&x.frame,&x.report)==CS_M68K_ARGUMENT);
    REQUIRE(cs_m68k_bus_step(&x.map,&x.gateway,NULL,&x.report)==CS_M68K_ARGUMENT);
    REQUIRE(cs_m68k_bus_step(&x.map,&x.gateway,&x.frame,NULL)==CS_M68K_ARGUMENT);
}
static void services(void)
{
    static const uint8_t program[]={CS_ALINE_PROGRAM_BYTES,0x4E,0x71};fixture x;setup(&x);
    memcpy(x.code,program,sizeof program);longword(x.stack+32,0x40100C);unsigned calls=0;
    for(unsigned i=0;i<6;++i) {REQUIRE(step(&x)==CS_M68K_OK);calls+=x.report.cpu.service_called;}
    REQUIRE(calls==3 && x.frame.pc==0x40100C && x.frame.a[7]==0x300024 && x.frame.d[0]==32 &&
        x.frame.a[0]==0x400100 && x.frame.sr==0x2710 && x.gateway.entries[32]==0x400100 &&
        x.stack[26]==0x27 && x.stack[27]==0x10 && x.stack[31]==0x0A);
    setup(&x);word(x.code,0xA146);x.frame.d[0]=0x46;x.regions[2].flags=1;unchanged_failure(&x,CS_M68K_ADDRESS);
    REQUIRE(x.report.cpu.service_called==0);
    setup(&x);word(x.code,0xA146);x.frame.d[0]=0x46;x.gateway.entries[0x46]=0x10000;
    unchanged_failure(&x,CS_M68K_SERVICE);REQUIRE(x.report.cpu.service.status==CS_TRAP_NEEDS_EXECUTION);
    setup(&x);word(x.code,0xA047);x.frame.d[0]=32;x.frame.a[0]=3;
    unchanged_failure(&x,CS_M68K_SERVICE);
}
static void probe(void)
{
    fixture x;setup(&x);cs_trap_gateway before=x.gateway;cs_memory_probe_result r;
    REQUIRE(cs_memory_probe(&x.gateway,&r) && r.checks==16 && r.status==0 && r.signature==0x4D454D31 &&
        r.steps==12 && r.subroutines==1 && memcmp(&before,&x.gateway,sizeof before)==0);
    REQUIRE(cs_memory_probe(&x.gateway,&r));
    REQUIRE(!cs_memory_probe(NULL,&r) && !cs_memory_probe(&x.gateway,NULL));
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"registers")) registers();
    else if(!strcmp(argv[1],"operands")) operands();
    else if(!strcmp(argv[1],"branches")) branches();
    else if(!strcmp(argv[1],"failures")) failures();
    else if(!strcmp(argv[1],"services")) services();
    else if(!strcmp(argv[1],"probe")) probe();
    else return 2;
    return 0;
}
