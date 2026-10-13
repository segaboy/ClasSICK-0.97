/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "guest-patches.h"
static const uint16_t words[CS_S1_MEMORY_TRAPS]={0xA11E,0xA01F,0xA020,0xA021,0xA122,0xA023,0xA024,0xA025,0xA029,0xA02A,0xA02E};
static void copy_word(cs_trap_word *to,const cs_trap_word *from)
{
    to->word=from->word;to->number=from->number;to->toolbox=from->toolbox;
    to->a0_result=from->a0_result;to->auto_pop=from->auto_pop;
    to->reserved=from->reserved;to->flags=from->flags;
}
static void copy_trap(cs_trap_report *to,const cs_trap_report *from)
{to->status=from->status;to->pc=from->pc;to->target=from->target;copy_word(&to->decoded,&from->decoded);}
static void copy_bus(cs_m68k_bus_report *to,const cs_m68k_bus_report *from)
{
    to->cpu.status=from->cpu.status;to->cpu.pc=from->cpu.pc;to->cpu.word=from->cpu.word;
    to->cpu.service_called=from->cpu.service_called;copy_trap(&to->cpu.service,&from->cpu.service);
    to->memory=from->memory;to->address=from->address;to->access=from->access;to->bytes=from->bytes;
    to->branch_taken=from->branch_taken;to->subroutine=from->subroutine;
}
static int overlap(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n!=0 && m!=0 && (a<=b?b-a<n:a-b<m);}
static int executable_window(const cs_s1_traps *s,uint32_t at)
{
    const cs_trap_config *c=&s->gateway->config;
    return (at&1u)==0 && ((at>=c->rom_base && at-c->rom_base<65536u) ||
        (at>=c->ram_begin && at<c->ram_end));
}
cs_trap_result cs_s1_entry_resolve(const cs_s1_traps *s,uint16_t word,cs_s1_entry *out)
{
    if(s==0 || out==0) return CS_TRAP_ARGUMENT;
    cs_trap_word d;cs_trap_result result=cs_trap_decode(word,&d);
    if(result!=CS_TRAP_OK) return result;
    if(cs_s1_traps_check(s)!=CS_TRAP_OK) return CS_TRAP_STATE;
    if(d.toolbox) return CS_TRAP_UNSUPPORTED_FORMAT;
    int known=d.number==CS_TRAP_GET || d.number==CS_TRAP_SET;
    for(unsigned i=0;i<CS_S1_MEMORY_TRAPS;++i) if(d.number==(words[i]&0xFFu)) known=1;
    if(!known) return CS_TRAP_UNIMPLEMENTED;
    if(d.flags && d.number!=0x1E && d.number!=0x22) return CS_TRAP_UNSUPPORTED_FLAGS;
    uint32_t target;result=cs_trap_get(s->gateway,d.number,&target);
    if(result!=CS_TRAP_OK) return result;
    cs_s1_entry_kind kind=CS_S1_GUEST_CODE;
    if(target>=s->token_base && target-s->token_base<CS_S1_MEMORY_TRAPS*2u) {
        unsigned index=(target-s->token_base)/2u;
        if((target&1u)!=0 || d.number!=(words[index]&0xFFu)) return CS_TRAP_UNSUPPORTED_FORMAT;
        kind=CS_S1_BUILTIN;
    } else if(target==s->gateway->config.get_token || target==s->gateway->config.set_token) {
        if((target==s->gateway->config.get_token?CS_TRAP_GET:CS_TRAP_SET)!=d.number)
            return CS_TRAP_UNSUPPORTED_FORMAT;
        kind=CS_S1_BUILTIN;
    } else if(target==s->gateway->config.unsupported_token) return CS_TRAP_UNIMPLEMENTED;
    else {
        uint32_t ignored;
        if(!executable_window(s,target) || cs_guest_read(s->memory->map,target,2,5,&ignored)!=CS_GUEST_OK)
            return CS_TRAP_ADDRESS;
    }
    out->kind=kind;out->target=target;copy_word(&out->signature,&d);return CS_TRAP_OK;
}
static int readonly_code(const cs_guest_map *m,uint32_t at,uint32_t size)
{
    for(size_t i=0;i<m->count;++i) {
        const cs_guest_region *r=&m->regions[i];
        if(r->flags==5 && at>=r->base && size<=r->size && at-r->base<=r->size-size) return 1;
    }
    return 0;
}
cs_m68k_call_result cs_s1_patches_init(cs_s1_patches *rt,cs_s1_traps *s,
    const cs_m68k_call_config *c,uint32_t budget)
{
    if(rt==0 || s==0 || c==0 || budget==0 || budget>CS_M68K_CALL_BUDGET) return CS_M68K_CALL_ARGUMENT;
    if(cs_s1_traps_check(s)!=CS_TRAP_OK) return CS_M68K_CALL_CONFIG;
    if(cs_guest_validate(s->memory->map)!=CS_GUEST_OK) return CS_M68K_CALL_CONFIG;
    cs_mm_request check;cs_mm_reply reply;check.operation=CS_MM_GET_PTR_SIZE;
    check.address=0;check.other=0;check.size=0;check.flags=0;check.interrupt_context=0;
    if(cs_mm_execute(s->memory,&check,&reply)!=CS_MM_OK) return CS_M68K_CALL_CONFIG;
    const cs_mm_config *m=&s->memory->config;uint32_t value;
    if(overlap(c->stack_base,c->stack_size,m->system.begin,m->system.end-m->system.begin) ||
        overlap(c->stack_base,c->stack_size,m->application.begin,m->application.end-m->application.begin) ||
        !readonly_code(s->memory->map,s->token_base,CS_S1_MEMORY_TRAPS*2u)) return CS_M68K_CALL_CONFIG;
    for(unsigned i=0;i<CS_S1_MEMORY_TRAPS;++i)
        if(cs_guest_read(s->memory->map,s->token_base+i*2u,2,5,&value)!=CS_GUEST_OK || value!=0x4E75u)
            return CS_M68K_CALL_CONFIG;
    cs_m68k_bus_options o;o.data_policy=s->data_policy;o.dispatch=0;o.context=0;
    cs_m68k_call_result result=cs_m68k_call_init(&rt->calls,s->memory->map,s->gateway,&o,c);
    if(result!=CS_M68K_CALL_OK) return result;
    rt->traps=s;rt->budget=budget;return CS_M68K_CALL_OK;
}
static void begin(cs_s1_patch_report *r,const cs_trap_frame *f)
{
    r->entered=0;r->nested_failure=CS_M68K_CALL_OK;
    r->call.status=CS_M68K_CALL_OK;r->call.cpu=CS_M68K_OK;r->call.memory=CS_GUEST_OK;
    r->call.pc=f->pc;r->call.steps=0;r->call.changed_preserved=0;r->call.entered=0;
    r->cpu.cpu.status=CS_M68K_OK;r->cpu.cpu.pc=f->pc;r->cpu.cpu.word=0;r->cpu.cpu.service_called=0;
    cs_trap_report *t=&r->cpu.cpu.service;
    t->status=CS_TRAP_OK;t->pc=f->pc;t->target=0;
    t->decoded.word=0;t->decoded.number=0;t->decoded.toolbox=0;t->decoded.a0_result=0;
    t->decoded.auto_pop=0;t->decoded.reserved=0;t->decoded.flags=0;
    r->cpu.memory=CS_GUEST_OK;r->cpu.address=f->pc;r->cpu.access=5;r->cpu.bytes=2;
    r->cpu.branch_taken=0;r->cpu.subroutine=0;
    copy_trap(&r->service.trap,t);r->service.memory.status=CS_MM_OK;r->service.memory.word=0;
    r->service.memory.reply.code=0;r->service.memory.reply.value=0;r->service.memory.reply.moved=0;
    r->service.memory_called=0;r->service.failure=CS_S1_NONE;
}
static cs_m68k_result finish(cs_s1_patch_report *r,cs_m68k_result result)
{r->cpu.cpu.status=result;return result;}
static int stack(const cs_m68k_stack_limits *l,uint32_t at,uint32_t n)
{return l->size!=0 && at>=l->base && n<=l->size && at-l->base<=l->size-n;}
typedef struct {cs_s1_patches *runtime;uint16_t word;unsigned interrupt_context;cs_s1_patch_report *parent;} context;
static cs_m68k_result execute(context *a,cs_trap_frame *f,cs_s1_patch_report *r,const cs_m68k_stack_limits *l);
static cs_m68k_result guest_step(void *opaque,cs_trap_frame *f,cs_m68k_bus_report *bus,const cs_m68k_stack_limits *l)
{
    context *a=opaque;cs_s1_patch_report r;
    cs_m68k_result result=execute(a,f,&r,l);copy_bus(bus,&r.cpu);
    if(result!=CS_M68K_OK && a->parent!=0)
        a->parent->nested_failure=r.nested_failure!=CS_M68K_CALL_OK?r.nested_failure:r.call.status;
    return result;
}
static cs_m68k_result veneer(context *a,cs_trap_frame *f,cs_s1_patch_report *r,const cs_m68k_stack_limits *l)
{
    cs_s1_traps *s=a->runtime->traps;unsigned i=(f->pc-s->token_base)/2u;
    uint32_t target,value;
    r->cpu.cpu.word=0x4E75;r->cpu.cpu.service_called=1;
    if(a->word==0 || (a->word&0xFFu)!=(words[i]&0xFFu)) {
        r->service.failure=CS_S1_SIGNATURE;r->service.trap.status=CS_TRAP_UNSUPPORTED_FORMAT;
        copy_trap(&r->cpu.cpu.service,&r->service.trap);
        return finish(r,CS_M68K_SERVICE);
    }
    if(!stack(l,f->a[7],4) || cs_guest_read(s->memory->map,f->a[7],4,1,&target)!=CS_GUEST_OK ||
        target>0xFFFFFEu || (target&1u)!=0 || cs_guest_read(s->memory->map,target,2,5,&value)!=CS_GUEST_OK) {
        r->cpu.memory=CS_GUEST_ADDRESS;return finish(r,CS_M68K_ADDRESS);
    }
    /* No return fault can occur after the built-in changes the manager. */
    cs_trap_result result=cs_s1_trap_direct(s,f->pc,a->word,0,f,&r->service);
    copy_trap(&r->cpu.cpu.service,&r->service.trap);
    if(result!=CS_TRAP_OK) return finish(r,CS_M68K_SERVICE);
    f->pc=target;f->a[7]+=4;r->cpu.subroutine=1;return finish(r,CS_M68K_OK);
}
static cs_m68k_result execute(context *a,cs_trap_frame *f,cs_s1_patch_report *r,const cs_m68k_stack_limits *l)
{
    begin(r,f);cs_s1_patches *rt=a->runtime;cs_s1_traps *s=rt->traps;
    if(cs_s1_traps_check(s)!=CS_TRAP_OK || rt->calls.map!=s->memory->map || rt->calls.gateway!=s->gateway)
        return finish(r,CS_M68K_ARGUMENT);
    if((f->sr&0xA000u)!=0x2000u) return finish(r,CS_M68K_MODE);
    if((f->a[7]&1u)!=0 || !stack(l,f->a[7],0) || (f->pc&1u)!=0 || f->pc>0xFFFFFCu)
        return finish(r,CS_M68K_ADDRESS);
    uint32_t word;r->cpu.memory=cs_guest_read(s->memory->map,f->pc,2,5,&word);
    if(r->cpu.memory!=CS_GUEST_OK) return finish(r,CS_M68K_ADDRESS);
    if(f->pc>=s->token_base && f->pc-s->token_base<CS_S1_MEMORY_TRAPS*2u)
        return veneer(a,f,r,l);
    if(f->pc==s->gateway->config.get_token || f->pc==s->gateway->config.set_token ||
        f->pc==s->gateway->config.unsupported_token) return finish(r,CS_M68K_SERVICE);
    if((word&0xF000u)!=0xA000u) {
        cs_m68k_result result=cs_m68k_bus_step_bounded(rt->calls.map,rt->calls.gateway,f,&r->cpu,&rt->calls.options,l);
        return finish(r,result);
    }
    r->cpu.cpu.word=(uint16_t)word;r->cpu.cpu.service_called=1;
    cs_s1_entry e;cs_trap_result selected=cs_s1_entry_resolve(s,(uint16_t)word,&e);
    if(selected!=CS_TRAP_OK) {
        r->service.trap.status=selected;r->cpu.cpu.service.status=selected;
        return finish(r,CS_M68K_SERVICE);
    }
    copy_word(&r->service.trap.decoded,&e.signature);r->service.trap.target=e.target;
    copy_trap(&r->cpu.cpu.service,&r->service.trap);
    cs_guest_view scratch;
    if(f->a[7]<6 || !stack(l,f->a[7]-6,6) ||
        cs_guest_window(rt->calls.map,f->a[7]-6,6,3,&scratch)!=CS_GUEST_OK) {
        r->cpu.memory=CS_GUEST_ADDRESS;return finish(r,CS_M68K_ADDRESS);
    }
    uint32_t at=f->a[7]-6;const cs_mm_config *m=&s->memory->config;
    if(at<0x400u || overlap(at,6,m->system.begin,m->system.end-m->system.begin) ||
        overlap(at,6,m->application.begin,m->application.end-m->application.begin) ||
        (e.signature.number==0x2E && f->d[0]<0x80000000u && overlap(at,6,f->a[1],f->d[0]))) {
        r->service.failure=CS_S1_SCRATCH;return finish(r,CS_M68K_SERVICE);
    }
    if(e.kind==CS_S1_BUILTIN) {
        selected=cs_s1_trap_call(s,(uint16_t)word,a->interrupt_context,f,&r->service);
        copy_trap(&r->cpu.cpu.service,&r->service.trap);
        if(selected!=CS_TRAP_OK) return finish(r,CS_M68K_SERVICE);
    } else {
        if(a->interrupt_context!=0) {r->call.status=CS_M68K_CALL_SIGNATURE;return finish(r,CS_M68K_SERVICE);}
        uint32_t d1=f->d[1],d2=f->d[2],a0=f->a[0],a1=f->a[1];uint16_t sr=f->sr;
        context child;child.runtime=rt;child.word=(uint16_t)word;
        child.interrupt_context=0;child.parent=r;
        cs_m68k_call_request q={e.target,f->a[5],rt->budget,CS_M68K_REGISTERS,CS_M68K_CALL_ORDINARY};
        cs_m68k_call_result result=cs_m68k_call_using(&rt->calls,&q,f,&r->call,guest_step,&child);
        r->entered=r->call.entered;
        if(result!=CS_M68K_CALL_OK) {
            r->service.trap.status=CS_TRAP_STATE;copy_trap(&r->cpu.cpu.service,&r->service.trap);
            return finish(r,CS_M68K_SERVICE);
        }
        f->d[1]=d1;f->d[2]=d2;f->a[1]=a1;if(!e.signature.a0_result) f->a[0]=a0;
        uint16_t low=(uint16_t)f->d[0];
        f->sr=(uint16_t)((sr&0xFFE0u)|(f->sr&0x10u)|(low==0?4u:((low&0x8000u)?8u:0u)));
    }
    f->pc+=2;
    scratch.writable[0]=(uint8_t)(f->sr>>8);scratch.writable[1]=(uint8_t)f->sr;
    for(unsigned i=0;i<4;++i) scratch.writable[i+2]=(uint8_t)(f->pc>>((3u-i)*8u));
    return finish(r,CS_M68K_OK);
}
cs_m68k_result cs_s1_patch_step(cs_s1_patches *rt,unsigned interrupt_context,cs_trap_frame *f,cs_s1_patch_report *r)
{
    if(rt==0 || f==0 || r==0 || interrupt_context>1 || rt->traps==0) return CS_M68K_ARGUMENT;
    context a;a.runtime=rt;a.word=0;a.interrupt_context=interrupt_context;a.parent=r;
    cs_m68k_stack_limits l={rt->calls.config.stack_base,rt->calls.config.stack_size};
    return execute(&a,f,r,&l);
}
