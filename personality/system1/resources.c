/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original bounded hosted ownership protocol. */
#include "resources.h"
#define RES_TAG UINT32_C(0x52533031)
typedef struct {const void *at;size_t bytes;} blob;
static int valid(blob b)
{return !b.bytes || (b.at && b.bytes<=UINTPTR_MAX-(uintptr_t)b.at);}
static int aligned(const void *p,size_t n,size_t a)
{return p && ((uintptr_t)p&(a-1u))==0 && valid((blob){p,n});}
#define OBJECT(p,t) aligned((p),sizeof(t),_Alignof(t))
static int overlap(blob a,blob b)
{uintptr_t x=(uintptr_t)a.at,y=(uintptr_t)b.at;return a.bytes && b.bytes && (x<=y?y-x<a.bytes:x-y<b.bytes);}
static void bytes_copy(void *a,const void *b,size_t n)
{uint8_t *d=a;const uint8_t *v=b;for(size_t i=0;i<n;++i)d[i]=v[i];}
static void clear(void *p,size_t n)
{uint8_t *v=p;for(size_t i=0;i<n;++i)v[i]=0;}
static uint32_t read32(const uint8_t *v)
{return ((uint32_t)v[0]<<24)|((uint32_t)v[1]<<16)|((uint32_t)v[2]<<8)|v[3];}
/* All mutable destinations remain untouched until native/guest alias checks.
 * Truthful pointer liveness is a host precondition, not pointer authentication. */
