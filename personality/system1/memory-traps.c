/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "memory-traps.h"
#define CS_S1_MAGIC UINT32_C(0x43534D54)
static const uint16_t numbers[CS_S1_MEMORY_TRAPS]={0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x29,0x2A,0x2E};
static int policy(cs_m68k_data_policy p) {return p==CS_M68K_STRICT || p==CS_M68K_DATA24;}
static int tokens(const cs_trap_gateway *g,uint32_t base)
{
    if((base&1u)!=0 || base<g->config.rom_base || base-g->config.rom_base>65514u) return 0;
    for(unsigned i=0;i<CS_S1_MEMORY_TRAPS;++i) {
        uint32_t at=base+i*2u;
        if(at==g->config.get_token || at==g->config.set_token || at==g->config.unsupported_token) return 0;
    }
    return 1;
}
static int valid(const cs_s1_traps *s)
{return s->magic==CS_S1_MAGIC && s->gateway!=0 && s->memory!=0 && s->memory->map!=0 && policy(s->data_policy);}
cs_trap_result cs_s1_traps_check(const cs_s1_traps *s)
{
    uint32_t ignored;
    if(s==0) return CS_TRAP_ARGUMENT;
    if(!valid(s) || cs_trap_get(s->gateway,0,&ignored)!=CS_TRAP_OK ||
        !tokens(s->gateway,s->token_base)) return CS_TRAP_STATE;
    return CS_TRAP_OK;
}
cs_trap_result cs_s1_traps_init(cs_s1_traps *out,cs_trap_gateway *g,
    cs_mm_manager *m,uint32_t base,cs_m68k_data_policy p)
{
    if(out==0 || g==0 || m==0 || !policy(p)) return CS_TRAP_ARGUMENT;
    uint32_t ignored;
    cs_trap_result result=cs_trap_get(g,0,&ignored);
    if(result!=CS_TRAP_OK) return result;
    if(!tokens(g,base)) return CS_TRAP_ADDRESS;
    cs_mm_request q;cs_mm_reply reply;q.operation=CS_MM_GET_PTR_SIZE;
    q.address=0;q.other=0;q.size=0;q.flags=0;q.interrupt_context=0;
    if(cs_mm_execute(m,&q,&reply)!=CS_MM_OK) return CS_TRAP_STATE;
    /* All validation precedes this bounded, infallible publication. */
    for(unsigned i=0;i<CS_S1_MEMORY_TRAPS;++i) g->entries[numbers[i]]=base+i*2u;
    out->magic=CS_S1_MAGIC;out->token_base=base;out->gateway=g;out->memory=m;out->data_policy=p;
    return CS_TRAP_OK;
}
static void begin(cs_s1_report *r,const cs_trap_frame *f,uint16_t word)
{
    r->trap.status=CS_TRAP_OK;r->trap.pc=f->pc;r->trap.target=0;
    r->trap.decoded.word=word;r->trap.decoded.number=0;r->trap.decoded.toolbox=0;
    r->trap.decoded.a0_result=0;r->trap.decoded.auto_pop=0;r->trap.decoded.reserved=0;r->trap.decoded.flags=0;
    r->memory.status=CS_MM_OK;r->memory.word=word;r->memory.reply.code=0;
    r->memory.reply.value=0;r->memory.reply.moved=0;r->memory_called=0;r->failure=CS_S1_NONE;
}
static cs_trap_result finish(cs_s1_report *r,cs_trap_result result)
{r->trap.status=result;return result;}
static int index_of(uint16_t number)
{for(unsigned i=0;i<CS_S1_MEMORY_TRAPS;++i) if(numbers[i]==number) return (int)i;return -1;}
static int overlaps(uint32_t a,uint32_t bytes,uint32_t b,uint32_t other)
{return bytes!=0 && other!=0 && (a<=b?b-a<bytes:a-b<other);}
static cs_trap_result invoke(cs_s1_traps *s,uint16_t word,unsigned interrupt_context,
    cs_trap_frame *f,cs_s1_report *r,int direct,uint32_t target,int guard,uint32_t scratch)
{
    begin(r,f,word);
    cs_trap_result result=cs_trap_decode(word,&r->trap.decoded);
    if(result!=CS_TRAP_OK) return finish(r,result);
    if(!valid(s)) return finish(r,CS_TRAP_STATE);
    uint32_t ignored;
    result=cs_trap_get(s->gateway,0,&ignored);
    if(result!=CS_TRAP_OK) return finish(r,result);
    if(!tokens(s->gateway,s->token_base)) return finish(r,CS_TRAP_STATE);
    if(!direct) {
        result=cs_trap_get(s->gateway,r->trap.decoded.number,&target);
        if(result!=CS_TRAP_OK) return finish(r,result);
    }
    r->trap.target=target;
    if(direct && ((target&1u)!=0 || !((target>=s->gateway->config.rom_base &&
        target-s->gateway->config.rom_base<65536u) || (target>=s->gateway->config.ram_begin &&
        target<s->gateway->config.ram_end)))) return finish(r,CS_TRAP_ADDRESS);
    if(r->trap.decoded.toolbox) return finish(r,CS_TRAP_UNSUPPORTED_FORMAT);
    int index=index_of(r->trap.decoded.number);
    int memory_target=target>=s->token_base && target-s->token_base<CS_S1_MEMORY_TRAPS*2u;
    if(memory_target && ((target&1u)!=0 || index<0 || target!=s->token_base+(unsigned)index*2u)) {
        r->failure=CS_S1_SIGNATURE;return finish(r,CS_TRAP_UNSUPPORTED_FORMAT);
    }
    if(index>=0 && !memory_target) {
        if(target==s->gateway->config.get_token || target==s->gateway->config.set_token) {
            r->failure=CS_S1_SIGNATURE;return finish(r,CS_TRAP_UNSUPPORTED_FORMAT);
        }
        return finish(r,target==s->gateway->config.unsupported_token?CS_TRAP_UNIMPLEMENTED:CS_TRAP_NEEDS_EXECUTION);
    }
    if(guard) {
        const cs_mm_config *c=&s->memory->config;
        if(scratch<0x400u || overlaps(scratch,6,c->system.begin,c->system.end-c->system.begin) ||
           overlaps(scratch,6,c->application.begin,c->application.end-c->application.begin) ||
           (r->trap.decoded.number==0x2E && f->d[0]<0x80000000u && overlaps(scratch,6,f->a[1],f->d[0]))) {
            r->failure=CS_S1_SCRATCH;return finish(r,CS_TRAP_STATE);
        }
    }
    if(memory_target) {
        r->memory_called=1;
        cs_mm_result mm=cs_mm_register_call(s->memory,word,interrupt_context,f,&r->memory);
        if(mm!=CS_MM_OK) {
            r->failure=CS_S1_MEMORY;
            return finish(r,mm==CS_MM_FLAGS?CS_TRAP_UNSUPPORTED_FLAGS:CS_TRAP_STATE);
        }
        return finish(r,CS_TRAP_OK);
    }
    if(direct) {
        if(r->trap.decoded.flags) return finish(r,CS_TRAP_UNSUPPORTED_FLAGS);
        result=cs_trap_direct(s->gateway,target,f,&r->trap);
        (void)cs_trap_decode(word,&r->trap.decoded);
    } else result=cs_trap_call(s->gateway,word,f,&r->trap);
    return finish(r,result);
}
cs_trap_result cs_s1_trap_call(cs_s1_traps *s,uint16_t word,unsigned interrupt_context,
    cs_trap_frame *f,cs_s1_report *r)
{
    if(s==0 || f==0 || r==0 || interrupt_context>1) return CS_TRAP_ARGUMENT;
    return invoke(s,word,interrupt_context,f,r,0,0,0,0);
}
cs_trap_result cs_s1_trap_direct(cs_s1_traps *s,uint32_t target,uint16_t word,
    unsigned interrupt_context,cs_trap_frame *f,cs_s1_report *r)
{
    if(s==0 || f==0 || r==0 || interrupt_context>1) return CS_TRAP_ARGUMENT;
    return invoke(s,word,interrupt_context,f,r,1,target,0,0);
}
typedef struct {cs_s1_traps *runtime;cs_s1_report *report;unsigned interrupt_context;uint32_t scratch;} adapter;
static cs_trap_result dispatch(void *context,uint16_t word,cs_trap_frame *f,cs_trap_report *r)
{
    adapter *a=context;
    cs_trap_result result=invoke(a->runtime,word,a->interrupt_context,f,a->report,0,0,1,a->scratch);
    const cs_trap_report *from=&a->report->trap;
    r->status=from->status;r->pc=from->pc;r->target=from->target;
    r->decoded.word=from->decoded.word;r->decoded.number=from->decoded.number;
    r->decoded.toolbox=from->decoded.toolbox;r->decoded.a0_result=from->decoded.a0_result;
    r->decoded.auto_pop=from->decoded.auto_pop;r->decoded.reserved=from->decoded.reserved;
    r->decoded.flags=from->decoded.flags;return result;
}
cs_m68k_result cs_s1_step(cs_s1_traps *s,unsigned interrupt_context,
    cs_trap_frame *f,cs_m68k_bus_report *cpu,cs_s1_report *service)
{
    if(s==0 || f==0 || cpu==0 || service==0 || interrupt_context>1 || !valid(s)) return CS_M68K_ARGUMENT;
    begin(service,f,0);
    adapter a;a.runtime=s;a.report=service;a.interrupt_context=interrupt_context;
    a.scratch=f->a[7]>=6?f->a[7]-6:0;
    cs_m68k_bus_options options;options.data_policy=s->data_policy;
    options.dispatch=dispatch;options.context=&a;
    return cs_m68k_bus_step_options(s->memory->map,s->gateway,f,cpu,&options);
}
