/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original scalar preflight, no executable owner. */
#include "code-preflight.h"
#define MANIFEST_TAG UINT32_C(0x43503031)
#define LAYOUT_TAG UINT32_C(0x434C3031)
typedef struct { const void *at;size_t bytes; } blob;
static int valid(blob b)
{return !b.bytes || (b.at && b.bytes<=UINTPTR_MAX-(uintptr_t)b.at);}
static int aligned(const void *p,size_t bytes,size_t a)
{return p && ((uintptr_t)p&(a-1u))==0 && valid((blob){p,bytes});}
#define OBJECT(p,t) aligned((p),sizeof(t),_Alignof(t))
static int overlap(blob a,blob b)
{uintptr_t x=(uintptr_t)a.at,y=(uintptr_t)b.at;return a.bytes && b.bytes && (x<=y?y-x<a.bytes:x-y<b.bytes);}
static int intersects(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n && m && (a<=b?b-a<n:a-b<m);}
static void clear(void *p,size_t n)
{uint8_t *b=p;for(size_t i=0;i<n;++i)b[i]=0;}
static void copy(void *p,const void *q,size_t n)
{uint8_t *b=p;const uint8_t *s=q;for(size_t i=0;i<n;++i)b[i]=s[i];}
static uint32_t be16(const uint8_t *b)
{return ((uint32_t)b[0]<<8)|b[1];}
static uint32_t be32(const uint8_t *b)
{return (be16(b)<<16)|be16(b+2);}
static int add(uint32_t a,uint32_t b,uint32_t *out)
{if(b>UINT32_MAX-a)return 0;*out=a+b;return 1;}
static int subtract(uint32_t a,uint32_t b,uint32_t *out)
{if(b>a)return 0;*out=a-b;return 1;}
/* No destinations are touched until every native object, borrowed buffer and
 * current protected backing has been checked. Maps are bounded before reading
 * descriptors; liveness itself is a truthful native caller precondition. */