static cs_res_result envelope(const cs_res_service *s,cs_life_controller *life,
    const cs_res_profile *profile,const cs_res_request *q,const cs_res_report *out)
{
    if(!OBJECT(s,cs_res_service) || !OBJECT(life,cs_life_controller) ||
        !OBJECT(life->manager,cs_mm_manager))return CS_RES_ARGUMENT;
    const cs_mm_manager *m=life->manager;const cs_guest_map *map=m->map;
    if(!OBJECT(map,cs_guest_map) || !map->count || map->count>CS_GUEST_MAX_REGIONS ||
        !aligned(map->regions,map->count*sizeof *map->regions,_Alignof(cs_guest_region)))return CS_RES_ARGUMENT;
    if(profile && !OBJECT(profile,cs_res_profile))return CS_RES_ARGUMENT;
    if(q && (!OBJECT(q,cs_res_request) || !OBJECT(out,cs_res_report)))return CS_RES_ARGUMENT;
    blob objects[20];unsigned n=0,borrowed=20,incoming=20;
    objects[n++]=(blob){s,sizeof *s};objects[n++]=(blob){life,sizeof *life};
    objects[n++]=(blob){m,sizeof *m};objects[n++]=(blob){map,sizeof *map};
    objects[n++]=(blob){map->regions,map->count*sizeof *map->regions};
    if(s->tag){
        if(s->region_count>CS_GUEST_MAX_REGIONS)return CS_RES_ARGUMENT;
        if(s->manager!=m){if(!OBJECT(s->manager,cs_mm_manager))return CS_RES_ARGUMENT;
            objects[n++]=(blob){s->manager,sizeof *s->manager};}
        if(s->map!=map){if(!OBJECT(s->map,cs_guest_map))return CS_RES_ARGUMENT;
            objects[n++]=(blob){s->map,sizeof *s->map};}
        if(s->region_descriptors!=map->regions){
            if(!aligned(s->region_descriptors,s->region_count*sizeof *s->region_descriptors,_Alignof(cs_guest_region)))return CS_RES_ARGUMENT;
            objects[n++]=(blob){s->region_descriptors,s->region_count*sizeof *s->region_descriptors};}
    }
    if(life->active){if(!OBJECT(life->active,cs_life_plan))return CS_RES_ARGUMENT;
        objects[n++]=(blob){life->active,sizeof *life->active};}
    if(profile)objects[n++]=(blob){profile,sizeof *profile};
    if(s->source_borrow){borrowed=n;objects[n++]=(blob){s->source,s->source_length};}
    if(q){objects[n++]=(blob){q,sizeof *q};objects[n++]=(blob){out,sizeof *out};
        if(q->ticket){if(!OBJECT(q->ticket,cs_res_ticket))return CS_RES_ARGUMENT;
            objects[n++]=(blob){q->ticket,sizeof *q->ticket};}
        if(q->source || q->length){incoming=n;objects[n++]=(blob){q->source,q->length};}
        if(q->context.registration){if(!OBJECT(q->context.registration,cs_life_entry))return CS_RES_ARGUMENT;
            objects[n++]=(blob){q->context.registration,sizeof *q->context.registration};}
        const cs_m68k_session *session=q->context.session;
        if(session){if(!OBJECT(session,cs_m68k_session))return CS_RES_ARGUMENT;
            objects[n++]=(blob){session,sizeof *session};
            if(session->calls.gateway){if(!OBJECT(session->calls.gateway,cs_trap_gateway))return CS_RES_ARGUMENT;
                objects[n++]=(blob){session->calls.gateway,sizeof *session->calls.gateway};}
            if(session->current){if(!OBJECT(session->current,cs_m68k_session_report))return CS_RES_ARGUMENT;
                objects[n++]=(blob){session->current,sizeof *session->current};}}
    }
    for(unsigned i=0;i<n;++i){if(!valid(objects[i]))return CS_RES_ARGUMENT;
        /* Two read-only source arguments may designate the same bytes. A
         * live/FAULTED OPEN will still refuse before using the new argument. */
        for(unsigned j=0;j<i;++j)if(!(i==incoming && j==borrowed) && overlap(objects[i],objects[j]))return CS_RES_ARGUMENT;
        for(size_t k=0;k<map->count;++k){const cs_guest_region *r=&map->regions[k];
            if(!valid((blob){r->bytes,r->size}) || (r->writable && !valid((blob){r->writable,r->size})))return CS_RES_ARGUMENT;
            if(overlap(objects[i],(blob){r->bytes,r->size}) ||
                (r->writable && overlap(objects[i],(blob){r->writable,r->size})))return CS_RES_ARGUMENT;
        }
        for(size_t k=0;s->tag && k<s->region_count;++k){const cs_guest_region *r=&s->regions[k];
            if(!valid((blob){r->bytes,r->size}) || (r->writable && !valid((blob){r->writable,r->size})))return CS_RES_ARGUMENT;
            if(overlap(objects[i],(blob){r->bytes,r->size}) ||
                (r->writable && overlap(objects[i],(blob){r->writable,r->size})))return CS_RES_ARGUMENT;
        }
    }
    return CS_RES_OK;
}
static int same_zone(const cs_mm_zone *a,const cs_mm_zone *b)
{return a->identity==b->identity && a->begin==b->begin && a->end==b->end;}
static int same_ticket(const cs_res_ticket *a,const cs_res_ticket *b)
{return a->owner==b->owner && a->generation==b->generation && a->epoch==b->epoch &&
    a->identity==b->identity && a->slot==b->slot && a->type==b->type && a->id==b->id &&
    a->handle==b->handle && a->length==b->length;}
