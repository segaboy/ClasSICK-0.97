/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "code-stream.h"
typedef struct {uintptr_t at;size_t size;} native_span;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define STREAM_ALIGN(type) _Alignof(type)
#else
#define STREAM_ALIGN_TYPE(type) typedef struct {uint8_t byte;type value;} stream_align_##type
STREAM_ALIGN_TYPE(cs_cstream_request);STREAM_ALIGN_TYPE(cs_cstream_workspace);
STREAM_ALIGN_TYPE(cs_cstream_manifest);STREAM_ALIGN_TYPE(cs_cstream_report);STREAM_ALIGN_TYPE(cs_rfs_view);
#define STREAM_ALIGN(type) offsetof(stream_align_##type,value)
#endif
static int span(native_span *s,const void *p,size_t bytes,size_t alignment)
{
    uintptr_t at=(uintptr_t)p;
    if(p==NULL || bytes==0 || bytes>UINTPTR_MAX-at || alignment==0 ||
        (alignment&(alignment-1u))!=0 || (at&(alignment-1u))!=0)return 0;
    s->at=at;s->size=bytes;return 1;
}
static int envelope(const cs_cstream_request *q,cs_cstream_workspace *w,size_t capacity,
    cs_cstream_manifest *out,cs_cstream_report *r)
{
    native_span s[5];unsigned i,j;
    if(capacity<sizeof *w || capacity>CS_RFS_NATIVE_LIMIT ||
        !span(&s[0],q,sizeof *q,STREAM_ALIGN(cs_cstream_request)) ||
        !span(&s[1],w,capacity,STREAM_ALIGN(cs_cstream_workspace)) ||
        !span(&s[2],out,sizeof *out,STREAM_ALIGN(cs_cstream_manifest)) ||
        !span(&s[3],r,sizeof *r,STREAM_ALIGN(cs_cstream_report)) ||
        !span(&s[4],q->source,sizeof *q->source,STREAM_ALIGN(cs_rfs_view)))return 0;
    for(i=0;i<5u;++i)for(j=0;j<i;++j)
        if(s[i].at<s[j].at+s[j].size && s[j].at<s[i].at+s[i].size)return 0;
    return 1;
}
static void clear(void *object,size_t count)
{uint8_t *p=object;size_t i;for(i=0;i<count;++i)p[i]=0;}
static void copy(void *out,const void *input,size_t count)
{uint8_t *d=out;const uint8_t *s=input;size_t i;for(i=0;i<count;++i)d[i]=s[i];}
static uint32_t be16(const uint8_t *p){return ((uint32_t)p[0]<<8)|p[1];}
static uint32_t be32(const uint8_t *p){return (be16(p)<<16)|be16(p+2);}
static cs_cstream_range_reason routine_range(uint32_t routine,uint32_t length,cs_cstream_base base)
{
    /* The table supplies a 16-bit word. The extra header therefore fits in uint32_t. */
    uint32_t address=routine+(base==CS_CSTREAM_AFTER_HEADER?4u:0u);
    if(address&1u)return CS_CSTREAM_RANGE_ODD;
    if(address<4u)return CS_CSTREAM_RANGE_HEADER;
    if(address>length)return CS_CSTREAM_RANGE_BEYOND;
    if(length-address<2u)return CS_CSTREAM_RANGE_SHORT_TAIL;
    return CS_CSTREAM_RANGE_NONE;
}
static cs_cstream_result fail(cs_cstream_report *r,cs_cstream_result result)
{r->cause=result;return result;}
static void position(cs_cstream_report *r,cs_cstream_field field,uint32_t at)
{r->field=field;r->position_valid=1;r->position=at;}
static cs_cstream_result reader_result(cs_cstream_report *r,cs_rfs_result result,
    const cs_rfs_report *detail)
{
    if(result!=CS_RFS_ARGUMENT){r->reader_valid=1;copy(&r->reader,detail,sizeof *detail);}
    if(result==CS_RFS_OK)return CS_CSTREAM_OK;
    if(result==CS_RFS_NOT_FOUND)return fail(r,CS_CSTREAM_NOT_FOUND);
    if(result==CS_RFS_DUPLICATE_TYPE || result==CS_RFS_DUPLICATE_ID)return fail(r,CS_CSTREAM_AMBIGUOUS);
    if(result==CS_RFS_REFERENCE)return fail(r,CS_CSTREAM_REFERENCE);
    return fail(r,CS_CSTREAM_SOURCE);
}
static cs_cstream_result find(cs_cstream_workspace *w,int32_t id,
    cs_cstream_resource *out,cs_cstream_report *r)
{
    cs_rf_entry entry;cs_rfs_report detail;cs_rfs_result result;cs_cstream_result status;
    r->key_valid=1;r->id=id;r->position_valid=0;r->reader_valid=0;r->field=CS_CSTREAM_FIELD_NONE;
    result=cs_rfs_find(&w->candidate.source,&w->resource,CS_CSTREAM_TYPE,id,&entry,&detail);
    status=reader_result(r,result,&detail);if(status!=CS_CSTREAM_OK)return status;
    if((entry.attributes&0x40u)!=0)return fail(r,CS_CSTREAM_STORAGE);
    copy(&out->entry,&entry,sizeof entry);out->reader_warnings=detail.warnings;
    w->candidate.reader_warnings|=detail.warnings;return CS_CSTREAM_OK;
}
static cs_cstream_result get(cs_cstream_workspace *w,const cs_rf_entry *entry,
    uint32_t offset,size_t count,cs_cstream_report *r)
{
    cs_rfs_report detail;cs_rfs_result result;cs_cstream_result status;
    r->reader_valid=0;
    result=cs_rfs_read(&w->candidate.source,entry,CS_RFS_PAYLOAD,offset,count,
        w->bytes,sizeof w->bytes,w->staging,sizeof w->staging,&detail);
    status=reader_result(r,result,&detail);
    if(status!=CS_CSTREAM_OK && r->reader_valid){r->position=detail.position;r->position_valid=1;}
    return status;
}
static cs_cstream_result header(cs_cstream_manifest *m,cs_cstream_report *r)
{
    uint32_t fields[4],end,required;unsigned i;
    fields[0]=m->above_a5;fields[1]=m->below_a5;fields[2]=m->table_length;fields[3]=m->table_offset;
    for(i=0;i<4u;++i){position(r,(cs_cstream_field)(i+1u),m->code0.entry.data.offset+4u*i);
        if((fields[i]&1u)!=0)return fail(r,CS_CSTREAM_CODE0_FORMAT);}
    position(r,CS_CSTREAM_FIELD_TABLE_LENGTH,m->code0.entry.data.offset+8u);
    if(m->table_length==0 || (m->table_length&7u)!=0)return fail(r,CS_CSTREAM_CODE0_FORMAT);
    position(r,CS_CSTREAM_FIELD_TABLE_OFFSET,m->code0.entry.data.offset+12u);
    if(m->table_offset<32u || m->table_offset>0x7ffeu || m->table_length>UINT32_MAX-m->table_offset)
        return fail(r,CS_CSTREAM_CODE0_FORMAT);
    end=m->table_offset+m->table_length;
    if(end>m->above_a5)return fail(r,CS_CSTREAM_CODE0_FORMAT);
    position(r,CS_CSTREAM_FIELD_TABLE_LENGTH,m->code0.entry.data.offset+8u);
    m->entry_count=m->table_length>>3;
    if(m->entry_count>m->maximum_entries)return fail(r,CS_CSTREAM_PROFILE_LIMIT);
    if(m->table_length>UINT32_MAX-16u)return fail(r,CS_CSTREAM_CODE0_FORMAT);
    required=16u+m->table_length;
    if(required>m->code0.entry.data.length)return fail(r,CS_CSTREAM_CODE0_FORMAT);
    if(m->table_offset!=32u)m->warnings|=CS_CSTREAM_JT_OFFSET;
    if(end<m->above_a5)m->warnings|=CS_CSTREAM_WORLD_SLACK;
    if(required<m->code0.entry.data.length)m->warnings|=CS_CSTREAM_TRAILING;
    return CS_CSTREAM_OK;
}
cs_cstream_result cs_code_stream_inspect(const cs_cstream_request *q,
    cs_cstream_workspace *w,size_t capacity,cs_cstream_manifest *out,cs_cstream_report *r)
{
    cs_cstream_manifest *m;cs_cstream_result result;uint32_t slot,s;
    if(!envelope(q,w,capacity,out,r))return CS_CSTREAM_ARGUMENT;
    clear(r,sizeof *r);
    if(q->context!=CS_CSTREAM_ORDINARY || q->active_root || q->active_callback || q->active_patch || q->active_manager)
        return fail(r,CS_CSTREAM_CONTEXT);
    if((q->routine_base!=CS_CSTREAM_RESOURCE_START && q->routine_base!=CS_CSTREAM_AFTER_HEADER) ||
        q->maximum_source==0 || q->maximum_entries==0 || q->maximum_segments==0 || q->maximum_segments>CS_CSTREAM_SEGMENTS)
        return fail(r,CS_CSTREAM_ARGUMENT);
    if(q->source->source.length>q->maximum_source)return fail(r,CS_CSTREAM_PROFILE_LIMIT);
    clear(w,sizeof *w);m=&w->candidate;copy(&m->source,q->source,sizeof m->source);
    m->maximum_source=q->maximum_source;m->maximum_entries=q->maximum_entries;
    m->maximum_segments=q->maximum_segments;
    result=find(w,0,&m->code0,r);if(result!=CS_CSTREAM_OK)return result;
    position(r,CS_CSTREAM_FIELD_ABOVE,m->code0.entry.data.offset);
    if(m->code0.entry.data.length<16u)return fail(r,CS_CSTREAM_CODE0_FORMAT);
    result=get(w,&m->code0.entry,0,16,r);if(result!=CS_CSTREAM_OK)return result;
    m->above_a5=be32(w->bytes);m->below_a5=be32(w->bytes+4);
    m->table_length=be32(w->bytes+8);m->table_offset=be32(w->bytes+12);
    result=header(m,r);if(result!=CS_CSTREAM_OK)return result;
    /* First pass preserves SPEC0032's entry-form/ID/capacity precedence before
     * any segment lookup. No array indexed by total table length is needed. */
    slot=0;
    while(slot<m->entry_count) {
        uint32_t rows=m->entry_count-slot,j;
        if(rows>CS_CSTREAM_CHUNK/8u)rows=CS_CSTREAM_CHUNK/8u;
        r->slot_valid=0;position(r,CS_CSTREAM_FIELD_ENTRY,m->code0.entry.data.offset+16u+(slot<<3));
        result=get(w,&m->code0.entry,16u+(slot<<3),(size_t)(rows<<3),r);
        if(result!=CS_CSTREAM_OK)return result;
        for(j=0;j<rows;++j) {
            const uint8_t *b=w->bytes+(size_t)(j<<3);uint32_t id=be16(b+4),at=slot+j;
            r->slot_valid=1;r->slot=at;
            position(r,CS_CSTREAM_FIELD_ENTRY,m->code0.entry.data.offset+16u+(at<<3));
            if(be16(b+2)!=0x3f3cu || be16(b+6)!=0xa9f0u || id==0 || id>32767u || (at==0 && id!=1))
                return fail(r,CS_CSTREAM_ENTRY_FORM);
            s=0;while(s<m->segment_count && m->segments[s].resource.entry.id!=(int32_t)id)++s;
            if(s==m->segment_count) {
                if(s==m->maximum_segments)return fail(r,CS_CSTREAM_PROFILE_LIMIT);
                m->segments[s].resource.entry.id=(int32_t)id;w->first[s]=at;++m->segment_count;
            }
            w->last[s]=at;++w->seen[s];++r->entries_checked;r->segments_seen=m->segment_count;
        }
        slot+=rows;
    }
    r->slot_valid=0;
    for(s=0;s<m->segment_count;++s) {
        cs_cstream_segment *segment=&m->segments[s];uint32_t length,offset,count,end;
        result=find(w,segment->resource.entry.id,&segment->resource,r);if(result!=CS_CSTREAM_OK)return result;
        position(r,CS_CSTREAM_FIELD_SEGMENT,segment->resource.entry.data.offset);
        length=segment->resource.entry.data.length;
        if(length>32768u)return fail(r,CS_CSTREAM_PROFILE_LIMIT);
        if(length<4u)return fail(r,CS_CSTREAM_SEGMENT_HEADER);
        result=get(w,&segment->resource.entry,0,4,r);if(result!=CS_CSTREAM_OK)return result;
        offset=be16(w->bytes);count=be16(w->bytes+2);
        end=offset+(count<<3); /* Both operands originated in uint16 fields. */
        if((offset&7u)!=0 || count==0 || end>m->table_length)
            return fail(r,CS_CSTREAM_SEGMENT_HEADER);
        segment->first_entry=offset>>3;segment->entry_count=count;
        if(segment->first_entry!=w->first[s] || count!=w->seen[s] ||
            w->last[s]-w->first[s]+1u!=count)return fail(r,CS_CSTREAM_SEGMENT_HEADER);
        slot=segment->first_entry;
        while(slot<(end>>3)) {
            uint32_t rows=(end>>3)-slot,j;
            if(rows>CS_CSTREAM_CHUNK/8u)rows=CS_CSTREAM_CHUNK/8u;
            r->slot_valid=0;position(r,CS_CSTREAM_FIELD_ROUTINE,m->code0.entry.data.offset+16u+(slot<<3));
            result=get(w,&m->code0.entry,16u+(slot<<3),(size_t)(rows<<3),r);
            if(result!=CS_CSTREAM_OK)return result;
            for(j=0;j<rows;++j) {
                uint32_t routine=be16(w->bytes+(size_t)(j<<3));r->slot_valid=1;r->slot=slot+j;
                position(r,CS_CSTREAM_FIELD_ROUTINE,m->code0.entry.data.offset+16u+((slot+j)<<3));
                r->range_reason=routine_range(routine,length,q->routine_base);
                if(r->range_reason!=CS_CSTREAM_RANGE_NONE)
                    return fail(r,CS_CSTREAM_ROUTINE_RANGE);
            }
            slot+=rows;
        }
        r->slot_valid=0;
    }
    m->tag=UINT32_C(0x43534d32);m->routine_base=q->routine_base;
    m->resource_start_literal=(q->routine_base==CS_CSTREAM_RESOURCE_START);
    m->historical_offset_parity_unverified=1;
    copy(out,m,sizeof *out);return fail(r,CS_CSTREAM_OK);
}