static int envelope(const cs_code_context *c,blob request,cs_code_workspace *w,size_t capacity,
    blob result,cs_code_report *r,blob source,blob snapshot)
{
    if(!OBJECT(w,cs_code_workspace) || capacity<sizeof *w || !valid((blob){w,capacity}) ||
        !OBJECT(r,cs_code_report) || !OBJECT(c->map,cs_guest_map))return 0;
    blob objects[24];unsigned n=0;const cs_guest_map *maps[3];unsigned count=1;
    objects[n++]=request;objects[n++]=(blob){w,capacity};objects[n++]=result;
    objects[n++]=(blob){r,sizeof *r};objects[n++]=source;objects[n++]=snapshot;
    maps[0]=c->map;
    if(c->lifecycle){const cs_life_controller *l=c->lifecycle;
        if(!OBJECT(l,cs_life_controller) || !OBJECT(l->manager,cs_mm_manager))return 0;
        objects[n++]=(blob){l,sizeof *l};objects[n++]=(blob){l->manager,sizeof *l->manager};
        if(l->active){const cs_life_plan *p=l->active;
            if(!OBJECT(p,cs_life_plan) || !OBJECT(p->input,cs_life_request) ||
                !OBJECT(p->request.scrap,cs_life_scrap) || p->request.count>CS_LIFE_INVENTORY)return 0;
            objects[n++]=(blob){p,sizeof *p};
            /* A refused layout must preserve outstanding reset inputs and
             * staged bytes as well as the plan's embedded snapshot. Borrowed
             * guards can coincide with one another; none may alias any of
             * this operation's six native inputs/destinations/source spans. */
            blob guards[7];unsigned guard_count=4;
            guards[0]=(blob){p->input,sizeof *p->input};guards[1]=(blob){p->request.scrap,sizeof *p->request.scrap};
            guards[2]=(blob){p->request.inventory,p->request.count*sizeof *p->request.inventory};
            guards[3]=(blob){p->request.staging,p->request.staging_bytes};
            if(p->request.gateway){if(!OBJECT(p->request.gateway,cs_trap_gateway))return 0;
                guards[guard_count++]=(blob){p->request.gateway,sizeof *p->request.gateway};}
            if(p->request.session){const cs_m68k_session *s=p->request.session;
                if(!OBJECT(s,cs_m68k_session))return 0;
                guards[guard_count++]=(blob){s,sizeof *s};
                if(s->current){if(!OBJECT(s->current,cs_m68k_session_report))return 0;
                    guards[guard_count++]=(blob){s->current,sizeof *s->current};}}
            for(unsigned j=0;j<guard_count;++j){if(!valid(guards[j]))return 0;
                for(unsigned i=0;i<6;++i)if(overlap(objects[i],guards[j]))return 0;}
            if(p->request.count && !aligned(p->request.inventory,p->request.count*sizeof *p->request.inventory,_Alignof(cs_life_entry)))return 0;
            for(size_t j=0;j<p->request.count;++j){const cs_life_entry *e=&p->request.inventory[j];
                if(e->kind==CS_LIFE_NATIVE){blob guard={e->native,e->bytes};if(!valid(guard))return 0;
                    for(unsigned i=0;i<6;++i)if(overlap(objects[i],guard))return 0;}}
        }
        const cs_guest_map *m=l->manager->map;
        if(!OBJECT(m,cs_guest_map))return 0;
        if(m!=maps[0])maps[count++]=m;
    }
    if(c->session){const cs_m68k_session *s=c->session;
        if(!OBJECT(s,cs_m68k_session))return 0;
        objects[n++]=(blob){s,sizeof *s};
        if(s->calls.gateway){if(!OBJECT(s->calls.gateway,cs_trap_gateway))return 0;
            objects[n++]=(blob){s->calls.gateway,sizeof *s->calls.gateway};}
        if(s->current){if(!OBJECT(s->current,cs_m68k_session_report))return 0;
            objects[n++]=(blob){s->current,sizeof *s->current};}
        const cs_guest_map *m=s->calls.map;if(!OBJECT(m,cs_guest_map))return 0;
        if(m!=maps[0] && (count==1 || m!=maps[1]))maps[count++]=m;
    }
    for(unsigned k=0;k<count;++k){const cs_guest_map *m=maps[k];
        if(!m->count || m->count>CS_GUEST_MAX_REGIONS ||
            !aligned(m->regions,m->count*sizeof *m->regions,_Alignof(cs_guest_region)))return 0;
        objects[n++]=(blob){m,sizeof *m};objects[n++]=(blob){m->regions,m->count*sizeof *m->regions};
    }
    for(unsigned i=0;i<n;++i){if(!valid(objects[i]))return 0;
        for(unsigned j=0;j<i;++j)if(overlap(objects[i],objects[j]))return 0;
        for(unsigned k=0;k<count;++k)for(size_t j=0;j<maps[k]->count;++j){const cs_guest_region *g=&maps[k]->regions[j];
            blob a={g->bytes,g->size},b={g->writable,g->writable?g->size:0};
            if(!valid(a) || !valid(b) || overlap(objects[i],a) || overlap(objects[i],b))return 0;
        }
    }
    return 1;
}
static void initialize(cs_code_report *r,cs_code_operation operation)
{clear(r,sizeof *r);r->operation=operation;}
static cs_code_result fail(cs_code_report *r,cs_code_result cause)
{r->cause=cause;return cause;}
static int inactive(const cs_code_context *c)
{
    if(c->context!=CS_LIFE_ORDINARY || c->active_root || c->active_callback || c->active_patch || c->active_manager)return 0;
    const cs_m68k_session *s=c->session;
    return !s || (s->tag==UINT32_C(0x53455331) && !s->active && !s->stepping && !s->exit_pending &&
        !s->current && !s->calls.depth && s->calls.map==c->map);
}
static void position(cs_code_report *r,cs_code_field field,uint32_t at)
{r->field=field;r->position_valid=1;r->position=at;}
static cs_code_result find(cs_code_workspace *w,int32_t id,cs_code_resource *resource,cs_code_report *r)
{
    r->key_valid=1;r->id=id;r->position_valid=0;r->reader_valid=0;r->field=CS_CODE_FIELD_NONE;
    cs_rf_result result=cs_rf_find(&w->view,CS_CODE_TYPE,id,&w->entry,&w->reader);
    if(result!=CS_RF_ARGUMENT){copy(&r->reader,&w->reader,sizeof r->reader);r->reader_valid=1;}
    if(result==CS_RF_NOT_FOUND)return fail(r,CS_CODE_NOT_FOUND);
    if(result==CS_RF_DUPLICATE_TYPE || result==CS_RF_DUPLICATE_ID)return fail(r,CS_CODE_AMBIGUOUS);
    if(result==CS_RF_REFERENCE)return fail(r,CS_CODE_UNRESOLVED_REFERENCE);
    if(result!=CS_RF_OK)return fail(r,CS_CODE_SOURCE);
    if(w->entry.attributes&0x40u)return fail(r,CS_CODE_UNSUPPORTED_STORAGE);
    copy(&resource->entry,&w->entry,sizeof resource->entry);resource->reader_warnings=w->reader.warnings;
    return CS_CODE_OK;
}
/* Inspection header order is normative. Snapshot validation separately checks
 * the same scalar invariants without source reads or inspection diagnostics. */
