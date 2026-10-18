/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0045 selection owner/coordinator. */
#include "code-run-internal.h"
typedef struct {
    cs_image_select_request request;
    cs_trap_config config;
    uint32_t magic;
} run_control;
static void clear(void *p,size_t n){uint8_t *b=p;for(size_t i=0;i<n;++i)b[i]=0;}
static void copy(void *p,const void *q,size_t n)
{uint8_t *a=p;const uint8_t *b=q;for(size_t i=0;i<n;++i)a[i]=b[i];}
static void frame_copy(cs_trap_frame *a,const cs_trap_frame *b)
{for(unsigned i=0;i<8;++i){a->a[i]=b->a[i];a->d[i]=b->d[i];}a->pc=b->pc;a->sr=b->sr;}
static int inside(uint32_t a,uint32_t n,uint32_t base,uint32_t size)
{return n && a>=base && n<=size && a-base<=size-n;}
static int intersects(uint32_t a,uint32_t n,uint32_t b,uint32_t m){return n && m && (a<=b?b-a<n:a-b<m);}
static int apart(const uint8_t *a,size_t n,const uint8_t *b,size_t m)
{uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return !n || !m || (x<=y?y-x>=n:x-y>=m);}
static uint32_t be32(const uint8_t *p)
{return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static void put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>((3u-i)*8u));}
static int table_slot(const cs_image_service *s,uint32_t pc)
{return pc>=s->layout.table_begin+2u && pc<s->layout.table_end && ((pc-s->layout.table_begin)&7u)==2u;}
static int opcode(const cs_image_service *s,uint32_t pc)
{
    for(uint32_t i=0;i<s->manifest.segment_count;++i){const cs_image_segment *p=&s->segments[i];
        if(p->length>=6 && inside(pc,2,p->address+4u,p->length-4u))return 1;}
    return table_slot(s,pc);
}
static int extension(const cs_image_service *s,const cs_m68k_guard_event *e)
{
    uint32_t pc=e->before->pc;
    if(table_slot(s,pc))return e->word==0x4EF9u && e->address==pc+2u && e->bytes==4;
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
/* Every guarded access revalidates the lease and exact-g owned integrity. */
static int check(const void *opaque,const cs_m68k_guard_event *e)
{
    const cs_image_select_policy *p=opaque;const cs_image_service *s=p->image;
    if(s->execution_state!=2 || s->execution_generation!=s->generation ||
        s->select.phase!=CS_IMAGE_SELECT_ACTIVE ||
        !cs_image_select_integrity(s,&p->request->run.context,s->select.g))return 0;
    switch(e->kind){
        case CS_M68K_GUARD_OPCODE:return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_EXTENSION:return extension(s,e);
        case CS_M68K_GUARD_READ:return memory(s,e,0);
        case CS_M68K_GUARD_WRITE:return memory(s,e,1);
        case CS_M68K_GUARD_INSTRUCTION:
            if((e->word&0xF000u)==0xA000u)return 0;
            return !(e->before->pc>=s->layout.table_begin && e->before->pc<s->layout.table_end) || e->word==0x4EF9u;
        case CS_M68K_GUARD_TARGET:
            if(e->address==p->token)return e->word==0x4E75u && e->before->a[7]==p->root;
            return e->bytes==2 && opcode(s,e->address);
        case CS_M68K_GUARD_COMMIT:
            if(e->proposed->a[5]!=s->layout.a5)return 0;
            if(e->proposed->pc==p->token)return e->word==0x4E75u &&
                e->before->a[7]==p->root && e->proposed->a[7]==s->layout.stack_top;
            return opcode(s,e->proposed->pc);
        default:return 0;
    }
}
static int token_ok(const cs_image_service *s,uint32_t at)
{
    cs_guest_view v;
    if((at&1u) || at>0xFFFFF8u || cs_guest_window(s->map,at,8,CS_GUEST_READ|CS_GUEST_EXECUTE,&v)!=CS_GUEST_OK)return 0;
    if(inside(at,8,s->config.system.begin,s->config.system.end-s->config.system.begin) ||
        inside(at,8,s->config.application.begin,s->config.application.end-s->config.application.begin) ||
        inside(at,8,s->layout.stack_base,s->layout.b-s->layout.stack_base))return 0;
    for(unsigned i=0;i<8;++i)if(v.bytes[i]!=(i&1u?0x71u:0x4Eu))return 0;
    return 1;
}
static int same_context(const cs_image_context *a,const cs_image_context *b)
{return a->context==b->context && a->active_root==b->active_root &&
    a->active_callback==b->active_callback && a->active_patch==b->active_patch &&
    a->active_manager==b->active_manager && a->session==b->session &&
    a->inventory==b->inventory && a->count==b->count && a->resources==b->resources &&
    a->resource_count==b->resource_count;}
static int same_control(const cs_image_select_request *q,const run_control *c)
{
    const cs_image_run_request *a=&q->run,*b=&c->request.run;
    if(q->selection!=c->request.selection || a->gateway!=b->gateway ||
        a->generation!=b->generation || a->epoch!=b->epoch || a->token!=b->token ||
        a->budget!=b->budget || !same_context(&a->context,&b->context))return 0;
    const cs_trap_config *x=&a->gateway->config,*y=&c->config;
    return a->gateway->magic==c->magic && x->rom_base==y->rom_base &&
        x->ram_begin==y->ram_begin && x->ram_end==y->ram_end && x->get_token==y->get_token &&
        x->set_token==y->set_token && x->unsupported_token==y->unsupported_token;
}
static int same_view(const cs_qd_view *a,const cs_qd_view *b)
{return a->port==b->port && a->x==b->x && a->y==b->y && a->top==b->top &&
    a->left==b->left && a->bottom==b->bottom && a->right==b->right;}
static int same_selection(const cs_qd_selection *a,const cs_qd_selection *snap)
{
    if(a->globals_begin!=snap->globals_begin || a->a5!=snap->a5 || a->g!=snap->g || a->count!=snap->count)return 0;
    for(uint32_t i=0;i<snap->count;++i)if(a->bindings[i].id!=snap->bindings[i].id ||
        a->bindings[i].view!=snap->bindings[i].view)return 0;
    return 1;
}
static void fill(const cs_image_service *s,cs_image_select_report *r,int run)
{
    const cs_image_select_state *o=&s->select;uint32_t current=0;
    r->profile=o->profile;r->phase=o->phase;r->generation=o->generation;
    r->consumed_generation=o->consumed_generation;r->g=o->g;
    if(o->profile && o->phase!=CS_IMAGE_SELECT_RETIRED &&
        cs_guest_read(s->map,o->g,4,CS_GUEST_READ,&current)!=CS_GUEST_OK)current=0;
    r->current=current;r->services=o->services;r->last_word=o->last_word;r->binding_count=o->count;
    r->first_stop_valid=o->stop_valid;r->first_stop_status=o->stop_status;
    r->first_stop_pc=o->stop_pc;r->first_stop_a7=o->stop_a7;
    r->first_fault_valid=o->fault_valid;r->first_fault_cause=o->fault_cause;
    r->first_fault_phase=o->fault_phase;r->first_fault_pc=o->fault_pc;r->first_fault_word=o->fault_word;
    r->borrow_unresolved=o->borrow_open;r->quarantined=o->phase==CS_IMAGE_SELECT_QUARANTINE;
    r->scrubbed_bytes=o->scrubbed;r->anchor_bytes=o->anchor_cleared;
    r->handback_started=o->handback_started;r->handback_result=o->handback_result;
    r->retired=o->phase==CS_IMAGE_SELECT_RETIRED;
    if(run && o->stop_valid){r->run.status=(cs_image_run_result)o->stop_status;
        r->run.cpu=(cs_m68k_result)o->stop_cpu;r->run.memory=(cs_guest_result)o->stop_memory;
        r->run.generation=o->generation;r->run.epoch=o->epoch;r->run.steps=o->stop_steps;
        r->run.entered=o->stop_entered;frame_copy(&r->run.guest,&o->stop_frame);}
}
/* Trusted owner revalidation: any mismatch after admission quarantines. */
static cs_image_select_cause owner(const cs_image_service *s,const cs_image_select_request *q,
    const cs_image_select_workspace *w,const run_control *control)
{
    if(!same_control(q,control))return CS_IMAGE_SELECT_BINDING;
    const cs_image_select_state *o=&s->select;const cs_qd_selection *c=q->selection;uint32_t seal;cs_guest_view v;
    if(s->execution_state!=2 || o->phase!=CS_IMAGE_SELECT_ACTIVE || o->depth || !o->borrow_open)
        return CS_IMAGE_SELECT_LEASE;
    if(o->generation!=s->generation || o->execution_generation!=s->execution_generation)
        return CS_IMAGE_SELECT_GENERATION;
    if(o->epoch!=s->epoch || o->epoch!=s->lifecycle->epoch)return CS_IMAGE_SELECT_EPOCH;
    if(w->selection.count!=o->count || w->selection.g!=o->g || !same_selection(c,&w->selection))
        return CS_IMAGE_SELECT_BINDING;
    for(uint32_t i=0;i<o->count;++i)if(w->selection.bindings[i].id!=o->ids[i] ||
        !same_view(c->bindings[i].view,&w->views[i]))return CS_IMAGE_SELECT_BINDING;
    if(cs_guest_read(s->map,o->a5,4,CS_GUEST_READ,&seal)!=CS_GUEST_OK || seal!=o->g)return CS_IMAGE_SELECT_SEAL;
    if(!cs_image_select_integrity(s,&q->run.context,o->g))return CS_IMAGE_SELECT_HEADER;
    for(uint32_t i=0;i<o->count;++i)
        if(cs_guest_window(s->map,o->ids[i],o->extent,CS_GUEST_READ|CS_GUEST_WRITE,&v)!=CS_GUEST_OK)
            return CS_IMAGE_SELECT_MAP;
    return CS_IMAGE_SELECT_OK;
}
/* Every fallible check precedes the claim; afterwards only total stores. */
static cs_image_select_cause admit(cs_image_service *s,const cs_image_select_request *q,
    cs_image_select_workspace *w,cs_image_select_report *r)
{
    const cs_image_run_request *run=&q->run;const cs_qd_selection *c=q->selection;cs_guest_view v;
    cs_image_select_state *o=&s->select;uint32_t a,b,root;
    if(o->phase!=CS_IMAGE_SELECT_NONE && o->phase!=CS_IMAGE_SELECT_RETIRED)return CS_IMAGE_SELECT_STATE;
    if(s->execution_state || s->execution_generation==s->generation ||
        (o->profile && o->consumed_generation==s->generation))return CS_IMAGE_SELECT_GENERATION;
    if(run->generation!=s->generation)return CS_IMAGE_SELECT_GENERATION;
    if(run->epoch!=s->epoch)return CS_IMAGE_SELECT_EPOCH;
    if(!run->budget || run->budget>1000000u)return CS_IMAGE_SELECT_STATE;
    if(!cs_image_execution_integrity(s,&run->context,0) || !token_ok(s,run->token) ||
        s->layout.stack_top<s->layout.stack_base+4u)return CS_IMAGE_SELECT_WORLD;
    for(size_t i=0;i<s->map->count;++i)for(size_t j=0;j<i;++j)
        if(!apart(s->map->regions[i].bytes,s->map->regions[i].size,s->map->regions[j].bytes,s->map->regions[j].size))
            return CS_IMAGE_SELECT_MAP;
    if(s->layout.globals_begin+s->manifest.below_a5!=s->layout.a5)return CS_IMAGE_SELECT_LAYOUT;
    /* Owner-made snapshot; caller workspace contents grant nothing. */
    clear(&w->selection,sizeof w->selection);
    w->selection.globals_begin=c->globals_begin;w->selection.a5=c->a5;w->selection.g=c->g;w->selection.count=c->count;
    for(uint32_t i=0;i<c->count;++i){w->selection.bindings[i].id=c->bindings[i].id;
        w->selection.bindings[i].view=c->bindings[i].view;copy(&w->views[i],c->bindings[i].view,sizeof w->views[i]);}
    if(c->a5!=s->layout.a5 || c->globals_begin!=s->layout.globals_begin ||
        !cs_qd_select_get(&w->selection,0,&w->plan))return CS_IMAGE_SELECT_CANDIDATE;
    if(cs_trap_get(run->gateway,CS_IMAGE_SELECT_SET_PORT&0x1FFu,&a)!=CS_TRAP_OK ||
        cs_trap_get(run->gateway,CS_IMAGE_SELECT_GET_PORT&0x1FFu,&b)!=CS_TRAP_OK ||
        a!=run->gateway->config.unsupported_token || b!=run->gateway->config.unsupported_token)
        return CS_IMAGE_SELECT_DIRECTORY;
    root=s->layout.stack_top-4u;
    if(cs_guest_window(s->map,c->a5,4,CS_GUEST_READ|CS_GUEST_WRITE,&w->seal)!=CS_GUEST_OK ||
        cs_guest_window(s->map,c->g,4,CS_GUEST_READ|CS_GUEST_WRITE,&w->cell)!=CS_GUEST_OK ||
        cs_guest_window(s->map,root,4,CS_GUEST_WRITE,&w->parameter)!=CS_GUEST_OK)return CS_IMAGE_SELECT_MAP;
    for(uint32_t i=0;i<c->count;++i)if(cs_guest_window(s->map,c->bindings[i].id,CS_QD_SELECT_RECORD_BYTES,
        CS_GUEST_READ|CS_GUEST_WRITE,&v)!=CS_GUEST_OK)return CS_IMAGE_SELECT_MAP;
    clear(&w->guest,sizeof w->guest);w->guest.pc=s->layout.table_begin+2u;w->guest.sr=0x2700;
    w->guest.a[5]=s->layout.a5;w->guest.a[7]=s->layout.stack_top;
    clear(&w->policy,sizeof w->policy);w->policy.image=s;w->policy.request=q;w->policy.root=root;w->policy.token=run->token;
    w->guard.check=check;w->guard.context=&w->policy;w->guard.context_bytes=sizeof w->policy;
    w->limits.base=s->layout.stack_base;w->limits.size=s->layout.stack_top-s->layout.stack_base;
    clear(&w->step,sizeof w->step);
    if(cs_m68k_bus_guard_validate(s->map,run->gateway,&w->guest,&w->step,&w->limits,&w->guard)!=CS_M68K_OK)
        return CS_IMAGE_SELECT_FRAME;
    clear(o,sizeof *o);o->profile=CS_IMAGE_SELECT_PROFILE;o->phase=CS_IMAGE_SELECT_ACTIVE;
    o->generation=s->generation;o->execution_generation=s->generation;o->epoch=s->epoch;
    o->consumed_generation=s->generation;o->owner=(uintptr_t)s;o->map=(uintptr_t)s->map;
    o->lifecycle=(uintptr_t)s->lifecycle;o->manager=(uintptr_t)s->manager;
    o->inventory_native=(uintptr_t)run->context.inventory[0].native;o->owner_id=s->profile.owner;
    o->inventory_kind=(uint32_t)run->context.inventory[0].kind;o->inventory_bytes=(uint32_t)run->context.inventory[0].bytes;
    copy(&o->layout,&s->layout,sizeof o->layout);o->globals_begin=s->layout.globals_begin;o->a5=s->layout.a5;
    o->below_a5=s->manifest.below_a5;o->above_a5=s->manifest.above_a5;
    o->stack_base=s->layout.stack_base;o->stack_top=s->layout.stack_top;o->root=root;o->token=run->token;
    o->g=c->g;o->reservation_begin=c->g-CS_IMAGE_SELECT_BELOW_G;o->count=c->count;
    for(uint32_t i=0;i<c->count;++i)o->ids[i]=c->bindings[i].id;
    o->extent=CS_QD_SELECT_RECORD_BYTES;o->borrow_open=1;
    s->execution_generation=s->generation;s->execution_state=1;s->execution_scrubbed=0;
    put32(w->seal.writable,c->g);put32(w->cell.writable,0);put32(w->parameter.writable,run->token);
    w->guest.a[7]=root;s->execution_state=2;
    r->run.generation=s->generation;r->run.epoch=s->epoch;r->run.entered=1;
    return CS_IMAGE_SELECT_OK;
}
static cs_image_run_result finish(cs_image_service *s,cs_image_select_workspace *w,size_t capacity,
    cs_image_select_report *r,cs_image_run_result result,cs_image_select_cause cause)
{
    cs_image_select_state *o=&s->select;
    frame_copy(&r->run.guest,&w->guest);r->run.status=result;
    s->execution_state=result==CS_IMAGE_RUN_RETURNED?3u:result==CS_IMAGE_RUN_QUARANTINED?5u:4u;
    o->phase=result==CS_IMAGE_RUN_QUARANTINED?CS_IMAGE_SELECT_QUARANTINE:CS_IMAGE_SELECT_STOPPED;
    if(!o->stop_valid){o->stop_valid=1;o->stop_status=(uint32_t)result;o->stop_pc=w->guest.pc;
        o->stop_a7=w->guest.a[7];o->stop_cpu=(uint32_t)r->run.cpu;o->stop_memory=(uint32_t)r->run.memory;
        o->stop_steps=r->run.steps;o->stop_entered=r->run.entered;frame_copy(&o->stop_frame,&w->guest);}
    if(cause!=CS_IMAGE_SELECT_OK && !o->fault_valid){o->fault_valid=1;o->fault_cause=cause;
        o->fault_phase=CS_IMAGE_SELECT_ACTIVE;o->fault_pc=w->guest.pc;o->fault_word=w->word;}
    o->quiescent=o->depth==0;o->borrow_open=o->quiescent?0u:1u;
    r->cause=cause;fill(s,r,0);
    if(o->quiescent)clear(w,capacity);
    return result;
}
/* Canonical A873/A874, SPEC-0042 section 10 order. Bad guest input is a
 * pre-effect policy stop; *cause reports trusted mismatches. */
static cs_m68k_result service(cs_image_service *s,cs_image_select_workspace *w,uint16_t word,
    cs_image_select_cause *cause)
{
    const cs_image_select_state *o=&s->select;cs_trap_frame *f=&w->guest;cs_m68k_bus_report *b=&w->step;
    const cs_trap_gateway *gw=w->policy.request->run.gateway;uint32_t sp=f->a[7],seal,x,y;cs_guest_view v;
    *cause=CS_IMAGE_SELECT_OK;
    b->cpu.pc=f->pc;b->cpu.word=word;b->cpu.service_called=1;b->cpu.service.pc=f->pc;
    b->cpu.service.decoded.word=word;b->cpu.service.decoded.number=(uint16_t)(word&0x1FFu);b->cpu.service.decoded.toolbox=1;
    if(cs_trap_get(gw,CS_IMAGE_SELECT_SET_PORT&0x1FFu,&x)!=CS_TRAP_OK ||
        cs_trap_get(gw,CS_IMAGE_SELECT_GET_PORT&0x1FFu,&y)!=CS_TRAP_OK ||
        x!=gw->config.unsupported_token || y!=gw->config.unsupported_token)return CS_M68K_POLICY;
    if((f->sr&0xA700u)!=0x2700u || f->a[5]!=o->a5 || (sp&1u) || sp<6u || sp>o->root || o->root-sp<4u ||
        !inside(sp-6u,10,o->stack_base,o->stack_top-o->stack_base) || !opcode(s,f->pc+2u))return CS_M68K_POLICY;
    b->address=sp;b->bytes=4;b->access=CS_GUEST_READ;
    b->memory=cs_guest_window(s->map,sp,4,CS_GUEST_READ,&w->parameter);
    if(b->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    w->parameter_value=be32(w->parameter.bytes);
    if(cs_guest_read(s->map,o->a5,4,CS_GUEST_READ,&seal)!=CS_GUEST_OK || seal!=o->g)
        {*cause=CS_IMAGE_SELECT_SEAL;return CS_M68K_POLICY;}
    if(cs_guest_window(s->map,o->g,4,CS_GUEST_READ|CS_GUEST_WRITE,&w->cell)!=CS_GUEST_OK)
        {*cause=CS_IMAGE_SELECT_MAP;return CS_M68K_POLICY;}
    w->current=be32(w->cell.bytes);
    if(word==CS_IMAGE_SELECT_SET_PORT){
        if(!cs_qd_select_set(&w->selection,w->current,w->parameter_value,&w->plan))return CS_M68K_POLICY;
    }else{uint32_t d=w->parameter_value;
        if(!cs_qd_select_get(&w->selection,w->current,&w->plan))return CS_M68K_POLICY;
        if((d&1u) || !inside(d,CS_IMAGE_SELECT_DEST_BYTES,o->globals_begin,o->a5-o->globals_begin) ||
            intersects(d,CS_IMAGE_SELECT_DEST_BYTES,o->reservation_begin,CS_IMAGE_SELECT_RESERVATION))return CS_M68K_POLICY;
        for(uint32_t i=0;i<o->count;++i)if(intersects(d,CS_IMAGE_SELECT_DEST_BYTES,o->ids[i],o->extent))return CS_M68K_POLICY;
        b->address=d;b->bytes=4;b->access=CS_GUEST_WRITE;
        b->memory=cs_guest_window(s->map,d,4,CS_GUEST_WRITE,&w->destination);
        if(b->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
        if(!apart(w->destination.writable,4,w->cell.bytes,4))return CS_M68K_POLICY;
        for(uint32_t i=0;i<o->count;++i){
            if(cs_guest_window(s->map,o->ids[i],o->extent,CS_GUEST_READ,&v)!=CS_GUEST_OK)
                {*cause=CS_IMAGE_SELECT_MAP;return CS_M68K_POLICY;}
            if(!apart(w->destination.writable,4,v.bytes,o->extent))return CS_M68K_POLICY;}
    }
    w->staged_value=w->plan.value;
    b->address=sp-6u;b->bytes=6;b->access=CS_GUEST_WRITE;
    b->memory=cs_guest_window(s->map,sp-6u,6,CS_GUEST_WRITE,&w->scratch);
    if(b->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    frame_copy(&w->next,f);w->next.pc+=2u;w->next.a[7]+=4u;
    w->scratch_bytes[0]=(uint8_t)(f->sr>>8);w->scratch_bytes[1]=(uint8_t)f->sr;
    put32(w->scratch_bytes+2,w->next.pc);put32(w->value_bytes,w->staged_value);
    /* Commit: total, no lookup or callback. */
    copy(w->scratch.writable,w->scratch_bytes,6);
    copy(word==CS_IMAGE_SELECT_SET_PORT?w->cell.writable:w->destination.writable,w->value_bytes,4);
    frame_copy(f,&w->next);
    ++s->select.services;s->select.last_word=word;b->cpu.service.status=CS_TRAP_OK;
    return CS_M68K_OK;
}
static cs_image_run_result drive(cs_image_service *s,const cs_image_select_request *q,
    cs_image_select_workspace *w,size_t capacity,cs_image_select_report *r,const run_control *control)
{
    const cs_image_run_request *run=&control->request.run;
    for(uint32_t i=0;i<run->budget;++i){
#ifdef CLASSICK_IMAGE_TEST_FAULTS
        if(s->select_test_kind && r->run.steps==s->select_test_after){
            if(s->select_test_kind==CS_IMAGE_SELECT_TEST_HEADER)w->seal.writable[3]^=1u;
            else if(s->select_test_kind==CS_IMAGE_SELECT_TEST_BINDING)w->views[0].x^=1;
            else s->select.depth=1;
            s->select_test_kind=0;s->select_test_after=0;}
#endif
        cs_image_select_cause cause=owner(s,q,w,control);
        w->word=0;
        if(cause!=CS_IMAGE_SELECT_OK)return finish(s,w,capacity,r,CS_IMAGE_RUN_QUARANTINED,cause);
        clear(&w->step,sizeof w->step);
        cs_m68k_result cpu;
        if(opcode(s,w->guest.pc) && cs_guest_read(s->map,w->guest.pc,2,CS_GUEST_READ,&w->word)==CS_GUEST_OK &&
            (w->word==CS_IMAGE_SELECT_SET_PORT || w->word==CS_IMAGE_SELECT_GET_PORT))
            cpu=service(s,w,(uint16_t)w->word,&cause);
        else cpu=cs_m68k_bus_step_admitted(s->map,run->gateway,&w->guest,&w->step,&w->limits,&w->guard,
            run->token,s->select.root);
        w->step.cpu.status=cpu;
        if(w->step.cpu.service_called && cpu!=CS_M68K_OK)
            w->step.cpu.service.status=cpu==CS_M68K_ADDRESS?CS_TRAP_ADDRESS:CS_TRAP_UNSUPPORTED_FORMAT;
        r->run.cpu=cpu;r->run.memory=w->step.memory;
        if(cause!=CS_IMAGE_SELECT_OK)return finish(s,w,capacity,r,CS_IMAGE_RUN_QUARANTINED,cause);
        if(cpu!=CS_M68K_OK){cause=owner(s,q,w,control);
            if(cause!=CS_IMAGE_SELECT_OK || cpu==CS_M68K_ARGUMENT)return finish(s,w,capacity,r,CS_IMAGE_RUN_QUARANTINED,
                cause!=CS_IMAGE_SELECT_OK?cause:CS_IMAGE_SELECT_LEASE);
            return finish(s,w,capacity,r,CS_IMAGE_RUN_CPU,CS_IMAGE_SELECT_OK);}
        ++r->run.steps;
        if(w->guest.pc==run->token){
            if(w->step.cpu.word!=0x4E75u || w->guest.a[7]!=s->layout.stack_top)
                return finish(s,w,capacity,r,CS_IMAGE_RUN_QUARANTINED,CS_IMAGE_SELECT_FRAME);
            return finish(s,w,capacity,r,CS_IMAGE_RUN_RETURNED,CS_IMAGE_SELECT_OK);}
    }
    return finish(s,w,capacity,r,CS_IMAGE_RUN_BUDGET,CS_IMAGE_SELECT_OK);
}
cs_image_run_result cs_image_run_selection(cs_image_service *s,const cs_image_select_request *q,
    cs_trap_frame *caller,cs_image_select_workspace *w,size_t capacity,cs_image_select_report *r)
{
    if(!cs_image_select_native(s,q,caller,w,capacity,r))return CS_IMAGE_RUN_ARGUMENT;
    run_control control;copy(&control.request,q,sizeof *q);
    copy(&control.config,&q->run.gateway->config,sizeof control.config);control.magic=q->run.gateway->magic;
    clear(r,sizeof *r);r->run.status=CS_IMAGE_RUN_REFUSED;
    cs_image_select_cause cause=admit(s,q,w,r);
    if(cause!=CS_IMAGE_SELECT_OK){fill(s,r,0);r->cause=cause;r->run.status=CS_IMAGE_RUN_REFUSED;
        clear(w,capacity);return CS_IMAGE_RUN_REFUSED;}
    return drive(s,q,w,capacity,r,&control);
}
static cs_image_result retire_fault(cs_image_service *s,cs_image_select_report *r,cs_image_select_cause cause)
{
    cs_image_select_state *o=&s->select;
    if(!o->fault_valid){o->fault_valid=1;o->fault_cause=cause;o->fault_phase=o->phase;
        o->fault_pc=o->stop_pc;o->fault_word=0;}
    s->execution_state=5;o->phase=CS_IMAGE_SELECT_QUARANTINE;
    fill(s,r,1);r->cause=cause;return CS_IMAGE_FAULT;
}
cs_image_result cs_image_run_selection_retire(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_select_report *r)
{
    if(!cs_image_select_retire_native(s,q,w,capacity,out,r))return CS_IMAGE_ARGUMENT;
    cs_image_select_state *o=&s->select;cs_guest_view below,anchor;
    clear(r,sizeof *r);
    if(o->phase!=CS_IMAGE_SELECT_STOPPED ||
        (s->execution_state!=3 && s->execution_state!=4) || q->generation!=s->generation || q->epoch!=s->epoch ||
        s->execution_generation!=s->generation){fill(s,r,1);r->cause=CS_IMAGE_SELECT_STATE;return CS_IMAGE_STATE;}
    if(o->borrow_open || !o->quiescent || o->depth){
        o->borrow_open=1;o->quiescent=0;return retire_fault(s,r,CS_IMAGE_SELECT_LEASE);}
    o->phase=CS_IMAGE_SELECT_PIN;
    if(!cs_image_select_integrity(s,&q->context,o->g))return retire_fault(s,r,CS_IMAGE_SELECT_HEADER);
    if(cs_guest_window(s->map,o->globals_begin,o->below_a5,CS_GUEST_WRITE,&below)!=CS_GUEST_OK ||
        cs_guest_window(s->map,o->a5,4,CS_GUEST_WRITE,&anchor)!=CS_GUEST_OK)return retire_fault(s,r,CS_IMAGE_SELECT_MAP);
    o->phase=CS_IMAGE_SELECT_SCRUB;
    for(uint32_t i=0;i<o->below_a5;++i){below.writable[i]=0;++o->scrubbed;}
    o->phase=CS_IMAGE_SELECT_ANCHOR;
    for(uint32_t i=0;i<4u;++i){anchor.writable[i]=0;++o->anchor_cleared;}
    o->phase=CS_IMAGE_SELECT_HANDBACK;o->handback_started=1;
    cs_image_request release;clear(&release,sizeof release);release.operation=CS_IMAGE_RELEASE;
    copy(&release.context,&q->context,sizeof release.context);release.generation=s->generation;release.epoch=s->epoch;
    release.b=s->layout.b;release.s=s->layout.s;release.h_min=s->layout.h_min;
    cs_image_result result=cs_image_select_release(s,&release,w,capacity,out,&r->release);
    o->handback_result=(uint32_t)result;
    if(result!=CS_IMAGE_OK){retire_fault(s,r,CS_IMAGE_SELECT_RELEASE);return result;}
    o->phase=CS_IMAGE_SELECT_RETIRED;fill(s,r,1);r->cause=CS_IMAGE_SELECT_OK;
    return CS_IMAGE_OK;
}
#ifdef CLASSICK_IMAGE_TEST_FAULTS
cs_image_result cs_image_selection_test_fault(cs_image_service *s,uint32_t after_steps,uint32_t kind)
{
    if(!s || ((uintptr_t)s&(uintptr_t)(_Alignof(cs_image_service)-1u)) || s->self!=s ||
        kind>CS_IMAGE_SELECT_TEST_QUIESCENCE || (kind && !after_steps))return CS_IMAGE_ARGUMENT;
    if(s->execution_state || (s->select.phase!=CS_IMAGE_SELECT_NONE && s->select.phase!=CS_IMAGE_SELECT_RETIRED))
        return CS_IMAGE_STATE;
    s->select_test_after=kind?after_steps:0u;s->select_test_kind=kind;return CS_IMAGE_OK;
}
#endif
