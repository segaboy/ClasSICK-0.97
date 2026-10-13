/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-session.h"
#define SESSION_TAG 0x53455331u
static void copy_frame(cs_trap_frame *to,const cs_trap_frame *from)
{
    for(unsigned i=0;i<8;++i) {to->d[i]=from->d[i];to->a[i]=from->a[i];}
    to->pc=from->pc;to->sr=from->sr;
}
static cs_m68k_session_result translate(cs_m68k_call_result result)
{
    switch(result) {
        case CS_M68K_CALL_OK:return CS_M68K_SESSION_RETURNED;
        case CS_M68K_CALL_ARGUMENT:return CS_M68K_SESSION_ARGUMENT;
        case CS_M68K_CALL_CONFIG:return CS_M68K_SESSION_CONFIG;
        case CS_M68K_CALL_SIGNATURE:return CS_M68K_SESSION_CONTEXT;
        case CS_M68K_CALL_ADDRESS:return CS_M68K_SESSION_ADDRESS;
        case CS_M68K_CALL_DEPTH_LIMIT:return CS_M68K_SESSION_DEPTH;
        case CS_M68K_CALL_BUDGET_LIMIT:return CS_M68K_SESSION_BUDGET;
        case CS_M68K_CALL_RETURN:return CS_M68K_SESSION_RETURN;
        default:return CS_M68K_SESSION_CPU;
    }
}
cs_m68k_session_result cs_m68k_session_init(cs_m68k_session *s,const cs_guest_map *m,
    cs_trap_gateway *g,const cs_m68k_bus_options *o,const cs_m68k_call_config *c)
{
    if(s==0) return CS_M68K_SESSION_ARGUMENT;
    cs_m68k_call_result result=cs_m68k_call_init(&s->calls,m,g,o,c);
    if(result!=CS_M68K_CALL_OK) return translate(result);
    s->driver=0;s->driver_context=0;s->current=0;s->tag=SESSION_TAG;
    s->active=0;s->stepping=0;s->exit_pending=0;s->exit_code=0;
    return CS_M68K_SESSION_OK;
}
static cs_m68k_result execute(void *context,cs_trap_frame *f,
    cs_m68k_bus_report *r,const cs_m68k_stack_limits *limits)
{
    cs_m68k_session *s=context;
    uint32_t previous=s->stepping;s->stepping=1;
    cs_m68k_result result=s->driver==0?
        cs_m68k_bus_step_bounded(s->calls.map,s->calls.gateway,f,r,&s->calls.options,limits):
        s->driver(s->driver_context,f,r,limits);
    s->stepping=previous;
    if(s->calls.depth==1) {
        copy_frame(&s->current->guest,f);
        s->current->cpu=result;s->current->memory=r->memory;
        /* Internal stop only after a completed trusted root instruction. The
         * public session report exposes the real successful CPU result. */
        if(result==CS_M68K_OK && s->exit_pending!=0) return CS_M68K_SERVICE;
    }
    return result;
}
cs_m68k_session_result cs_m68k_session_request_exit(cs_m68k_session *s,uint32_t code)
{
    if(s==0) return CS_M68K_SESSION_ARGUMENT;
    if(s->tag!=SESSION_TAG) return CS_M68K_SESSION_CONFIG;
    if(s->active!=1 || s->stepping!=1 || s->calls.depth!=1 || s->exit_pending!=0)
        return CS_M68K_SESSION_CONTEXT;
    s->exit_pending=1;s->exit_code=code;
    return CS_M68K_SESSION_OK;
}
cs_m68k_call_result cs_m68k_session_call(cs_m68k_session *s,
    const cs_m68k_call_request *q,cs_trap_frame *f,cs_m68k_call_report *r)
{
    if(s==0 || q==0 || f==0 || r==0) return CS_M68K_CALL_ARGUMENT;
    if(s->tag!=SESSION_TAG || s->active!=1 || s->stepping!=1 || s->exit_pending!=0) {
        r->status=s->tag!=SESSION_TAG?CS_M68K_CALL_CONFIG:CS_M68K_CALL_SIGNATURE;
        r->cpu=CS_M68K_OK;r->memory=CS_GUEST_OK;r->pc=q->entry;r->steps=0;
        r->changed_preserved=0;r->entered=0;
        return r->status;
    }
    return cs_m68k_call_using(&s->calls,q,f,r,execute,s);
}
cs_m68k_session_result cs_m68k_session_run_using(cs_m68k_session *s,
    const cs_m68k_session_request *q,cs_trap_frame *f,cs_m68k_session_report *r,
    cs_m68k_call_step step,void *context)
{
    if(s==0 || q==0 || f==0 || r==0) return CS_M68K_SESSION_ARGUMENT;
    r->status=CS_M68K_SESSION_OK;r->cpu=CS_M68K_OK;r->memory=CS_GUEST_OK;
    r->pc=q->entry;r->steps=0;r->exit_code=0;r->entered=0;copy_frame(&r->guest,f);
    if(s->tag!=SESSION_TAG) {r->status=CS_M68K_SESSION_CONFIG;return r->status;}
    if(s->active!=0 || s->calls.depth!=0) {r->status=CS_M68K_SESSION_CONTEXT;return r->status;}
    cs_trap_frame caller;copy_frame(&caller,f);
    cs_m68k_call_request request={q->entry,q->a5,q->budget,CS_M68K_REGISTERS,q->context};
    cs_m68k_call_report call;
    s->driver=step;s->driver_context=context;s->current=r;s->active=1;s->stepping=0;
    s->exit_pending=0;s->exit_code=0;
    cs_m68k_call_result result=cs_m68k_call_using(&s->calls,&request,f,&call,execute,s);
    cs_m68k_result actual_cpu=r->cpu;
    r->status=translate(result);r->cpu=call.cpu;r->memory=call.memory;
    r->pc=call.pc;r->steps=call.steps;r->entered=call.entered;
    if(call.entered!=0) {
        if(result==CS_M68K_CALL_CPU && s->exit_pending!=0 && actual_cpu==CS_M68K_OK) {
            r->status=CS_M68K_SESSION_EXITED;r->exit_code=s->exit_code;
            r->cpu=actual_cpu;
        }
    }
    copy_frame(f,&caller);
    s->driver=0;s->driver_context=0;s->current=0;s->active=0;s->stepping=0;
    s->exit_pending=0;s->exit_code=0;
    return r->status;
}
cs_m68k_session_result cs_m68k_session_run(cs_m68k_session *s,
    const cs_m68k_session_request *q,cs_trap_frame *f,cs_m68k_session_report *r)
{return cs_m68k_session_run_using(s,q,f,r,0,0);}