static cs_code_result header(cs_code_manifest *m,cs_code_report *r)
{
    uint32_t fields[4];fields[0]=m->above_a5;fields[1]=m->below_a5;
    fields[2]=m->table_length;fields[3]=m->table_offset;
    for(unsigned i=0;i<4;++i){position(r,(cs_code_field)(i+1u),m->code0.entry.data.offset+i*4u);
        if(fields[i]&1u)return fail(r,CS_CODE_CODE0_FORMAT);}
    position(r,CS_CODE_FIELD_TABLE_LENGTH,m->code0.entry.data.offset+8u);
    if(!m->table_length || (m->table_length&7u))return fail(r,CS_CODE_CODE0_FORMAT);
    position(r,CS_CODE_FIELD_TABLE_OFFSET,m->code0.entry.data.offset+12u);
    if(m->table_offset<32 || m->table_offset>0x7FFEu)return fail(r,CS_CODE_CODE0_FORMAT);
    uint32_t end;
    if(!add(m->table_offset,m->table_length,&end) || end>m->above_a5)return fail(r,CS_CODE_CODE0_FORMAT);
    position(r,CS_CODE_FIELD_TABLE_LENGTH,m->code0.entry.data.offset+8u);
    if(m->table_length/8u>CS_CODE_ENTRIES)return fail(r,CS_CODE_PROFILE_LIMIT);
    uint32_t span;
    if(!add(16,m->table_length,&span) || span>m->code0.entry.data.length)return fail(r,CS_CODE_CODE0_FORMAT);
    m->entry_count=m->table_length/8u;m->warnings=0;
    if(m->table_offset!=32)m->warnings|=CS_CODE_NONCANONICAL_JT_OFFSET;
    if(end<m->above_a5)m->warnings|=CS_CODE_WORLD_SLACK;
    if(span<m->code0.entry.data.length)m->warnings|=CS_CODE_TRAILING_BYTES;
    return CS_CODE_OK;
}
cs_code_result cs_code_inspect(const cs_code_inspect_request *q,cs_code_workspace *w,
    size_t capacity,cs_code_manifest *out,cs_code_report *r)
{
    if(!OBJECT(q,cs_code_inspect_request) || !OBJECT(out,cs_code_manifest))return CS_CODE_ARGUMENT;
    if(!envelope(&q->context,(blob){q,sizeof *q},w,capacity,(blob){out,sizeof *out},r,
        (blob){q->source,q->length},(blob){0,0}))return CS_CODE_ARGUMENT;
    initialize(r,CS_CODE_INSPECT);
    if(!inactive(&q->context))return fail(r,CS_CODE_CONTEXT);
    if(!q->maximum_source)return fail(r,CS_CODE_ARGUMENT);
    if(q->length>UINT32_MAX || q->length>q->maximum_source)return fail(r,CS_CODE_PROFILE_LIMIT);
    clear(w,sizeof *w);cs_code_manifest *m=&w->manifest;
    cs_rf_result rf=cs_rf_open(q->source,q->length,&w->view,&w->reader);
    if(rf!=CS_RF_ARGUMENT){r->reader_valid=1;copy(&r->reader,&w->reader,sizeof r->reader);}
    if(rf!=CS_RF_OK)return fail(r,CS_CODE_SOURCE);
    cs_code_result result=find(w,0,&m->code0,r);if(result!=CS_CODE_OK)return result;
    position(r,CS_CODE_FIELD_ABOVE,m->code0.entry.data.offset);
    if(m->code0.entry.data.length<16)return fail(r,CS_CODE_CODE0_FORMAT);
    const uint8_t *b=q->source+m->code0.entry.data.offset;
    m->above_a5=be32(b);m->below_a5=be32(b+4);m->table_length=be32(b+8);m->table_offset=be32(b+12);
    result=header(m,r);if(result!=CS_CODE_OK)return result;
    for(uint32_t i=0;i<m->entry_count;++i){uint32_t at=m->code0.entry.data.offset+16u+i*8u;
        b=q->source+at;r->slot_valid=1;r->slot=i;position(r,CS_CODE_FIELD_ENTRY,at);
        uint32_t id=be16(b+4);
        if(be16(b+2)!=0x3F3C || be16(b+6)!=0xA9F0 || !id || id>32767 || (!i && id!=1))return fail(r,CS_CODE_ENTRY_FORM);
        uint32_t s=0;
        while(s<m->segment_count && m->segments[s].resource.entry.id!=(int32_t)id)++s;
        if(s==m->segment_count){if(s==CS_CODE_SEGMENTS)return fail(r,CS_CODE_PROFILE_LIMIT);
            m->segments[s].resource.entry.id=(int32_t)id;++m->segment_count;}
        m->entries[i].routine_offset=be16(b);m->entries[i].segment_index=s;
    }
    r->slot_valid=0;
    for(uint32_t s=0;s<m->segment_count;++s){cs_code_segment *segment=&m->segments[s];
        result=find(w,segment->resource.entry.id,&segment->resource,r);if(result!=CS_CODE_OK)return result;
        position(r,CS_CODE_FIELD_SEGMENT,segment->resource.entry.data.offset);
        uint32_t length=segment->resource.entry.data.length;
        if(length>32768)return fail(r,CS_CODE_PROFILE_LIMIT);
        if(length<4)return fail(r,CS_CODE_SEGMENT_HEADER);
        b=q->source+segment->resource.entry.data.offset;
        uint32_t offset=be16(b),count=be16(b+2),end;
        if((offset&7u) || !count || count>CS_CODE_ENTRIES || !add(offset,count*8u,&end) || end>m->table_length)
            return fail(r,CS_CODE_SEGMENT_HEADER);
        segment->first_entry=offset/8u;segment->entry_count=count;
        for(uint32_t i=0;i<m->entry_count;++i){int covered=i>=segment->first_entry && i-segment->first_entry<count;
            r->slot_valid=1;r->slot=i;
            if((m->entries[i].segment_index==s)!=covered)return fail(r,CS_CODE_SEGMENT_HEADER);
        }
        for(uint32_t i=segment->first_entry;i<end/8u;++i){uint32_t routine=m->entries[i].routine_offset;
            r->slot_valid=1;r->slot=i;position(r,CS_CODE_FIELD_ROUTINE,m->code0.entry.data.offset+16u+i*8u);
            if((routine&1u) || routine<4 || routine>length || length-routine<2)return fail(r,CS_CODE_ROUTINE_RANGE);
        }
        r->slot_valid=0;
    }
    m->tag=MANIFEST_TAG;m->source_length=(uint32_t)q->length;m->maximum_source=q->maximum_source;
    m->offset_policy=CS_CODE_RESOURCE_START_LITERAL;m->historical_offset_parity_unverified=1;
    copy(out,m,sizeof *out);r->cause=CS_CODE_OK;return CS_CODE_OK;
}
static int resource_valid(const cs_code_resource *s,uint32_t length,int32_t id)
{
    const cs_rf_entry *e=&s->entry;
    return e->type==CS_CODE_TYPE && e->id==id && !(e->attributes&~255u) && !(e->attributes&0xC0u) &&
        e->name_present<=1 && e->reference<=length && length-e->reference>=12 &&
        e->data.offset<=length && e->data.length<=length-e->data.offset &&
        e->name.offset<=length && e->name.length<=length-e->name.offset;
}
/* Not authentication: reject malformed scalar/count/index protocol data before
 * use. Do not reinterpret this as a current source lookup or resource lease. */
