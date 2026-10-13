/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "m68k-call.h"
#define CALL_TAG 0x43414C31u
static void copy_frame(cs_trap_frame *to,const cs_trap_frame *from)
{
    for(unsigned i=0;i<8;++i) {to->d[i]=from->d[i];to->a[i]=from->a[i];}
    to->pc=from->pc;to->sr=from->sr;
}
static int interval(const cs_guest_map *m,uint32_t base,uint32_t size,uint32_t flags)
{
    cs_guest_view view;
    if(((base|size)&1u)!=0 || cs_guest_window(m,base,size,flags,&view)!=CS_GUEST_OK) return 0;
    for(size_t i=0;i<m->count;++i) {
        const cs_guest_region *r=&m->regions[i];
        if(base>=r->base && size<=r->size && base-r->base<=r->size-size)
            return r->flags==flags;
    }
    return 0;
}
static int valid(const cs_guest_map *m,const cs_m68k_call_config *c,const cs_m68k_bus_options *o)
{
    if(cs_guest_validate(m)!=CS_GUEST_OK || c->max_depth==0 || c->max_depth>CS_M68K_CALL_DEPTH ||
        c->stack_base<0x400u || !interval(m,c->stack_base,c->stack_size,3) ||
        !interval(m,c->sentinel_base,CS_M68K_CALL_DEPTH*2u,5) ||
        (o!=0 && o->data_policy!=CS_M68K_STRICT && o->data_policy!=CS_M68K_DATA24)) return 0;
    for(unsigned i=0;i<CS_M68K_CALL_DEPTH;++i) {
        uint32_t word;
        if(cs_guest_read(m,c->sentinel_base+i*2u,2,5,&word)!=CS_GUEST_OK || word!=0x4E71u) return 0;
    }
    return 1;
}
cs_m68k_call_result cs_m68k_call_init(cs_m68k_calls *rt,const cs_guest_map *m,
    cs_trap_gateway *g,const cs_m68k_bus_options *o,const cs_m68k_call_config *c)
{
    if(rt==0 || m==0 || g==0 || c==0) return CS_M68K_CALL_ARGUMENT;
    if(!valid(m,c,o)) return CS_M68K_CALL_CONFIG;
    rt->map=m;rt->gateway=g;
    rt->options.data_policy=o==0?CS_M68K_STRICT:o->data_policy;
    rt->options.dispatch=o==0?0:o->dispatch;rt->options.context=o==0?0:o->context;
    rt->config.stack_base=c->stack_base;rt->config.stack_size=c->stack_size;
    rt->config.sentinel_base=c->sentinel_base;rt->config.max_depth=c->max_depth;
    rt->tag=CALL_TAG;rt->depth=0;
    for(unsigned i=0;i<CS_M68K_CALL_DEPTH;++i) {rt->remaining[i]=0;rt->used[i]=0;rt->sp[i]=0;}
    return CS_M68K_CALL_OK;
}
static cs_m68k_call_result finish(cs_m68k_call_report *r,cs_m68k_call_result status)
{r->status=status;return status;}
static int sentinel(const cs_m68k_calls *rt,uint32_t at)
{return at>=rt->config.sentinel_base && at-rt->config.sentinel_base<CS_M68K_CALL_DEPTH*2u;}
cs_m68k_call_result cs_m68k_call(cs_m68k_calls *rt,const cs_m68k_call_request *q,
    cs_trap_frame *f,cs_m68k_call_report *r)
{
    if(rt==0 || q==0 || f==0 || r==0) return CS_M68K_CALL_ARGUMENT;
    r->status=CS_M68K_CALL_OK;r->cpu=CS_M68K_OK;r->memory=CS_GUEST_OK;
    r->pc=q->entry;r->steps=0;r->changed_preserved=0;
    if(rt->tag!=CALL_TAG || rt->gateway==0 || !valid(rt->map,&rt->config,&rt->options))
        return finish(r,CS_M68K_CALL_CONFIG);
    if(q->signature!=CS_M68K_PROC0 || q->context!=CS_M68K_CALL_ORDINARY)
        return finish(r,CS_M68K_CALL_SIGNATURE);
    if(q->budget==0 || q->budget>CS_M68K_CALL_BUDGET) return finish(r,CS_M68K_CALL_ARGUMENT);
    if(rt->depth>=rt->config.max_depth) return finish(r,CS_M68K_CALL_DEPTH_LIMIT);
    for(unsigned i=0;i<rt->depth;++i) if(rt->remaining[i]==0)
        return finish(r,CS_M68K_CALL_BUDGET_LIMIT);
    uint32_t top=rt->config.stack_base+rt->config.stack_size,unused;
    cs_guest_view push,a5;
    if((f->sr&0xA000u)!=0x2000u || (f->a[7]&1u)!=0 || f->a[7]<rt->config.stack_base+4u ||
        f->a[7]>top || (rt->depth!=0 && f->a[7]>rt->sp[rt->depth-1u]-4u) ||
        sentinel(rt,q->entry) || cs_guest_read(rt->map,q->entry,2,5,&unused)!=CS_GUEST_OK ||
        (q->a5&1u)!=0 || cs_guest_window(rt->map,q->a5,4,3,&a5)!=CS_GUEST_OK ||
        cs_guest_window(rt->map,f->a[7]-4u,4,3,&push)!=CS_GUEST_OK) {
        r->memory=CS_GUEST_ADDRESS;return finish(r,CS_M68K_CALL_ADDRESS);
    }
    cs_trap_frame caller;copy_frame(&caller,f);
    unsigned slot=rt->depth;
    uint32_t token=rt->config.sentinel_base+slot*2u;
    rt->remaining[slot]=q->budget;rt->used[slot]=0;rt->sp[slot]=f->a[7];++rt->depth;
    for(unsigned i=0;i<4;++i) push.writable[i]=(uint8_t)(token>>((3u-i)*8u));
    f->a[7]-=4;f->a[5]=q->a5;f->pc=q->entry;
    cs_m68k_stack_limits limits={rt->config.stack_base,caller.a[7]-rt->config.stack_base};
    cs_m68k_call_result status=CS_M68K_CALL_OK;
    cs_m68k_bus_report bus;
    for(;;) {
        int exhausted=0;
        for(unsigned i=0;i<rt->depth;++i) if(rt->remaining[i]==0) exhausted=1;
        if(exhausted) {status=CS_M68K_CALL_BUDGET_LIMIT;break;}
        for(unsigned i=0;i<rt->depth;++i) {--rt->remaining[i];++rt->used[i];}
        r->pc=f->pc;
        r->cpu=cs_m68k_bus_step_bounded(rt->map,rt->gateway,f,&bus,&rt->options,&limits);
        r->memory=bus.memory;
        if(r->cpu!=CS_M68K_OK) {status=CS_M68K_CALL_CPU;break;}
        if(sentinel(rt,f->pc)) {
            if(f->pc!=token || bus.cpu.word!=0x4E75u || f->a[7]!=caller.a[7])
                status=CS_M68K_CALL_RETURN;
            break;
        }
    }
    r->steps=rt->used[slot];r->pc=f->pc;
    if(status==CS_M68K_CALL_OK) {
        for(unsigned i=0;i<8;++i) {
            if(i>=3 && f->d[i]!=caller.d[i]) r->changed_preserved|=1u<<i;
            if(i>=2 && i<=6 && f->a[i]!=(i==5?q->a5:caller.a[i])) r->changed_preserved|=1u<<(i+8u);
        }
        f->pc=caller.pc;f->a[5]=caller.a[5];f->a[7]=caller.a[7];
    } else copy_frame(f,&caller);
    rt->remaining[slot]=0;rt->used[slot]=0;rt->sp[slot]=0;--rt->depth;
    return finish(r,status);
}