static int identity(const cs_res_service *s)
{return s->tag==RES_TAG && s->self==s && s->manager==s->lifecycle->manager && s->map==s->manager->map;}
static cs_res_result topology(const cs_res_service *s)
{
    const cs_guest_map *m=s->map;
    if(s->region_count!=m->count || s->region_descriptors!=m->regions || !same_zone(&s->config.system,&s->manager->config.system) ||
        !same_zone(&s->config.application,&s->manager->config.application))return CS_RES_CONFLICT;
    for(size_t i=0;i<m->count;++i){const cs_guest_region *a=&m->regions[i],*b=&s->regions[i];
        if(a->bytes!=b->bytes || a->writable!=b->writable || a->base!=b->base || a->size!=b->size || a->flags!=b->flags)
            return CS_RES_CONFLICT;}
    return CS_RES_OK;
}
static int cached_valid(const cs_res_service *s,const cs_res_cached *c)
{
    if(c->manager_slot>=CS_MM_BLOCKS || c->handle!=s->config.application.begin+c->manager_slot*4u ||
        c->length>=UINT32_C(0x80000000) || c->extent!=(c->length?((c->length+1u)&~1u):2u))return 0;
    const cs_mm_block *b=&s->manager->blocks[c->manager_slot];cs_guest_view cell,data;
    return b->kind==2 && b->zone==1 && b->address==c->address && b->size==c->length && b->extent==c->extent &&
        cs_guest_window(s->map,c->handle,4,CS_GUEST_READ,&cell)==CS_GUEST_OK &&
        read32(cell.bytes)==(CS_MM_LOCK|c->address) &&
        cs_guest_window(s->map,c->address,c->extent,CS_GUEST_READ|CS_GUEST_WRITE,&data)==CS_GUEST_OK;
}
static cs_res_result ownership(const cs_res_service *s)
{
    unsigned counts[CS_RES_CACHE];for(unsigned i=0;i<CS_RES_CACHE;++i)counts[i]=0;
    for(unsigned i=0;i<CS_RES_TICKETS;++i){const cs_res_lease *l=&s->leases[i];if(!l->live)continue;
        const cs_res_ticket *t=&l->ticket;
        if(l->live!=1 || t->owner!=s || t->generation!=s->generation || t->epoch!=s->epoch ||
            !t->identity || t->identity>s->last_ticket_id || t->slot>=CS_RES_CACHE)return CS_RES_CONFLICT;
        const cs_res_cached *c=&s->cache[t->slot];
        if(!c->live || t->type!=c->entry.type || t->id!=c->entry.id || t->handle!=c->handle || t->length!=c->length)
            return CS_RES_CONFLICT;
        for(unsigned j=0;j<i;++j)if(s->leases[j].live && s->leases[j].ticket.identity==t->identity)return CS_RES_CONFLICT;
        ++counts[t->slot];
    }
    for(unsigned i=0;i<CS_RES_CACHE;++i){const cs_res_cached *c=&s->cache[i];
        if(c->live!=0 && c->live!=1)return CS_RES_CONFLICT;
        if(c->leases!=counts[i] || (c->live && !cached_valid(s,c)))return CS_RES_CONFLICT;
        if(s->state==CS_RES_CLOSED && (c->live || counts[i]))return CS_RES_CONFLICT;
        for(unsigned j=0;c->live && j<i;++j)if(s->cache[j].live &&
            (s->cache[j].handle==c->handle || (s->cache[j].entry.type==c->entry.type && s->cache[j].entry.id==c->entry.id)))return CS_RES_CONFLICT;
    }
    return CS_RES_OK;
}
static cs_res_result admission(const cs_res_service *s,const cs_res_context *c)
{
    if(c->context!=CS_LIFE_ORDINARY || c->active_root || c->active_callback || c->active_patch || c->active_manager)
        return CS_RES_CONTEXT;
    if(c->session && (c->session->tag!=UINT32_C(0x53455331) || c->session->active || c->session->stepping ||
        c->session->exit_pending || c->session->current || c->session->calls.depth || c->session->calls.map!=s->map))return CS_RES_CONTEXT;
    if(s->lifecycle->state!=CS_LIFE_STABLE || s->lifecycle->active || !s->lifecycle->epoch)return CS_RES_CONTEXT;
    if(cs_life_admit(s->lifecycle)!=CS_LIFE_OK)return CS_RES_CONFLICT;
    if(s->state==CS_RES_OPENED && s->epoch!=s->lifecycle->epoch)return CS_RES_CONFLICT;
    cs_res_result r=topology(s);if(r!=CS_RES_OK)return r;
    cs_guest_view zone;
    if(cs_guest_window(s->map,0x118,4,CS_GUEST_READ,&zone)!=CS_GUEST_OK ||
        read32(zone.bytes)!=s->config.application.identity)return CS_RES_CONFLICT;
    return ownership(s);
}
static int registered(const cs_res_service *s,const cs_life_entry *e)
{return e && e->kind==CS_LIFE_FUTURE && e->action==CS_LIFE_REFUSE && e->owner==s->profile.owner &&
    e->native==s && e->bytes==sizeof *s && e->address==0 && e->value==0;}
