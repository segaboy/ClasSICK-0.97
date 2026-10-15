/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original dedicated execution coordinator. */
#include "code-run-internal.h"
typedef struct {cs_image_service *image;const cs_image_run_request *request;uint32_t root;} policy;
static void clear(void *p,size_t n){uint8_t *b=p;for(size_t i=0;i<n;++i)b[i]=0;}
static void copy(void *p,const void *q,size_t n)
{uint8_t *a=p;const uint8_t *b=q;for(size_t i=0;i<n;++i)a[i]=b[i];}
static void frame_copy(cs_trap_frame *a,const cs_trap_frame *b)
{for(unsigned i=0;i<8;++i){a->a[i]=b->a[i];a->d[i]=b->d[i];}a->pc=b->pc;a->sr=b->sr;}
static int inside(uint32_t a,uint32_t n,uint32_t base,uint32_t size)
{return n && a>=base && n<=size && a-base<=size-n;}
static int opcode(const cs_image_service *s,uint32_t pc)
{
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=6 && inside(pc,2,p->address+4u,p->length-4u))return 1;}
    return pc>=s->layout.table_begin+2u && pc<s->layout.table_end &&
        (pc-s->layout.table_begin)%8u==2u;
}
static int extension(const cs_image_service *s,const cs_m68k_guard_event *e)
{
    uint32_t pc=e->before->pc;
    if(pc>=s->layout.table_begin+2u && pc<s->layout.table_end &&
        (pc-s->layout.table_begin)%8u==2u)
        return e->word==0x4EF9u && e->address==pc+2u && e->bytes==4;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=6 && inside(pc,2,p->address+4u,p->length-4u) &&
            inside(e->address,e->bytes,p->address+4u,p->length-4u))return 1;}
    return 0;
}
static int memory(const cs_image_service *s,const cs_m68k_guard_event *e,int writing)
{
    if(inside(e->address,e->bytes,s->layout.globals_begin,s->manifest.below_a5) ||
        inside(e->address,e->bytes,s->layout.stack_base,s->layout.stack_top-s->layout.stack_base))return 1;
    if(writing)return 0;
    if(inside(e->address,e->bytes,s->layout.a5,s->manifest.above_a5))return 1;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=4 && inside(e->address,e->bytes,p->address+4u,p->length-4u))return 1;}
    return 0;
}
static int check(const void *opaque,const cs_m68k_guard_event *e)
{
    const policy *p=opaque;const cs_image_service *s=p->image;
    int entry=s->execution_state==1 && e->kind==CS_M68K_GUARD_WRITE && e->word==0 &&
        e->address==p->root && e->bytes==4 && e->before->a[7]==s->layout.stack_top;
    if((s->execution_state!=2 && !entry) || s->execution_generation!=s->generation ||
        !cs_image_execution_integrity(s,&p->request->context,1))return 0;
    switch(e->kind){
        case CS_M68K_GUARD_OPCODE:return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_EXTENSION:return extension(s,e);
        case CS_M68K_GUARD_READ:return memory(s,e,0);
        case CS_M68K_GUARD_WRITE:return memory(s,e,1);
        case CS_M68K_GUARD_INSTRUCTION:
            if((e->word&0xF000u)==0xA000u)return 0;
            return !(e->before->pc>=s->layout.table_begin && e->before->pc<s->layout.table_end) || e->word==0x4EF9u;
        case CS_M68K_GUARD_TARGET:
            if(e->address==p->request->token)return e->word==0x4E75u && e->before->a[7]==p->root;
            return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_COMMIT:
            if(e->proposed->a[5]!=s->layout.a5)return 0;
            if(e->proposed->pc==p->request->token)return e->word==0x4E75u &&
                e->before->a[7]==p->root && e->proposed->a[7]==s->layout.stack_top;
            return opcode(s,e->proposed->pc);
        default:return 0;
    }
}
static int token_valid(const cs_image_service *s,uint32_t at)
{
    cs_guest_view v;
    if((at&1u) || at>0xFFFFF8u || cs_guest_window(s->map,at,8,CS_GUEST_READ|CS_GUEST_EXECUTE,&v)!=CS_GUEST_OK)return 0;
    if(inside(at,8,s->config.system.begin,s->config.system.end-s->config.system.begin) ||
        inside(at,8,s->config.application.begin,s->config.application.end-s->config.application.begin) ||
        inside(at,8,s->layout.stack_base,s->layout.b-s->layout.stack_base))return 0;
    for(unsigned i=0;i<8;++i)if(v.bytes[i]!=(i&1u?0x71u:0x4Eu))return 0;
    return 1;
}
static cs_image_run_result finish(cs_image_service *s,cs_image_run_report *r,
    cs_image_run_result result,const cs_trap_frame *guest)
{
    frame_copy(&r->guest,guest);r->status=result;
    if(result==CS_IMAGE_RUN_RETURNED)s->execution_state=3;
    else if(result==CS_IMAGE_RUN_QUARANTINED)s->execution_state=5;
    else s->execution_state=4;
    return result;
}
cs_image_run_result cs_image_run(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r)
{
    if(!cs_image_execution_native(s,q,caller,r))return CS_IMAGE_RUN_ARGUMENT;
    clear(r,sizeof *r);r->status=CS_IMAGE_RUN_REFUSED;
    if(s->execution_state || s->execution_generation==s->generation ||
        q->generation!=s->generation || q->epoch!=s->epoch || !q->budget || q->budget>1000000u ||
        !cs_image_execution_integrity(s,&q->context,0) || !token_valid(s,q->token) ||
        s->layout.stack_top<s->layout.stack_base+4u)return r->status;
    /* X-1's envelope rejects all distinct backing aliases and known native
     * controls. Check the same invariant before claiming or writing the token. */
    for(size_t i=0;i<s->map->count;++i)for(size_t j=0;j<i;++j){
        uintptr_t a=(uintptr_t)s->map->regions[i].bytes,b=(uintptr_t)s->map->regions[j].bytes;
        size_t n=s->map->regions[i].size,m=s->map->regions[j].size;
        if(a<=b?b-a<n:a-b<m)return r->status;
    }
    cs_trap_frame guest;clear(&guest,sizeof guest);
    guest.pc=s->layout.table_begin+2u;guest.sr=0x2700;guest.a[5]=s->layout.a5;guest.a[7]=s->layout.stack_top;
    r->generation=s->generation;r->epoch=s->epoch;
    policy p;clear(&p,sizeof p);p.image=s;p.request=q;p.root=s->layout.stack_top-4u;
    cs_m68k_guard guard;guard.check=check;guard.context=&p;guard.context_bytes=sizeof p;
    cs_m68k_stack_limits limits;limits.base=s->layout.stack_base;limits.size=s->layout.stack_top-s->layout.stack_base;
    cs_m68k_bus_report step;clear(&step,sizeof step);
    if(cs_m68k_bus_guard_validate(s->map,q->gateway,&guest,&step,&limits,&guard)!=CS_M68K_OK)return r->status;
    s->execution_generation=s->generation;s->execution_state=1;s->execution_scrubbed=0;
    /* Entry is a separately admitted, whole bounded stack write. There is no
     * fallible operation after its store and before publishing RUNNING/A7. */
    cs_m68k_guard_event entry;clear(&entry,sizeof entry);entry.kind=CS_M68K_GUARD_WRITE;
    entry.address=p.root;entry.bytes=4;entry.access=CS_GUEST_WRITE;entry.before=&guest;entry.proposed=&guest;
    if(!check(&p,&entry) || cs_guest_write(s->map,p.root,4,q->token)!=CS_GUEST_OK)
        return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
    guest.a[7]=p.root;r->entered=1;s->execution_state=2;
    for(uint32_t i=0;i<q->budget;++i){
        if(!cs_image_execution_integrity(s,&q->context,1))return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
        clear(&step,sizeof step);
        cs_m68k_result cpu=cs_m68k_bus_step_admitted(s->map,q->gateway,&guest,&step,&limits,&guard,q->token,p.root);
        r->cpu=cpu;r->memory=step.memory;
        if(cpu!=CS_M68K_OK){
            if(!cs_image_execution_integrity(s,&q->context,1) || cpu==CS_M68K_ARGUMENT)
                return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
            return finish(s,r,CS_IMAGE_RUN_CPU,&guest);
        }
        ++r->steps;
        if(guest.pc==q->token){
            if(step.cpu.word!=0x4E75u || guest.a[7]!=s->layout.stack_top)
                return finish(s,r,CS_IMAGE_RUN_QUARANTINED,&guest);
            return finish(s,r,CS_IMAGE_RUN_RETURNED,&guest);
        }
    }
    return finish(s,r,CS_IMAGE_RUN_BUDGET,&guest);
}
cs_image_result cs_image_run_retire(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_report *r)
{
    if(!cs_image_execution_retire_native(s,q,w,capacity,out,r))return CS_IMAGE_ARGUMENT;
    if((s->execution_state!=3 && s->execution_state!=4) || q->generation!=s->generation ||
        q->epoch!=s->epoch || s->execution_generation!=s->generation)return CS_IMAGE_STATE;
    if(!cs_image_execution_integrity(s,&q->context,1)){s->execution_state=5;return CS_IMAGE_FAULT;}
    /* Establish every scrub destination before changing any byte. Only below-A5
     * becomes canonical; stack and the caller-owned stop snapshot are retained. */
    for(uint32_t i=0;i<s->manifest.below_a5;++i){cs_guest_view v;
        if(cs_guest_window(s->map,s->layout.globals_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK){s->execution_state=5;return CS_IMAGE_FAULT;}}
    for(uint32_t i=0;i<s->manifest.below_a5;++i){cs_guest_view v;
        if(cs_guest_window(s->map,s->layout.globals_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK){s->execution_state=5;return CS_IMAGE_FAULT;}
        v.writable[0]=0;
        ++s->execution_scrubbed;
    }
    if(!cs_image_execution_integrity(s,&q->context,0)){s->execution_state=5;return CS_IMAGE_FAULT;}
    cs_image_request release;clear(&release,sizeof release);release.operation=CS_IMAGE_RELEASE;
    copy(&release.context,&q->context,sizeof release.context);release.generation=s->generation;release.epoch=s->epoch;
    release.b=s->layout.b;release.s=s->layout.s;release.h_min=s->layout.h_min;
    s->execution_state=0;
    cs_image_result result=cs_image_execute(s,&release,w,capacity,out,r);
    r->execution_scrubbed=s->execution_scrubbed;
    if(result!=CS_IMAGE_OK)s->execution_state=5;
    return result;
}
