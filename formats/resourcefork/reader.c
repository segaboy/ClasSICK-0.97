/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "reader.h"
#define RF_TAG UINT32_C(0x43524631)
static uint32_t u16(const uint8_t *b,uint32_t p)
{return ((uint32_t)b[p]<<8)|b[p+1u];}
static uint32_t u24(const uint8_t *b,uint32_t p)
{return ((uint32_t)b[p]<<16)|((uint32_t)b[p+1u]<<8)|b[p+2u];}
static uint32_t u32(const uint8_t *b,uint32_t p)
{return ((uint32_t)b[p]<<24)|((uint32_t)b[p+1u]<<16)|((uint32_t)b[p+2u]<<8)|b[p+3u];}
static uint32_t count(uint32_t value){return value==65535u?0u:value+1u;}
static uint32_t extent(uint32_t n){return (n<<3)+(n<<2);}
static int32_t signed_id(uint32_t value)
{return value>=32768u?(int32_t)value-65536:(int32_t)value;}
static int overlaps(uint32_t a,uint32_t n,uint32_t b,uint32_t m)
{return n!=0 && m!=0 && (a<=b?b-a<n:a-b<m);}
static cs_rf_result done(cs_rf_report *r,cs_rf_result status,cs_rf_field field,uint32_t position)
{r->status=status;r->field=field;r->position=position;return status;}
static void begin(cs_rf_report *r,uint32_t warnings)
{r->status=CS_RF_OK;r->field=CS_RF_FIELD_NONE;r->position=0;r->warnings=warnings;r->types=0;r->references=0;}
static cs_rf_result end(uint32_t base,uint32_t size,uint32_t limit,uint32_t *out,
    cs_rf_report *r,cs_rf_field field,uint32_t position,cs_rf_result bounded)
{
    if(size>UINT32_MAX-base) return done(r,CS_RF_OVERFLOW,field,position);
    *out=base+size;
    if(*out>limit) return done(r,bounded,field,position);
    return CS_RF_OK;
}
static void copy_view(cs_rf_view *to,const cs_rf_view *from)
{
    to->bytes=from->bytes;to->tag=from->tag;to->length=from->length;
    to->data.offset=from->data.offset;to->data.length=from->data.length;
    to->map.offset=from->map.offset;to->map.length=from->map.length;
    to->reserved.offset=from->reserved.offset;to->reserved.length=from->reserved.length;
    to->application.offset=from->application.offset;to->application.length=from->application.length;
    to->type_list=from->type_list;to->name_list=from->name_list;
    to->type_count=from->type_count;to->warnings=from->warnings;
}
static void copy_entry(cs_rf_entry *to,const cs_rf_entry *from)
{
    to->type=from->type;to->id=from->id;to->reference=from->reference;to->attributes=from->attributes;
    to->name_present=from->name_present;to->specification=from->specification;
    to->name.offset=from->name.offset;to->name.length=from->name.length;
    to->data.offset=from->data.offset;to->data.length=from->data.length;
}
static cs_rf_result record(const cs_rf_view *v,uint32_t at,cs_rf_entry *e,cs_rf_report *r)
{
    const uint8_t *b=v->bytes;uint32_t map_end=v->map.offset+v->map.length,finish;
    uint32_t name=u16(b,at+2u);e->reference=at;e->attributes=b[at+4u];
    e->name_present=name!=65535u;e->name.offset=0;e->name.length=0;
    e->data.offset=0;e->data.length=0;e->specification=0;
    if(e->name_present) {
        cs_rf_result result=end(v->name_list,name,map_end,&finish,r,CS_RF_FIELD_NAME,at+2u,CS_RF_NAME);
        if(result!=CS_RF_OK) return result;
        if(finish==map_end) return done(r,CS_RF_NAME,CS_RF_FIELD_NAME,at+2u);
        e->name.offset=finish+1u;e->name.length=b[finish];
        result=end(e->name.offset,e->name.length,map_end,&finish,r,CS_RF_FIELD_NAME,at+2u,CS_RF_NAME);
        if(result!=CS_RF_OK) return result;
    }
    if(e->attributes&3u) r->warnings|=CS_RF_WARN_ATTRIBUTES;
    if(e->attributes&128u) {
        e->specification=u32(b,at+8u);r->warnings|=CS_RF_WARN_REFERENCE;return CS_RF_REFERENCE;
    }
    if(u32(b,at+8u)!=0) r->warnings|=CS_RF_WARN_HANDLE;
    uint32_t data_end=v->data.offset+v->data.length;
    cs_rf_result result=end(v->data.offset,u24(b,at+5u),data_end,&finish,r,CS_RF_FIELD_PAYLOAD,at+5u,CS_RF_OUT_OF_RANGE);
    if(result!=CS_RF_OK) return result;
    uint32_t prefix=finish;
    result=end(prefix,4,data_end,&finish,r,CS_RF_FIELD_PAYLOAD,at+5u,CS_RF_OUT_OF_RANGE);
    if(result!=CS_RF_OK) return result;
    e->data.offset=finish;e->data.length=u32(b,prefix);
    result=end(e->data.offset,e->data.length,data_end,&finish,r,CS_RF_FIELD_PAYLOAD,prefix,CS_RF_OUT_OF_RANGE);
    if(result!=CS_RF_OK) return result;
    if(e->data.length>32768u) r->warnings|=CS_RF_WARN_LARGE;
    return CS_RF_OK;
}
cs_rf_result cs_rf_open(const uint8_t *b,size_t length,cs_rf_view *out,cs_rf_report *r)
{
    if(b==0 || out==0 || r==0) return CS_RF_ARGUMENT;
#if SIZE_MAX > UINT32_MAX
    if(length>UINT32_MAX) return CS_RF_ARGUMENT;
#endif
    begin(r,0);uint32_t size=(uint32_t)length;
    if(size<16) return done(r,CS_RF_TRUNCATED,CS_RF_FIELD_HEADER,0);
    cs_rf_view v;v.bytes=b;v.tag=RF_TAG;v.length=size;
    v.data.offset=u32(b,0);v.data.length=u32(b,8);v.map.offset=u32(b,4);v.map.length=u32(b,12);
    uint32_t data_end,map_end,table_end;
    cs_rf_result result=end(v.data.offset,v.data.length,size,&data_end,r,CS_RF_FIELD_DATA,8,CS_RF_TRUNCATED);
    if(result!=CS_RF_OK) return result;
    result=end(v.map.offset,v.map.length,size,&map_end,r,CS_RF_FIELD_MAP,12,CS_RF_TRUNCATED);
    if(result!=CS_RF_OK) return result;
    if(overlaps(0,16,v.data.offset,v.data.length)) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_DATA,0);
    if(overlaps(0,16,v.map.offset,v.map.length) || overlaps(v.data.offset,v.data.length,v.map.offset,v.map.length))
        return done(r,CS_RF_OVERLAP,CS_RF_FIELD_MAP,4);
    if(v.map.length<30) return done(r,CS_RF_OUT_OF_RANGE,CS_RF_FIELD_MAP,12);
    uint32_t types=u16(b,v.map.offset+24u),names=u16(b,v.map.offset+26u);
    if(types<28) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_TYPE_LIST,v.map.offset+24u);
    result=end(types,2,v.map.length,&table_end,r,CS_RF_FIELD_TYPE_LIST,v.map.offset+24u,CS_RF_OUT_OF_RANGE);
    if(result!=CS_RF_OK) return result;
    if(names<28) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_NAME_LIST,v.map.offset+26u);
    if(names>v.map.length) return done(r,CS_RF_OUT_OF_RANGE,CS_RF_FIELD_NAME_LIST,v.map.offset+26u);
    v.type_list=v.map.offset+types;v.name_list=v.map.offset+names;v.type_count=count(u16(b,v.type_list));
    result=end(table_end,v.type_count<<3,v.map.length,&table_end,r,CS_RF_FIELD_TYPE_LIST,v.type_list,CS_RF_OUT_OF_RANGE);
    if(result!=CS_RF_OK) return result;
    if(table_end>names) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_TYPE_LIST,v.type_list);
    table_end+=v.map.offset;
    if(v.data.offset!=256u || v.map.offset!=data_end || map_end!=size || types!=28u) r->warnings|=CS_RF_WARN_LAYOUT;
    for(uint32_t p=0;p<22u;++p) if(b[v.map.offset+p]!=0) r->warnings|=CS_RF_WARN_MAP_RESERVED;
    uint32_t previous=table_end;
    /* All structural reference lists precede all entry-content validation. */
    for(uint32_t i=0;i<v.type_count;++i) {
        uint32_t at=v.type_list+2u+(i<<3),n=count(u16(b,at+4u)),start,finish;++r->types;
        result=end(v.type_list,u16(b,at+6u),map_end,&start,r,CS_RF_FIELD_REFERENCES,at+6u,CS_RF_OUT_OF_RANGE);
        if(result!=CS_RF_OK) return result;
        result=end(start,extent(n),map_end,&finish,r,CS_RF_FIELD_REFERENCES,at+4u,CS_RF_OUT_OF_RANGE);
        if(result!=CS_RF_OK) return result;
        if(start<previous) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_REFERENCES,at+6u);
        if(finish>v.name_list) return done(r,CS_RF_OVERLAP,CS_RF_FIELD_REFERENCES,at+4u);
        if(start!=previous) r->warnings|=CS_RF_WARN_LAYOUT;
        if(n==0) r->warnings|=CS_RF_WARN_EMPTY_LIST;
        previous=finish;
    }
    if(previous!=v.name_list) r->warnings|=CS_RF_WARN_LAYOUT;
    for(uint32_t i=0;i<v.type_count;++i) {
        uint32_t at=v.type_list+2u+(i<<3),n=count(u16(b,at+4u)),start=v.type_list+u16(b,at+6u);
        for(uint32_t j=0;j<n;++j) {
            cs_rf_entry e;++r->references;
            result=record(&v,start+extent(j),&e,r);
            if(result!=CS_RF_OK && result!=CS_RF_REFERENCE) return result;
        }
    }
    v.reserved.offset=size>=128u?16u:0u;v.reserved.length=size>=128u?112u:0u;
    v.application.offset=size>=256u?128u:0u;v.application.length=size>=256u?128u:0u;
    v.warnings=r->warnings;copy_view(out,&v);return done(r,CS_RF_OK,CS_RF_FIELD_NONE,0);
}
static int valid(const cs_rf_view *v)
{
    if(v->tag!=RF_TAG || v->bytes==0 || v->length<16 || v->data.offset>v->length ||
        v->data.length>v->length-v->data.offset || v->map.offset>v->length ||
        v->map.length>v->length-v->map.offset || v->map.length<30 || v->type_count>65535u) return 0;
    uint32_t first=v->map.offset+28u,last=v->map.offset+v->map.length;
    return v->type_list>=first && v->type_list<=last-2u && v->name_list>=first && v->name_list<=last &&
        v->name_list>=v->type_list+2u && (v->type_count<<3)<=v->name_list-v->type_list-2u;
}
cs_rf_result cs_rf_find(const cs_rf_view *v,uint32_t type,int32_t id,cs_rf_entry *out,cs_rf_report *r)
{
    if(v==0 || out==0 || r==0 || id< -32768 || id>32767 || !valid(v)) return CS_RF_ARGUMENT;
    begin(r,v->warnings);uint32_t selected=0,matches=0,duplicate=0;
    for(uint32_t i=0;i<v->type_count;++i) {
        uint32_t at=v->type_list+2u+(i<<3);++r->types;
        if(u32(v->bytes,at)==type) {
            if(matches==0) selected=at;
            if(matches==1) duplicate=at;
            ++matches;
        }
    }
    if(matches==0) return done(r,CS_RF_NOT_FOUND,CS_RF_FIELD_TYPE_KEY,0);
    if(matches>1) return done(r,CS_RF_DUPLICATE_TYPE,CS_RF_FIELD_TYPE_KEY,duplicate);
    uint32_t n=count(u16(v->bytes,selected+4u)),start=v->type_list+u16(v->bytes,selected+6u),chosen=0;
    matches=0;duplicate=0;
    for(uint32_t i=0;i<n;++i) {
        uint32_t at=start+extent(i);++r->references;
        if(signed_id(u16(v->bytes,at))==id) {
            if(matches==0) chosen=at;
            if(matches==1) duplicate=at;
            ++matches;
        }
    }
    if(matches==0) return done(r,CS_RF_NOT_FOUND,CS_RF_FIELD_ID,selected);
    if(matches>1) return done(r,CS_RF_DUPLICATE_ID,CS_RF_FIELD_ID,duplicate);
    cs_rf_entry e;e.type=type;e.id=id;
    cs_rf_result result=record(v,chosen,&e,r);
    if(result!=CS_RF_OK && result!=CS_RF_REFERENCE) return result;
    copy_entry(out,&e);return done(r,result,CS_RF_FIELD_NONE,chosen);
}
