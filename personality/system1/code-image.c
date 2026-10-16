/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0033 data-only owner. */
#include "code-image.h"
#include "code-run-internal.h"
#define TAG UINT32_C(0x494D4731)
typedef struct {const void *p;size_t n;} span;
typedef struct {span spans[10];unsigned count;const void *owner;} envelope;
static void zero(void *p,size_t n){uint8_t *b=p;for(size_t i=0;i<n;++i)b[i]=0;}
static void copy(void *p,const void *q,size_t n){uint8_t *a=p;const uint8_t *b=q;for(size_t i=0;i<n;++i)a[i]=b[i];}
static int equal(const void *p,const void *q,size_t n){const uint8_t *a=p,*b=q;for(size_t i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static int valid(span a){return !a.n || (a.p && a.n<=UINTPTR_MAX-(uintptr_t)a.p);}
static int aligned(const void *p,size_t n,size_t a){return p && !((uintptr_t)p&(a-1u)) && valid((span){p,n});}
#define OBJ(p,t) aligned((p),sizeof(t),_Alignof(t))
static int overlap(span a,span b){uintptr_t x=(uintptr_t)a.p,y=(uintptr_t)b.p;return a.n && b.n && (x<=y?y-x<a.n:x-y<b.n);}
static int intersects(uint32_t a,uint32_t n,uint32_t b,uint32_t m){return n && m && (a<=b?b-a<n:a-b<m);}
static uint32_t u16(const uint8_t *p){return ((uint32_t)p[0]<<8)|p[1];}
static uint32_t u32(const uint8_t *p){return (u16(p)<<16)|u16(p+2);}
static uint16_t bit(uint32_t i){return (uint16_t)(UINT32_C(1)<<i);}
static int protect(const envelope *e,span a){if(!valid(a))return 0;for(unsigned i=0;i<e->count;++i)if(overlap(a,e->spans[i]))return 0;return 1;}
static int add_span(envelope *e,span a){if(!protect(e,a))return 0;e->spans[e->count++]=a;return 1;}
static int map_safe(const envelope *e,const cs_guest_map *m)
{
    if(!OBJ(m,cs_guest_map) || !protect(e,(span){m,sizeof *m}) || !m->count || m->count>CS_GUEST_MAX_REGIONS ||
        !aligned(m->regions,m->count*sizeof *m->regions,_Alignof(cs_guest_region)) ||
        !protect(e,(span){m->regions,m->count*sizeof *m->regions}))return 0;
    for(size_t i=0;i<m->count;++i){const cs_guest_region *g=&m->regions[i];
        if(!protect(e,(span){g->bytes,g->size}) || !protect(e,(span){g->writable,g->writable?g->size:0}))return 0;}
    return 1;
}
static int session_safe(const envelope *e,const cs_m68k_session *s)
{
    if(!s)return 1;
    if(!OBJ(s,cs_m68k_session) || !protect(e,(span){s,sizeof *s}) || !map_safe(e,s->calls.map))return 0;
    if(s->calls.gateway && (!OBJ(s->calls.gateway,cs_trap_gateway) || !protect(e,(span){s->calls.gateway,sizeof *s->calls.gateway})))return 0;
    return !s->current || (OBJ(s->current,cs_m68k_session_report) && protect(e,(span){s->current,sizeof *s->current}));
}
static int inventory_safe(const envelope *e,const cs_life_entry *v,size_t n)
{
    if(n>CS_LIFE_INVENTORY || (n && !aligned(v,n*sizeof *v,_Alignof(cs_life_entry))))return 0;
    for(size_t i=0;i<n;++i)if(v[i].kind==CS_LIFE_NATIVE || v[i].kind==CS_LIFE_FUTURE){
        if(v[i].native==e->owner && v[i].bytes==sizeof(cs_image_service))continue;
        if(!protect(e,(span){v[i].native,v[i].bytes}))return 0;}
    return 1;
}
static int life_safe(const envelope *e,const cs_life_controller *l)
{
    if(!OBJ(l,cs_life_controller) || !protect(e,(span){l,sizeof *l}) || !OBJ(l->manager,cs_mm_manager) ||
        !protect(e,(span){l->manager,sizeof *l->manager}) || !map_safe(e,l->manager->map))return 0;
    if(l->active){const cs_life_plan *p=l->active;
        if(!OBJ(p,cs_life_plan) || !protect(e,(span){p,sizeof *p}) || !OBJ(p->input,cs_life_request) ||
            !protect(e,(span){p->input,sizeof *p->input}) || !OBJ(p->request.scrap,cs_life_scrap) ||
            !protect(e,(span){p->request.scrap,sizeof *p->request.scrap}) || p->request.count>CS_LIFE_INVENTORY ||
            !protect(e,(span){p->request.inventory,p->request.count*sizeof *p->request.inventory}) ||
            !inventory_safe(e,p->request.inventory,p->request.count) ||
            !protect(e,(span){p->request.staging,p->request.staging_bytes}) || !session_safe(e,p->request.session))return 0;
        if(p->request.gateway && (!OBJ(p->request.gateway,cs_trap_gateway) || !protect(e,(span){p->request.gateway,sizeof *p->request.gateway})))return 0;
    }
    return 1;
}
static int resource_safe(const envelope *e,const cs_res_service *r,const cs_image_service *s)
{
    if(!OBJ(r,cs_res_service) || !protect(e,(span){r,sizeof *r}) || r->lifecycle!=s->lifecycle ||
        r->manager!=s->manager || r->map!=s->map || r->region_count>CS_GUEST_MAX_REGIONS ||
        !aligned(r->region_descriptors,r->region_count*sizeof *r->region_descriptors,_Alignof(cs_guest_region)) ||
        !protect(e,(span){r->region_descriptors,r->region_count*sizeof *r->region_descriptors}))return 0;
    for(size_t i=0;i<r->region_count;++i)if(!protect(e,(span){r->regions[i].bytes,r->regions[i].size}) ||
        !protect(e,(span){r->regions[i].writable,r->regions[i].writable?r->regions[i].size:0}))return 0;
    return !r->source_borrow || protect(e,(span){r->source,r->source_length});
}
/* Protect all writable outputs and synchronous inputs before writing even a
 * diagnostic. Duplicate protected views are allowed; output/input aliases are
 * not. Retained source pointers in CLOSED caches are deliberately not read. */
static int native_safe(cs_image_service *s,const cs_image_request *q,cs_image_workspace *w,size_t capacity,
    cs_image_description *out,cs_image_report *r)
{
    if(!OBJ(s,cs_image_service) || !OBJ(q,cs_image_request) || !OBJ(w,cs_image_workspace) ||
        capacity<sizeof *w || !OBJ(out,cs_image_description) || !OBJ(r,cs_image_report) ||
        q->context.count>CS_LIFE_INVENTORY || q->context.resource_count>CS_IMAGE_RESOURCES)return 0;
    envelope e;e.count=0;e.owner=s;
    if(!add_span(&e,(span){q,sizeof *q}) || !add_span(&e,(span){w,capacity}) || !add_span(&e,(span){out,sizeof *out}) ||
        !add_span(&e,(span){r,sizeof *r}) || !add_span(&e,(span){q->source,q->length}) ||
        !add_span(&e,(span){q->staging,q->staging_capacity}))return 0;
    if(q->context.count && !aligned(q->context.inventory,q->context.count*sizeof *q->context.inventory,_Alignof(cs_life_entry)))return 0;
    if(q->context.resource_count && !aligned(q->context.resources,q->context.resource_count*sizeof *q->context.resources,_Alignof(cs_res_service *)))return 0;
    if(!add_span(&e,(span){q->context.inventory,q->context.count*sizeof *q->context.inventory}) ||
        !add_span(&e,(span){q->context.resources,q->context.resource_count*sizeof *q->context.resources}) ||
        !add_span(&e,(span){s,sizeof *s}))return 0;
    if(!life_safe(&e,s->lifecycle) || !OBJ(s->manager,cs_mm_manager) || !protect(&e,(span){s->manager,sizeof *s->manager}) ||
        !map_safe(&e,s->manager->map) || !map_safe(&e,s->map) || !session_safe(&e,q->context.session) ||
        !inventory_safe(&e,q->context.inventory,q->context.count))return 0;
    if(s->region_count>CS_GUEST_MAX_REGIONS || s->inventory_count>CS_LIFE_INVENTORY || s->resource_count>CS_IMAGE_RESOURCES)return 0;
    if(!aligned(s->descriptors,s->region_count*sizeof *s->descriptors,_Alignof(cs_guest_region)) ||
        !protect(&e,(span){s->descriptors,s->region_count*sizeof *s->descriptors}))return 0;
    for(size_t i=0;i<s->region_count;++i)if(!protect(&e,(span){s->regions[i].bytes,s->regions[i].size}) ||
        !protect(&e,(span){s->regions[i].writable,s->regions[i].writable?s->regions[i].size:0}))return 0;
    if(s->staging_borrow && !protect(&e,(span){s->staging,s->staging_capacity}))return 0;
    /* The seal's own FUTURE entry intentionally denotes s, a protected object. */
    if(!inventory_safe(&e,s->inventory,s->inventory_count) || !session_safe(&e,s->session))return 0;
    for(size_t i=0;i<q->context.resource_count;++i)if(!resource_safe(&e,q->context.resources[i],s))return 0;
    for(size_t i=0;i<s->resource_count;++i)if(!resource_safe(&e,s->resources[i],s))return 0;
    return 1;
}
static int identity(const cs_image_service *s){return s->tag==TAG && s->self==s;}
static int inactive(const cs_image_service *s,const cs_image_context *c)
{
    if(c->context!=CS_LIFE_ORDINARY || c->active_root || c->active_callback || c->active_patch || c->active_manager)return 0;
    const cs_m68k_session *v=c->session;
    return !v || (v->tag==UINT32_C(0x53455331) && !v->active && !v->stepping && !v->exit_pending &&
        !v->current && !v->calls.depth && v->calls.map==s->map);
}
static const uint32_t globals[6]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
static int views(const cs_image_service *s,uint32_t *g,uint32_t c[2][CS_MM_BLOCKS])
{
    for(unsigned i=0;i<6;++i)if(cs_guest_read(s->map,globals[i],4,CS_GUEST_READ,&g[i])!=CS_GUEST_OK)return 0;
    for(unsigned z=0;z<2;++z){uint32_t base=z?s->config.application.begin:s->config.system.begin;
        for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(cs_guest_read(s->map,base+i*4u,4,CS_GUEST_READ,&c[z][i])!=CS_GUEST_OK)return 0;}
    return 1;
}
static int binding(const cs_image_service *s)
{
    if(s->lifecycle->manager!=s->manager || s->manager->map!=s->map || s->map->regions!=s->descriptors ||
        s->map->count!=s->region_count || !equal(&s->config,&s->manager->config,sizeof s->config) || cs_guest_validate(s->map)!=CS_GUEST_OK)return 0;
    for(size_t i=0;i<s->region_count;++i){const cs_guest_region *a=&s->regions[i],*b=&s->map->regions[i];
        if(a->base!=b->base || a->size!=b->size || a->flags!=b->flags || a->bytes!=b->bytes || a->writable!=b->writable)return 0;}
    return 1;
}
static int physical_alias(const cs_guest_map *m,uint32_t a,uint32_t n,uint32_t b,uint32_t count)
{
    if(a>0x1000000u || n>0x1000000u-a || b>0x1000000u || count>0x1000000u-b)return 1;
    for(size_t i=0;i<m->count;++i){const cs_guest_region *x=&m->regions[i];
        if(x->base>0x1000000u || x->size>0x1000000u-x->base)return 1;
        uint32_t lo=a>x->base?a:x->base,hi=a+n<x->base+x->size?a+n:x->base+x->size;if(lo>=hi)continue;
        for(size_t j=0;j<m->count;++j){const cs_guest_region *y=&m->regions[j];
            if(y->base>0x1000000u || y->size>0x1000000u-y->base)return 1;
            uint32_t first=b>y->base?b:y->base,last=b+count<y->base+y->size?b+count:y->base+y->size;
            if(first<last && overlap((span){x->bytes+(lo-x->base),hi-lo},(span){y->bytes+(first-y->base),last-first}))return 1;
        }
    }
    return 0;
}
static int native_outside_guest(const cs_guest_map *m,span bytes)
{for(size_t i=0;i<m->count;++i)if(overlap(bytes,(span){m->regions[i].bytes,m->regions[i].size}) ||
    overlap(bytes,(span){m->regions[i].writable,m->regions[i].writable?m->regions[i].size:0}))return 0;
    return 1;}
static int acknowledged(const cs_image_service *s,const cs_image_context *c,const cs_code_layout *layout,int cancel)
{
    unsigned own=0;uint32_t resources=0;
    for(size_t i=0;i<c->count;++i){const cs_life_entry *a=&c->inventory[i];if(!a->owner)return 0;
        if(a->native==s){if(a->kind!=CS_LIFE_FUTURE || a->action!=CS_LIFE_REFUSE || a->owner!=s->profile.owner ||
                a->bytes!=sizeof *s || a->address || a->value || own++)return 0;
            continue;}
        if(a->owner==s->profile.owner)return 0;
        if(a->kind==CS_LIFE_GLOBAL){unsigned j=0;while(j<6 && a->address!=globals[j])++j;uint32_t value;
            if(j==6 || a->bytes!=4 || a->native || a->action!=CS_LIFE_PRESERVE ||
                (!cancel && (cs_guest_read(s->map,a->address,4,CS_GUEST_READ,&value)!=CS_GUEST_OK || value!=a->value)))return 0;
        }else if(a->kind==CS_LIFE_NATIVE){if(a->action!=CS_LIFE_PRESERVE || !a->native || !a->bytes || a->address || a->value ||
                (!cancel && !native_outside_guest(s->map,(span){a->native,a->bytes})))return 0;
            for(size_t j=0;j<c->resource_count;++j)if(a->native==c->resources[j] && a->bytes==sizeof(cs_res_service))resources|=UINT32_C(1)<<j;
        }else if(a->kind==CS_LIFE_GUEST){cs_guest_view v;
            if(a->action!=CS_LIFE_PRESERVE || a->native || a->value || !a->bytes ||
                intersects(a->address,a->bytes,s->config.system.begin,s->config.system.end-s->config.system.begin) ||
                intersects(a->address,a->bytes,s->config.application.begin,s->config.application.end-s->config.application.begin) ||
                (layout && intersects(a->address,a->bytes,layout->stack_base,layout->b-layout->stack_base)) ||
                (!cancel && (cs_guest_window(s->map,a->address,a->bytes,CS_GUEST_READ,&v)!=CS_GUEST_OK ||
                    physical_alias(s->map,a->address,a->bytes,s->config.system.begin,s->config.system.end-s->config.system.begin) ||
                    physical_alias(s->map,a->address,a->bytes,s->config.application.begin,s->config.application.end-s->config.application.begin) ||
                    (layout && physical_alias(s->map,a->address,a->bytes,layout->stack_base,layout->b-layout->stack_base)))))return 0;
        }else return 0;
    }
    return own==1 && resources==((UINT32_C(1)<<c->resource_count)-1u);
}
static int same_inventory(const cs_image_service *s,const cs_image_context *c)
{
    if(c->count!=s->inventory_count || c->resource_count!=s->resource_count || c->session!=s->session)return 0;
    for(size_t i=0;i<c->count;++i){const cs_life_entry *a=&c->inventory[i],*b=&s->inventory[i];
        if(a->kind!=b->kind || a->action!=b->action || a->owner!=b->owner || a->address!=b->address ||
            a->bytes!=b->bytes || a->value!=b->value || a->native!=b->native)return 0;}
    for(size_t i=0;i<c->resource_count;++i)if(c->resources[i]!=s->resources[i])return 0;
    if(c->session && (!equal(&c->session->calls.config,&s->session_config,sizeof s->session_config) ||
        !equal(&c->session->calls.options,&s->session_options,sizeof s->session_options) || c->session->calls.gateway!=s->gateway))return 0;
    return 1;
}
static cs_image_result failure(cs_image_report *r,cs_image_result result){r->cause=result;return result;}
static void status(const cs_image_service *s,cs_image_report *r)
{
    r->state=s->state;r->masks_valid=1;
    if(s->state!=CS_IMAGE_EMPTY || !r->completed){r->generation=s->generation;r->generation_valid=s->state!=CS_IMAGE_EMPTY;
        r->epoch=s->epoch;r->epoch_valid=s->state!=CS_IMAGE_EMPTY;}
    if(s->state!=CS_IMAGE_EMPTY || !r->completed){
        r->allocated=s->allocated;r->locked=s->locked;r->copied=s->copied;r->live=s->live;r->disposed=s->disposed;r->retired=s->retired;}
}
static cs_image_result fault(cs_image_service *s,cs_image_report *r)
{s->state=CS_IMAGE_FAULTED;r->native_committed=1;status(s,r);r->cause=CS_IMAGE_FAULT;r->retained_fault=1;copy(&s->first_fault,r,sizeof *r);return CS_IMAGE_FAULT;}
static int unit(cs_image_service *s,cs_image_report *r,int guest)
{
    if(r->phase!=CS_IMAGE_ALLOCATION && r->phase!=CS_IMAGE_LOCKING && r->phase!=CS_IMAGE_DISPOSAL)++r->attempted;
    ++r->completed;r->native_committed=1;if(guest)r->guest_committed=1;
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    if(s->test_after && r->completed==s->test_after)return 0;
#else
    (void)s;
#endif
    return 1;
}
static int manager_call(cs_image_service *s,cs_image_report *r,cs_mm_operation operation,uint32_t address,uint32_t size,uint32_t index)
{
    r->lifecycle_valid=0;r->manager_valid=0;r->reply_valid=0;
    ++r->attempted;
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    uint32_t kind=operation==CS_MM_NEW_HANDLE?1u:operation==CS_MM_HLOCK?2u:3u;
    if(s->test_call==kind && s->test_index==index)return 0;
#else
    (void)index;
#endif
    cs_mm_request q={operation,address,0,size,0,0};
    r->lifecycle=cs_life_manager_call(s->lifecycle,&q,&r->reply,&r->manager);r->lifecycle_valid=1;
    if(r->lifecycle!=CS_LIFE_OK)return 0;
    r->manager_valid=1;if(r->manager!=CS_MM_OK)return 0;
    r->reply_valid=1;r->moved+=r->reply.moved;return 1;
}
static int owned(const cs_image_service *s,uint32_t i,int locked)
{
    const cs_image_segment *a=&s->segments[i];if(a->slot>=CS_MM_BLOCKS)return 0;
    const cs_mm_block *b=&s->manager->blocks[a->slot];cs_guest_view v;uint32_t cell;
    if(b->kind!=2 || b->zone!=1 || b->address!=a->address || b->size!=a->length || b->extent!=a->extent ||
        a->handle!=s->config.application.begin+a->slot*4u || a->address>0xFFFFFFu || (a->address&1u) ||
        a->extent<a->length || cs_guest_window(s->map,a->address,a->extent,CS_GUEST_READ|CS_GUEST_WRITE,&v)!=CS_GUEST_OK ||
        cs_guest_read(s->map,a->handle,4,CS_GUEST_READ,&cell)!=CS_GUEST_OK)return 0;
    return cell==(a->address|(locked?CS_MM_LOCK:0));
}
static int all_owned(const cs_image_service *s)
{for(uint32_t i=0;i<s->manifest.segment_count;++i)if((s->live&bit(i)) && !owned(s,i,(s->locked&bit(i))!=0))return 0;return 1;}
static int nonexec(const cs_image_service *s,const cs_code_layout *l)
{
    uint32_t app=s->config.application.begin,n=s->config.application.end-app;
    if(physical_alias(s->map,app,n,s->config.system.begin,s->config.system.end-s->config.system.begin))return 0;
    for(unsigned i=0;i<6;++i)if(physical_alias(s->map,app,n,globals[i],4))return 0;
    for(size_t i=0;i<s->map->count;++i){const cs_guest_region *a=&s->map->regions[i];
        if(!intersects(a->base,a->size,s->config.application.begin,s->config.application.end-s->config.application.begin) &&
            !intersects(a->base,a->size,l->stack_base,l->b-l->stack_base))continue;
        if(a->flags&CS_GUEST_EXECUTE)return 0;
        for(size_t j=0;j<s->map->count;++j){const cs_guest_region *b=&s->map->regions[j];if(!(b->flags&CS_GUEST_EXECUTE))continue;
            if(overlap((span){a->bytes,a->size},(span){b->bytes,b->size}) ||
                overlap((span){a->writable,a->writable?a->size:0},(span){b->bytes,b->size}))return 0;}
    }
    return 1;
}
static void code_context(const cs_image_service *s,const cs_image_context *c,cs_code_context *v)
{v->context=c->context;v->active_root=c->active_root;v->active_callback=c->active_callback;v->active_patch=c->active_patch;v->active_manager=c->active_manager;v->map=s->map;v->lifecycle=s->lifecycle;v->session=c->session;}
static cs_image_result layout(cs_image_service *s,const cs_image_request *q,cs_image_workspace *w,cs_image_report *r,const cs_code_manifest *m)
{
    zero(&w->check,sizeof w->check);code_context(s,&q->context,&w->check.context);w->check.manifest=m;
    int fresh=q->operation==CS_IMAGE_PREPARE || q->operation==CS_IMAGE_LOAD;
    w->check.b=fresh?q->b:s->layout.b;w->check.s=fresh?q->s:s->layout.s;w->check.h_min=fresh?q->h_min:s->layout.h_min;
    cs_code_result v=cs_code_check_layout(&w->check,&w->parser,sizeof w->parser,&w->layout,&w->report);
    r->preflight_valid=1;copy(&r->preflight,&w->report,sizeof r->preflight);
    if(v!=CS_CODE_OK)return CS_IMAGE_CODE;
    uint32_t limit,zone;if(cs_guest_read(s->map,0x130,4,CS_GUEST_READ,&limit)!=CS_GUEST_OK ||
        cs_guest_read(s->map,0x118,4,CS_GUEST_READ,&zone)!=CS_GUEST_OK)return CS_IMAGE_CHANGED;
    if(limit!=w->layout.candidate_appl_limit || zone!=s->config.application.identity || !nonexec(s,&w->layout) ||
        !acknowledged(s,&q->context,&w->layout,0))return CS_IMAGE_CHANGED;
    return CS_IMAGE_OK;
}
static int capacity_ok(const cs_image_service *s,const cs_code_manifest *m,const cs_code_layout *l)
{
    uint32_t slots=0,available=l->candidate_appl_limit-s->config.application.begin-CS_MM_CELL_BYTES,need=0;
    for(unsigned i=0;i<CS_MM_BLOCKS;++i){const cs_mm_block *b=&s->manager->blocks[i];if(!b->kind){++slots;continue;}
        if(b->zone==1 && b->address<l->candidate_appl_limit){uint32_t n=l->candidate_appl_limit-b->address;
            if(n>b->extent)n=b->extent;
            if(n>available)return 0;
            available-=n;}}
    for(uint32_t i=0;i<m->segment_count;++i)need+=(m->segments[i].resource.entry.data.length+1u)&~UINT32_C(1);
    return slots>=m->segment_count && need<=available;
}
static int staged(const cs_image_service *s)
{
    const cs_code_manifest *m=&s->manifest;uint32_t at=16u+m->table_length;
    if(!s->staging_borrow || !s->staging || !m->entry_count || m->entry_count>CS_CODE_ENTRIES ||
        !m->segment_count || m->segment_count>CS_CODE_SEGMENTS || m->table_length!=m->entry_count*8u ||
        at>s->staging_bytes || s->staging_bytes>s->staging_capacity || s->staging_bytes>s->profile.maximum_staging)return 0;
    const uint8_t *b=s->staging;
    if(u32(b)!=m->above_a5 || u32(b+4)!=m->below_a5 || u32(b+8)!=m->table_length || u32(b+12)!=m->table_offset)return 0;
    for(uint32_t i=0;i<m->entry_count;++i){uint32_t j=m->entries[i].segment_index;if(j>=m->segment_count)return 0;
        b=s->staging+16u+i*8u;
        if(u16(b)!=m->entries[i].routine_offset || u16(b+2)!=0x3F3C || u16(b+4)!=(uint32_t)m->segments[j].resource.entry.id || u16(b+6)!=0xA9F0)return 0;}
    for(uint32_t i=0;i<m->segment_count;++i){const cs_code_segment *p=&m->segments[i];uint32_t n=p->resource.entry.data.length;
        if(s->offsets[i]!=at || n<6 || n>32768 || n>s->staging_bytes-at)return 0;
        b=s->staging+at;if(u16(b)!=p->first_entry*8u || u16(b+2)!=p->entry_count)return 0;at+=n;}
    return at==s->staging_bytes;
}
static int seal_equal(const cs_image_service *s,const cs_image_workspace *w)
{
    if(s->manager->magic!=s->sealed_manager.magic || !equal(s->manager->blocks,s->sealed_manager.blocks,sizeof s->manager->blocks) ||
        !equal(w->globals,s->globals,sizeof s->globals) || !equal(w->cells,s->cells,sizeof s->cells) ||
        !equal(&w->layout,&s->layout,sizeof s->layout))return 0;
    return 1;
}
static void describe(const cs_image_service *s,cs_image_description *d)
{
    zero(d,sizeof *d);d->state=s->state;if(s->state==CS_IMAGE_EMPTY)return;
    d->generation=s->generation;d->generation_valid=1;d->epoch=s->epoch;d->epoch_valid=1;
    d->entry_count=s->manifest.entry_count;d->segment_count=s->manifest.segment_count;d->warnings=s->manifest.warnings;
    copy(&d->layout,&s->layout,sizeof d->layout);d->live=s->live;copy(d->segments,s->segments,sizeof d->segments);
}
static void empty(cs_image_service *s)
{
    s->execution_state=0;
    s->state=CS_IMAGE_EMPTY;s->reservation=0;s->staging_borrow=0;s->staging=0;s->staging_capacity=0;s->staging_bytes=0;
    s->session=0;s->gateway=0;s->inventory_count=0;s->resource_count=0;
    s->allocated=0;s->locked=0;s->copied=0;s->live=0;s->disposed=0;s->retired=0;s->world_written=0;
}
static cs_image_result prepare(cs_image_service *s,const cs_image_request *q,cs_image_workspace *w,cs_image_report *r)
{
    if(s->last_generation==UINT32_MAX)return CS_IMAGE_COUNTER_EXHAUSTED;
    if(!q->source || !q->length || !q->staging || !q->staging_capacity)return CS_IMAGE_ARGUMENT;
    if(q->length>s->profile.maximum_source)return CS_IMAGE_PROFILE_LIMIT;
    zero(&w->inspect,sizeof w->inspect);code_context(s,&q->context,&w->inspect.context);
    w->inspect.source=q->source;w->inspect.length=q->length;w->inspect.maximum_source=s->profile.maximum_source;
    r->phase=CS_IMAGE_SOURCE;
    cs_code_result v=cs_code_inspect(&w->inspect,&w->parser,sizeof w->parser,&w->manifest,&w->report);
    w->inspect.source=0;w->inspect.length=0;w->parser.view.bytes=0;
    r->preflight_valid=1;copy(&r->preflight,&w->report,sizeof r->preflight);if(v!=CS_CODE_OK)return CS_IMAGE_CODE;
    r->phase=CS_IMAGE_LAYOUT;cs_image_result result=layout(s,q,w,r,&w->manifest);if(result!=CS_IMAGE_OK)return result;
    if(w->layout.b-w->layout.stack_base>s->profile.maximum_world)return CS_IMAGE_PROFILE_LIMIT;
    if(!capacity_ok(s,&w->manifest,&w->layout))return CS_IMAGE_CAPACITY;
    uint32_t bytes=16u+w->manifest.table_length;
    for(uint32_t i=0;i<w->manifest.segment_count;++i)bytes+=w->manifest.segments[i].resource.entry.data.length;
    if(bytes>s->profile.maximum_staging || bytes>q->staging_capacity)return CS_IMAGE_CAPACITY;
    if(!views(s,w->globals,w->cells))return CS_IMAGE_CHANGED;
    r->phase=CS_IMAGE_STAGING;
    copy(q->staging,q->source+w->manifest.code0.entry.data.offset,16u+w->manifest.table_length);
    uint32_t at=16u+w->manifest.table_length;
    for(uint32_t i=0;i<w->manifest.segment_count;++i){const cs_rf_span *d=&w->manifest.segments[i].resource.entry.data;
        copy(q->staging+at,q->source+d->offset,d->length);at+=d->length;}
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    if(s->test_prepare)return CS_IMAGE_CHANGED;
#endif
    /* No fallible work follows this native publication. Source has no retained
     * representation: every resource field below is scalar metadata. */
    s->generation=++s->last_generation;s->epoch=s->lifecycle->epoch;
    copy(&s->manifest,&w->manifest,sizeof s->manifest);copy(&s->layout,&w->layout,sizeof s->layout);
    copy(&s->sealed_manager,s->manager,sizeof s->sealed_manager);copy(s->globals,w->globals,sizeof s->globals);copy(s->cells,w->cells,sizeof s->cells);
    s->staging=q->staging;s->staging_capacity=q->staging_capacity;s->staging_bytes=bytes;s->staging_borrow=1;s->reservation=1;
    at=16u+s->manifest.table_length;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){s->offsets[i]=at;at+=s->manifest.segments[i].resource.entry.data.length;}
    s->inventory_count=q->context.count;copy(s->inventory,q->context.inventory,s->inventory_count*sizeof *s->inventory);
    s->resource_count=q->context.resource_count;copy(s->resources,q->context.resources,s->resource_count*sizeof *s->resources);
    s->session=q->context.session;
    if(s->session){copy(&s->session_config,&s->session->calls.config,sizeof s->session_config);
        copy(&s->session_options,&s->session->calls.options,sizeof s->session_options);s->gateway=s->session->calls.gateway;}
    zero(s->segments,sizeof s->segments);s->state=CS_IMAGE_PREPARED;r->native_committed=1;r->attempted=1;r->completed=1;
    return CS_IMAGE_OK;
}
/* Derive a table byte from immutable scalar ownership, never a guest pointer. */
static uint8_t table_byte(const cs_image_service *s,uint32_t i)
{
    const cs_code_entry *e=&s->manifest.entries[i/8u];const cs_image_segment *p=&s->segments[e->segment_index];
    uint32_t id=(uint32_t)s->manifest.segments[e->segment_index].resource.entry.id,target=p->address+e->routine_offset;
    switch(i&7u){case 0:return (uint8_t)(id>>8);case 1:return (uint8_t)id;case 2:return 0x4E;case 3:return 0xF9;
        default:return (uint8_t)(target>>((7u-(i&7u))*8u));}
}
static int world_valid(const cs_image_service *s,int loaded)
{
    cs_guest_view v;uint32_t begin=s->layout.globals_begin,n=s->manifest.below_a5+s->manifest.above_a5;
    /* Layout admits separate above/below windows; do not invent a contiguous
     * native span across their boundary. */
    for(uint32_t i=0;i<n;++i){uint32_t at=begin+i;
        if(cs_guest_window(s->map,at,1,CS_GUEST_READ|CS_GUEST_WRITE,&v)!=CS_GUEST_OK)return 0;
        uint8_t expected=loaded && at>=s->layout.table_begin && at<s->layout.table_end?table_byte(s,at-s->layout.table_begin):0;
        if(v.bytes[0]!=expected)return 0;}
    return 1;
}
static int world_clear(cs_image_service *s,cs_image_report *r)
{
    cs_guest_view v;uint32_t n=s->manifest.below_a5+s->manifest.above_a5;
    r->phase=CS_IMAGE_WORLD;
    for(uint32_t i=0;i<n;++i){if(cs_guest_window(s->map,s->layout.globals_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK)return 0;
        v.writable[0]=0;++r->world_bytes;++s->world_written;if(!unit(s,r,1))return 0;}
    return 1;
}
static cs_image_result load(cs_image_service *s,const cs_image_request *q,cs_image_workspace *w,cs_image_report *r)
{
    r->phase=CS_IMAGE_LAYOUT;
    if(!same_inventory(s,&q->context) || !views(s,w->globals,w->cells))return CS_IMAGE_CHANGED;
    cs_image_result result=layout(s,q,w,r,&s->manifest);if(result!=CS_IMAGE_OK)return result;
    if(!seal_equal(s,w) || !staged(s))return CS_IMAGE_CHANGED;
    if(!capacity_ok(s,&s->manifest,&s->layout))return CS_IMAGE_CAPACITY;
    for(uint32_t i=0;i<s->manifest.segment_count;++i){r->segment_valid=1;r->segment=i;r->phase=CS_IMAGE_ALLOCATION;
        if(!all_owned(s) || !manager_call(s,r,CS_MM_NEW_HANDLE,0,s->manifest.segments[i].resource.entry.data.length,i+1u))return fault(s,r);
        if(r->reply.code==CS_MM_FULL){s->state=s->live?CS_IMAGE_PARTIAL:CS_IMAGE_PREPARED;status(s,r);return CS_IMAGE_CAPACITY;}
        if(r->reply.code || !r->reply.value)return fault(s,r);
        cs_image_segment *p=&s->segments[i];p->handle=r->reply.value;p->slot=(p->handle-s->config.application.begin)/4u;
        s->allocated|=bit(i);s->live|=bit(i);s->state=CS_IMAGE_POPULATING;
        if(p->slot<CS_MM_BLOCKS){const cs_mm_block *b=&s->manager->blocks[p->slot];p->address=b->address;p->length=b->size;p->extent=b->extent;}
        if(!unit(s,r,1) || p->length!=s->manifest.segments[i].resource.entry.data.length || !owned(s,i,0))return fault(s,r);
        r->phase=CS_IMAGE_LOCKING;if(!manager_call(s,r,CS_MM_HLOCK,p->handle,0,i+1u) || r->reply.code)return fault(s,r);
        s->locked|=bit(i);if(!unit(s,r,1) || !owned(s,i,1))return fault(s,r);
        r->phase=CS_IMAGE_COPYING;cs_guest_view v;
        if(cs_guest_window(s->map,p->address,p->extent,CS_GUEST_READ|CS_GUEST_WRITE,&v)!=CS_GUEST_OK)return fault(s,r);
        for(uint32_t j=0;j<p->length;++j){v.writable[j]=s->staging[s->offsets[i]+j];
            if(j+1u==p->length)s->copied|=bit(i);
            if(!unit(s,r,1))return fault(s,r);}
    }
    if(!all_owned(s) || !world_clear(s,r))return fault(s,r);
    r->phase=CS_IMAGE_TABLE;
    for(uint32_t i=0;i<s->manifest.table_length;++i){cs_guest_view v;
        if(cs_guest_window(s->map,s->layout.table_begin+i,1,CS_GUEST_WRITE,&v)!=CS_GUEST_OK)return fault(s,r);
        v.writable[0]=table_byte(s,i);++r->table_bytes;if(!unit(s,r,1))return fault(s,r);}
    if(!all_owned(s) || !world_valid(s,1))return fault(s,r);
    for(uint32_t i=0;i<s->manifest.segment_count;++i){cs_guest_view v;const cs_image_segment *p=&s->segments[i];
        if(cs_guest_window(s->map,p->address,p->length,CS_GUEST_READ,&v)!=CS_GUEST_OK || !equal(v.bytes,s->staging+s->offsets[i],p->length))return fault(s,r);}
    r->phase=CS_IMAGE_DONE;s->state=CS_IMAGE_LOADED;if(!unit(s,r,0))return fault(s,r);
    s->staging_borrow=0;s->staging=0;s->staging_capacity=0;return CS_IMAGE_OK;
}
static cs_image_result release(cs_image_service *s,const cs_image_request *q,cs_image_report *r)
{
    if(!same_inventory(s,&q->context) || !all_owned(s) || (s->state==CS_IMAGE_PARTIAL && s->world_written) ||
        (s->state==CS_IMAGE_LOADED && !world_valid(s,1)))return CS_IMAGE_CHANGED;
    int loaded=s->state==CS_IMAGE_LOADED;s->state=CS_IMAGE_RETIRING;
    if(loaded && !world_clear(s,r))return fault(s,r);
    for(uint32_t i=s->manifest.segment_count;i>0;--i){uint32_t j=i-1u;if(!(s->live&bit(j)))continue;
        r->phase=CS_IMAGE_DISPOSAL;r->segment_valid=1;r->segment=j;
        if(!all_owned(s) || !manager_call(s,r,CS_MM_DISPOS_HANDLE,s->segments[j].handle,0,j+1u) || r->reply.code)return fault(s,r);
        s->live&=(uint16_t)~bit(j);s->disposed|=bit(j);status(s,r);
        if(!unit(s,r,1))return fault(s,r);
        uint32_t cell;if(s->manager->blocks[s->segments[j].slot].kind ||
            cs_guest_read(s->map,s->segments[j].handle,4,CS_GUEST_READ,&cell)!=CS_GUEST_OK || cell)return fault(s,r);
        r->phase=CS_IMAGE_RETIREMENT;s->retired|=bit(j);status(s,r);if(!unit(s,r,0))return fault(s,r);
    }
    s->state=CS_IMAGE_EMPTY;if(!unit(s,r,0))return fault(s,r);
    empty(s);return CS_IMAGE_OK;
}
cs_image_result cs_image_init(cs_image_service *s,cs_life_controller *l,const cs_image_profile *p)
{
    if(!OBJ(s,cs_image_service) || !OBJ(p,cs_image_profile))return CS_IMAGE_ARGUMENT;
    envelope e;e.count=0;e.owner=0;if(!add_span(&e,(span){s,sizeof *s}) || !add_span(&e,(span){p,sizeof *p}) || !life_safe(&e,l))return CS_IMAGE_ARGUMENT;
    if(s->tag)return CS_IMAGE_STATE;
    if(!p->maximum_source || !p->maximum_staging || !p->maximum_world || !p->owner)return CS_IMAGE_ARGUMENT;
    if(cs_life_admit(l)!=CS_LIFE_OK)return CS_IMAGE_STATE;
    zero(s,sizeof *s);s->tag=TAG;s->self=s;s->lifecycle=l;s->manager=l->manager;s->map=l->manager->map;
    s->descriptors=s->map->regions;s->region_count=s->map->count;
    copy(s->regions,s->map->regions,s->region_count*sizeof *s->regions);copy(&s->config,&s->manager->config,sizeof s->config);
    copy(&s->profile,p,sizeof *p);return CS_IMAGE_OK;
}
cs_image_result cs_image_execute(cs_image_service *s,const cs_image_request *q,cs_image_workspace *w,size_t capacity,
    cs_image_description *out,cs_image_report *r)
{
    if(!native_safe(s,q,w,capacity,out,r))return CS_IMAGE_ARGUMENT;
    zero(r,sizeof *r);r->operation=q->operation;r->phase=CS_IMAGE_ADMISSION;
    if(!identity(s) || s->state>CS_IMAGE_FAULTED || q->operation<CS_IMAGE_PREPARE || q->operation>CS_IMAGE_RELEASE)return failure(r,CS_IMAGE_STATE);
    if(s->execution_state)return failure(r,s->execution_state==5?CS_IMAGE_FAULT:CS_IMAGE_IN_USE);
    if(s->state==CS_IMAGE_FAULTED){r->state=CS_IMAGE_FAULTED;return failure(r,CS_IMAGE_FAULT);}
    status(s,r);
    if(s->state==CS_IMAGE_POPULATING || s->state==CS_IMAGE_RETIRING ||
        (q->operation==CS_IMAGE_PREPARE && s->state!=CS_IMAGE_EMPTY) ||
        ((q->operation==CS_IMAGE_LOAD || q->operation==CS_IMAGE_CANCEL) && s->state!=CS_IMAGE_PREPARED) ||
        (q->operation==CS_IMAGE_RELEASE && s->state!=CS_IMAGE_LOADED && s->state!=CS_IMAGE_PARTIAL))return failure(r,CS_IMAGE_STATE);
    if(!inactive(s,&q->context))return failure(r,CS_IMAGE_CONTEXT);
    if(!acknowledged(s,&q->context,s->state==CS_IMAGE_EMPTY?0:&s->layout,q->operation==CS_IMAGE_CANCEL))return failure(r,CS_IMAGE_IN_USE);
    if(q->operation!=CS_IMAGE_PREPARE && (q->source || q->length || q->staging || q->staging_capacity))return failure(r,CS_IMAGE_ARGUMENT);
    if((q->operation==CS_IMAGE_LOAD || q->operation==CS_IMAGE_CANCEL || q->operation==CS_IMAGE_RELEASE) &&
        (q->generation!=s->generation || q->epoch!=s->epoch))return failure(r,CS_IMAGE_CHANGED);
    if(q->operation==CS_IMAGE_CANCEL){
        if(s->allocated || s->live || s->disposed || s->world_written || !s->staging_borrow)return failure(r,CS_IMAGE_STATE);
        empty(s);r->native_committed=1;r->attempted=1;r->completed=1;
    }else{
        if(!binding(s))return failure(r,CS_IMAGE_CHANGED);
        r->lifecycle_valid=1;r->lifecycle=cs_life_admit(s->lifecycle);if(r->lifecycle!=CS_LIFE_OK)return failure(r,CS_IMAGE_CHANGED);
        r->heap_epoch_valid=1;r->heap_epoch=s->lifecycle->epoch;
        if(s->state!=CS_IMAGE_EMPTY && s->epoch!=s->lifecycle->epoch)return failure(r,CS_IMAGE_CHANGED);
        uint32_t zone,limit;
        if(cs_guest_read(s->map,0x118,4,CS_GUEST_READ,&zone)!=CS_GUEST_OK || zone!=s->config.application.identity ||
            cs_guest_read(s->map,0x130,4,CS_GUEST_READ,&limit)!=CS_GUEST_OK ||
            (s->state!=CS_IMAGE_EMPTY && limit!=s->layout.candidate_appl_limit))return failure(r,CS_IMAGE_CHANGED);
        for(size_t i=0;i<q->context.resource_count;++i)if(cs_res_external_admit(q->context.resources[i])!=CS_RES_OK)return failure(r,CS_IMAGE_IN_USE);
        cs_image_result result=CS_IMAGE_OK;
        if(q->operation==CS_IMAGE_PREPARE)result=prepare(s,q,w,r);
        else if(q->operation==CS_IMAGE_LOAD)result=load(s,q,w,r);
        else if(s->state!=CS_IMAGE_EMPTY){
            result=layout(s,q,w,r,&s->manifest);
            if(result==CS_IMAGE_OK && !equal(&w->layout,&s->layout,sizeof s->layout))result=CS_IMAGE_CHANGED;
            if(result==CS_IMAGE_OK && q->operation==CS_IMAGE_RELEASE)result=release(s,q,r);
            else if(result==CS_IMAGE_OK && !same_inventory(s,&q->context))result=CS_IMAGE_CHANGED;}
        if(result!=CS_IMAGE_OK){if(result!=CS_IMAGE_FAULT)status(s,r);return failure(r,result);}
    }
    status(s,r);r->phase=CS_IMAGE_DONE;describe(s,out);return CS_IMAGE_OK;
}
cs_image_result cs_image_external_admit(const cs_image_service *s)
{
    if(!OBJ(s,cs_image_service) || !identity(s))return CS_IMAGE_ARGUMENT;
    if(s->execution_state==5)return CS_IMAGE_FAULT;
    if(s->state==CS_IMAGE_FAULTED)return CS_IMAGE_FAULT;
    if(s->state!=CS_IMAGE_EMPTY)return CS_IMAGE_IN_USE;
    if(!binding(s) || cs_life_admit(s->lifecycle)!=CS_LIFE_OK)return CS_IMAGE_CHANGED;
    return CS_IMAGE_OK;
}
/* The image itself is the sole FUTURE/REFUSE execution participant. This
 * dedicated first profile admits no other native/session/resource owner. */
static int execution_safe(cs_image_service *s,const cs_image_run_request *q,
    const span *outputs,unsigned count)
{
    if(!OBJ(s,cs_image_service) || !OBJ(q,cs_image_run_request) ||
        !OBJ(q->gateway,cs_trap_gateway) || q->context.count!=1 ||
        !OBJ(q->context.inventory,cs_life_entry) || q->context.resources ||
        q->context.resource_count || q->context.session)return 0;
    envelope e;e.count=0;e.owner=s;
    if(!add_span(&e,(span){s,sizeof *s}) || !add_span(&e,(span){q,sizeof *q}) ||
        !add_span(&e,(span){q->gateway,sizeof *q->gateway}) ||
        !add_span(&e,(span){q->context.inventory,sizeof *q->context.inventory}))return 0;
    for(unsigned i=0;i<count;++i)if(!add_span(&e,outputs[i]))return 0;
    if(!life_safe(&e,s->lifecycle) || !map_safe(&e,s->map) ||
        s->region_count>CS_GUEST_MAX_REGIONS || s->inventory_count>CS_LIFE_INVENTORY ||
        !aligned(s->descriptors,s->region_count*sizeof *s->descriptors,_Alignof(cs_guest_region)) ||
        !protect(&e,(span){s->descriptors,s->region_count*sizeof *s->descriptors}))return 0;
    for(size_t i=0;i<s->region_count;++i)
        if(!protect(&e,(span){s->regions[i].bytes,s->regions[i].size}) ||
            !protect(&e,(span){s->regions[i].writable,s->regions[i].writable?s->regions[i].size:0}))return 0;
    return inventory_safe(&e,s->inventory,s->inventory_count);
}
int cs_image_execution_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r)
{
    if(!OBJ(caller,cs_trap_frame) || !OBJ(r,cs_image_run_report))return 0;
    span outputs[2]={{caller,sizeof *caller},{r,sizeof *r}};
    return execution_safe(s,q,outputs,2);
}
int cs_image_execution_rect_native(cs_image_service *s,const cs_image_run_request *q,
    cs_trap_frame *caller,cs_image_run_report *r,cs_qd_rect_port *port,
    cs_image_rect_scratch *scratch)
{
    if(!OBJ(caller,cs_trap_frame) || !OBJ(r,cs_image_run_report) ||
        !OBJ(port,cs_qd_rect_port) || !OBJ(scratch,cs_image_rect_scratch))return 0;
    span outputs[4]={{caller,sizeof *caller},{r,sizeof *r},
        {port,sizeof *port},{scratch,sizeof *scratch}};
    return execution_safe(s,q,outputs,4);
}
int cs_image_execution_retire_native(cs_image_service *s,const cs_image_run_request *q,
    cs_image_workspace *w,size_t capacity,cs_image_description *out,cs_image_report *r)
{
    if(!OBJ(w,cs_image_workspace) || capacity<sizeof *w || !OBJ(out,cs_image_description) ||
        !OBJ(r,cs_image_report))return 0;
    span outputs[3]={{w,capacity},{out,sizeof *out},{r,sizeof *r}};
    return execution_safe(s,q,outputs,3);
}
int cs_image_execution_integrity(const cs_image_service *s,const cs_image_context *c,int dirty)
{
    if(!identity(s) || s->state!=CS_IMAGE_LOADED || !inactive(s,c) || c->count!=1 ||
        c->resource_count || c->session || s->resource_count || s->session ||
        !same_inventory(s,c) || !acknowledged(s,c,&s->layout,0) || !binding(s) ||
        cs_life_admit(s->lifecycle)!=CS_LIFE_OK || s->epoch!=s->lifecycle->epoch ||
        s->manifest.segment_count!=1 || s->manifest.entry_count!=1 ||
        s->manifest.table_length!=s->manifest.entry_count*8u || !nonexec(s,&s->layout))return 0;
    uint16_t mask=(uint16_t)((UINT32_C(1)<<s->manifest.segment_count)-1u);
    if(s->allocated!=mask || s->locked!=mask || s->copied!=mask || s->live!=mask ||
        s->disposed || s->retired || !all_owned(s))return 0;
    for(unsigned i=0;i<6;++i){uint32_t value;
        if(cs_guest_read(s->map,globals[i],4,CS_GUEST_READ,&value)!=CS_GUEST_OK || value!=s->globals[i])return 0;}
    for(unsigned i=0;i<CS_MM_BLOCKS;++i){int own=0;
        for(uint32_t j=0;j<s->manifest.segment_count;++j)if(s->segments[j].slot==i)own=1;
        if(!own && !equal(&s->manager->blocks[i],&s->sealed_manager.blocks[i],sizeof(cs_mm_block)))return 0;
        for(unsigned z=0;z<2;++z){uint32_t value,base=z?s->config.application.begin:s->config.system.begin;
            if(cs_guest_read(s->map,base+i*4u,4,CS_GUEST_READ,&value)!=CS_GUEST_OK)return 0;
            if((z==0 || !own) && value!=s->cells[z][i])return 0;}
    }
    if(!dirty)return world_valid(s,1);
    for(uint32_t i=s->manifest.below_a5;i<s->manifest.below_a5+s->manifest.above_a5;++i){
        uint32_t at=s->layout.globals_begin+i;cs_guest_view v;
        if(cs_guest_window(s->map,at,1,CS_GUEST_READ,&v)!=CS_GUEST_OK)return 0;
        uint8_t expected=at>=s->layout.table_begin && at<s->layout.table_end?table_byte(s,at-s->layout.table_begin):0;
        if(v.bytes[0]!=expected)return 0;
    }
    return 1;
}
#ifdef CLASSICK_IMAGE_TEST_FAULTS
cs_image_result cs_image_test_fault(cs_image_service *s,uint32_t after,uint32_t call,uint32_t index,uint32_t prepare_flag)
{
    if(!OBJ(s,cs_image_service) || !identity(s) || s->state==CS_IMAGE_FAULTED || call>3 || (call && (!index || index>CS_CODE_SEGMENTS)))return CS_IMAGE_ARGUMENT;
    s->test_after=after;s->test_call=call;s->test_index=index;s->test_prepare=prepare_flag;return CS_IMAGE_OK;
}
#endif
