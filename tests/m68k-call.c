/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/cpu/m68k-call.h"
#include "../apps/guest-call-probe/program.h"
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct {
    uint8_t code[128],tokens[8],data[64],stack[128];cs_guest_region regions[4];cs_guest_map map;
    cs_trap_gateway gateway;cs_trap_frame frame;cs_m68k_calls calls;cs_m68k_call_request request;
    cs_m68k_call_report report;cs_m68k_bus_report bus;
} fixture;
static void word(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void longword(uint8_t *p,uint32_t v) {word(p,(uint16_t)(v>>16));word(p+2,(uint16_t)v);}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->data,0xCC,sizeof x->data);memset(x->stack,0xCC,sizeof x->stack);
    for(unsigned i=0;i<128;i+=2) word(x->code+i,0x4E71);
    for(unsigned i=0;i<8;i+=2) word(x->tokens+i,0x4E71);
    x->regions[0]=(cs_guest_region){x->code,0,0x6000,128,5};
    x->regions[1]=(cs_guest_region){x->tokens,0,0x400180,8,5};
    x->regions[2]=(cs_guest_region){x->data,x->data,0x1800,64,3};
    x->regions[3]=(cs_guest_region){x->stack,x->stack,0x8000,128,3};
    x->map=(cs_guest_map){x->regions,4};
    cs_trap_config g={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&g)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xABC00000u+i;x->frame.a[i]=0x1800;}
    x->frame.a[5]=0x1810;x->frame.a[7]=0x8080;x->frame.pc=0x6000;x->frame.sr=0x271F;
    cs_m68k_call_config c={0x8000,128,0x400180,4};
    REQUIRE(cs_m68k_call_init(&x->calls,&x->map,&x->gateway,0,&c)==CS_M68K_CALL_OK);
    x->request=(cs_m68k_call_request){0x6000,0x1800,64,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};
}
static cs_m68k_call_result call(fixture *x) {return cs_m68k_call(&x->calls,&x->request,&x->frame,&x->report);}
static cs_m68k_result step(fixture *x) {return cs_m68k_bus_step(&x->map,&x->gateway,&x->frame,&x->bus);}
static void bad_step(fixture *x,cs_m68k_result expected,const cs_m68k_stack_limits *limits)
{
    cs_trap_frame f=x->frame;cs_trap_gateway g=x->gateway;
    uint8_t code[128],data[64],stack[128];memcpy(code,x->code,128);memcpy(data,x->data,64);memcpy(stack,x->stack,128);
    cs_m68k_result r=limits==0?step(x):cs_m68k_bus_step_bounded(&x->map,&x->gateway,&x->frame,&x->bus,0,limits);
    REQUIRE(r==expected && !memcmp(&f,&x->frame,sizeof f) && !memcmp(&g,&x->gateway,sizeof g));
    REQUIRE(!memcmp(code,x->code,128) && !memcmp(data,x->data,64) && !memcmp(stack,x->stack,128));
}
static void opcodes(void)
{
    for(unsigned jump=0;jump<2;++jump) for(unsigned ea=0;ea<64;++ea) {
        fixture x;setup(&x);unsigned mode=ea>>3,reg=ea&7u,len=2;
        word(x.code,(uint16_t)((jump?0x4EC0u:0x4E80u)+ea));
        int legal=mode==2 || mode==5 || mode==6 || (mode==7 && reg<=3);
        if(!legal) {bad_step(&x,CS_M68K_OPCODE,0);continue;}
        if(mode==2) x.frame.a[reg]=0x6020;
        if(mode==5) {x.frame.a[reg]=reg==7?0x6020u:0x6010u;word(x.code+2,(uint16_t)(reg==7?0:16));len=4;}
        if(mode==6) {x.frame.a[reg]=reg==7?0x6020u:0x6000u;x.frame.d[0]=reg==7?0u:32u;word(x.code+2,0);len=4;}
        if(mode==7) {
            len=4;
            if(reg==0) word(x.code+2,0x6020);
            if(reg==1) {longword(x.code+2,0x6020);len=6;}
            if(reg==2) word(x.code+2,30);
            if(reg==3) {x.frame.d[0]=32;word(x.code+2,0x00FE);}
        }
        if(mode!=7 && reg==7) {x.regions[0].flags=7;x.regions[0].writable=x.code;}
        cs_trap_frame before=x.frame;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==0x6020 && x.frame.sr==before.sr);
        REQUIRE(x.bus.branch_taken==1 && x.bus.subroutine==(unsigned)!jump);
        REQUIRE(x.frame.a[7]==before.a[7]-(jump?0u:4u));
        if(!jump) {uint32_t out;REQUIRE(cs_guest_read(&x.map,x.frame.a[7],4,1,&out)==CS_GUEST_OK && out==0x6000+len);}
    }
}
static void indexes(void)
{
    for(unsigned jump=0;jump<2;++jump) for(unsigned pc=0;pc<2;++pc)
    for(unsigned addr=0;addr<2;++addr) for(unsigned reg=0;reg<8;++reg)
    for(unsigned wide=0;wide<2;++wide) for(unsigned negative=0;negative<2;++negative) {
        fixture x;setup(&x);x.regions[0].base=0x20000;x.frame.pc=0x20000;
        uint32_t raw=negative?0xFFFFfff8u:(wide?8u:0xABCD0008u);
        if(addr && reg==7) raw=x.frame.a[7];
        else if(addr) x.frame.a[reg]=raw;else x.frame.d[reg]=raw;
        int64_t index=wide?(int64_t)(int32_t)raw:(int64_t)(int16_t)(uint16_t)raw;
        /* Independent signed oracle; A7 indexes its actual stack value. */
        int64_t expected=pc?0x20002+index-2:0x20040;
        unsigned base_reg=addr && reg==1?2u:1u;
        if(!pc) x.frame.a[base_reg]=(uint32_t)(expected-index+2);
        unsigned ea=pc?59u:48u+base_reg;
        word(x.code,(uint16_t)((jump?0x4EC0u:0x4E80u)+ea));
        word(x.code+2,(uint16_t)(addr*32768u+reg*4096u+wide*2048u+254u));
        REQUIRE(expected>=0 && expected<=0xFFFFFE && (expected&1)==0);
        if(expected<0x20000 || expected>=0x20080) x.regions[1].base=(uint32_t)expected;
        REQUIRE(step(&x)==CS_M68K_OK && x.frame.pc==(uint32_t)expected && x.frame.sr==0x271F);
    }
    for(unsigned bits=1;bits<8;++bits) {
        fixture x;setup(&x);word(x.code,0x4EB0);word(x.code+2,(uint16_t)(bits*256u));
        bad_step(&x,CS_M68K_OPCODE,0);
    }
}
static void controls(void)
{
    for(unsigned n=0;n<13;++n) {
        fixture x;setup(&x);word(x.code,0x4EB9);longword(x.code+2,0x6020);
        switch(n) {
            case 0:longword(x.code+2,0x6021);break;
            case 1:longword(x.code+2,0xFF006020);break;
            case 2:longword(x.code+2,0x1800);break;
            case 3:x.regions[0].size=4;break;
            case 4:x.regions[0].flags=1;break;
            case 5:x.frame.a[7]=0x8002;break;
            case 6:x.frame.a[7]=0x8081;break;
            case 7:x.regions[3].flags=1;break;
            case 8:word(x.code,0x4EA8);word(x.code+2,0xFFFF);x.frame.a[0]=0;break;
            case 9:word(x.code,0x4EB0);word(x.code+2,0x0800);x.frame.a[0]=UINT32_MAX;x.frame.d[0]=4;break;
            case 10:word(x.code,0x4EB8);word(x.code+2,0xFFFF);break;
            case 11:x.regions[0].base=0xFFFFFA;x.regions[0].size=6;x.frame.pc=0xFFFFFA;longword(x.code+2,0x400180);break;
            default:word(x.code,0x4EB0);word(x.code+2,0x0800);x.frame.a[0]=4;x.frame.d[0]=0x80000000;break;
        }
        bad_step(&x,CS_M68K_ADDRESS,0);
    }
    cs_m68k_stack_limits limits={0x8020,64};fixture x;setup(&x);x.frame.a[7]=0x8020;word(x.code,0x6102);
    bad_step(&x,CS_M68K_ADDRESS,&limits);
    setup(&x);x.frame.a[7]=0x8060;word(x.code,0x4E75);longword(x.stack+96,0x6002);
    bad_step(&x,CS_M68K_ADDRESS,&limits);
    setup(&x);x.frame.a[7]=0x8040;word(x.code,0x2E7C);longword(x.code+2,0x1800);
    bad_step(&x,CS_M68K_ADDRESS,&limits);
    setup(&x);x.frame.a[7]=0x8024;word(x.code,0xA146);x.frame.d[0]=0x46;
    bad_step(&x,CS_M68K_ADDRESS,&limits);REQUIRE(!x.bus.cpu.service_called);
    setup(&x);x.frame.a[7]=0x8040;word(x.code,0x4EB9);longword(x.code+2,0x6020);
    REQUIRE(cs_m68k_bus_step_bounded(&x.map,&x.gateway,&x.frame,&x.bus,0,&limits)==CS_M68K_OK && x.frame.a[7]==0x803C);
    setup(&x);cs_m68k_bus_options data24={CS_M68K_DATA24,0,0};word(x.code,0x4ED0);x.frame.a[0]=0xFA006020;
    REQUIRE(cs_m68k_bus_step_options(&x.map,&x.gateway,&x.frame,&x.bus,&data24)==CS_M68K_ADDRESS);
}
static void programs(void)
{
    fixture x;setup(&x);const uint8_t program[]={CS_GUEST_CALL_PROGRAM_BYTES};memcpy(x.code,program,sizeof program);
    cs_trap_frame before=x.frame;
    REQUIRE(sizeof program==20 && call(&x)==CS_M68K_CALL_OK && x.report.steps==6 && x.report.changed_preserved==0);
    REQUIRE(x.frame.d[0]==17 && x.frame.d[1]==42 && x.frame.sr==0x2710);
    REQUIRE(x.frame.pc==before.pc && x.frame.a[5]==before.a[5] && x.frame.a[7]==before.a[7] && x.calls.depth==0);
    REQUIRE(call(&x)==CS_M68K_CALL_OK);
    setup(&x);word(x.code,0xA146);word(x.code+2,0x4E75);x.frame.d[0]=0x46;
    REQUIRE(call(&x)==CS_M68K_CALL_OK && x.frame.a[0]==0x400100 && x.report.steps==2);
    setup(&x);word(x.code,0x7612);word(x.code+2,0x4E75);
    REQUIRE(call(&x)==CS_M68K_CALL_OK && x.report.changed_preserved==(1u<<3) && x.frame.d[3]==18);
    setup(&x);word(x.code,0x2A7C);longword(x.code+2,0x1804);word(x.code+6,0x4E75);
    REQUIRE(call(&x)==CS_M68K_CALL_OK && x.report.changed_preserved==(1u<<13) && x.frame.a[5]==0x1810);
}
static void failures(void)
{
    for(unsigned n=0;n<14;++n) {
        fixture x;setup(&x);word(x.code,0x4E75);cs_m68k_call_result expected=CS_M68K_CALL_ADDRESS;
        switch(n) {
            case 0:x.request.signature=(cs_m68k_call_signature)0;expected=CS_M68K_CALL_SIGNATURE;break;
            case 1:x.request.budget=0;expected=CS_M68K_CALL_ARGUMENT;break;
            case 2:x.request.budget=CS_M68K_CALL_BUDGET+1;expected=CS_M68K_CALL_ARGUMENT;break;
            case 3:x.request.entry++;break;case 4:x.request.entry=0x1006000;break;
            case 5:x.request.entry=x.calls.config.sentinel_base;break;
            case 6:x.request.a5++;break;case 7:x.request.a5=0x6000;break;
            case 8:x.frame.a[7]=0x8002;break;case 9:x.frame.sr=0x700;break;
            case 10:x.tokens[0]=0;expected=CS_M68K_CALL_CONFIG;break;
            case 11:x.calls.config.max_depth=5;expected=CS_M68K_CALL_CONFIG;break;
            case 12:x.request.context=CS_M68K_CALL_INTERRUPT;expected=CS_M68K_CALL_SIGNATURE;break;
            default:x.request.context=(cs_m68k_call_context)2;expected=CS_M68K_CALL_SIGNATURE;break;
        }
        cs_trap_frame f=x.frame;cs_m68k_calls rt=x.calls;uint8_t stack[128];memcpy(stack,x.stack,128);
        REQUIRE(call(&x)==expected && x.report.steps==0 && !memcmp(&f,&x.frame,sizeof f));
        REQUIRE(!memcmp(&rt,&x.calls,sizeof rt) && !memcmp(stack,x.stack,128));
    }
    for(unsigned n=0;n<7;++n) {
        fixture x;setup(&x);cs_trap_frame f=x.frame;cs_m68k_call_result expected=CS_M68K_CALL_CPU;
        if(n==0) {word(x.code,0x4EF9);longword(x.code+2,0x400180);expected=CS_M68K_CALL_RETURN;}
        if(n==1) {word(x.code,0x2F3C);longword(x.code+2,0x400180);word(x.code+6,0x4E75);expected=CS_M68K_CALL_RETURN;}
        if(n==2) {word(x.code,0x60FE);x.request.budget=3;expected=CS_M68K_CALL_BUDGET_LIMIT;}
        if(n==3) {word(x.code,0x6100);word(x.code+2,0xFFFE);x.request.budget=64;}
        if(n==4) {word(x.code,0x4AFC);}
        if(n==5) {word(x.code,0x23FC);longword(x.code+2,0x12345678);longword(x.code+6,0x1800);word(x.code+10,0x4AFC);}
        if(n==6) {word(x.code,0xA047);word(x.code+2,0x4AFC);x.frame.d[0]=32;x.frame.a[0]=0x400100;f=x.frame;}
        REQUIRE(call(&x)==expected && !memcmp(&f,&x.frame,sizeof f) && x.calls.depth==0 && x.report.steps>0);
        if(n==2) REQUIRE(x.report.steps==3);
        if(n==5) REQUIRE(x.data[0]==0x12 && x.data[3]==0x78);
        if(n==6) REQUIRE(x.gateway.entries[32]==0x400100);
        word(x.code,0x4E75);REQUIRE(call(&x)==CS_M68K_CALL_OK);
    }
    fixture x;setup(&x);cs_m68k_calls before=x.calls;cs_m68k_call_config bad={0x8000,128,0x6000,0};
    REQUIRE(cs_m68k_call_init(&x.calls,&x.map,&x.gateway,0,&bad)==CS_M68K_CALL_CONFIG && !memcmp(&before,&x.calls,sizeof before));
    REQUIRE(cs_m68k_call(0,&x.request,&x.frame,&x.report)==CS_M68K_CALL_ARGUMENT);
    REQUIRE(cs_m68k_call(&x.calls,0,&x.frame,&x.report)==CS_M68K_CALL_ARGUMENT);
    REQUIRE(cs_m68k_call(&x.calls,&x.request,0,&x.report)==CS_M68K_CALL_ARGUMENT);
    REQUIRE(cs_m68k_call(&x.calls,&x.request,&x.frame,0)==CS_M68K_CALL_ARGUMENT);
}
typedef struct { fixture *fixture;unsigned recurse;cs_m68k_call_result nested;uint32_t nested_steps,refusals,early_atomic; } nested_context;
static cs_trap_result nested_dispatch(void *opaque,uint16_t trap,cs_trap_frame *frame,cs_trap_report *report)
{
    nested_context *ctx=opaque;fixture *x=ctx->fixture;
    if(trap==0xA000) {
        cs_m68k_call_request q={ctx->recurse?0x6000u:0x6040u,0x1800,64,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};
        cs_m68k_call_report r;
        uint32_t token_before=0,token_after=0;
        int empty=x->calls.remaining[0]==0;
        if(empty) REQUIRE(cs_guest_read(&x->map,frame->a[7]-4u,4,1,&token_before)==CS_GUEST_OK);
        ctx->nested=cs_m68k_call(&x->calls,&q,frame,&r);ctx->nested_steps=r.steps;
        if(empty) {
            REQUIRE(cs_guest_read(&x->map,frame->a[7]-4u,4,1,&token_after)==CS_GUEST_OK);
            REQUIRE(ctx->nested==CS_M68K_CALL_BUDGET_LIMIT && r.steps==0 && token_before==token_after);
            ++ctx->early_atomic;
        }
        if(ctx->nested==CS_M68K_CALL_DEPTH_LIMIT) ++ctx->refusals;
        frame->d[0]=(uint32_t)ctx->nested;
        /* Handle nested failure as an explicit guest result; this trusted hook
         * succeeds and makes no failed-instruction rollback promise. */
        report->status=CS_TRAP_OK;return CS_TRAP_OK;
    }
    return cs_trap_call(&x->gateway,trap,frame,report);
}
static void nesting(void)
{
    for(unsigned test=0;test<4;++test) {
        fixture x;setup(&x);word(x.code,0xA000);word(x.code+2,0x4E75);
        word(x.code+64,0xA146);word(x.code+66,0x4E75);x.frame.d[0]=0x46;
        nested_context ctx={&x,test==2,CS_M68K_CALL_OK,0,0,0};
        x.calls.options.dispatch=nested_dispatch;x.calls.options.context=&ctx;
        if(test==1) x.request.budget=2;
        if(test==3) x.request.budget=1;
        cs_trap_frame before=x.frame;
        cs_m68k_call_result result=call(&x);
        REQUIRE(x.calls.depth==0 && x.frame.a[5]==before.a[5] && x.frame.a[7]==before.a[7]);
        if(test==0) REQUIRE(result==CS_M68K_CALL_OK && x.report.steps==4 && ctx.nested==CS_M68K_CALL_OK && ctx.nested_steps==2);
        if(test==1) REQUIRE(result==CS_M68K_CALL_BUDGET_LIMIT && ctx.nested==CS_M68K_CALL_BUDGET_LIMIT && x.report.steps==2 && !memcmp(&before,&x.frame,sizeof before));
        if(test==2) REQUIRE(result==CS_M68K_CALL_OK && x.report.steps==8 && x.frame.d[0]==0 && ctx.nested==CS_M68K_CALL_OK && ctx.refusals==1);
        if(test==3) REQUIRE(result==CS_M68K_CALL_BUDGET_LIMIT && ctx.nested_steps==0 && ctx.early_atomic==1 && x.report.steps==1);
        for(unsigned i=0;i<4;++i) REQUIRE(x.calls.remaining[i]==0 && x.calls.used[i]==0 && x.calls.sp[i]==0);
    }
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"opcodes")) opcodes();else if(!strcmp(argv[1],"indexes")) indexes();
    else if(!strcmp(argv[1],"controls")) controls();else if(!strcmp(argv[1],"programs")) programs();
    else if(!strcmp(argv[1],"failures")) failures();else if(!strcmp(argv[1],"nesting")) nesting();else return 2;
    return 0;
}
