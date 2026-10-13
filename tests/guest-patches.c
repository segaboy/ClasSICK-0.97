/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original synthetic conformance fixtures. */
#include "../personality/system1/guest-patches.h"
#include "../apps/guest-patch-probe/program.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}} while(0)
typedef struct {
    uint8_t ram[0x4000],code[256],veneer[22],sentinels[8],root[8];
    cs_guest_region regions[5];cs_guest_map map;cs_mm_manager manager;
    cs_trap_gateway gateway;cs_s1_traps traps;cs_s1_patches runtime;
    cs_trap_frame frame;cs_s1_patch_report report;
} fixture;
static void word(uint8_t *p,uint16_t v){p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void longword(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>((3u-i)*8u));}
static int setup(fixture *x)
{
    memset(x,0,sizeof *x);
    for(unsigned i=0;i<256;i+=2) word(x->code+i,0x4E71);
    for(unsigned i=0;i<22;i+=2) word(x->veneer+i,0x4E75);
    for(unsigned i=0;i<8;i+=2) {word(x->sentinels+i,0x4E71);word(x->root+i,0x4E71);}
    x->regions[0]=(cs_guest_region){x->ram,x->ram,0,sizeof x->ram,3};
    x->regions[1]=(cs_guest_region){x->code,0,0x10000,sizeof x->code,5};
    x->regions[2]=(cs_guest_region){x->veneer,0,0x400200,sizeof x->veneer,5};
    x->regions[3]=(cs_guest_region){x->sentinels,0,0x401000,sizeof x->sentinels,5};
    x->regions[4]=(cs_guest_region){x->root,0,0x402000,sizeof x->root,5};
    x->map=(cs_guest_map){x->regions,5};
    cs_mm_config m={{0x500,0x1000,0x1800},{0x600,0x2000,0x3000}};
    cs_trap_config g={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&m)==CS_MM_OK);
    REQUIRE(cs_trap_init(&x->gateway,&g)==CS_TRAP_OK);
    REQUIRE(cs_s1_traps_init(&x->traps,&x->gateway,&x->manager,0x400200,CS_M68K_DATA24)==CS_TRAP_OK);
    cs_m68k_call_config c={0x3800,0x800,0x401000,4};
    REQUIRE(cs_s1_patches_init(&x->runtime,&x->traps,&c,64)==CS_M68K_CALL_OK);
    for(unsigned i=0;i<8;++i){x->frame.d[i]=0xCAFE0100u+i;x->frame.a[i]=0x3500u+i*8u;}
    x->frame.a[5]=0x3600;x->frame.a[7]=0x3F00;x->frame.pc=0x402000;x->frame.sr=0x271F;
    return 0;
}
static int patch(fixture *x,uint16_t w,unsigned index)
{
    static const uint8_t code[]={CS_GUEST_PATCH_PROGRAM_BYTES};
    memcpy(x->code,code,sizeof code);longword(x->code+2,0x400200u+index*2u);word(x->root,w);
    REQUIRE(cs_trap_set(&x->gateway,(uint16_t)(w&0xFFu),0x10000)==CS_TRAP_OK);return 0;
}
static cs_m68k_result step(fixture *x){return cs_s1_patch_step(&x->runtime,0,&x->frame,&x->report);}
static int allocate(fixture *x,int handle,uint32_t *at)
{
    cs_trap_frame f=x->frame;cs_s1_report r;f.d[0]=16;
    REQUIRE(cs_s1_trap_direct(&x->traps,handle?0x400208u:0x400200u,handle?0xA322:0xA31E,0,&f,&r)==CS_TRAP_OK);
    REQUIRE((uint16_t)f.d[0]==0);*at=f.a[0];return 0;
}
static int routes(void)
{
    static const uint16_t words[]={0xA11E,0xA01F,0xA020,0xA021,0xA122,0xA023,0xA024,0xA025,0xA029,0xA02A,0xA02E};
    for(unsigned i=0;i<11;++i){for(unsigned flags=0;flags<(i==0 || i==4?4u:1u);++flags){
        fixture x;REQUIRE(setup(&x)==0);uint32_t at=0;
        if(i>=1 && i<=3) REQUIRE(allocate(&x,0,&at)==0);
        if(i>=5 && i<=9) REQUIRE(allocate(&x,1,&at)==0);
        x.frame.a[0]=at;x.frame.d[0]=16;
        if(i==10){x.frame.a[0]=0x3400;x.frame.a[1]=0x3420;memset(x.ram+0x3400,0xC5,16);}
        cs_trap_frame expected=x.frame;cs_s1_report direct;
        uint16_t w=(uint16_t)(words[i]|(flags<<9));
        /* Separate original manager determines the register/API outcome. */
        fixture y;REQUIRE(setup(&y)==0);
        if(i>=1 && i<=3) REQUIRE(allocate(&y,0,&at)==0);
        if(i>=5 && i<=9) REQUIRE(allocate(&y,1,&at)==0);
        if(i==10) memset(y.ram+0x3400,0xC5,16);
        REQUIRE(cs_s1_trap_direct(&y.traps,0x400200u+i*2u,w,0,&expected,&direct)==CS_TRAP_OK);
        REQUIRE(patch(&x,w,i)==0 && step(&x)==CS_M68K_OK);
        REQUIRE(x.report.entered==1 && x.report.call.steps==4 && x.runtime.calls.depth==0);
        REQUIRE(x.frame.d[0]==expected.d[0] && x.frame.a[0]==expected.a[0]);
        REQUIRE(x.frame.d[1]==0xCAFE0101 && x.frame.a[1]==(i==10?0x3420u:0x3508u));
        REQUIRE(x.frame.a[5]==0x3600 && x.frame.a[7]==0x3F00 && x.frame.pc==0x402002);
        REQUIRE((x.frame.sr&0xFFE0u)==0x2700u && (x.frame.sr&15u)==(expected.sr&15u));
        REQUIRE(x.ram[0x3EFA]==(uint8_t)(x.frame.sr>>8) && x.ram[0x3EFF]==2);
        if(i==10) REQUIRE(!memcmp(x.ram+0x3420,y.ram+0x3420,16));
    }}return 0;
}
static int conventions(void)
{
    for(unsigned result=0;result<3;++result){for(unsigned bit=0;bit<2;++bit){
        fixture x;REQUIRE(setup(&x)==0);word(x.root,(uint16_t)(0xA01E|(bit<<8)));
        REQUIRE(cs_trap_set(&x.gateway,0x1E,0x10000)==CS_TRAP_OK);
        word(x.code,0x207C);longword(x.code+2,0x123456);word(x.code+6,0x72F0);word(x.code+8,0x74E0);
        word(x.code+10,0x227C);longword(x.code+12,0x456789);
        word(x.code+16,(uint16_t)(0x7000u+(result==0?0u:(result==1?7u:255u))));word(x.code+18,0x4E75);
        REQUIRE(step(&x)==CS_M68K_OK && x.report.call.steps==6);
        REQUIRE(x.frame.a[0]==(bit?0x123456u:0x3500u) && x.frame.a[1]==0x3508);
        REQUIRE(x.frame.d[1]==0xCAFE0101 && x.frame.d[2]==0xCAFE0102);
        REQUIRE((x.frame.sr&31u)==(result==0?20u:(result==1?16u:24u)));
    }}
    fixture x;REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);
    /* Tail chaining returns straight to the owned sentinel. */
    word(x.code,0x4EF9);word(x.code+6,0x4E71);x.frame.d[0]=16;
    REQUIRE(step(&x)==CS_M68K_OK && x.report.call.steps==2 && (uint16_t)x.frame.d[0]==0);
    /* Legacy atomic API continues refusing the very same guest patch. */
    x.frame.pc=0x402000;cs_trap_frame before=x.frame;cs_m68k_bus_report cpu;cs_s1_report s;
    REQUIRE(cs_s1_step(&x.traps,0,&x.frame,&cpu,&s)==CS_M68K_SERVICE && s.trap.status==CS_TRAP_NEEDS_EXECUTION);
    REQUIRE(!memcmp(&before,&x.frame,sizeof before));return 0;
}
static int failures(void)
{
    for(unsigned test=0;test<9;++test){
        fixture x;REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);unsigned context=0;
        switch(test){case 0:x.gateway.entries[0x1E]=0x10001;break;
        case 1:x.gateway.entries[0x1E]=0x10100;break;
        case 2:x.gateway.entries[0x1E]=0x400202;break;
        case 3:context=1;break;case 4:x.frame.a[5]=0x3601;break;
        case 5:x.frame.a[7]=0x3804;break;case 6:word(x.root,0xA81E);break;
        case 7:word(x.root,0xA221);break;default:x.regions[1].flags=1;break;}
        cs_trap_frame before=x.frame;cs_mm_manager manager=x.manager;cs_trap_gateway gateway=x.gateway;
        uint8_t ram[sizeof x.ram];memcpy(ram,x.ram,sizeof ram);
        REQUIRE(cs_s1_patch_step(&x.runtime,context,&x.frame,&x.report)!=CS_M68K_OK);
        REQUIRE(x.report.entered==0 && !memcmp(&before,&x.frame,sizeof before));
        REQUIRE(!memcmp(ram,x.ram,sizeof ram) && !memcmp(&manager,&x.manager,sizeof manager));
        REQUIRE(!memcmp(&gateway,&x.gateway,sizeof gateway) && x.runtime.calls.depth==0);
    }
    fixture x;REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);x.frame.d[0]=16;
    /* Allocate, publish its address as guest data, patch another directory entry,
     * then hit an unsupported instruction: completed effects must survive. */
    word(x.code+6,0x23C8);longword(x.code+8,0x3500);
    word(x.code+12,0x207C);longword(x.code+14,0x10080);
    word(x.code+18,0x7021);word(x.code+20,0xA047);word(x.code+22,0xFFFF);
    cs_trap_frame before=x.frame;
    REQUIRE(step(&x)==CS_M68K_SERVICE && x.report.entered==1 && x.report.call.cpu==CS_M68K_OPCODE);
    REQUIRE(!memcmp(&before,&x.frame,sizeof before) && x.runtime.calls.depth==0);
    uint32_t address,target;REQUIRE(cs_guest_read(&x.map,0x3500,4,1,&address)==CS_GUEST_OK && address!=0);
    REQUIRE(cs_trap_get(&x.gateway,0x21,&target)==CS_TRAP_OK && target==0x10080);
    cs_mm_request q={CS_MM_GET_PTR_SIZE,address,0,0,0,0};cs_mm_reply reply;
    REQUIRE(cs_mm_execute(&x.manager,&q,&reply)==CS_MM_OK && reply.code==0 && reply.value==16);
    REQUIRE(x.ram[0x3EFC]!=0 || x.ram[0x3EFD]!=0);
    /* A direct veneer must reject a bad return before it allocates. */
    REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);x.frame.d[0]=16;
    word(x.code,0x2EBC);longword(x.code+2,1);word(x.code+6,0x4EF9);longword(x.code+8,0x400200);
    cs_mm_manager manager=x.manager;
    REQUIRE(step(&x)==CS_M68K_SERVICE && x.report.entered==1 && x.report.call.cpu==CS_M68K_ADDRESS);
    REQUIRE(!memcmp(&manager,&x.manager,sizeof manager));return 0;
}
static int nesting(void)
{
    fixture x;REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);x.frame.d[0]=16;
    word(x.code+6,0xA021);word(x.code+8,0x4E75);
    word(x.code+0x40,0x4EB9);longword(x.code+0x42,0x400206);word(x.code+0x46,0x4E75);
    REQUIRE(cs_trap_set(&x.gateway,0x21,0x10040)==CS_TRAP_OK);
    REQUIRE(step(&x)==CS_M68K_OK && x.frame.d[0]==16 && x.report.call.steps==7 && x.runtime.calls.depth==0);
    for(unsigned budget=1;budget<7;++budget){
        fixture y;REQUIRE(setup(&y)==0);memcpy(y.code,x.code,sizeof x.code);word(y.root,0xA11E);
        REQUIRE(cs_trap_set(&y.gateway,0x1E,0x10000)==CS_TRAP_OK && cs_trap_set(&y.gateway,0x21,0x10040)==CS_TRAP_OK);
        y.runtime.budget=budget;y.frame.d[0]=16;cs_trap_frame before=y.frame;
        REQUIRE(step(&y)==CS_M68K_SERVICE && y.report.entered==1 && y.report.call.steps<=budget);
        REQUIRE(!memcmp(&before,&y.frame,sizeof before) && y.runtime.calls.depth==0);
    }
    REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);
    word(x.code,0xA11E);word(x.code+2,0x4E75);
    REQUIRE(step(&x)==CS_M68K_SERVICE && x.report.entered==1 && x.report.call.steps==4 && x.runtime.calls.depth==0);
    REQUIRE(x.report.nested_failure==CS_M68K_CALL_DEPTH_LIMIT);
    REQUIRE(setup(&x)==0 && patch(&x,0xA11E,0)==0);word(x.code,0x60FE);x.runtime.budget=3;
    REQUIRE(step(&x)==CS_M68K_SERVICE && x.report.call.status==CS_M68K_CALL_BUDGET_LIMIT && x.report.call.steps==3);
    return 0;
}
static int identities(void)
{
    fixture x,y;REQUIRE(setup(&x)==0 && setup(&y)==0);cs_s1_entry e;
    REQUIRE(cs_s1_entry_resolve(&x.traps,0xA11E,&e)==CS_TRAP_OK && e.kind==CS_S1_BUILTIN && e.target==0x400200);
    REQUIRE(patch(&x,0xA11E,0)==0 && cs_s1_entry_resolve(&x.traps,0xA11E,&e)==CS_TRAP_OK && e.kind==CS_S1_GUEST_CODE);
    REQUIRE(cs_s1_entry_resolve(&y.traps,0xA11E,&e)==CS_TRAP_OK && e.kind==CS_S1_BUILTIN);
    word(x.root,0xA146);x.frame.d[0]=0x1E;REQUIRE(step(&x)==CS_M68K_OK && x.frame.a[0]==0x10000 && x.report.entered==0);
    x.frame.pc=0x402000;word(x.root,0xA047);x.frame.d[0]=0x1E;x.frame.a[0]=0x400200;
    REQUIRE(step(&x)==CS_M68K_OK && cs_s1_entry_resolve(&x.traps,0xA11E,&e)==CS_TRAP_OK && e.kind==CS_S1_BUILTIN);
    cs_m68k_call_config c=x.runtime.calls.config;cs_s1_patches before=x.runtime;
    c.stack_base=0x2000;c.stack_size=0x100;
    REQUIRE(cs_s1_patches_init(&x.runtime,&x.traps,&c,64)==CS_M68K_CALL_CONFIG && !memcmp(&before,&x.runtime,sizeof before));
    c=x.runtime.calls.config;x.veneer[0]=0;
    REQUIRE(cs_s1_patches_init(&x.runtime,&x.traps,&c,64)==CS_M68K_CALL_CONFIG && !memcmp(&before,&x.runtime,sizeof before));
    x.veneer[0]=0x4E;x.map.count=9;
    REQUIRE(cs_s1_patches_init(&x.runtime,&x.traps,&c,64)==CS_M68K_CALL_CONFIG && !memcmp(&before,&x.runtime,sizeof before));
    x.map.count=5;x.manager.magic=0;
    REQUIRE(cs_s1_patches_init(&x.runtime,&x.traps,&c,64)==CS_M68K_CALL_CONFIG && !memcmp(&before,&x.runtime,sizeof before));
    return 0;
}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"routes"))return routes();
    if(!strcmp(argv[1],"conventions"))return conventions();
    if(!strcmp(argv[1],"failures"))return failures();
    if(!strcmp(argv[1],"nesting"))return nesting();
    if(!strcmp(argv[1],"identities"))return identities();
    return 2;
}