static void initialize_report(cs_res_report *r,const cs_res_service *s,cs_res_operation op)
{clear(r,sizeof *r);r->cause=CS_RES_OK;r->operation=op;r->phase=CS_RES_PREFLIGHT;r->state=s->state;
    r->generation=s->generation;r->epoch=s->epoch;
    r->generation_valid=s->source_borrow && s->generation!=0;r->epoch_valid=s->source_borrow && s->epoch!=0;
    r->heap_epoch=s->lifecycle->epoch;r->heap_epoch_valid=s->lifecycle->epoch!=0;}
static cs_res_result refuse(cs_res_report *r,cs_res_result cause)
{r->cause=cause;return cause;}
static cs_res_result fault(cs_res_service *s,cs_res_report *r)
{s->state=CS_RES_FAULTED;r->state=CS_RES_FAULTED;r->cause=CS_RES_FAULT;
    bytes_copy(&s->first_fault,r,sizeof *r);return CS_RES_FAULT;}
static int unit(cs_res_service *s,cs_res_report *r,int guest,int native)
{
    ++r->completed;if(guest)r->guest_committed=1;if(native)r->native_committed=1;
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
    if(s->test_after && r->completed==s->test_after){(void)fault(s,r);return 0;}
#else
    (void)s;
#endif
    return 1;
}
static cs_res_result call(cs_res_service *s,cs_res_report *r,cs_mm_operation op,uint32_t address,uint32_t size,unsigned index)
{
    cs_mm_request q;cs_mm_reply reply;cs_mm_result result=CS_MM_STATE;
    q.operation=op;q.address=address;q.other=0;q.size=size;q.flags=0;q.interrupt_context=0;
    r->manager_valid=0;r->reply_valid=0;r->lifecycle_valid=0;
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
    unsigned kind=op==CS_MM_NEW_HANDLE?1u:op==CS_MM_HLOCK?2u:3u;
    if(s->test_call==kind && s->test_call_index==index)return fault(s,r);
#else
    (void)index;
#endif
    cs_life_result life=cs_life_manager_call(s->lifecycle,&q,&reply,&result);
    r->lifecycle_valid=1;r->lifecycle_result=life;
    if(life!=CS_LIFE_OK)return fault(s,r);
    r->manager_valid=1;r->manager_result=result;
    if(result!=CS_MM_OK)return fault(s,r);
    r->reply_valid=1;r->reply.code=reply.code;r->reply.value=reply.value;r->reply.moved=reply.moved;
    if(op==CS_MM_NEW_HANDLE && reply.code==CS_MM_FULL)return CS_RES_MANAGER_CAPACITY;
    if(reply.code!=CS_MM_NO_ERROR)return fault(s,r);
    return CS_RES_OK;
}
cs_res_result cs_res_init(cs_res_service *s,cs_life_controller *life,const cs_res_profile *p)
{
    if(!p)return CS_RES_ARGUMENT;
    cs_res_result r=envelope(s,life,p,0,0);if(r!=CS_RES_OK)return r;
    if(s->tag)return CS_RES_STATE;
    if(!p->maximum_source || !p->maximum_resource || !p->owner)return CS_RES_ARGUMENT;
    if(cs_life_admit(life)!=CS_LIFE_OK)return CS_RES_STATE;
    clear(s,sizeof *s);s->self=s;s->lifecycle=life;s->manager=life->manager;s->map=life->manager->map;
    bytes_copy(&s->profile,p,sizeof *p);bytes_copy(&s->config,&s->manager->config,sizeof s->config);
    s->region_count=s->map->count;s->region_descriptors=s->map->regions;
    for(size_t i=0;i<s->region_count;++i)bytes_copy(&s->regions[i],&s->map->regions[i],sizeof s->regions[i]);
    s->state=CS_RES_CLOSED;s->tag=RES_TAG;return CS_RES_OK;
}
static unsigned cache_slot(const cs_res_service *s,uint32_t type,int32_t id)
{for(unsigned i=0;i<CS_RES_CACHE;++i)if(s->cache[i].live && s->cache[i].entry.type==type && s->cache[i].entry.id==id)return i;return CS_RES_CACHE;}
static unsigned lease_slot(const cs_res_service *s,const cs_res_ticket *t)
{if(t)for(unsigned i=0;i<CS_RES_TICKETS;++i)if(s->leases[i].live && same_ticket(t,&s->leases[i].ticket))return i;return CS_RES_TICKETS;}
static void resident_report(cs_res_report *r,const cs_res_cached *c)
{r->entry_valid=1;bytes_copy(&r->entry,&c->entry,sizeof r->entry);
    r->reader_valid=1;bytes_copy(&r->reader,&c->reader,sizeof r->reader);
    r->cache_valid=1;r->handle_valid=1;r->handle=c->handle;r->address=c->address;r->length=c->length;}
