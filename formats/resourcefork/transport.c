/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "transport.h"
#define RFS_TAG UINT32_C(0x43525331)
static uint32_t u16(const uint8_t *p){return ((uint32_t)p[0]<<8)|p[1];}
static uint32_t u24(const uint8_t *p){return ((uint32_t)p[0]<<16)|((uint32_t)p[1]<<8)|p[2];}
static uint32_t u32(const uint8_t *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static uint32_t count(uint32_t n){return n==65535u?0u:n+1u;}
static uint32_t extent(uint32_t n){return (n<<3)+(n<<2);}
static int32_t signed_id(uint32_t n){return n>=32768u?(int32_t)n-65536:(int32_t)n;}
static int overlaps(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n!=0 && m!=0 && (a<=b?b-a<n:a-b<m);}
static void copy(void *to,const void *from,size_t n)
{uint8_t *d=to;const uint8_t *s=from;size_t i;for(i=0;i<n;++i)d[i]=s[i];}
static void zero(void *to,size_t n){uint8_t *d=to;size_t i;for(i=0;i<n;++i)d[i]=0;}
static cs_rfs_result done(cs_rfs_report *r,cs_rfs_result status,cs_rf_field field,uint32_t position)
{r->status=status;r->field=field;r->position=position;return status;}
static void begin(cs_rfs_report *r,uint32_t warnings)
{r->status=CS_RFS_OK;r->field=CS_RF_FIELD_NONE;r->position=0;r->warnings=warnings;r->types=0;r->references=0;}
static cs_rfs_result end(uint32_t base,uint32_t size,uint32_t limit,uint32_t *out,
    cs_rfs_report *r,cs_rf_field field,uint32_t position,cs_rfs_result bounded)
{
    if(size>UINT32_MAX-base)return done(r,CS_RFS_OVERFLOW,field,position);
    *out=base+size;if(*out>limit)return done(r,bounded,field,position);return CS_RFS_OK;
}
static cs_rfs_result io_read(const cs_rfs_source *s,uint32_t at,uint16_t n,
    uint8_t *to,cs_rfs_report *r,cs_rf_field field)
{
    cs_rfs_io result;
    if(n==0 || n>CS_RFS_CHUNK || at>s->length || (uint32_t)n>s->length-at)
        return done(r,CS_RFS_OUT_OF_RANGE,field,at);
    result=s->read(s->context,at,n,to);
    if(result==CS_RFS_IO_OK)return CS_RFS_OK;
    return done(r,result==CS_RFS_IO_UNAVAILABLE?CS_RFS_UNAVAILABLE:
        result==CS_RFS_IO_ERROR?CS_RFS_IO:CS_RFS_PROVIDER,field,at);
}
static cs_rfs_result fetch(const cs_rfs_source *s,cs_rfs_workspace *w,uint32_t at,
    unsigned n,uint8_t *to,cs_rfs_report *r,cs_rf_field field)
{
    unsigned copied=0;
    if(at>s->length || (uint32_t)n>s->length-at)return done(r,CS_RFS_TRUNCATED,field,at);
    while(copied<n) {
        uint32_t position=at+copied;unsigned inside,take,i;
        if(w->count==0 || position<w->offset || position-w->offset>=w->count) {
            uint32_t available;uint16_t amount;cs_rfs_result result;
            w->count=0;w->offset=(position>>8)<<8;available=s->length-w->offset;
            amount=(uint16_t)(available>CS_RFS_CHUNK?CS_RFS_CHUNK:available);
            result=io_read(s,w->offset,amount,w->bytes,r,field);
            if(result!=CS_RFS_OK)return result;
            w->count=amount;
        }
        inside=(unsigned)(position-w->offset);take=(unsigned)w->count-inside;
        if(take>n-copied)take=n-copied;
        for(i=0;i<take;++i)to[copied+i]=w->bytes[inside+i];
        copied+=take;
    }
    return CS_RFS_OK;
}
#define FETCH(s,w,at,n,b,r,field) do { cs_rfs_result fetched=fetch((s),(w),(at),(n),(b),(r),(field)); if(fetched!=CS_RFS_OK)return fetched; } while(0)

static cs_rfs_result record(const cs_rfs_view *v,cs_rfs_workspace *w,uint32_t at,
    cs_rf_entry *e,cs_rfs_report *r)
{
    uint8_t b[12],prefix_bytes[4];uint32_t map_end=v->map.offset+v->map.length,finish,name;
    cs_rfs_result result;
    FETCH(&v->source,w,at,12,b,r,CS_RF_FIELD_REFERENCES);
    name=u16(b+2);e->reference=at;e->attributes=b[4];e->name_present=name!=65535u;
    e->name.offset=0;e->name.length=0;e->data.offset=0;e->data.length=0;e->specification=0;
    if(e->name_present) {
        result=end(v->name_list,name,map_end,&finish,r,CS_RF_FIELD_NAME,at+2u,CS_RFS_NAME);
        if(result!=CS_RFS_OK)return result;
        if(finish==map_end)return done(r,CS_RFS_NAME,CS_RF_FIELD_NAME,at+2u);
        FETCH(&v->source,w,finish,1,prefix_bytes,r,CS_RF_FIELD_NAME);
        e->name.offset=finish+1u;e->name.length=prefix_bytes[0];
        result=end(e->name.offset,e->name.length,map_end,&finish,r,CS_RF_FIELD_NAME,at+2u,CS_RFS_NAME);
        if(result!=CS_RFS_OK)return result;
    }
    if((e->attributes&3u)!=0)r->warnings|=CS_RF_WARN_ATTRIBUTES;
    if((e->attributes&128u)!=0) {
        e->specification=u32(b+8);r->warnings|=CS_RF_WARN_REFERENCE;return CS_RFS_REFERENCE;
    }
    if(u32(b+8)!=0)r->warnings|=CS_RF_WARN_HANDLE;
    {
        uint32_t data_end=v->data.offset+v->data.length,prefix;
        result=end(v->data.offset,u24(b+5),data_end,&finish,r,CS_RF_FIELD_PAYLOAD,at+5u,CS_RFS_OUT_OF_RANGE);
        if(result!=CS_RFS_OK)return result;
        prefix=finish;
        result=end(prefix,4,data_end,&finish,r,CS_RF_FIELD_PAYLOAD,at+5u,CS_RFS_OUT_OF_RANGE);
        if(result!=CS_RFS_OK)return result;
        FETCH(&v->source,w,prefix,4,prefix_bytes,r,CS_RF_FIELD_PAYLOAD);
        e->data.offset=finish;e->data.length=u32(prefix_bytes);
        result=end(e->data.offset,e->data.length,data_end,&finish,r,CS_RF_FIELD_PAYLOAD,prefix,CS_RFS_OUT_OF_RANGE);
        if(result!=CS_RFS_OK)return result;
    }
    if(e->data.length>32768u)r->warnings|=CS_RF_WARN_LARGE;
    return CS_RFS_OK;
}

cs_rfs_result cs_rfs_open(const cs_rfs_source *s,cs_rfs_workspace *w,
    cs_rfs_view *out,cs_rfs_report *r)
{
    cs_rfs_view v;uint8_t b[28];uint32_t data_end,map_end,table_end,types,names,previous,i;
    cs_rfs_result result;unsigned reserved_nonzero=0;
    if(s==NULL || w==NULL || out==NULL || r==NULL || s->read==NULL)return CS_RFS_ARGUMENT;
    begin(r,0);w->count=0;zero(&v,sizeof v);
    if(s->length<16)return done(r,CS_RFS_TRUNCATED,CS_RF_FIELD_HEADER,0);
    FETCH(s,w,0,16,b,r,CS_RF_FIELD_HEADER);
    v.source.length=s->length;v.source.read=s->read;v.source.context=s->context;v.tag=RFS_TAG;
    v.data.offset=u32(b);v.map.offset=u32(b+4);v.data.length=u32(b+8);v.map.length=u32(b+12);
    result=end(v.data.offset,v.data.length,s->length,&data_end,r,CS_RF_FIELD_DATA,8,CS_RFS_TRUNCATED);
    if(result!=CS_RFS_OK)return result;
    result=end(v.map.offset,v.map.length,s->length,&map_end,r,CS_RF_FIELD_MAP,12,CS_RFS_TRUNCATED);
    if(result!=CS_RFS_OK)return result;
    if(overlaps(0,16,v.data.offset,v.data.length))return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_DATA,0);
    if(overlaps(0,16,v.map.offset,v.map.length) || overlaps(v.data.offset,v.data.length,v.map.offset,v.map.length))
        return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_MAP,4);
    if(v.map.length<30)return done(r,CS_RFS_OUT_OF_RANGE,CS_RF_FIELD_MAP,12);
    FETCH(s,w,v.map.offset,28,b,r,CS_RF_FIELD_MAP);
    types=u16(b+24);names=u16(b+26);
    if(types<28)return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_TYPE_LIST,v.map.offset+24u);
    result=end(types,2,v.map.length,&table_end,r,CS_RF_FIELD_TYPE_LIST,v.map.offset+24u,CS_RFS_OUT_OF_RANGE);
    if(result!=CS_RFS_OK)return result;
    if(names<28)return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_NAME_LIST,v.map.offset+26u);
    if(names>v.map.length)return done(r,CS_RFS_OUT_OF_RANGE,CS_RF_FIELD_NAME_LIST,v.map.offset+26u);
    for(i=0;i<22u;++i)if(b[(unsigned)i]!=0)reserved_nonzero=1;
    v.type_list=v.map.offset+types;v.name_list=v.map.offset+names;
    FETCH(s,w,v.type_list,2,b,r,CS_RF_FIELD_TYPE_LIST);
    v.type_count=count(u16(b));
    result=end(table_end,v.type_count<<3,v.map.length,&table_end,r,CS_RF_FIELD_TYPE_LIST,v.type_list,CS_RFS_OUT_OF_RANGE);
    if(result!=CS_RFS_OK)return result;
    if(table_end>names)return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_TYPE_LIST,v.type_list);
    table_end+=v.map.offset;
    if(v.data.offset!=256u || v.map.offset!=data_end || map_end!=s->length || types!=28u)r->warnings|=CS_RF_WARN_LAYOUT;
    if(reserved_nonzero)r->warnings|=CS_RF_WARN_MAP_RESERVED;
    previous=table_end;
    for(i=0;i<v.type_count;++i) {
        uint32_t at=v.type_list+2u+(i<<3),n,start,finish;++r->types;
        FETCH(s,w,at,8,b,r,CS_RF_FIELD_TYPE_LIST);n=count(u16(b+4));
        result=end(v.type_list,u16(b+6),map_end,&start,r,CS_RF_FIELD_REFERENCES,at+6u,CS_RFS_OUT_OF_RANGE);
        if(result!=CS_RFS_OK)return result;
        result=end(start,extent(n),map_end,&finish,r,CS_RF_FIELD_REFERENCES,at+4u,CS_RFS_OUT_OF_RANGE);
        if(result!=CS_RFS_OK)return result;
        if(start<previous)return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_REFERENCES,at+6u);
        if(finish>v.name_list)return done(r,CS_RFS_OVERLAP,CS_RF_FIELD_REFERENCES,at+4u);
        if(start!=previous)r->warnings|=CS_RF_WARN_LAYOUT;
        if(n==0)r->warnings|=CS_RF_WARN_EMPTY_LIST;
        previous=finish;
    }
    if(previous!=v.name_list)r->warnings|=CS_RF_WARN_LAYOUT;
    for(i=0;i<v.type_count;++i) {
        uint32_t at=v.type_list+2u+(i<<3),n,start,j;
        FETCH(s,w,at,8,b,r,CS_RF_FIELD_TYPE_LIST);n=count(u16(b+4));start=v.type_list+u16(b+6);
        for(j=0;j<n;++j) {
            cs_rf_entry e;++r->references;
            result=record(&v,w,start+extent(j),&e,r);
            if(result!=CS_RFS_OK && result!=CS_RFS_REFERENCE)return result;
        }
    }
    v.reserved.offset=s->length>=128u?16u:0u;v.reserved.length=s->length>=128u?112u:0u;
    v.application.offset=s->length>=256u?128u:0u;v.application.length=s->length>=256u?128u:0u;
    v.warnings=r->warnings;copy(out,&v,sizeof v);return done(r,CS_RFS_OK,CS_RF_FIELD_NONE,0);
}
static int valid(const cs_rfs_view *v)
{
    uint32_t first,last;
    if(v==NULL || v->tag!=RFS_TAG || v->source.read==NULL || v->source.length<16 ||
        v->data.offset>v->source.length || v->data.length>v->source.length-v->data.offset ||
        v->map.offset>v->source.length || v->map.length>v->source.length-v->map.offset ||
        v->map.length<30 || v->type_count>65535u)return 0;
    first=v->map.offset+28u;last=v->map.offset+v->map.length;
    return v->type_list>=first && v->type_list<=last-2u && v->name_list>=first && v->name_list<=last &&
        v->name_list>=v->type_list+2u && (v->type_count<<3)<=v->name_list-v->type_list-2u;
}
cs_rfs_result cs_rfs_find(const cs_rfs_view *v,cs_rfs_workspace *w,uint32_t type,
    int32_t id,cs_rf_entry *out,cs_rfs_report *r)
{
    uint32_t selected=0,matches=0,duplicate=0,i,n,start,chosen=0;uint8_t b[12];
    cs_rf_entry e;cs_rfs_result result;
    if(!valid(v) || w==NULL || out==NULL || r==NULL || id< -32768 || id>32767)return CS_RFS_ARGUMENT;
    begin(r,v->warnings);w->count=0;
    for(i=0;i<v->type_count;++i) {
        uint32_t at=v->type_list+2u+(i<<3);++r->types;
        FETCH(&v->source,w,at,8,b,r,CS_RF_FIELD_TYPE_KEY);
        if(u32(b)==type){if(matches==0)selected=at;if(matches==1)duplicate=at;++matches;}
    }
    if(matches==0)return done(r,CS_RFS_NOT_FOUND,CS_RF_FIELD_TYPE_KEY,0);
    if(matches>1)return done(r,CS_RFS_DUPLICATE_TYPE,CS_RF_FIELD_TYPE_KEY,duplicate);
    FETCH(&v->source,w,selected,8,b,r,CS_RF_FIELD_TYPE_KEY);n=count(u16(b+4));start=v->type_list+u16(b+6);
    matches=0;duplicate=0;
    for(i=0;i<n;++i) {
        uint32_t at=start+extent(i);++r->references;
        FETCH(&v->source,w,at,2,b,r,CS_RF_FIELD_ID);
        if(signed_id(u16(b))==id){if(matches==0)chosen=at;if(matches==1)duplicate=at;++matches;}
    }
    if(matches==0)return done(r,CS_RFS_NOT_FOUND,CS_RF_FIELD_ID,selected);
    if(matches>1)return done(r,CS_RFS_DUPLICATE_ID,CS_RF_FIELD_ID,duplicate);
    zero(&e,sizeof e);e.type=type;e.id=id;result=record(v,w,chosen,&e,r);
    if(result!=CS_RFS_OK && result!=CS_RFS_REFERENCE)return result;
    copy(out,&e,sizeof e);return done(r,result,CS_RF_FIELD_NONE,chosen);
}
cs_rfs_result cs_rfs_read(const cs_rfs_view *v,const cs_rf_entry *e,cs_rfs_part part,
    uint32_t offset,size_t n,uint8_t *output,size_t output_capacity,
    uint8_t *staging,size_t staging_capacity,cs_rfs_report *r)
{
    cs_rf_span span,region;cs_rf_field field;size_t written=0;cs_rfs_result result;
    if(!valid(v) || e==NULL || r==NULL || output==NULL || staging==NULL || output==staging ||
        n>CS_RFS_NATIVE_LIMIT || output_capacity>CS_RFS_NATIVE_LIMIT || staging_capacity>CS_RFS_NATIVE_LIMIT ||
        (part!=CS_RFS_PAYLOAD && part!=CS_RFS_RESOURCE_NAME))return CS_RFS_ARGUMENT;
    begin(r,v->warnings);field=part==CS_RFS_PAYLOAD?CS_RF_FIELD_PAYLOAD:CS_RF_FIELD_NAME;
    if(part==CS_RFS_PAYLOAD && (e->attributes&128u)!=0)return done(r,CS_RFS_REFERENCE,field,e->reference);
    if(part==CS_RFS_RESOURCE_NAME && !e->name_present)return done(r,CS_RFS_NOT_FOUND,field,e->reference);
    span=part==CS_RFS_PAYLOAD?e->data:e->name;region=part==CS_RFS_PAYLOAD?v->data:v->map;
    if(span.offset<region.offset || span.offset-region.offset>region.length ||
        span.length>region.length-(span.offset-region.offset))return done(r,CS_RFS_OUT_OF_RANGE,field,span.offset);
    if(offset>span.length || (uint32_t)n>span.length-offset)return done(r,CS_RFS_OUT_OF_RANGE,field,span.offset);
    if(output_capacity<n || staging_capacity<n)return done(r,CS_RFS_CAPACITY,field,span.offset);
    while(written<n) {
        uint16_t amount=(uint16_t)(n-written>CS_RFS_CHUNK?CS_RFS_CHUNK:n-written);
        result=io_read(&v->source,span.offset+offset+(uint32_t)written,amount,staging+written,r,field);
        if(result!=CS_RFS_OK)return result;
        written+=amount;
    }
    copy(output,staging,n);return done(r,CS_RFS_OK,CS_RF_FIELD_NONE,span.offset+offset);
}
