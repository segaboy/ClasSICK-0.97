/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "lifecycle.h"
#define LIFE_TAG UINT32_C(0x4C463031)
#define PLAN_TAG UINT32_C(0x4C503031)
static const uint32_t locations[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
typedef struct { const void *at;size_t bytes; } blob;
static uint32_t read32(const uint8_t *b)
{return ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];}
static void write32(uint8_t *b,uint32_t v)
{for(unsigned i=0;i<4;++i)b[i]=(uint8_t)(v>>((3u-i)*8u));}
static int native_valid(blob b)
{return b.bytes==0 || (b.at!=0 && b.bytes<=UINTPTR_MAX-(uintptr_t)b.at);}
static int overlap(blob a,blob b)
{
    if(!a.bytes || !b.bytes)return 0;
    uintptr_t x=(uintptr_t)a.at,y=(uintptr_t)b.at;
    return x<=y?y-x<a.bytes:x-y<b.bytes;
}
static int guest_overlap(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n && m && (a<=b?b-a<n:a-b<m);}
static int same_bytes(const uint8_t *a,const uint8_t *b,uint32_t n)
{for(uint32_t i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static void copy_report(cs_life_report *d,const cs_life_report *s)
{
    d->cause=s->cause;d->operation=s->operation;d->phase=s->phase;
    d->prepared=s->prepared;d->committed=s->committed;d->epoch_valid=s->epoch_valid;
    d->epoch_before=s->epoch_before;d->epoch_after=s->epoch_after;
    d->attempted=s->attempted;d->completed=s->completed;d->scrap_valid=s->scrap_valid;
    d->old_handle=s->old_handle;d->new_handle=s->new_handle;d->new_handle_valid=s->new_handle_valid;d->size=s->size;
    d->globals_valid=s->globals_valid;d->state=s->state;
    for(unsigned i=0;i<6;++i)d->globals[i]=s->globals[i];
}
static void report(cs_life_report *r,const cs_life_controller *c,cs_life_operation op)
{
    r->cause=CS_LIFE_OK;r->operation=op;r->phase=CS_LIFE_PREFLIGHT;
    r->prepared=0;r->committed=0;r->epoch_valid=1;r->epoch_before=c->epoch;
    r->epoch_after=c->epoch;r->attempted=0;r->completed=0;r->scrap_valid=0;
    r->old_handle=0;r->new_handle=0;r->new_handle_valid=0;r->size=0;r->globals_valid=0;r->state=c->state;
    for(unsigned i=0;i<6;++i)r->globals[i]=0;
}
static cs_life_result integrity(cs_mm_manager *m)
{
    if(!m || !m->map)return CS_LIFE_ARGUMENT;
    cs_mm_request q;cs_mm_reply r;
    q.operation=CS_MM_GET_PTR_SIZE;q.address=0;q.other=0;q.size=0;q.flags=0;q.interrupt_context=0;
    return cs_mm_execute(m,&q,&r)==CS_MM_OK && r.code==CS_MM_WRONG_ZONE?
        CS_LIFE_OK:CS_LIFE_MEMORY;
}
static blob object_span(const cs_life_controller *c,const cs_life_request *q,
    const cs_life_plan *p,const cs_life_report *out,unsigned i)
{
    const cs_mm_manager *m=c->manager;
    switch(i){
    case 0:return (blob){c,sizeof *c};case 1:return (blob){p,sizeof *p};
    case 2:return (blob){q==&p->request?p->input:q,sizeof *q};case 3:return (blob){out,sizeof *out};
    case 4:return (blob){m,sizeof *m};case 5:return (blob){m->map,sizeof *m->map};
    case 6:return (blob){m->map->regions,m->map->count*sizeof *m->map->regions};
    case 7:return (blob){q->scrap,sizeof *q->scrap};
    case 8:return (blob){q->inventory,q->count*sizeof *q->inventory};
    case 9:return (blob){q->staging,q->staging_bytes};
    case 10:return (blob){q->gateway,q->gateway?sizeof *q->gateway:0};
    case 11:return (blob){q->session,q->session?sizeof *q->session:0};
    default:{const cs_life_entry *e=&q->inventory[i-12u];
        return (blob){e->native,e->kind==CS_LIFE_NATIVE?e->bytes:0};}
    }
}
/* No mutable destination is used until this check establishes its disjointness. */
static cs_life_result envelope(const cs_life_controller *c,const cs_life_request *q,
    const cs_life_plan *p,const cs_life_report *out)
{
    if(!c || !q || !p || !out || !c->manager || !c->manager->map || !q->scrap ||
        !q->inventory || q->count<6 || q->count>CS_LIFE_INVENTORY ||
        c->manager->map->count>CS_GUEST_MAX_REGIONS)return CS_LIFE_ARGUMENT;
    const cs_guest_map *map=c->manager->map;
    if(cs_guest_validate(map)!=CS_GUEST_OK)return CS_LIFE_MEMORY;
    unsigned n=12u+(unsigned)q->count;
    for(unsigned i=0;i<n;++i){
        blob a=object_span(c,q,p,out,i);if(!native_valid(a))return CS_LIFE_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(overlap(a,object_span(c,q,p,out,j)))return CS_LIFE_ALIAS;
        for(size_t j=0;j<map->count;++j){
            const cs_guest_region *v=&map->regions[j];
            if(!native_valid((blob){v->bytes,v->size}))return CS_LIFE_MEMORY;
            if(overlap(a,(blob){v->bytes,v->size}))return CS_LIFE_ALIAS;
        }
    }
    for(size_t i=0;i<map->count;++i)for(size_t j=0;j<i;++j)
        if(overlap((blob){map->regions[i].bytes,map->regions[i].size},
            (blob){map->regions[j].bytes,map->regions[j].size}))return CS_LIFE_ALIAS;
    return CS_LIFE_OK;
}
static int same_zone(const cs_mm_zone *a,const cs_mm_zone *b)
{return a->identity==b->identity && a->begin==b->begin && a->end==b->end;}
static void copy_zone(cs_mm_zone *a,const cs_mm_zone *b)
{a->identity=b->identity;a->begin=b->begin;a->end=b->end;}
static int same_block(const cs_mm_block *a,const cs_mm_block *b)
{return a->kind==b->kind && a->zone==b->zone && a->address==b->address && a->size==b->size && a->extent==b->extent;}
static void copy_block(cs_mm_block *a,const cs_mm_block *b)
{a->kind=b->kind;a->zone=b->zone;a->address=b->address;a->size=b->size;a->extent=b->extent;}
static void copy_scrap(cs_life_scrap *a,const cs_life_scrap *b)
{a->size=b->size;a->residency=b->residency;a->handle=b->handle;a->name_address=b->name_address;a->name_bytes=b->name_bytes;}
static int same_scrap(const cs_life_scrap *a,const cs_life_scrap *b)
{return a->size==b->size && a->residency==b->residency && a->handle==b->handle && a->name_address==b->name_address && a->name_bytes==b->name_bytes;}
static void copy_entry(cs_life_entry *a,const cs_life_entry *b)
{a->kind=b->kind;a->action=b->action;a->owner=b->owner;a->address=b->address;a->bytes=b->bytes;a->value=b->value;a->native=b->native;}
static int same_entry(const cs_life_entry *a,const cs_life_entry *b)
{return a->kind==b->kind && a->action==b->action && a->owner==b->owner && a->address==b->address && a->bytes==b->bytes && a->value==b->value && a->native==b->native;}
static void copy_request(cs_life_request *a,const cs_life_request *b)
{
    a->operation=b->operation;a->context=b->context;a->active_root=b->active_root;
    a->active_callback=b->active_callback;a->active_patch=b->active_patch;a->active_manager=b->active_manager;
    a->stack_base=b->stack_base;a->stack_size=b->stack_size;a->stack_reference=b->stack_reference;
    a->reserve=b->reserve;a->minimum=b->minimum;a->scrap=b->scrap;a->staging=b->staging;
    a->staging_bytes=b->staging_bytes;a->inventory=b->inventory;a->count=b->count;
    a->gateway=b->gateway;a->session=b->session;
}
static int same_request(const cs_life_request *a,const cs_life_request *b)
{
    return a->operation==b->operation && a->context==b->context &&
        a->active_root==b->active_root && a->active_callback==b->active_callback &&
        a->active_patch==b->active_patch && a->active_manager==b->active_manager &&
        a->stack_base==b->stack_base && a->stack_size==b->stack_size &&
        a->stack_reference==b->stack_reference && a->reserve==b->reserve && a->minimum==b->minimum &&
        a->scrap==b->scrap && a->staging==b->staging && a->staging_bytes==b->staging_bytes &&
        a->inventory==b->inventory && a->count==b->count && a->gateway==b->gateway && a->session==b->session;
}
static void copy_config(cs_trap_config *a,const cs_trap_config *b)
{a->rom_base=b->rom_base;a->ram_begin=b->ram_begin;a->ram_end=b->ram_end;a->get_token=b->get_token;a->set_token=b->set_token;a->unsupported_token=b->unsupported_token;}
static int same_config(const cs_trap_config *a,const cs_trap_config *b)
{return a->rom_base==b->rom_base && a->ram_begin==b->ram_begin && a->ram_end==b->ram_end && a->get_token==b->get_token && a->set_token==b->set_token && a->unsupported_token==b->unsupported_token;}
static cs_life_result gateway_check(const cs_life_request *q)
{
    for(unsigned i=0;i<CS_TRAP_ENTRIES && q->gateway;++i){
        uint32_t value;if(cs_trap_get(q->gateway,(uint16_t)i,&value)!=CS_TRAP_OK)return CS_LIFE_REFERENCE;
        const cs_trap_config *g=&q->gateway->config;
        uint32_t expected=i==CS_TRAP_GET?g->get_token:i==CS_TRAP_SET?g->set_token:g->unsupported_token;
        for(size_t j=0;j<q->count;++j){const cs_life_entry *e=&q->inventory[j];
            if(e->kind==CS_LIFE_BUILTIN && e->address==i)expected=e->value;}
        if(value!=expected)return CS_LIFE_REFERENCE;
    }
    return CS_LIFE_OK;
}
static cs_life_result inventory_check(const cs_mm_manager *m,const cs_life_request *q,
    const cs_guest_view *global)
{
    unsigned found=0;
    for(size_t i=0;i<q->count;++i){const cs_life_entry *e=&q->inventory[i];
        if(!e->owner || e->kind==CS_LIFE_FUTURE || e->action==CS_LIFE_REFUSE || e->action==CS_LIFE_CLEAR)
            return CS_LIFE_REFERENCE;
        if(e->kind==CS_LIFE_GLOBAL){
            unsigned k=0;while(k<6 && locations[k]!=e->address)++k;
            if(k==6 || e->native || e->bytes!=4 || e->value!=read32(global[k].bytes) ||
                (found&(1u<<k)) || e->action!=((k==1 || k==2)?CS_LIFE_REBIND:CS_LIFE_PRESERVE))return CS_LIFE_REFERENCE;
            found|=1u<<k;
        }else if(e->kind==CS_LIFE_BUILTIN){
            if(!q->gateway || e->native || e->bytes!=4 || e->address>=CS_TRAP_ENTRIES ||
                e->action!=CS_LIFE_PRESERVE || e->value<q->gateway->config.rom_base ||
                e->value-q->gateway->config.rom_base>65534 || (e->value&1u))return CS_LIFE_REFERENCE;
            for(size_t j=0;j<i;++j)if(q->inventory[j].kind==CS_LIFE_BUILTIN && q->inventory[j].address==e->address)
                return CS_LIFE_REFERENCE;
        }else if(e->kind==CS_LIFE_NATIVE){
            if(!e->native || !e->bytes || e->address || e->value || e->action!=CS_LIFE_PRESERVE)return CS_LIFE_REFERENCE;
        }else if(e->kind==CS_LIFE_GUEST){
            cs_guest_view v;
            if(e->native || !e->bytes || e->value || e->action!=CS_LIFE_PRESERVE ||
                cs_guest_window(m->map,e->address,e->bytes,CS_GUEST_READ,&v)!=CS_GUEST_OK)return CS_LIFE_REFERENCE;
            if(guest_overlap(e->address,e->bytes,m->config.application.begin,m->config.application.end-m->config.application.begin))return CS_LIFE_REFERENCE;
            for(unsigned k=0;k<6;++k)if(guest_overlap(e->address,e->bytes,locations[k],4))return CS_LIFE_ALIAS;
            for(size_t j=0;j<i;++j)if(q->inventory[j].kind==CS_LIFE_GUEST &&
                guest_overlap(e->address,e->bytes,q->inventory[j].address,q->inventory[j].bytes))return CS_LIFE_ALIAS;
        }else return CS_LIFE_REFERENCE;
    }
    return found==63?gateway_check(q):CS_LIFE_REFERENCE;
}
typedef struct { cs_guest_view zones[2],global[6],source;uint32_t limit,slot,address,extent,app; } layout;
static cs_life_result preflight(cs_life_controller *c,const cs_life_request *q,layout *l)
{
    cs_mm_manager *m=c->manager;const cs_mm_zone *a=&m->config.application,*s=&m->config.system;
    if(q->operation!=CS_LIFE_RESET && q->operation!=CS_LIFE_TEARDOWN)return CS_LIFE_ARGUMENT;
    if(q->context!=CS_LIFE_ORDINARY || q->active_root || q->active_callback || q->active_patch || q->active_manager)
        return CS_LIFE_CONTEXT;
    if(q->session && (q->session->active || q->session->stepping || q->session->exit_pending ||
        q->session->current || q->session->calls.depth || q->session->calls.map!=m->map ||
        q->session->calls.gateway!=q->gateway || q->session->calls.config.stack_base!=q->stack_base ||
        q->session->calls.config.stack_size!=q->stack_size))return CS_LIFE_CONTEXT;
    if(c->epoch==UINT32_MAX)return CS_LIFE_EPOCH;
    cs_life_result r=integrity(m);if(r!=CS_LIFE_OK)return r;
    cs_guest_view stack;
    if((q->stack_base|q->stack_size|q->stack_reference|q->reserve|q->minimum)&1u ||
        !q->stack_size || q->stack_base<0x400u || q->stack_base>=0x1000000u || q->stack_size>0x1000000u-q->stack_base ||
        q->stack_reference<q->stack_base || q->stack_reference>q->stack_base+q->stack_size ||
        q->reserve>q->stack_reference ||
        cs_guest_window(m->map,q->stack_base,q->stack_size,CS_GUEST_READ|CS_GUEST_WRITE,&stack)!=CS_GUEST_OK)
        return CS_LIFE_CAPACITY;
    if(guest_overlap(q->stack_base,q->stack_size,a->begin,a->end-a->begin) ||
        guest_overlap(q->stack_base,q->stack_size,s->begin,s->end-s->begin))return CS_LIFE_ALIAS;
    for(unsigned i=0;i<6;++i)if(guest_overlap(q->stack_base,q->stack_size,locations[i],4))return CS_LIFE_ALIAS;
    l->limit=q->stack_reference-q->reserve;if(l->limit>a->end)l->limit=a->end;
    if(l->limit<a->begin+CS_MM_CELL_BYTES || q->minimum>l->limit-a->begin-CS_MM_CELL_BYTES)return CS_LIFE_CAPACITY;
    if(cs_guest_window(m->map,a->begin,a->end-a->begin,3,&l->zones[1])!=CS_GUEST_OK ||
        cs_guest_window(m->map,s->begin,s->end-s->begin,3,&l->zones[0])!=CS_GUEST_OK)return CS_LIFE_MEMORY;
    for(unsigned i=0;i<6;++i)if(cs_guest_window(m->map,locations[i],4,3,&l->global[i])!=CS_GUEST_OK)return CS_LIFE_MEMORY;
    r=inventory_check(m,q,l->global);if(r!=CS_LIFE_OK)return r;
    l->source.bytes=0;l->source.writable=0;l->slot=CS_MM_BLOCKS;l->address=0;l->extent=0;l->app=0;
    const cs_life_scrap *b=q->scrap;
    if(b->size<0 || (b->residency!=CS_LIFE_NONE && b->residency!=CS_LIFE_RESIDENT))return CS_LIFE_SCRAP;
    if(!b->name_bytes && b->name_address)return CS_LIFE_SCRAP;
    if(b->name_bytes){cs_guest_view name;
        if(cs_guest_window(m->map,b->name_address,b->name_bytes,CS_GUEST_READ,&name)!=CS_GUEST_OK ||
            guest_overlap(b->name_address,b->name_bytes,a->begin,a->end-a->begin))return CS_LIFE_SCRAP;
        for(unsigned i=0;i<6;++i)if(guest_overlap(b->name_address,b->name_bytes,locations[i],4))return CS_LIFE_SCRAP;
    }
    if(b->residency==CS_LIFE_NONE)return b->size==0 && b->handle==0?CS_LIFE_OK:CS_LIFE_SCRAP;
    unsigned old=0;
    for(;old<CS_MM_BLOCKS;++old){const cs_mm_block *v=&m->blocks[old];
        const cs_mm_zone *z=v->zone? a:s;
        if(v->kind==2 && b->handle==z->begin+old*4u)break;
    }
    if(old==CS_MM_BLOCKS || m->blocks[old].size!=(uint32_t)b->size)return CS_LIFE_SCRAP;
    const cs_mm_block *v=&m->blocks[old];
    if(!(read32(l->zones[v->zone].bytes+old*4u)&0xFFFFFFu))return CS_LIFE_SCRAP;
    if(v->zone==0)return CS_LIFE_OK;
    l->app=1;l->extent=b->size==0?2u:((uint32_t)b->size+1u)&~1u;
    if(l->extent>l->limit-a->begin-CS_MM_CELL_BYTES || (uint32_t)b->size>q->staging_bytes)return CS_LIFE_CAPACITY;
    if(b->size && !q->staging)return CS_LIFE_ARGUMENT;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(!m->blocks[i].kind || m->blocks[i].zone==1){l->slot=i;break;}
    if(l->slot==CS_MM_BLOCKS)return CS_LIFE_CAPACITY;
    l->address=l->limit-l->extent;
    if(b->size && cs_guest_window(m->map,v->address,(uint32_t)b->size,CS_GUEST_READ,&l->source)!=CS_GUEST_OK)return CS_LIFE_SCRAP;
    return CS_LIFE_OK;
}
cs_life_result cs_life_init(cs_life_controller *c,cs_mm_manager *m)
{
    if(!c || !m || !m->map)return CS_LIFE_ARGUMENT;
    if(c->tag!=0)return CS_LIFE_STATE;
    if(overlap((blob){c,sizeof *c},(blob){m,sizeof *m}) ||
        overlap((blob){c,sizeof *c},(blob){m->map,sizeof *m->map}))return CS_LIFE_ALIAS;
    if(cs_guest_validate(m->map)!=CS_GUEST_OK)return CS_LIFE_MEMORY;
    if(overlap((blob){c,sizeof *c},(blob){m->map->regions,m->map->count*sizeof *m->map->regions}))return CS_LIFE_ALIAS;
    for(size_t i=0;i<m->map->count;++i)if(overlap((blob){c,sizeof *c},
        (blob){m->map->regions[i].bytes,m->map->regions[i].size}))return CS_LIFE_ALIAS;
    cs_life_result r=integrity(m);if(r!=CS_LIFE_OK)return r;
    c->manager=m;c->epoch=1;c->state=CS_LIFE_STABLE;c->active=0;c->tag=LIFE_TAG;
    report(&c->first_fault,c,CS_LIFE_RESET);
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    c->test_fault_after=0;
#endif
    return CS_LIFE_OK;
}
cs_life_result cs_life_admit(const cs_life_controller *c)
{
    if(!c || c->tag!=LIFE_TAG || !c->epoch)return CS_LIFE_STATE;
    if(c->state==CS_LIFE_FAULTED)return CS_LIFE_FAULT;
    if(c->state!=CS_LIFE_STABLE || c->active)return CS_LIFE_STATE;
    return integrity(c->manager);
}
cs_life_result cs_life_manager_call(cs_life_controller *c,const cs_mm_request *q,
    cs_mm_reply *out,cs_mm_result *result)
{
    if(!q || !out || !result)return CS_LIFE_ARGUMENT;
    cs_life_result r=cs_life_admit(c);if(r!=CS_LIFE_OK)return r;
    *result=cs_mm_execute(c->manager,q,out);return CS_LIFE_OK;
}
cs_life_result cs_life_prepare(cs_life_controller *c,const cs_life_request *q,cs_life_plan *p,cs_life_report *out)
{
    if(!c || c->tag!=LIFE_TAG || !c->epoch)return CS_LIFE_STATE;
    if(p && q==&p->request)return CS_LIFE_ALIAS;
    cs_life_result r=envelope(c,q,p,out);if(r!=CS_LIFE_OK)return r;
    if(c->state==CS_LIFE_FAULTED){copy_report(out,&c->first_fault);return CS_LIFE_FAULT;}
    report(out,c,q->operation);
    if(c->state!=CS_LIFE_STABLE || c->active){out->cause=CS_LIFE_STATE;return CS_LIFE_STATE;}
    if((p->tag!=0 && p->tag!=PLAN_TAG) || p->ready){out->cause=CS_LIFE_STATE;return CS_LIFE_STATE;}
    layout l;r=preflight(c,q,&l);if(r!=CS_LIFE_OK){out->cause=r;return r;}
    p->ready=0;p->owner=c;p->input=q;p->epoch=c->epoch;copy_request(&p->request,q);copy_scrap(&p->scrap,q->scrap);
    p->manager.magic=c->manager->magic;p->manager.map=c->manager->map;
    copy_zone(&p->manager.config.system,&c->manager->config.system);copy_zone(&p->manager.config.application,&c->manager->config.application);
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)copy_block(&p->manager.blocks[i],&c->manager->blocks[i]);
    p->map.regions=c->manager->map->regions;p->map.count=c->manager->map->count;
    for(size_t i=0;i<p->map.count;++i){const cs_guest_region *v=&p->map.regions[i];cs_guest_region *d=&p->regions[i];
        d->bytes=v->bytes;d->writable=v->writable;d->base=v->base;d->size=v->size;d->flags=v->flags;}
    for(size_t i=0;i<q->count;++i)copy_entry(&p->inventory[i],&q->inventory[i]);
    for(unsigned i=0;i<6;++i){p->globals[i]=read32(l.global[i].bytes);
        p->global_views[i].bytes=l.global[i].bytes;p->global_views[i].writable=l.global[i].writable;}
    for(unsigned z=0;z<2;++z){p->zones[z].bytes=l.zones[z].bytes;p->zones[z].writable=l.zones[z].writable;
        for(unsigned i=0;i<CS_MM_BLOCKS;++i)p->cells[z][i]=read32(l.zones[z].bytes+i*4u);}
    if(q->gateway){copy_config(&p->gateway_config,&q->gateway->config);p->gateway_magic=q->gateway->magic;
        for(unsigned i=0;i<CS_TRAP_ENTRIES;++i)p->entries[i]=q->gateway->entries[i];}
    p->limit=l.limit;p->slot=l.slot;p->address=l.address;p->extent=l.extent;p->app_scrap=l.app;
    p->retire_mask=0;p->write_units=36u;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(c->manager->blocks[i].kind && c->manager->blocks[i].zone==1){
        p->retire_mask|=UINT32_C(1)<<i;++p->write_units;}
    if(l.app)p->write_units+=(uint32_t)p->scrap.size+2u;
    p->source.bytes=l.source.bytes;p->source.writable=l.source.writable;
    out->phase=CS_LIFE_STAGING;
    for(uint32_t i=0;l.app && i<(uint32_t)q->scrap->size;++i)q->staging[i]=l.source.bytes[i];
    p->tag=PLAN_TAG;p->ready=1;c->active=p;c->state=CS_LIFE_PREPARED;
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    p->test_fault_after=c->test_fault_after;
#endif
    out->prepared=1;out->state=c->state;out->scrap_valid=1;
    out->old_handle=q->scrap->handle;out->size=(uint32_t)q->scrap->size;
    out->globals_valid=63;for(unsigned i=0;i<6;++i)out->globals[i]=p->globals[i];
    return CS_LIFE_OK;
}
static int current_plan(const cs_life_controller *c,const cs_life_plan *p)
{return c && p && c->tag==LIFE_TAG && c->state==CS_LIFE_PREPARED && c->active==p && p->tag==PLAN_TAG && p->ready && p->owner==c;}
static int output_safe(const cs_life_controller *c,const cs_life_plan *p,const cs_life_report *out)
{
    blob target={out,sizeof *out};if(!native_valid(target) || !c->manager || !c->manager->map ||
        !c->manager->map->regions || c->manager->map->count>CS_GUEST_MAX_REGIONS ||
        p->request.count>CS_LIFE_INVENTORY || p->map.count>CS_GUEST_MAX_REGIONS)return 0;
    for(unsigned i=0;i<12u+(unsigned)p->request.count;++i){
        if(i==3)continue;
        blob source=object_span(c,&p->request,p,out,i);
        if(!native_valid(source) || overlap(target,source))return 0;
    }
    for(size_t i=0;i<p->map.count;++i)
        if(overlap(target,(blob){p->regions[i].bytes,p->regions[i].size}))return 0;
    /* Even an invalid changed map must not turn a refusal report into a guest write. */
    for(size_t i=0;i<c->manager->map->count;++i){const cs_guest_region *v=&c->manager->map->regions[i];
        blob read={v->bytes,v->size};
        if(!native_valid(read) || overlap(target,read))return 0;
        if(v->flags&CS_GUEST_WRITE){blob write={v->writable,v->size};
            if(!native_valid(write) || overlap(target,write))return 0;}
    }
    return 1;
}
static int revalidate(cs_life_controller *c,const cs_life_plan *p)
{
    const cs_life_request *q=p->input;const cs_mm_manager *m=c->manager;
    if(!q || p->epoch!=c->epoch || !same_request(&p->request,q) || !same_scrap(&p->scrap,q->scrap) ||
        p->manager.magic!=m->magic || p->manager.map!=m->map ||
        !same_zone(&p->manager.config.system,&m->config.system) || !same_zone(&p->manager.config.application,&m->config.application) ||
        p->map.count!=m->map->count || p->map.regions!=m->map->regions)return 0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(!same_block(&p->manager.blocks[i],&m->blocks[i]))return 0;
    uint32_t mask=0,writes=36u;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(m->blocks[i].kind && m->blocks[i].zone==1){mask|=UINT32_C(1)<<i;++writes;}
    if(p->app_scrap)writes+=(uint32_t)p->scrap.size+2u;
    if(mask!=p->retire_mask || writes!=p->write_units)return 0;
    for(size_t i=0;i<p->map.count;++i){const cs_guest_region *a=&p->regions[i],*b=&m->map->regions[i];
        if(a->bytes!=b->bytes || a->writable!=b->writable || a->base!=b->base || a->size!=b->size || a->flags!=b->flags)return 0;}
    for(size_t i=0;i<q->count;++i)if(!same_entry(&p->inventory[i],&q->inventory[i]))return 0;
    if(q->gateway){if(p->gateway_magic!=q->gateway->magic || !same_config(&p->gateway_config,&q->gateway->config))return 0;
        for(unsigned i=0;i<CS_TRAP_ENTRIES;++i)if(p->entries[i]!=q->gateway->entries[i])return 0;}
    layout l;if(preflight(c,q,&l)!=CS_LIFE_OK || l.limit!=p->limit || l.slot!=p->slot ||
        l.address!=p->address || l.extent!=p->extent || l.app!=p->app_scrap ||
        l.source.bytes!=p->source.bytes || l.source.writable!=p->source.writable)return 0;
    for(unsigned i=0;i<6;++i)if(p->globals[i]!=read32(l.global[i].bytes) ||
        p->global_views[i].bytes!=l.global[i].bytes || p->global_views[i].writable!=l.global[i].writable)return 0;
    for(unsigned z=0;z<2;++z){if(p->zones[z].bytes!=l.zones[z].bytes || p->zones[z].writable!=l.zones[z].writable)return 0;
        for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(p->cells[z][i]!=read32(l.zones[z].bytes+i*4u))return 0;}
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    if(p->test_fault_after!=c->test_fault_after)return 0;
#endif
    return !l.app || !p->scrap.size || same_bytes(q->staging,l.source.bytes,(uint32_t)p->scrap.size);
}
static void invalidate(cs_life_controller *c,cs_life_plan *p)
{p->ready=0;c->active=0;c->state=CS_LIFE_STABLE;}
static int completed(cs_life_controller *c,cs_life_report *r)
{
    ++r->attempted;++r->completed;
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    if(c->test_fault_after && r->completed==c->test_fault_after)return 0;
#else
    (void)c;
#endif
    return 1;
}
static cs_life_result fault(cs_life_controller *c,const cs_life_plan *p,cs_life_report *r)
{
    c->state=CS_LIFE_FAULTED;r->state=c->state;r->cause=CS_LIFE_INVARIANT;
    r->scrap_valid=0;
    if(!r->new_handle_valid)r->new_handle=0;
    r->globals_valid=63;
    for(unsigned i=0;i<6;++i)r->globals[i]=read32(p->global_views[i].bytes);
    copy_report(&c->first_fault,r);return CS_LIFE_INVARIANT;
}
static int invariants(const cs_life_controller *c,const cs_life_plan *p)
{
    const cs_mm_manager *m=c->manager;
    if(c->epoch!=p->epoch+1u || read32(p->global_views[1].bytes)!=m->config.application.identity ||
        read32(p->global_views[2].bytes)!=p->limit)return 0;
    for(unsigned i=0;i<6;++i)if(i!=1 && i!=2 && read32(p->global_views[i].bytes)!=p->globals[i])return 0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i){const cs_mm_block *b=&m->blocks[i],*old=&p->manager.blocks[i];
        if(old->kind && old->zone==0){if(!same_block(b,old))return 0;}
        else if(p->app_scrap && i==p->slot){if(b->kind!=2 || b->zone!=1 || b->address!=p->address ||
            b->size!=(uint32_t)p->scrap.size || b->extent!=p->extent)return 0;}
        else if(b->kind || b->zone || b->address || b->size || b->extent)return 0;
        if(read32(p->zones[0].bytes+i*4u)!=p->cells[0][i])return 0;
        uint32_t expected=p->app_scrap && i==p->slot?CS_MM_LOCK|p->address:0;
        if(read32(p->zones[1].bytes+i*4u)!=expected)return 0;
    }
    if(p->app_scrap && p->scrap.size && !same_bytes(p->request.staging,
        p->zones[1].bytes+p->address-m->config.application.begin,(uint32_t)p->scrap.size))return 0;
    return 1;
}
cs_life_result cs_life_commit(cs_life_controller *c,cs_life_plan *p,cs_life_report *out)
{
    if(!c || c->tag!=LIFE_TAG || !p || !out)return CS_LIFE_ARGUMENT;
    if(c->state==CS_LIFE_FAULTED){
        if(p->tag!=PLAN_TAG || p->owner!=c || !output_safe(c,p,out))return CS_LIFE_ALIAS;
        copy_report(out,&c->first_fault);return CS_LIFE_FAULT;
    }
    if(!current_plan(c,p))return CS_LIFE_STATE;
    if(!output_safe(c,p,out))return CS_LIFE_ALIAS;
    /* Use sealed pointers for output safety; changed live request is checked later. */
    cs_life_result e=envelope(c,&p->request,p,out);
    if(e!=CS_LIFE_OK){
        report(out,c,p->request.operation);invalidate(c,p);out->cause=CS_LIFE_CHANGED;out->state=c->state;return CS_LIFE_CHANGED;}
    report(out,c,p->request.operation);
    if(!revalidate(c,p)){invalidate(c,p);out->cause=CS_LIFE_CHANGED;out->state=c->state;return CS_LIFE_CHANGED;}
    out->prepared=1;out->old_handle=p->scrap.handle;out->size=(uint32_t)p->scrap.size;
    p->ready=0;c->active=0;c->state=CS_LIFE_COMMITTING;
    out->phase=CS_LIFE_COMMIT;out->committed=1;++c->epoch;out->epoch_after=c->epoch;
    if(!completed(c,out))return fault(c,p,out);
    for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(p->retire_mask&(UINT32_C(1)<<i)){
        cs_mm_block *b=&c->manager->blocks[i];b->kind=0;b->zone=0;b->address=0;b->size=0;b->extent=0;
        if(!completed(c,out))return fault(c,p,out);}
    for(unsigned i=0;i<CS_MM_BLOCKS;++i){write32(p->zones[1].writable+i*4u,0);
        if(!completed(c,out))return fault(c,p,out);}
    write32(p->global_views[1].writable,c->manager->config.application.identity);
    if(!completed(c,out))return fault(c,p,out);
    write32(p->global_views[2].writable,p->limit);if(!completed(c,out))return fault(c,p,out);
    if(p->request.operation==CS_LIFE_RESET){c->state=CS_LIFE_POPULATING;out->phase=CS_LIFE_POPULATION;}
    else out->phase=CS_LIFE_SCRAP_PRESERVE;
    if(p->app_scrap){
        uint8_t *payload=p->zones[1].writable+p->address-c->manager->config.application.begin;
        for(uint32_t i=0;i<(uint32_t)p->scrap.size;++i){payload[i]=p->request.staging[i];
            if(!completed(c,out))return fault(c,p,out);}
        cs_mm_block *b=&c->manager->blocks[p->slot];b->kind=2;b->zone=1;b->address=p->address;
        b->size=(uint32_t)p->scrap.size;b->extent=p->extent;
        if(!completed(c,out))return fault(c,p,out);
        write32(p->zones[1].writable+p->slot*4u,CS_MM_LOCK|p->address);
        if(!completed(c,out))return fault(c,p,out);
    }
    if(!invariants(c,p))return fault(c,p,out);
    copy_scrap(p->request.scrap,&p->scrap);
    if(p->app_scrap)p->request.scrap->handle=c->manager->config.application.begin+p->slot*4u;
    out->new_handle=p->request.scrap->handle;out->new_handle_valid=1;
    if(!completed(c,out))return fault(c,p,out);
    if(out->completed!=p->write_units)return fault(c,p,out);
    c->state=CS_LIFE_STABLE;out->state=c->state;out->phase=CS_LIFE_DONE;
    out->scrap_valid=1;out->globals_valid=63;
    for(unsigned i=0;i<6;++i)out->globals[i]=read32(p->global_views[i].bytes);
    return CS_LIFE_OK;
}
cs_life_result cs_life_cancel(cs_life_controller *c,cs_life_plan *p,cs_life_report *out)
{
    if(!current_plan(c,p))return c && c->tag==LIFE_TAG && c->state==CS_LIFE_FAULTED?CS_LIFE_FAULT:CS_LIFE_STATE;
    cs_life_result e=envelope(c,p->input,p,out);if(e!=CS_LIFE_OK)return e;
    report(out,c,p->request.operation);invalidate(c,p);out->state=c->state;return CS_LIFE_OK;
}
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
cs_life_result cs_life_test_fault_after(cs_life_controller *c,uint32_t writes)
{if(cs_life_admit(c)!=CS_LIFE_OK)return CS_LIFE_STATE;c->test_fault_after=writes;return CS_LIFE_OK;}
#endif