static int snapshot_valid(const cs_code_manifest *m)
{
    if(m->tag!=MANIFEST_TAG || !m->maximum_source || m->source_length>m->maximum_source ||
        !m->entry_count || m->entry_count>CS_CODE_ENTRIES || !m->segment_count || m->segment_count>CS_CODE_SEGMENTS ||
        m->offset_policy!=CS_CODE_RESOURCE_START_LITERAL || m->historical_offset_parity_unverified!=1 || m->execution_authorized ||
        !resource_valid(&m->code0,m->source_length,0) || m->code0.entry.data.length<16)return 0;
    uint32_t end,span;
    if((m->above_a5|m->below_a5|m->table_length|m->table_offset)&1u ||
        m->table_length!=m->entry_count*8u || m->table_offset<32 || m->table_offset>0x7FFEu ||
        !add(m->table_offset,m->table_length,&end) || end>m->above_a5 ||
        !add(16,m->table_length,&span) || span>m->code0.entry.data.length)return 0;
    uint32_t warnings=(m->table_offset!=32?CS_CODE_NONCANONICAL_JT_OFFSET:0u)|
        (end<m->above_a5?CS_CODE_WORLD_SLACK:0u)|(span<m->code0.entry.data.length?CS_CODE_TRAILING_BYTES:0u);
    if(m->warnings!=warnings || m->segments[0].resource.entry.id!=1)return 0;
    uint32_t next=0;
    for(uint32_t i=0;i<m->entry_count;++i){uint32_t s=m->entries[i].segment_index;
        if(s>=m->segment_count || s>next)return 0;
        if(s==next)++next;
    }
    if(next!=m->segment_count)return 0;
    for(uint32_t s=0;s<m->segment_count;++s){const cs_code_segment *segment=&m->segments[s];
        int32_t id=segment->resource.entry.id;uint32_t length=segment->resource.entry.data.length;
        if(id<1 || id>32767 || !resource_valid(&segment->resource,m->source_length,id) ||
            length<6 || length>32768 || !segment->entry_count || segment->entry_count>m->entry_count ||
            segment->first_entry>m->entry_count-segment->entry_count)return 0;
        for(uint32_t j=0;j<s;++j)if(m->segments[j].resource.entry.id==id)return 0;
        for(uint32_t i=0;i<m->entry_count;++i){int covered=i>=segment->first_entry && i-segment->first_entry<segment->entry_count;
            if((m->entries[i].segment_index==s)!=covered)return 0;
            if(covered){uint32_t ro=m->entries[i].routine_offset;
                if((ro&1u) || ro<4 || ro>length || length-ro<2)return 0;}
        }
    }
    return 1;
}
/* Compare backing pieces before asking for permission-bearing windows. This
 * keeps arithmetic/alias refusal ahead of missing/readonly map-window failures,
 * and also sees aliases when a logical span crosses a region boundary. */
