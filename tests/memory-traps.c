/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../personality/system1/memory-traps.h"
#include "../apps/guest-heap-probe/program.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
typedef struct {
    uint8_t ram[0x4000],code[256];cs_guest_region regions[2];cs_guest_map map;
    cs_mm_manager memory;cs_trap_gateway gateway;cs_s1_traps runtime;
    cs_trap_frame frame;cs_m68k_bus_report cpu;cs_s1_report service;
} fixture;
static const uint16_t words[11]={0xA11E,0xA01F,0xA020,0xA021,0xA122,0xA023,0xA024,0xA025,0xA029,0xA02A,0xA02E};
static void put16(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void put32(uint8_t *p,uint32_t v) {put16(p,(uint16_t)(v>>16));put16(p+2,(uint16_t)v);}
static uint32_t get32(const uint8_t *p)
{return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->ram,0xCC,sizeof x->ram);
    for(unsigned i=0;i<sizeof x->code;i+=2) put16(x->code+i,0x4E71);
    x->regions[0]=(cs_guest_region){x->ram,x->ram,0,sizeof x->ram,3};
    x->regions[1]=(cs_guest_region){x->code,0,0x401000,sizeof x->code,5};
    x->map=(cs_guest_map){x->regions,2};
    cs_mm_config mc={{0x500,0x1000,0x1800},{0x600,0x2000,0x3000}};
    REQUIRE(cs_mm_init(&x->memory,&x->map,&mc)==CS_MM_OK);
    cs_trap_config tc={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&tc)==CS_TRAP_OK);
    REQUIRE(cs_s1_traps_init(&x->runtime,&x->gateway,&x->memory,0x400200,CS_M68K_DATA24)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xCAFE0100u+i;x->frame.a[i]=0x3500u+i*8u;}
    x->frame.pc=0x401000;x->frame.a[7]=0x3F00;x->frame.sr=0x271F;
}
static cs_m68k_result step(fixture *x)
{return cs_s1_step(&x->runtime,0,&x->frame,&x->cpu,&x->service);}
static uint32_t allocate(fixture *x,int handle,unsigned bytes)
{
    cs_trap_frame f=x->frame;cs_s1_report r;f.d[0]=bytes;
    REQUIRE(cs_s1_trap_call(&x->runtime,handle?0xA322:0xA31E,0,&f,&r)==CS_TRAP_OK && r.memory.reply.code==0);
    return f.a[0];
}
static void failure(fixture *x,cs_m68k_result expected,unsigned interrupt_context)
{
    cs_trap_frame f=x->frame;cs_mm_manager m=x->memory;cs_trap_gateway g=x->gateway;
    uint8_t ram[sizeof x->ram],code[sizeof x->code];memcpy(ram,x->ram,sizeof ram);memcpy(code,x->code,sizeof code);
    REQUIRE(cs_s1_step(&x->runtime,interrupt_context,&x->frame,&x->cpu,&x->service)==expected);
    REQUIRE(!memcmp(&f,&x->frame,sizeof f) && !memcmp(&m,&x->memory,sizeof m) && !memcmp(&g,&x->gateway,sizeof g));
    REQUIRE(!memcmp(ram,x->ram,sizeof ram) && !memcmp(code,x->code,sizeof code));
}
static void bindings(void)
{
    fixture x;setup(&x);
    for(unsigned i=0;i<11;++i) REQUIRE(x.gateway.entries[words[i]&255u]==0x400200u+i*2u);
    const uint32_t bases[]={0x400201,0x3FFFFE,0x40FFEC,0x400100,0x40010C,0x10000,0xFFFFFFFE};
    for(unsigned i=0;i<sizeof bases/sizeof bases[0];++i) {
        cs_s1_traps s=x.runtime;cs_trap_gateway g=x.gateway;cs_mm_manager m=x.memory;
        uint8_t ram[sizeof x.ram];memcpy(ram,x.ram,sizeof ram);
        REQUIRE(cs_s1_traps_init(&x.runtime,&x.gateway,&x.memory,bases[i],CS_M68K_DATA24)==CS_TRAP_ADDRESS);
        REQUIRE(!memcmp(&s,&x.runtime,sizeof s) && !memcmp(&g,&x.gateway,sizeof g) &&
            !memcmp(&m,&x.memory,sizeof m) && !memcmp(ram,x.ram,sizeof ram));
    }
    REQUIRE(cs_s1_traps_init(&x.runtime,&x.gateway,&x.memory,0x40FFEA,CS_M68K_STRICT)==CS_TRAP_OK);
    REQUIRE(x.gateway.entries[0x2E]==0x40FFFE);
    setup(&x);x.memory.magic=0;cs_trap_gateway g=x.gateway;cs_s1_traps s=x.runtime;
    REQUIRE(cs_s1_traps_init(&x.runtime,&x.gateway,&x.memory,0x400300,CS_M68K_STRICT)==CS_TRAP_STATE);
    REQUIRE(!memcmp(&g,&x.gateway,sizeof g) && !memcmp(&s,&x.runtime,sizeof s));
    setup(&x);REQUIRE(cs_s1_traps_init(NULL,&x.gateway,&x.memory,0x400200,CS_M68K_STRICT)==CS_TRAP_ARGUMENT);
    REQUIRE(cs_s1_traps_init(&x.runtime,&x.gateway,&x.memory,0x400200,(cs_m68k_data_policy)2)==CS_TRAP_ARGUMENT);
}
static void routes(void)
{
    /* Existing independently tested register boundary is the marshalling oracle;
     * CPU continuation/exception bytes and untouched registers are checked here. */
    for(unsigned i=0;i<11;++i) for(unsigned flags=0;flags<(i==0 || i==4?4u:1u);++flags)
    for(unsigned a0=0;a0<2;++a0) {
        fixture x,y;setup(&x);setup(&y);
        uint32_t object=allocate(&x,i>=4 && i<=9,16),other=allocate(&y,i>=4 && i<=9,16);
        REQUIRE(object==other);x.frame.a[0]=object;x.frame.a[1]=0x3400;x.frame.d[0]=8;y.frame=x.frame;
        uint16_t word=(uint16_t)((words[i]&0xFEFFu)|(a0?0x100u:0u)|flags*512u);
        put16(x.code,word);cs_mm_register_report oracle;
        REQUIRE(cs_mm_register_call(&y.memory,word,0,&y.frame,&oracle)==CS_MM_OK);
        REQUIRE(step(&x)==CS_M68K_OK && x.service.memory_called==1 && x.service.memory.status==CS_MM_OK);
        REQUIRE(!memcmp(x.frame.d,y.frame.d,sizeof x.frame.d) && !memcmp(x.frame.a,y.frame.a,sizeof x.frame.a) && x.frame.sr==y.frame.sr);
        REQUIRE(x.frame.pc==y.frame.pc+2 && x.frame.a[7]==0x3F00);
        REQUIRE(x.service.memory.reply.code==oracle.reply.code && x.service.memory.reply.value==oracle.reply.value);
        REQUIRE(!memcmp(x.memory.blocks,y.memory.blocks,sizeof x.memory.blocks));
        REQUIRE(!memcmp(x.ram,y.ram,0x3EFA) && !memcmp(x.ram+0x3F00,y.ram+0x3F00,0x100));
        REQUIRE(x.ram[0x3EFA]==(uint8_t)(x.frame.sr>>8) && x.ram[0x3EFB]==(uint8_t)x.frame.sr && get32(x.ram+0x3EFC)==0x401002);
    }
    fixture x;setup(&x);x.frame.d[0]=0x1E;put16(x.code,0xA146);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x400200);
    cs_trap_frame f=x.frame;f.d[0]=12;cs_s1_report r;
    REQUIRE(cs_s1_trap_direct(&x.runtime,0x400200,0xA31E,0,&f,&r)==CS_TRAP_OK && r.memory.reply.code==0 && f.pc==x.frame.pc);
    x.frame.pc=0x401000;x.frame.d[0]=0x1E;x.frame.a[0]=0x10000;put16(x.code,0xA047);
    REQUIRE(step(&x)==CS_M68K_OK && x.gateway.entries[0x1E]==0x10000);
    x.frame.pc=0x401000;put16(x.code,0xA11E);failure(&x,CS_M68K_SERVICE,0);
    REQUIRE(x.cpu.cpu.service.status==CS_TRAP_NEEDS_EXECUTION && x.service.memory_called==0);
    REQUIRE(cs_s1_trap_direct(&x.runtime,0x400200,0xA31E,0,&f,&r)==CS_TRAP_OK);
    cs_trap_frame saved=f;uint8_t ram[sizeof x.ram];memcpy(ram,x.ram,sizeof ram);
    REQUIRE(cs_s1_trap_direct(&x.runtime,0x400208,0xA31E,0,&f,&r)==CS_TRAP_UNSUPPORTED_FORMAT && r.failure==CS_S1_SIGNATURE);
    REQUIRE(!memcmp(&saved,&f,sizeof f) && !memcmp(ram,x.ram,sizeof ram));
    REQUIRE(cs_s1_trap_direct(&x.runtime,0x80400200,0xA31E,0,&f,&r)==CS_TRAP_ADDRESS);
    REQUIRE(!memcmp(&saved,&f,sizeof f) && !memcmp(ram,x.ram,sizeof ram));
    setup(&x);x.frame.d[0]=16;put16(x.code,0xA11E);
    REQUIRE(cs_m68k_bus_step(&x.map,&x.gateway,&x.frame,&x.cpu)==CS_M68K_SERVICE &&
        x.cpu.cpu.service.status==CS_TRAP_NEEDS_EXECUTION && x.memory.blocks[0].kind==0);
    fixture y;setup(&y);REQUIRE(y.gateway.entries[0x1E]==0x400200 && y.memory.blocks[0].kind==0);
}
static void failures(void)
{
    fixture x;
    for(unsigned i=0;i<17;++i) {
        setup(&x);put16(x.code,0xA11E);x.frame.d[0]=16;
        switch(i) {
            case 0:x.frame.a[7]=4;break;
            case 1:x.frame.a[7]++;break;
            case 2:x.frame.a[7]=0x1003F00;break;
            case 3:x.frame.sr=0x071F;break;
            case 4:x.frame.pc=0x80401000;break;
            case 5:x.regions[0].flags=1;break;
            case 6:x.gateway.entries[0x1E]=0x400208;break;
            case 7:x.gateway.entries[0x1E]=0x400100;break;
            case 8:x.gateway.entries[0x1E]=0x400140;break;
            case 9:put16(x.code,0xA820);break;
            case 10:put16(x.code,0xA229);break;
            case 11:x.memory.magic=0;break;
            case 12:put32(x.ram+0x118,0);break;
            case 13:x.frame.a[7]=0x2800;break;
            case 14:put16(x.code,0xA02E);x.frame.a[0]=0x3400;x.frame.a[1]=0x3EF9;x.frame.d[0]=8;break;
            case 15:x.frame.a[7]=0x31C;break;
            default:x.frame.a[7]=0x1400;break;
        }
        failure(&x,i<=2 || i==4 || i==5?CS_M68K_ADDRESS:i==3?CS_M68K_MODE:CS_M68K_SERVICE,0);
        if(i==6 || i==7) REQUIRE(x.service.failure==CS_S1_SIGNATURE);
        if(i>=13) REQUIRE(x.service.failure==CS_S1_SCRATCH && x.service.memory_called==0);
    }
    setup(&x);put16(x.code,0xA11E);failure(&x,CS_M68K_SERVICE,1);
    REQUIRE(x.service.memory.status==CS_MM_INTERRUPT);
    setup(&x);put16(x.code,0xA11E);x.frame.d[0]=0x7FFFFFFF;
    REQUIRE(step(&x)==CS_M68K_OK && x.service.memory.reply.code==CS_MM_FULL && x.frame.a[0]==0 &&
        (uint16_t)x.frame.d[0]==(uint16_t)CS_MM_FULL && x.frame.pc==0x401002 && (x.frame.sr&15u)==8);
    setup(&x);put16(x.code,0xA025);x.frame.a[0]=0;
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[0]==(uint32_t)(int32_t)CS_MM_NIL_HANDLE && x.frame.pc==0x401002);
    cs_s1_report before=x.service;cs_m68k_bus_report cpu=x.cpu;
    REQUIRE(cs_s1_step(&x.runtime,2,&x.frame,&x.cpu,&x.service)==CS_M68K_ARGUMENT &&
        !memcmp(&before,&x.service,sizeof before) && !memcmp(&cpu,&x.cpu,sizeof cpu));
}
static void addresses(void)
{
    for(unsigned tag=0;tag<256;++tag) for(unsigned width=1;width<=4;width*=2) {
        fixture x;setup(&x);x.frame.a[0]=(tag<<24)|0x3400u;
        unsigned kind=width==1?1u:width==2?3u:2u;
        put16(x.code,(uint16_t)(kind*4096u+16u));put32(x.ram+0x3400,0x81234567);
        cs_trap_frame before=x.frame;REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==before.a[0]);
        uint32_t value=width==1?0x81u:width==2?0x8123u:0x81234567u;
        uint32_t mask=width==1?255u:width==2?65535u:UINT32_MAX;
        REQUIRE(x.frame.d[0]==((before.d[0]&~mask)|value) && x.cpu.address==0x3400);
        x.frame.pc=0x401000;put16(x.code,(uint16_t)(kind*4096u+128u));x.frame.d[0]=0x12345678;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==before.a[0]);
        REQUIRE(x.ram[0x3400]==(width==1?0x78:width==2?0x56:0x12));
    }
    fixture x;setup(&x);x.frame.a[0]=0x80003401;put16(x.code,0x2018);failure(&x,CS_M68K_ADDRESS,0);
    setup(&x);x.runtime.data_policy=CS_M68K_STRICT;x.frame.a[0]=0x80003400;put16(x.code,0x2010);failure(&x,CS_M68K_ADDRESS,0);
    setup(&x);x.frame.a[0]=0x80003400;put16(x.code,0x2018);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x80003404);
    setup(&x);put16(x.code,0x4E75);put32(x.ram+0x3F00,0x80401000);failure(&x,CS_M68K_ADDRESS,0);
    setup(&x);put16(x.code,0x6102);x.frame.a[7]=0x80003F00;failure(&x,CS_M68K_ADDRESS,0);
    setup(&x);x.regions[0].base=0xFFF000;x.regions[0].size=0x1000;x.frame.a[0]=0xFFFFFFFE;
    put16(x.code,0x2010);failure(&x,CS_M68K_ADDRESS,0);
    setup(&x);put16(x.code,0xA029);x.frame.a[0]=0x80002000;
    REQUIRE(step(&x)==CS_M68K_OK && x.service.memory.reply.code==CS_MM_WRONG_ZONE);
}
static void program(void)
{
    static const uint8_t bytes[]={CS_GUEST_HEAP_PROGRAM_BYTES};fixture x;setup(&x);
    REQUIRE(sizeof bytes==82);memcpy(x.code,bytes,sizeof bytes);put32(x.ram+0x3F00,0x401000+(uint32_t)sizeof bytes);
    unsigned calls=0;uint32_t old_payload=0;
    for(unsigned i=0;i<35;++i) {
        REQUIRE(step(&x)==CS_M68K_OK);calls+=x.cpu.cpu.service_called;
        if(x.service.memory_called) REQUIRE(x.service.memory.reply.code==0);
        if(i==1 || i==25) {
            uint32_t at=i==1?get32(x.ram+x.frame.a[0]):x.frame.a[0];
            for(unsigned n=0;n<16;++n) REQUIRE(x.ram[at+n]==0);
            if(i==1) {REQUIRE(x.frame.a[0]>=0x2000 && x.frame.a[0]<0x2080);old_payload=at;}
            else REQUIRE(at>=0x1080 && at<0x1800);
        }
        if(i==4 || i==15) REQUIRE((x.frame.a[1]&CS_MM_LOCK)!=0);
        if(i==11) REQUIRE(get32(x.ram+x.frame.a[2])!=old_payload);
    }
    REQUIRE(calls==15 && x.frame.pc==0x401052 && x.frame.a[7]==0x3F04 && x.frame.sr==0x2710);
    REQUIRE(x.frame.d[2]==0x11223344 && x.frame.d[3]==16 && x.frame.d[4]==24 &&
        x.frame.d[5]==0x11223344 && x.frame.d[6]==8 && x.frame.d[7]==0x47545231);
    REQUIRE(get32(x.ram+0x3400)==0x11223344 && get32(x.ram+x.frame.a[2])==0);
    for(unsigned i=0;i<CS_MM_BLOCKS;++i) REQUIRE(x.memory.blocks[i].kind==0);
    printf("Guest heap program PASS: 82 bytes, 35 steps, 15 traps, signature 47545231\n");
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"bindings")) bindings();else if(!strcmp(argv[1],"routes")) routes();
    else if(!strcmp(argv[1],"failures")) failures();else if(!strcmp(argv[1],"addresses")) addresses();
    else if(!strcmp(argv[1],"program")) program();else return 2;
    return 0;
}
