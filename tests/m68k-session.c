/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/cpu/m68k-session.h"
#include "../apps/guest-session-probe/program.h"
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct {
    uint8_t code[128],tokens[8],data[64],stack[128];
    cs_guest_region regions[4];cs_guest_map map;cs_trap_gateway gateway;
    cs_trap_frame frame;cs_m68k_session session;
    cs_m68k_session_request request;cs_m68k_session_report report;
} fixture;
static void word(uint8_t *p,uint16_t value) {p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static void longword(uint8_t *p,uint32_t value) {word(p,(uint16_t)(value>>16));word(p+2,(uint16_t)value);}
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
    cs_trap_config gateway={0x400000,0x10000,0x20000,0x400100,0x400120,0x400140};
    REQUIRE(cs_trap_init(&x->gateway,&gateway)==CS_TRAP_OK);
    for(unsigned i=0;i<8;++i) {x->frame.d[i]=0xABCD0000u+i;x->frame.a[i]=0x1800;}
    x->frame.a[5]=0x1810;x->frame.a[7]=0x8080;x->frame.pc=0x6000;x->frame.sr=0x271F;
    cs_m68k_call_config config={0x8000,128,0x400180,4};
    REQUIRE(cs_m68k_session_init(&x->session,&x->map,&x->gateway,0,&config)==CS_M68K_SESSION_OK);
    x->request=(cs_m68k_session_request){0x6000,0x1800,64,CS_M68K_CALL_ORDINARY};
}
static cs_m68k_session_result run(fixture *x)
{return cs_m68k_session_run(&x->session,&x->request,&x->frame,&x->report);}
static void clean(const fixture *x)
{
    REQUIRE(x->session.active==0 && x->session.stepping==0 && x->session.exit_pending==0);
    REQUIRE(x->session.exit_code==0 && x->session.current==0 && x->session.driver==0 && x->session.driver_context==0);
    REQUIRE(x->session.calls.depth==0);
    for(unsigned i=0;i<4;++i) REQUIRE(x->session.calls.remaining[i]==0 && x->session.calls.used[i]==0 && x->session.calls.sp[i]==0);
}
typedef struct {fixture *x;unsigned pushes,steps;} observe_context;
static cs_m68k_result observe(void *context,cs_trap_frame *f,cs_m68k_bus_report *r,const cs_m68k_stack_limits *limits)
{
    observe_context *c=context;fixture *x=c->x;uint32_t old_pc=f->pc,old_sp=f->a[7];
    cs_m68k_result result=cs_m68k_bus_step_bounded(&x->map,&x->gateway,f,r,&x->session.calls.options,limits);
    ++c->steps;
    if(old_pc==0x6002) {
        REQUIRE(result==CS_M68K_OK && f->pc==0x6006 && f->a[7]==old_sp-2);
        uint32_t value;REQUIRE(cs_guest_read(&x->map,f->a[7],2,1,&value)==CS_GUEST_OK && value==0x1234);
        REQUIRE((f->sr&0x1Fu)==0x10);++c->pushes;
    }
    return result;
}
static void programs(void)
{
    fixture x;setup(&x);const uint8_t bytes[]={CS_GUEST_SESSION_PROGRAM_BYTES};
    REQUIRE(sizeof bytes==18);memcpy(x.code,bytes,sizeof bytes);x.request.budget=7;
    cs_trap_frame before=x.frame;observe_context c={&x,0,0};
    REQUIRE(cs_m68k_session_run_using(&x.session,&x.request,&x.frame,&x.report,observe,&c)==CS_M68K_SESSION_RETURNED);
    REQUIRE(c.steps==7 && c.pushes==1 && x.report.steps==7 && x.report.entered==1 && x.report.exit_code==0);
    REQUIRE(!memcmp(&before,&x.frame,sizeof before));
    REQUIRE(x.report.guest.d[0]==17 && x.report.guest.d[1]==0xABCD1234 && x.report.guest.d[2]==42);
    REQUIRE(x.report.guest.pc==0x400180 && x.report.pc==0x400180 && x.report.guest.a[7]==0x8080);
    REQUIRE(x.report.guest.a[5]==0x1800 && x.report.guest.sr==0x2710 && x.report.cpu==CS_M68K_OK);
    clean(&x);REQUIRE(run(&x)==CS_M68K_SESSION_RETURNED && !memcmp(&before,&x.frame,sizeof before));clean(&x);
    setup(&x);word(x.code,0xA146);word(x.code+2,0x4E75);x.frame.d[0]=0x46;before=x.frame;
    REQUIRE(run(&x)==CS_M68K_SESSION_RETURNED && x.report.steps==2);
    REQUIRE(x.report.guest.a[0]==0x400100 && !memcmp(&before,&x.frame,sizeof before));clean(&x);
}
typedef struct {fixture *x;unsigned repeat;cs_m68k_session_result first,second;} exit_context;
static cs_trap_result exit_dispatch(void *context,uint16_t trap,cs_trap_frame *f,cs_trap_report *r)
{
    exit_context *c=context;
    if(trap!=0xA000) return cs_trap_call(&c->x->gateway,trap,f,r);
    c->first=cs_m68k_session_request_exit(&c->x->session,0x12345678);
    if(c->repeat) c->second=cs_m68k_session_request_exit(&c->x->session,99);
    f->d[0]=77;r->status=CS_TRAP_OK;return CS_TRAP_OK;
}
static void exits(void)
{
    for(unsigned nested=0;nested<2;++nested) for(unsigned repeat=0;repeat<2;++repeat) {
        fixture x;setup(&x);exit_context c={&x,repeat,CS_M68K_SESSION_CONTEXT,CS_M68K_SESSION_OK};
        x.session.calls.options.dispatch=exit_dispatch;x.session.calls.options.context=&c;
        if(nested) {word(x.code,0x6100);word(x.code+2,0x003E);word(x.code+64,0xA000);}
        else word(x.code,0xA000);
        x.request.budget=nested?2u:1u;cs_trap_frame before=x.frame;
        REQUIRE(run(&x)==CS_M68K_SESSION_EXITED && c.first==CS_M68K_SESSION_OK);
        REQUIRE(!repeat || c.second==CS_M68K_SESSION_CONTEXT);
        REQUIRE(x.report.steps==(nested?2u:1u) && x.report.exit_code==0x12345678 && x.report.cpu==CS_M68K_OK);
        REQUIRE(x.report.guest.d[0]==77 && x.report.guest.pc==(nested?0x6042u:0x6002u));
        REQUIRE(x.report.guest.a[7]==(nested?0x8078u:0x807Cu) && !memcmp(&before,&x.frame,sizeof before));
        clean(&x);word(x.code,0x4E75);REQUIRE(run(&x)==CS_M68K_SESSION_RETURNED);clean(&x);
    }
}
static void admission(void)
{
    for(unsigned test=0;test<18;++test) {
        fixture x;setup(&x);word(x.code,0x4E75);cs_m68k_session_result expected=CS_M68K_SESSION_ADDRESS;
        switch(test) {
            case 0:x.request.budget=0;expected=CS_M68K_SESSION_ARGUMENT;break;
            case 1:x.request.budget=CS_M68K_CALL_BUDGET+1;expected=CS_M68K_SESSION_ARGUMENT;break;
            case 2:x.request.entry++;break;
            case 3:x.request.entry=0x1006000;break;
            case 4:x.request.entry=0x400182;break;
            case 5:x.request.a5++;break;
            case 6:x.request.a5=0x6000;break;
            case 7:x.frame.a[7]=0x8002;break;
            case 8:x.frame.a[7]=0x8081;break;
            case 9:x.frame.sr=0x700;break;
            case 10:x.frame.sr=0xA000;break;
            case 11:x.tokens[0]=0;expected=CS_M68K_SESSION_CONFIG;break;
            case 12:x.session.calls.config.max_depth=0;expected=CS_M68K_SESSION_CONFIG;break;
            case 13:x.session.calls.config.max_depth=5;expected=CS_M68K_SESSION_CONFIG;break;
            case 14:x.request.context=CS_M68K_CALL_INTERRUPT;expected=CS_M68K_SESSION_CONTEXT;break;
            case 15:x.request.context=(cs_m68k_call_context)9;expected=CS_M68K_SESSION_CONTEXT;break;
            case 16:x.session.tag=0;expected=CS_M68K_SESSION_CONFIG;break;
            default:x.regions[3].flags=1;expected=CS_M68K_SESSION_CONFIG;break;
        }
        cs_trap_frame before=x.frame;cs_m68k_session session=x.session;cs_trap_gateway gateway=x.gateway;
        uint8_t stack[128],data[64];memcpy(stack,x.stack,128);memcpy(data,x.data,64);
        REQUIRE(run(&x)==expected && x.report.entered==0 && x.report.steps==0);
        REQUIRE(!memcmp(&before,&x.frame,sizeof before) && !memcmp(&session,&x.session,sizeof session));
        REQUIRE(!memcmp(&gateway,&x.gateway,sizeof gateway) && !memcmp(stack,x.stack,128) && !memcmp(data,x.data,64));
    }
    fixture x;setup(&x);cs_m68k_session session=x.session;
    REQUIRE(cs_m68k_session_request_exit(&x.session,4)==CS_M68K_SESSION_CONTEXT && !memcmp(&session,&x.session,sizeof session));
    cs_m68k_session_report report=x.report;
    REQUIRE(cs_m68k_session_run(0,&x.request,&x.frame,&x.report)==CS_M68K_SESSION_ARGUMENT);
    REQUIRE(cs_m68k_session_run(&x.session,0,&x.frame,&x.report)==CS_M68K_SESSION_ARGUMENT);
    REQUIRE(cs_m68k_session_run(&x.session,&x.request,0,&x.report)==CS_M68K_SESSION_ARGUMENT);
    REQUIRE(cs_m68k_session_run(&x.session,&x.request,&x.frame,0)==CS_M68K_SESSION_ARGUMENT);
    REQUIRE(!memcmp(&report,&x.report,sizeof report));
    cs_m68k_call_config bad={0x8000,128,0x400180,0};
    REQUIRE(cs_m68k_session_init(&x.session,&x.map,&x.gateway,0,&bad)==CS_M68K_SESSION_CONFIG && !memcmp(&session,&x.session,sizeof session));
    REQUIRE(cs_m68k_session_init(0,&x.map,&x.gateway,0,&bad)==CS_M68K_SESSION_ARGUMENT);
    REQUIRE(cs_m68k_session_request_exit(0,0)==CS_M68K_SESSION_ARGUMENT);
    x.session.tag=0;REQUIRE(cs_m68k_session_request_exit(&x.session,0)==CS_M68K_SESSION_CONFIG);
}
static void failures(void)
{
    for(unsigned test=0;test<9;++test) {
        fixture x;setup(&x);cs_m68k_session_result expected=CS_M68K_SESSION_CPU;
        if(test==0) {word(x.code,0x60FE);x.request.budget=3;expected=CS_M68K_SESSION_BUDGET;}
        if(test==1) {word(x.code,0x4EF9);longword(x.code+2,0x400180);expected=CS_M68K_SESSION_RETURN;}
        if(test==2) {word(x.code,0x2F3C);longword(x.code+2,0x400180);word(x.code+6,0x4E75);expected=CS_M68K_SESSION_RETURN;}
        if(test==3) {word(x.code,0x4EF9);longword(x.code+2,0x400182);expected=CS_M68K_SESSION_RETURN;}
        if(test==4) {word(x.code,0x4AFC);}
        if(test==5) {word(x.code,0x23FC);longword(x.code+2,0x12345678);longword(x.code+6,0x1800);word(x.code+10,0x4AFC);}
        if(test==6) {word(x.code,0xA047);word(x.code+2,0x4AFC);x.frame.d[0]=32;x.frame.a[0]=0x400100;}
        if(test==7) {word(x.code,0x6100);word(x.code+2,0xFFFE);}
        if(test==8) {word(x.code,0x3F3C);word(x.code+2,0x1234);x.frame.a[7]=0x8004;}
        cs_trap_frame before=x.frame;
        REQUIRE(run(&x)==expected && x.report.entered==1 && x.report.steps>0);
        REQUIRE(!memcmp(&before,&x.frame,sizeof before) && x.report.exit_code==0);clean(&x);
        if(test==0) REQUIRE(x.report.steps==3 && x.report.guest.pc==0x6000);
        if(test==5) REQUIRE(x.data[0]==0x12 && x.data[3]==0x78 && x.report.guest.pc==0x600A);
        if(test==6) REQUIRE(x.gateway.entries[32]==0x400100);
        if(test==8) REQUIRE(x.stack[0]==0 && x.stack[1]==0x40 && x.stack[2]==1 && x.stack[3]==0x80);
        word(x.code,0x4E75);REQUIRE(run(&x)==CS_M68K_SESSION_RETURNED);clean(&x);
    }
}
typedef struct {
    fixture *x;unsigned recursive,exit_in_callback,reentry,requests,depth_refusals;
    cs_m68k_call_result nested;uint32_t nested_steps;
} nested_context;
static cs_trap_result nested_dispatch(void *context,uint16_t trap,cs_trap_frame *f,cs_trap_report *r)
{
    nested_context *c=context;fixture *x=c->x;
    if(trap!=0xA000) return cs_trap_call(&x->gateway,trap,f,r);
    if(x->session.calls.depth>1 && c->exit_in_callback) {
        REQUIRE(cs_m68k_session_request_exit(&x->session,7)==CS_M68K_SESSION_CONTEXT);++c->requests;
    } else {
        if(c->reentry) {
            cs_trap_frame before=*f;cs_m68k_session_report report;cs_m68k_session saved=x->session;
            REQUIRE(cs_m68k_session_run(&x->session,&x->request,f,&report)==CS_M68K_SESSION_CONTEXT);
            REQUIRE(!memcmp(&before,f,sizeof before) && !memcmp(&saved,&x->session,sizeof saved));
        }
        cs_m68k_call_request request={c->recursive?0x6000u:0x6040u,0x1800,64,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};
        cs_m68k_call_report report;
        c->nested=cs_m68k_session_call(&x->session,&request,f,&report);c->nested_steps=report.steps;
        if(c->nested==CS_M68K_CALL_DEPTH_LIMIT) ++c->depth_refusals;
        f->d[0]=(uint32_t)c->nested;
    }
    r->status=CS_TRAP_OK;return CS_TRAP_OK;
}
static void nesting(void)
{
    for(unsigned test=0;test<6;++test) {
        fixture x;setup(&x);word(x.code,0xA000);word(x.code+2,0x4E75);
        word(x.code+64,test==4?0xA000:0x722A);word(x.code+66,0x4E75);
        nested_context c={&x,test==2,test==4,test==5,0,0,CS_M68K_CALL_OK,0};
        x.session.calls.options.dispatch=nested_dispatch;x.session.calls.options.context=&c;
        if(test==1) x.request.budget=2;
        if(test==3) x.request.budget=1;
        cs_trap_frame before=x.frame;cs_m68k_session_result result=run(&x);
        REQUIRE(!memcmp(&before,&x.frame,sizeof before));clean(&x);
        if(test==0 || test==5) REQUIRE(result==CS_M68K_SESSION_RETURNED && x.report.steps==4 && c.nested_steps==2 && x.report.guest.d[1]==42);
        if(test==1) REQUIRE(result==CS_M68K_SESSION_BUDGET && x.report.steps==2 && c.nested==CS_M68K_CALL_BUDGET_LIMIT);
        if(test==2) REQUIRE(result==CS_M68K_SESSION_RETURNED && x.report.steps==8 && c.depth_refusals==1);
        if(test==3) REQUIRE(result==CS_M68K_SESSION_BUDGET && x.report.steps==1 && c.nested_steps==0 && c.nested==CS_M68K_CALL_BUDGET_LIMIT);
        if(test==4) REQUIRE(result==CS_M68K_SESSION_RETURNED && x.report.steps==4 && c.requests==1 && x.report.exit_code==0);
    }
    fixture x;setup(&x);cs_m68k_call_request request={0x6000,0x1800,64,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};
    cs_m68k_call_report report;cs_m68k_session before=x.session;
    REQUIRE(cs_m68k_session_call(&x.session,&request,&x.frame,&report)==CS_M68K_CALL_SIGNATURE && report.entered==0);
    REQUIRE(!memcmp(&before,&x.session,sizeof before));
    REQUIRE(cs_m68k_session_call(0,&request,&x.frame,&report)==CS_M68K_CALL_ARGUMENT);
}
typedef struct {fixture *x;unsigned fault;cs_m68k_session_result request;} precedence_context;
static cs_m68k_result precedence(void *context,cs_trap_frame *f,cs_m68k_bus_report *r,const cs_m68k_stack_limits *limits)
{
    precedence_context *c=context;
    c->request=cs_m68k_session_request_exit(&c->x->session,99);
    cs_m68k_call_request request={0x6040,0x1800,64,CS_M68K_PROC0,CS_M68K_CALL_ORDINARY};
    cs_m68k_call_report call;cs_trap_frame before=*f;uint8_t stack[128];memcpy(stack,c->x->stack,128);
    REQUIRE(cs_m68k_session_call(&c->x->session,&request,f,&call)==CS_M68K_CALL_SIGNATURE && call.steps==0);
    REQUIRE(!memcmp(&before,f,sizeof before) && !memcmp(stack,c->x->stack,128));
    (void)c->fault;
    return cs_m68k_bus_step_bounded(&c->x->map,&c->x->gateway,f,r,&c->x->session.calls.options,limits);
}
static void boundaries(void)
{
    for(unsigned fault=0;fault<2;++fault) {
        fixture x;setup(&x);word(x.code,fault?0x4AFC:0x4E71);x.request.budget=1;
        precedence_context c={&x,fault,CS_M68K_SESSION_CONTEXT};cs_trap_frame before=x.frame;
        REQUIRE(cs_m68k_session_run_using(&x.session,&x.request,&x.frame,&x.report,precedence,&c)==
            (fault?CS_M68K_SESSION_CPU:CS_M68K_SESSION_EXITED));
        REQUIRE(c.request==CS_M68K_SESSION_OK && x.report.steps==1 && !memcmp(&before,&x.frame,sizeof before));
        REQUIRE(x.report.cpu==(fault?CS_M68K_OPCODE:CS_M68K_OK) && x.report.exit_code==(fault?0u:99u));clean(&x);
    }
    fixture x;setup(&x);word(x.code,0x3F3C);x.regions[0].size=2;cs_trap_frame before=x.frame;
    REQUIRE(run(&x)==CS_M68K_SESSION_CPU && x.report.steps==1 && x.report.cpu==CS_M68K_ADDRESS);
    REQUIRE(!memcmp(&before,&x.frame,sizeof before));clean(&x);
    setup(&x);word(x.code,0x4E75);x.session.calls.config.max_depth=1;
    REQUIRE(run(&x)==CS_M68K_SESSION_RETURNED);clean(&x);
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    if(!strcmp(argv[1],"programs")) programs();else if(!strcmp(argv[1],"exits")) exits();
    else if(!strcmp(argv[1],"admission")) admission();else if(!strcmp(argv[1],"failures")) failures();
    else if(!strcmp(argv[1],"nesting")) nesting();else if(!strcmp(argv[1],"boundaries")) boundaries();else return 2;
    return 0;
}