static int backing_alias(const cs_guest_map *map,uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{
    for(size_t i=0;i<map->count;++i){const cs_guest_region *x=&map->regions[i];uint32_t xe;
        if(!add(x->base,x->size,&xe))continue;
        uint32_t lo=a>x->base?a:x->base,hi=a+n<xe?a+n:xe;
        if(lo>=hi)continue;
        for(size_t j=0;j<map->count;++j){const cs_guest_region *y=&map->regions[j];uint32_t ye;
            if(!add(y->base,y->size,&ye))continue;
            uint32_t bl=b>y->base?b:y->base,bh=b+m<ye?b+m:ye;
            if(bl>=bh)continue;
            blob xs[2],ys[2];xs[0]=(blob){x->bytes+(lo-x->base),hi-lo};
            xs[1]=(blob){x->writable?x->writable+(lo-x->base):0,x->writable?hi-lo:0};
            ys[0]=(blob){y->bytes+(bl-y->base),bh-bl};
            ys[1]=(blob){y->writable?y->writable+(bl-y->base):0,y->writable?bh-bl:0};
            for(unsigned k=0;k<2;++k)for(unsigned l=0;l<2;++l)if(overlap(xs[k],ys[l]))return 1;
        }
    }
    return 0;
}
cs_code_result cs_code_check_layout(const cs_code_layout_request *q,cs_code_workspace *w,
    size_t capacity,cs_code_layout *out,cs_code_report *r)
{
    if(!OBJECT(q,cs_code_layout_request) || !OBJECT(out,cs_code_layout) ||
        !OBJECT(q->manifest,cs_code_manifest) || !q->context.lifecycle)return CS_CODE_ARGUMENT;
    if(!envelope(&q->context,(blob){q,sizeof *q},w,capacity,(blob){out,sizeof *out},r,
        (blob){0,0},(blob){q->manifest,sizeof *q->manifest}))return CS_CODE_ARGUMENT;
    initialize(r,CS_CODE_CHECK_LAYOUT);
    if(!snapshot_valid(q->manifest))return fail(r,CS_CODE_ARGUMENT);
    if(!inactive(&q->context))return fail(r,CS_CODE_CONTEXT);
    const cs_life_controller *life=q->context.lifecycle;
    if(life->manager->map!=q->context.map)return fail(r,CS_CODE_STATE);
    r->lifecycle=cs_life_admit(life);r->lifecycle_valid=1;
    if(r->lifecycle!=CS_LIFE_OK)return fail(r,CS_CODE_STATE);
    clear(&w->layout,sizeof w->layout);cs_code_layout *l=&w->layout;const cs_code_manifest *m=q->manifest;
    const cs_mm_config *config=&life->manager->config;const cs_guest_map *map=q->context.map;
    uint32_t app_cells,sys_cells,minimum,parameter_end;
    r->field=CS_CODE_FIELD_LAYOUT;
    if(!q->s || ((q->b|q->s|q->h_min)&1u) ||
        !subtract(q->b,m->above_a5,&l->a5) || !subtract(l->a5,m->below_a5,&l->globals_begin) ||
        !subtract(l->globals_begin,q->s,&l->stack_base) || !add(l->a5,m->table_offset,&l->table_begin) ||
        !add(l->table_begin,m->table_length,&l->table_end) ||
        !add(config->application.begin,CS_MM_CELL_BYTES,&app_cells) ||
        !add(config->system.begin,CS_MM_CELL_BYTES,&sys_cells) ||
        !add(config->application.begin,q->h_min,&minimum) || !add(l->a5,32,&parameter_end))return fail(r,CS_CODE_LAYOUT);
    l->stack_top=l->globals_begin;
    l->candidate_appl_limit=config->application.end<l->stack_base?config->application.end:l->stack_base;
    if(!q->b || q->b>0x1000000 || l->stack_base<0x400 || l->table_end>q->b ||
        l->table_begin<parameter_end || ((l->a5|l->globals_begin|l->stack_base|l->table_begin|l->table_end)&1u))return fail(r,CS_CODE_LAYOUT);
    if(intersects(l->stack_base,q->s,config->system.begin,config->system.end-config->system.begin) ||
        intersects(l->stack_base,q->s,config->application.begin,config->application.end-config->application.begin))
        return fail(r,CS_CODE_LAYOUT_UNSUPPORTED);
    if(app_cells>l->candidate_appl_limit || minimum>l->candidate_appl_limit ||
        sys_cells>config->system.end)return fail(r,CS_CODE_LAYOUT);
    uint32_t begin[4],length[4];begin[0]=config->application.begin;length[0]=l->candidate_appl_limit-begin[0];
    begin[1]=config->system.begin;length[1]=config->system.end-begin[1];
    begin[2]=l->stack_base;length[2]=q->s;begin[3]=l->globals_begin;length[3]=q->b-l->globals_begin;
    for(unsigned i=0;i<4;++i){if(begin[i]<0x400 || !length[i] || begin[i]>=0x1000000 || length[i]>0x1000000-begin[i])return fail(r,CS_CODE_LAYOUT);
        for(unsigned j=0;j<i;++j)if(intersects(begin[i],length[i],begin[j],length[j]) ||
            backing_alias(map,begin[i],length[i],begin[j],length[j]))return fail(r,CS_CODE_LAYOUT);
        /* Both cell bands are covered by their containing zones. Low-memory
         * aliases need a separate physical check as it is outside those zones. */
        if(backing_alias(map,begin[i],length[i],0,0x400))return fail(r,CS_CODE_LAYOUT);
    }
    if(m->below_a5 && backing_alias(map,l->globals_begin,m->below_a5,l->a5,m->above_a5))return fail(r,CS_CODE_LAYOUT);
    cs_guest_view view;
    /* Above and below may be separate valid windows. Do not require an extra
     * combined world window spanning their descriptor boundary. */
    for(unsigned i=0;i<3;++i)if(cs_guest_window(map,begin[i],length[i],CS_GUEST_READ|CS_GUEST_WRITE,&view)!=CS_GUEST_OK)
        return fail(r,CS_CODE_LAYOUT);
    if(cs_guest_window(map,l->a5,32,CS_GUEST_READ|CS_GUEST_WRITE,&view)!=CS_GUEST_OK ||
        cs_guest_window(map,l->a5,m->above_a5,CS_GUEST_READ|CS_GUEST_WRITE,&view)!=CS_GUEST_OK ||
        (m->below_a5 && cs_guest_window(map,l->globals_begin,m->below_a5,CS_GUEST_READ|CS_GUEST_WRITE,&view)!=CS_GUEST_OK) ||
        cs_guest_window(map,l->table_begin,m->table_length,CS_GUEST_READ|CS_GUEST_WRITE,&view)!=CS_GUEST_OK)return fail(r,CS_CODE_LAYOUT);
    l->tag=LAYOUT_TAG;l->b=q->b;l->s=q->s;l->h_min=q->h_min;l->heap_begin=config->application.begin;
    l->heap_epoch=life->epoch;l->epoch_observed=1;l->warnings=m->warnings;
    copy(out,l,sizeof *out);r->cause=CS_CODE_OK;return CS_CODE_OK;
}