static int publish_ticket(cs_res_service *s,cs_res_report *r,unsigned ci,unsigned ti)
{
    cs_res_cached *c=&s->cache[ci];cs_res_lease *l=&s->leases[ti];cs_res_ticket *t=&l->ticket;
    t->owner=s;t->generation=s->generation;t->epoch=s->epoch;t->identity=s->last_ticket_id+1u;
    t->slot=ci;t->type=c->entry.type;t->id=c->entry.id;t->handle=c->handle;t->length=c->length;
    l->live=1;++c->leases;s->last_ticket_id=t->identity;r->ticket_valid=1;
    bytes_copy(&r->ticket,t,sizeof r->ticket);r->phase=CS_RES_TICKET_PUBLICATION;
    return unit(s,r,0,1);
}
static cs_res_result acquire(cs_res_service *s,const cs_res_request *q,cs_res_report *r)
{
    cs_rf_entry entry;cs_rf_report rr;clear(&rr,sizeof rr);r->phase=CS_RES_LOOKUP;
    cs_rf_result status=cs_rf_find(&s->view,q->type,q->id,&entry,&rr);
    r->reader_valid=status!=CS_RF_ARGUMENT;bytes_copy(&r->reader,&rr,sizeof rr);
    if(status==CS_RF_NOT_FOUND)return refuse(r,CS_RES_NOT_FOUND);
    if(status==CS_RF_DUPLICATE_TYPE || status==CS_RF_DUPLICATE_ID)return refuse(r,CS_RES_AMBIGUOUS);
    if(status!=CS_RF_OK && status!=CS_RF_REFERENCE)return refuse(r,CS_RES_SOURCE);
    r->entry_valid=1;bytes_copy(&r->entry,&entry,sizeof entry);
    if(status==CS_RF_REFERENCE)return refuse(r,CS_RES_UNRESOLVED_REFERENCE);
    if(entry.attributes&0x40u)return refuse(r,CS_RES_SYSTEM_HEAP_UNSUPPORTED);
    if(entry.data.length>s->profile.maximum_resource || entry.data.length>=UINT32_C(0x80000000))return refuse(r,CS_RES_PROFILE_LIMIT);
    unsigned ci=cache_slot(s,q->type,q->id),ti=0;
    if(s->last_ticket_id==UINT32_MAX)return refuse(r,CS_RES_COUNTER_EXHAUSTED);
    while(ti<CS_RES_TICKETS && s->leases[ti].live)++ti;
    if(ti==CS_RES_TICKETS)return refuse(r,CS_RES_TICKETS_FULL);
    if(ci<CS_RES_CACHE){resident_report(r,&s->cache[ci]);s->state=CS_RES_BUSY;
        return publish_ticket(s,r,ci,ti)?CS_RES_OK:CS_RES_FAULT;}
    ci=0;while(ci<CS_RES_CACHE && s->cache[ci].live)++ci;
    if(ci==CS_RES_CACHE)return refuse(r,CS_RES_CACHE_FULL);
    s->state=CS_RES_BUSY;r->phase=CS_RES_ALLOCATION;
    cs_res_result result=call(s,r,CS_MM_NEW_HANDLE,0,entry.data.length,1);
    if(result==CS_RES_MANAGER_CAPACITY){s->state=CS_RES_OPENED;r->state=s->state;return refuse(r,result);}
    if(result!=CS_RES_OK)return result;
    r->allocated=1;r->moved=r->reply.moved;r->handle_valid=1;r->handle=r->reply.value;
    if(!unit(s,r,1,0))return CS_RES_FAULT;
    uint32_t handle=r->handle,begin=s->config.application.begin;
    if(handle<begin || handle-begin>=CS_MM_CELL_BYTES || ((handle-begin)&3u))return fault(s,r);
    unsigned slot=(handle-begin)/4u;const cs_mm_block *b=&s->manager->blocks[slot];cs_guest_view cell,data;
    uint32_t extent=entry.data.length?((entry.data.length+1u)&~1u):2u;
    if(b->kind!=2 || b->zone!=1 || b->size!=entry.data.length || b->extent!=extent ||
        cs_guest_window(s->map,handle,4,CS_GUEST_READ,&cell)!=CS_GUEST_OK || read32(cell.bytes)!=b->address ||
        cs_guest_window(s->map,b->address,extent,CS_GUEST_READ|CS_GUEST_WRITE,&data)!=CS_GUEST_OK)return fault(s,r);
    r->address=b->address;r->length=b->size;r->phase=CS_RES_LOCKING;
    cs_res_cached expected;clear(&expected,sizeof expected);expected.manager_slot=slot;
    expected.handle=handle;expected.address=b->address;expected.length=b->size;expected.extent=extent;
    result=call(s,r,CS_MM_HLOCK,handle,0,1);if(result!=CS_RES_OK)return result;
    if(!unit(s,r,1,0))return CS_RES_FAULT;
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
    if(s->test_call==4 && s->test_call_index==1)s->manager->blocks[slot].address+=2u;
    if(s->test_call==5 && s->test_call_index==1)s->manager->blocks[slot].extent+=2u;
#endif
    if(!cached_valid(s,&expected) ||
        cs_guest_window(s->map,expected.address,expected.extent,CS_GUEST_READ|CS_GUEST_WRITE,&data)!=CS_GUEST_OK)return fault(s,r);
    r->phase=CS_RES_COPYING;
    for(uint32_t i=0;i<entry.data.length;++i){data.writable[i]=s->view.bytes[entry.data.offset+i];
        if(!unit(s,r,1,0))return CS_RES_FAULT;}
    if(!cached_valid(s,&expected))return fault(s,r);
    cs_res_cached *c=&s->cache[ci];c->live=1;c->leases=0;c->manager_slot=slot;c->handle=handle;
    c->address=b->address;c->length=b->size;c->extent=b->extent;bytes_copy(&c->entry,&entry,sizeof entry);
    bytes_copy(&c->reader,&rr,sizeof rr);
    r->phase=CS_RES_CACHE_PUBLICATION;resident_report(r,c);
    if(!unit(s,r,0,1))return CS_RES_FAULT;
    return publish_ticket(s,r,ci,ti)?CS_RES_OK:CS_RES_FAULT;
}
static cs_res_result dispose(cs_res_service *s,cs_res_report *r,unsigned slot,unsigned index)
{
    cs_res_cached *c=&s->cache[slot];r->phase=CS_RES_DISPOSAL;
    resident_report(r,c);cs_res_result result=call(s,r,CS_MM_DISPOS_HANDLE,c->handle,0,index);
    if(result!=CS_RES_OK)return result;
    r->disposed_mask=(uint16_t)(r->disposed_mask|(uint16_t)(1u<<slot));r->cache_valid=0;r->handle_valid=0;
    if(!unit(s,r,1,0))return CS_RES_FAULT;
    c->live=0;c->leases=0;r->phase=CS_RES_RETIREMENT;
    r->retired_mask=(uint16_t)(r->retired_mask|(uint16_t)(1u<<slot));
    return unit(s,r,0,1)?CS_RES_OK:CS_RES_FAULT;
}
cs_res_result cs_res_execute(cs_res_service *s,const cs_res_request *q,cs_res_report *r)
{
    if(!OBJECT(s,cs_res_service))return CS_RES_ARGUMENT;
    cs_res_result result=envelope(s,s->lifecycle,0,q,r);if(result!=CS_RES_OK)return result;
    if(!q || !r)return CS_RES_ARGUMENT;
    if(!identity(s)){initialize_report(r,s,q->operation);return refuse(r,CS_RES_STATE);}
    initialize_report(r,s,q->operation);
    if(s->state==CS_RES_FAULTED){r->retained_fault=1;return refuse(r,CS_RES_FAULT);}
    if(q->operation<CS_RES_OPEN || q->operation>CS_RES_CLOSE || s->state==CS_RES_BUSY ||
        (s->state!=CS_RES_CLOSED && s->state!=CS_RES_OPENED))return refuse(r,CS_RES_STATE);
    if((q->operation==CS_RES_OPEN && s->state!=CS_RES_CLOSED) ||
        (q->operation!=CS_RES_OPEN && q->operation!=CS_RES_CLOSE && s->state!=CS_RES_OPENED))return refuse(r,CS_RES_STATE);
    result=admission(s,&q->context);if(result!=CS_RES_OK)return refuse(r,result);
    if((s->state==CS_RES_OPENED || q->operation==CS_RES_OPEN) && !registered(s,q->context.registration))return refuse(r,CS_RES_CONFLICT);
    if(!s->profile.maximum_resource || !s->profile.maximum_source || !s->profile.owner)return refuse(r,CS_RES_CONFLICT);
    if(q->operation!=CS_RES_OPEN && (q->source || q->length))return refuse(r,CS_RES_ARGUMENT);
    if((q->operation==CS_RES_ACQUIRE || q->operation==CS_RES_EVICT) && (q->id<INT16_MIN || q->id>INT16_MAX))return refuse(r,CS_RES_ARGUMENT);
    if(q->operation==CS_RES_OPEN){
        if(!q->source || !q->length)return refuse(r,CS_RES_ARGUMENT);
        if(q->length>UINT32_MAX || q->length>s->profile.maximum_source)return refuse(r,CS_RES_PROFILE_LIMIT);
        cs_rf_view view;cs_rf_report rr;clear(&rr,sizeof rr);cs_rf_result status=cs_rf_open(q->source,q->length,&view,&rr);
        r->reader_valid=status!=CS_RF_ARGUMENT;bytes_copy(&r->reader,&rr,sizeof rr);
        if(status!=CS_RF_OK)return refuse(r,CS_RES_SOURCE);
        if(s->last_open_generation==UINT32_MAX)return refuse(r,CS_RES_COUNTER_EXHAUSTED);
        s->state=CS_RES_BUSY;s->source=q->source;s->source_length=q->length;s->source_borrow=1;
        bytes_copy(&s->view,&view,sizeof view);s->generation=++s->last_open_generation;s->epoch=s->lifecycle->epoch;
        r->generation=s->generation;r->generation_valid=1;r->epoch=s->epoch;r->epoch_valid=1;r->phase=CS_RES_OWNERSHIP;
        if(!unit(s,r,0,1))return CS_RES_FAULT;
        s->state=CS_RES_OPENED;
    }else if(q->operation==CS_RES_ACQUIRE){result=acquire(s,q,r);if(result!=CS_RES_OK)return result;s->state=CS_RES_OPENED;
    }else if(q->operation==CS_RES_DESCRIBE || q->operation==CS_RES_RELEASE){
        unsigned ti=lease_slot(s,q->ticket);if(ti==CS_RES_TICKETS)return refuse(r,CS_RES_INVALID_TICKET);
        cs_res_lease *l=&s->leases[ti];cs_res_cached *c=&s->cache[l->ticket.slot];resident_report(r,c);
        if(q->operation==CS_RES_RELEASE){s->state=CS_RES_BUSY;l->live=0;--c->leases;r->phase=CS_RES_RETIREMENT;
            if(!unit(s,r,0,1))return CS_RES_FAULT;
            s->state=CS_RES_OPENED;
        }else{r->ticket_valid=1;bytes_copy(&r->ticket,&l->ticket,sizeof r->ticket);r->empty=c->length==0;
            if(c->length){cs_guest_view data;
                if(cs_guest_window(s->map,c->address,c->length,CS_GUEST_READ,&data)!=CS_GUEST_OK)return fault(s,r);
                r->span.bytes=data.bytes;r->span.writable=0;}
            r->span_valid=1;
        }
    }else if(q->operation==CS_RES_EVICT){
        unsigned ci=cache_slot(s,q->type,q->id);if(ci==CS_RES_CACHE)return refuse(r,CS_RES_NOT_RESIDENT);
        if(s->cache[ci].leases)return refuse(r,CS_RES_IN_USE);
        s->state=CS_RES_BUSY;result=dispose(s,r,ci,1);if(result!=CS_RES_OK)return result;s->state=CS_RES_OPENED;
    }else if(s->state==CS_RES_OPENED){
        for(unsigned i=0;i<CS_RES_TICKETS;++i)if(s->leases[i].live)return refuse(r,CS_RES_IN_USE);
        s->state=CS_RES_BUSY;unsigned index=0;
        for(unsigned i=0;i<CS_RES_CACHE;++i)if(s->cache[i].live){result=dispose(s,r,i,++index);if(result!=CS_RES_OK)return result;}
        r->phase=CS_RES_OWNERSHIP;
        /* Keep forensic source identity until success return; post-unit fault
         * still owns it. Old CLOSED numbers/views are explicitly non-live. */
        s->state=CS_RES_CLOSED;
        if(!unit(s,r,0,1))return CS_RES_FAULT;
        s->source_borrow=0;
    }
    r->state=s->state;r->phase=CS_RES_DONE;
    if(s->state==CS_RES_CLOSED){r->generation_valid=0;r->epoch_valid=0;}
    return CS_RES_OK;
}
cs_res_result cs_res_external_admit(const cs_res_service *s)
{
    if(!OBJECT(s,cs_res_service) || !OBJECT(s->lifecycle,cs_life_controller) ||
        !OBJECT(s->manager,cs_mm_manager) || !identity(s))return CS_RES_STATE;
    if(s->state==CS_RES_FAULTED)return CS_RES_FAULT;
    if(s->state!=CS_RES_CLOSED || s->source_borrow)return CS_RES_STATE;
    if(cs_life_admit(s->lifecycle)!=CS_LIFE_OK || topology(s)!=CS_RES_OK || ownership(s)!=CS_RES_OK)return CS_RES_CONFLICT;
    return CS_RES_OK;
}
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
cs_res_result cs_res_test_fault(cs_res_service *s,uint32_t after,uint32_t call_kind,uint32_t index)
{
    if(!OBJECT(s,cs_res_service) || !identity(s) || s->state==CS_RES_BUSY || s->state==CS_RES_FAULTED || call_kind>5 ||
        (call_kind && !index) || (call_kind>=4 && index!=1))return CS_RES_STATE;
    s->test_after=after;s->test_call=call_kind;s->test_call_index=index;return CS_RES_OK;
}
#endif
