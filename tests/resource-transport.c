/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0116 host fixtures. */
#include "../formats/resourcefork/mfs-transport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
static const uint32_t keys[2]={UINT32_C(0x414200ff),UINT32_C(0xff001100)};
static const int32_t ids[4]={-32768,32767,-1,0};
typedef struct {
    uint8_t *bytes;
    uint32_t size,data,map,types,refs,names,prefix[4],length[4];
} fixture;
static void be16(uint8_t *b,uint32_t p,uint32_t n)
{ b[p]=(uint8_t)(n>>8); b[p+1u]=(uint8_t)n; }
static void be24(uint8_t *b,uint32_t p,uint32_t n)
{ b[p]=(uint8_t)(n>>16); be16(b,p+1u,n); }
static void be32(uint8_t *b,uint32_t p,uint32_t n)
{ be16(b,p,n>>16); be16(b,p+2u,n); }
static uint8_t payload(unsigned resource,uint32_t position)
{ return (uint8_t)((position*37u+(uint32_t)resource*59u+19u)&255u); }
/* Four resources, two unrelated types, signed IDs, embedded NUL/high bytes,
 * absent and empty names, and a maximum counted name. No serialized C structs. */
static int make(fixture *f,uint32_t first_length,uint32_t base,uint32_t gap)
{
    uint32_t i,j,at;
    f->length[0]=first_length; f->length[1]=3; f->length[2]=0; f->length[3]=7;
    f->data=base; at=base;
    for(i=0;i<4;++i) { f->prefix[i]=at; at+=4u+f->length[i]; }
    f->map=at+gap; f->types=f->map+28u; f->refs=f->map+46u;
    f->names=f->map+94u; f->size=f->names+261u;
    f->bytes=calloc(f->size,1); CHECK(f->bytes!=NULL);
    be32(f->bytes,0,f->data); be32(f->bytes,4,f->map);
    be32(f->bytes,8,at-base); be32(f->bytes,12,f->size-f->map);
    be16(f->bytes,f->map+24u,28); be16(f->bytes,f->map+26u,94);
    be16(f->bytes,f->types,1);
    for(i=0;i<2;++i) {
        uint32_t t=f->types+2u+8u*i;
        be32(f->bytes,t,keys[i]); be16(f->bytes,t+4u,1);
        be16(f->bytes,t+6u,18u+24u*i);
    }
    for(i=0;i<4;++i) {
        uint32_t r=f->refs+12u*i;
        be16(f->bytes,r,(uint32_t)ids[i]&65535u);
        be16(f->bytes,r+2u,i==1?65535u:(i==0?0u:(i==2?4u:5u)));
        be24(f->bytes,r+5u,f->prefix[i]-base);
        be32(f->bytes,f->prefix[i],f->length[i]);
        for(j=0;j<f->length[i];++j) f->bytes[f->prefix[i]+4u+j]=payload(i,j);
    }
    f->bytes[f->names]=3; f->bytes[f->names+1u]=0x80;
    f->bytes[f->names+2u]=0; f->bytes[f->names+3u]=0xff;
    f->bytes[f->names+4u]=0; f->bytes[f->names+5u]=255;
    for(i=0;i<255;++i) f->bytes[f->names+6u+i]=(uint8_t)(255u-i);
    return 0;
}

typedef struct {
    const fixture *file;
    uint32_t length,calls,fail_call,fail_byte,last,highest;
    uint32_t relocated_data,relocated_map;
    cs_rfs_io failure;
    uint16_t maximum,last_count;
    unsigned bad,sparse,fail_at_byte;
    uint8_t header[16];
} provider;
static void source_for(const fixture *f,provider *p,cs_rfs_source *s);
static uint8_t source_byte(const provider *p,uint32_t at)
{
    const fixture *f=p->file;
    if(!p->sparse) return f->bytes[at];
    if(at<16u) return p->header[at];
    if(at>=p->relocated_data && at-p->relocated_data<f->map-f->data)
        return f->bytes[f->data+at-p->relocated_data];
    if(at>=p->relocated_map && at-p->relocated_map<f->size-f->map)
        return f->bytes[f->map+at-p->relocated_map];
    return 0;
}
static cs_rfs_io read_at(void *context,uint32_t at,uint16_t n,uint8_t *to)
{
    provider *p=context; uint16_t i;
    ++p->calls; p->last=at; p->last_count=n;
    if(n==0 || n>256u || at>p->length || (uint32_t)n>p->length-at) {
        ++p->bad; return CS_RFS_IO_PROVIDER;
    }
    if(n>p->maximum) p->maximum=n;
    if(at+(uint32_t)n>p->highest) p->highest=at+(uint32_t)n;
    if(p->calls==p->fail_call || (p->fail_at_byte && at<=p->fail_byte && p->fail_byte-at<n)) {
        for(i=0;i<(uint16_t)(n/2u+1u);++i) to[i]=0xde;
        return p->failure;
    }
    for(i=0;i<n;++i) to[i]=source_byte(p,at+i);
    return CS_RFS_IO_OK;
}
static void source_for(const fixture *f,provider *p,cs_rfs_source *s)
{
    memset(p,0,sizeof *p); p->file=f; p->length=f->size; p->failure=CS_RFS_IO_ERROR;
    s->length=f->size; s->read=read_at; s->context=p;
}
static int same_report(const cs_rf_report *a,const cs_rfs_report *b)
{
    CHECK((unsigned)a->status==(unsigned)b->status); CHECK(a->field==b->field);
    CHECK(a->position==b->position); CHECK(a->warnings==b->warnings);
    CHECK(a->types==b->types); CHECK(a->references==b->references); return 0;
}
static int same_entry(const cs_rf_entry *a,const cs_rf_entry *b)
{
    CHECK(a->type==b->type && a->id==b->id && a->reference==b->reference);
    CHECK(a->attributes==b->attributes && a->name_present==b->name_present);
    CHECK(a->specification==b->specification);
    CHECK(a->name.offset==b->name.offset && a->name.length==b->name.length);
    CHECK(a->data.offset==b->data.offset && a->data.length==b->data.length); return 0;
}
static int same_view(const cs_rf_view *a,const cs_rfs_view *b)
{
    CHECK(a->length==b->source.length && a->warnings==b->warnings);
    CHECK(a->data.offset==b->data.offset && a->data.length==b->data.length);
    CHECK(a->map.offset==b->map.offset && a->map.length==b->map.length);
    CHECK(a->type_list==b->type_list && a->type_count==b->type_count && a->name_list==b->name_list);
    CHECK(a->reserved.offset==b->reserved.offset && a->reserved.length==b->reserved.length);
    CHECK(a->application.offset==b->application.offset && a->application.length==b->application.length); return 0;
}
static int compare_open(fixture *f,cs_rfs_result expected,cs_rf_field field,uint32_t at,int exact)
{
    cs_rf_view cv,cb; cs_rfs_view sv,sb; cs_rf_report cr; cs_rfs_report sr;
    cs_rfs_workspace w; cs_rfs_source s; provider p;
    memset(&cv,0xa5,sizeof cv); memcpy(&cb,&cv,sizeof cb);
    memset(&sv,0x5a,sizeof sv); memcpy(&sb,&sv,sizeof sb); memset(&w,0xff,sizeof w);
    source_for(f,&p,&s);
    cs_rf_result a=cs_rf_open(f->bytes,f->size,&cv,&cr);
    cs_rfs_result b=cs_rfs_open(&s,&w,&sv,&sr);
    CHECK((unsigned)a==(unsigned)b); CHECK(same_report(&cr,&sr)==0); CHECK(p.bad==0);
    if(exact) { CHECK(b==expected); CHECK(sr.field==field && sr.position==at); }
    if(b==CS_RFS_OK) CHECK(same_view(&cv,&sv)==0);
    else { CHECK(memcmp(&cv,&cb,sizeof cv)==0); CHECK(memcmp(&sv,&sb,sizeof sv)==0); }
    return 0;
}
static int compare_find(fixture *f,uint32_t type,int32_t id,cs_rfs_result expected,uint32_t position)
{
    cs_rf_view cv; cs_rfs_view sv; cs_rf_report cr; cs_rfs_report sr;
    cs_rf_entry ce,se,cb,sb; cs_rfs_workspace w; cs_rfs_source s; provider p;
    source_for(f,&p,&s); CHECK(cs_rf_open(f->bytes,f->size,&cv,&cr)==CS_RF_OK);
    CHECK(cs_rfs_open(&s,&w,&sv,&sr)==CS_RFS_OK);
    memset(&ce,0xa5,sizeof ce); memcpy(&cb,&ce,sizeof cb);
    memset(&se,0x5a,sizeof se); memcpy(&sb,&se,sizeof sb);
    CHECK((unsigned)cs_rf_find(&cv,type,id,&ce,&cr)==(unsigned)expected);
    CHECK(cs_rfs_find(&sv,&w,type,id,&se,&sr)==expected);
    CHECK(sr.position==position); CHECK(same_report(&cr,&sr)==0); CHECK(p.bad==0);
    if(expected==CS_RFS_OK || expected==CS_RFS_REFERENCE) CHECK(same_entry(&ce,&se)==0);
    else { CHECK(memcmp(&ce,&cb,sizeof ce)==0); CHECK(memcmp(&se,&sb,sizeof se)==0); }
    return 0;
}

static int valid_and_alignment(void)
{
    unsigned shift,i;
    for(shift=0;shift<256;++shift) {
        fixture f; provider p; cs_rfs_source s; cs_rfs_workspace w;
        cs_rfs_view v; cs_rfs_report r; cs_rf_entry e;
        uint8_t output[258],stage[258];
        CHECK(make(&f,shift==0?520u:1u,256,shift)==0);
        CHECK(compare_open(&f,CS_RFS_OK,CS_RF_FIELD_NONE,0,1)==0);
        source_for(&f,&p,&s); CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK);
        CHECK(r.types==2 && r.references==4);
        CHECK(r.warnings==(shift==0?0u:CS_RF_WARN_LAYOUT));
        for(i=0;i<4;++i) {
            CHECK(compare_find(&f,keys[i/2u],ids[i],CS_RFS_OK,f.refs+12u*i)==0);
            CHECK(cs_rfs_find(&v,&w,keys[i/2u],ids[i],&e,&r)==CS_RFS_OK);
            CHECK(e.data.offset==f.prefix[i]+4u && e.data.length==f.length[i]);
            CHECK(e.name_present==(i!=1));
            memset(output,0xa5,sizeof output); memset(stage,0x3c,sizeof stage);
            size_t n=e.data.length>256?256u:(size_t)e.data.length;
            CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,n,output+1,256,stage+1,256,&r)==CS_RFS_OK);
            CHECK(memcmp(output+1,f.bytes+f.prefix[i]+4u,n)==0);
            CHECK(output[0]==0xa5 && output[n+1u]==0xa5 && output[257]==0xa5);
            CHECK(stage[0]==0x3c && stage[n+1u]==0x3c && stage[257]==0x3c);
            uint32_t before=p.calls;
            CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,e.data.length,0,output+1,256,stage+1,256,&r)==CS_RFS_OK);
            CHECK(p.calls==before);
            if(i==1) CHECK(cs_rfs_read(&v,&e,CS_RFS_RESOURCE_NAME,0,0,output+1,256,stage+1,256,&r)==CS_RFS_NOT_FOUND);
            else {
                CHECK(e.name.length==(i==0?3u:(i==2?0u:255u)));
                CHECK(cs_rfs_read(&v,&e,CS_RFS_RESOURCE_NAME,0,e.name.length,output+1,256,stage+1,256,&r)==CS_RFS_OK);
                CHECK(memcmp(output+1,f.bytes+e.name.offset,e.name.length)==0);
            }
        }
        CHECK(p.bad==0 && p.maximum<=256); free(f.bytes);
    }
    for(i=0;i<3;++i) {
        fixture f; CHECK(make(&f,1,i==0?16u:(i==1?128u:256u),0)==0);
        if(i==2) { /* Map first, with independently relocated header regions. */
            uint8_t *old=malloc(f.size); CHECK(old!=NULL); memcpy(old,f.bytes,f.size);
            uint32_t data_size=f.map-f.data,map_size=f.size-f.map;
            memcpy(f.bytes+256,old+f.map,map_size); memcpy(f.bytes+256+map_size,old+f.data,data_size);
            be32(f.bytes,0,256+map_size); be32(f.bytes,4,256); free(old);
        }
        CHECK(compare_open(&f,CS_RFS_OK,CS_RF_FIELD_NONE,0,1)==0); free(f.bytes);
    }
    return 0;
}
static int malformed(void)
{
    unsigned k;
    for(k=0;k<30;++k) {
        fixture f; CHECK(make(&f,520,256,0)==0);
        uint32_t t=f.types+2u,r=f.refs,at=t+6u;
        cs_rfs_result result=CS_RFS_OUT_OF_RANGE; cs_rf_field field=CS_RF_FIELD_REFERENCES;
        switch(k) {
        case 0:f.size=15;result=CS_RFS_TRUNCATED;field=CS_RF_FIELD_HEADER;at=0;break;
        case 1:be32(f.bytes,0,UINT32_MAX);be32(f.bytes,8,2);result=CS_RFS_OVERFLOW;field=CS_RF_FIELD_DATA;at=8;break;
        case 2:be32(f.bytes,4,UINT32_MAX);be32(f.bytes,12,2);result=CS_RFS_OVERFLOW;field=CS_RF_FIELD_MAP;at=12;break;
        case 3:be32(f.bytes,8,f.size);result=CS_RFS_TRUNCATED;field=CS_RF_FIELD_DATA;at=8;break;
        case 4:be32(f.bytes,12,f.size);result=CS_RFS_TRUNCATED;field=CS_RF_FIELD_MAP;at=12;break;
        case 5:be32(f.bytes,0,15);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_DATA;at=0;break;
        case 6:be32(f.bytes,4,0);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_MAP;at=4;break;
        case 7:be32(f.bytes,4,256);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_MAP;at=4;break;
        case 8:be32(f.bytes,12,29);field=CS_RF_FIELD_MAP;at=12;break;
        case 9:be16(f.bytes,f.map+24u,27);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_TYPE_LIST;at=f.map+24u;break;
        case 10:be16(f.bytes,f.map+24u,354);field=CS_RF_FIELD_TYPE_LIST;at=f.map+24u;break;
        case 11:be16(f.bytes,f.map+26u,27);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_NAME_LIST;at=f.map+26u;break;
        case 12:be16(f.bytes,f.map+26u,356);field=CS_RF_FIELD_NAME_LIST;at=f.map+26u;break;
        case 13:be16(f.bytes,f.types,65534);field=CS_RF_FIELD_TYPE_LIST;at=f.types;break;
        case 14:be16(f.bytes,f.map+26u,44);result=CS_RFS_OVERLAP;field=CS_RF_FIELD_TYPE_LIST;at=f.types;break;
        case 15:be16(f.bytes,t+6u,0);result=CS_RFS_OVERLAP;break;
        case 16:be16(f.bytes,t+6u,65535);break;
        case 17:be16(f.bytes,t+4u,65534);at=t+4u;break;
        case 18:be16(f.bytes,t+6u,67);result=CS_RFS_OVERLAP;at=t+4u;break;
        case 19:be16(f.bytes,t+14u,18);result=CS_RFS_OVERLAP;at=t+14u;break;
        case 20:be16(f.bytes,r+2u,261);result=CS_RFS_NAME;field=CS_RF_FIELD_NAME;at=r+2u;break;
        case 21:f.bytes[f.names+5u]=255;be16(f.bytes,r+38u,260);result=CS_RFS_NAME;field=CS_RF_FIELD_NAME;at=r+38u;break;
        case 22:be24(f.bytes,r+5u,546);field=CS_RF_FIELD_PAYLOAD;at=r+5u;break;
        case 23:be24(f.bytes,r+5u,544);field=CS_RF_FIELD_PAYLOAD;at=r+5u;break;
        case 24:be32(f.bytes,f.data,UINT32_MAX);result=CS_RFS_OVERFLOW;field=CS_RF_FIELD_PAYLOAD;at=f.data;break;
        case 25:be32(f.bytes,f.data,543);field=CS_RF_FIELD_PAYLOAD;at=f.data;break;
        case 26:be16(f.bytes,r+2u,261);be16(f.bytes,t+14u,0);result=CS_RFS_OVERLAP;at=t+14u;break;
        case 27:be16(f.bytes,r+2u,261);be24(f.bytes,r+5u,0xffffff);result=CS_RFS_NAME;field=CS_RF_FIELD_NAME;at=r+2u;break;
        case 28:f.bytes[f.map]=1;be16(f.bytes,f.types,65534);field=CS_RF_FIELD_TYPE_LIST;at=f.types;break;
        default:be16(f.bytes,r+2u,65534);result=CS_RFS_NAME;field=CS_RF_FIELD_NAME;at=r+2u;break;
        }
        CHECK(compare_open(&f,result,field,at,1)==0); free(f.bytes);
    }
    fixture f; CHECK(make(&f,520,256,0)==0); uint32_t full=f.size;
    for(uint32_t n=0;n<full;++n) { f.size=n; CHECK(compare_open(&f,CS_RFS_TRUNCATED,CS_RF_FIELD_NONE,0,0)==0); }
    f.size=full;
    /* Reproducible local mutation corpus supplements the explicit refusal cases. */
    uint32_t state=UINT32_C(0x73260ac1);
    for(k=0;k<3000;++k) {
        state=state*UINT32_C(1664525)+UINT32_C(1013904223); uint32_t at=state%full;
        uint8_t old=f.bytes[at]; f.bytes[at]^=(uint8_t)((state>>24)|1u);
        CHECK(compare_open(&f,CS_RFS_OK,CS_RF_FIELD_NONE,0,0)==0); f.bytes[at]=old;
    }
    free(f.bytes); return 0;
}
static int lookup_special(void)
{
    unsigned k;
    for(k=0;k<8;++k) {
        fixture f; CHECK(make(&f,520,256,0)==0);
        cs_rfs_result expected=CS_RFS_OK; uint32_t pos=f.refs,type=keys[0]; int32_t id=-32768;
        switch(k) {
        case 0:type^=1u;expected=CS_RFS_NOT_FOUND;pos=0;break;
        case 1:id=1;expected=CS_RFS_NOT_FOUND;pos=f.types+2u;break;
        case 2:be32(f.bytes,f.types+10u,keys[0]);expected=CS_RFS_DUPLICATE_TYPE;pos=f.types+10u;break;
        case 3:be16(f.bytes,f.refs+12u,32768);expected=CS_RFS_DUPLICATE_ID;pos=f.refs+12u;break;
        case 4:be16(f.bytes,f.refs+36u,65535);break; /* Unrelated duplicate ID. */
        case 5:f.bytes[f.refs+4u]=0x83;be24(f.bytes,f.refs+5u,0xffffff);be32(f.bytes,f.refs+8u,0x81ff8001);expected=CS_RFS_REFERENCE;break;
        case 6:be16(f.bytes,f.types,65535);expected=CS_RFS_NOT_FOUND;pos=0;break;
        default:be16(f.bytes,f.types+6u,65535);expected=CS_RFS_NOT_FOUND;pos=f.types+2u;break;
        }
        CHECK(compare_find(&f,type,id,expected,pos)==0);
        if(k==5) {
            provider p;cs_rfs_source s;cs_rfs_workspace w;cs_rfs_view v;cs_rf_entry e;cs_rfs_report r;
            uint8_t out[3]={1,2,3},stage[3]={4,5,6}; source_for(&f,&p,&s);
            CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK);
            CHECK((r.warnings&(CS_RF_WARN_REFERENCE|CS_RF_WARN_ATTRIBUTES))==(CS_RF_WARN_REFERENCE|CS_RF_WARN_ATTRIBUTES));
            CHECK(!(r.warnings&CS_RF_WARN_HANDLE)); CHECK(cs_rfs_find(&v,&w,type,id,&e,&r)==CS_RFS_REFERENCE);
            CHECK(e.specification==UINT32_C(0x81ff8001) && e.data.offset==0 && e.data.length==0);
            uint32_t calls=p.calls;
            CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,0,out,3,stage,3,&r)==CS_RFS_REFERENCE);
            CHECK(p.calls==calls && out[0]==1 && stage[0]==4);
            CHECK(cs_rfs_read(&v,&e,CS_RFS_RESOURCE_NAME,0,3,out,3,stage,3,&r)==CS_RFS_OK);
            CHECK(out[0]==0x80 && out[1]==0 && out[2]==0xff);
        }
        free(f.bytes);
    }
    return 0;
}

static int io_failures(void)
{
    fixture f;provider p;cs_rfs_source s;cs_rfs_workspace w;cs_rfs_view v,before;
    cs_rf_entry e,eb;cs_rfs_report r;CHECK(make(&f,520,256,79)==0);source_for(&f,&p,&s);
    CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK);uint32_t open_calls=p.calls;
    p.calls=0;CHECK(cs_rfs_find(&v,&w,keys[0],ids[0],&e,&r)==CS_RFS_OK);uint32_t find_calls=p.calls;
    static const cs_rfs_io failures[]={CS_RFS_IO_ERROR,CS_RFS_IO_UNAVAILABLE,CS_RFS_IO_PROVIDER,(cs_rfs_io)91};
    static const cs_rfs_result statuses[]={CS_RFS_IO,CS_RFS_UNAVAILABLE,CS_RFS_PROVIDER,CS_RFS_PROVIDER};
    for(unsigned kind=0;kind<4;++kind) {
        for(uint32_t call=1;call<=open_calls;++call) {
            memset(&v,0xa5,sizeof v);memcpy(&before,&v,sizeof v);
            source_for(&f,&p,&s);p.fail_call=call;p.failure=failures[kind];
            CHECK(cs_rfs_open(&s,&w,&v,&r)==statuses[kind]);
            CHECK(p.calls==call && p.bad==0 && r.position==p.last && !(p.last&255u));
            CHECK(memcmp(&v,&before,sizeof v)==0 && w.count==0);
        }
        source_for(&f,&p,&s);CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK);
        for(uint32_t call=1;call<=find_calls;++call) {
            p.calls=0;p.fail_call=call;p.failure=failures[kind];
            memset(&e,0x3c,sizeof e);memcpy(&eb,&e,sizeof e);
            CHECK(cs_rfs_find(&v,&w,keys[0],ids[0],&e,&r)==statuses[kind]);
            CHECK(p.calls==call && p.bad==0 && r.position==p.last && w.count==0);
            CHECK(memcmp(&e,&eb,sizeof e)==0);
        }
    }
    /* Byte100 is unused prefix content, but header fetch legitimately reads it. */
    source_for(&f,&p,&s);p.fail_at_byte=1;p.fail_byte=100;p.failure=CS_RFS_IO_UNAVAILABLE;
    memset(&v,0xa5,sizeof v);memcpy(&before,&v,sizeof v);
    CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_UNAVAILABLE);
    CHECK(r.field==CS_RF_FIELD_HEADER && r.position==0 && p.last_count==256 && p.calls==1);
    CHECK(memcmp(&v,&before,sizeof v)==0);
    p.fail_at_byte=0;CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK); /* Failed cache cannot poison retry. */
    free(f.bytes);return 0;
}

static int ranges_and_arguments(void)
{
    fixture f;provider p;cs_rfs_source s;cs_rfs_workspace w,wb;cs_rfs_view v,vb;
    cs_rf_entry e,eb;cs_rfs_report r,rb;
    uint8_t *out=malloc(32769),*stage=malloc(32769);CHECK(out!=NULL && stage!=NULL);
    CHECK(make(&f,70001,256,0)==0);source_for(&f,&p,&s);
    CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK && (r.warnings&CS_RF_WARN_LARGE));
    CHECK(compare_open(&f,CS_RFS_OK,CS_RF_FIELD_NONE,0,1)==0);
    CHECK(cs_rfs_find(&v,&w,keys[0],ids[0],&e,&r)==CS_RFS_OK);
    memset(out,0xa5,32769);memset(stage,0x3c,32769);p.calls=0;
    CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,32760,32767,out+1,32767,stage+1,32767,&r)==CS_RFS_OK);
    CHECK(p.calls==128 && p.maximum==256 && p.last_count==255 && p.bad==0);
    for(uint32_t n=0;n<32767;++n) CHECK(out[n+1u]==payload(0,n+32760u));
    CHECK(out[0]==0xa5 && out[32768]==0xa5 && stage[0]==0x3c && stage[32768]==0x3c);
    for(unsigned kind=0;kind<3;++kind) {
        for(uint32_t call=1;call<=128;call+=call==1?1u:63u) {
            memset(out,0xa5,32769);memset(stage,0x3c,32769);p.calls=0;p.fail_call=call;
            p.failure=kind==0?CS_RFS_IO_ERROR:(kind==1?CS_RFS_IO_UNAVAILABLE:(cs_rfs_io)99);
            cs_rfs_result expected=kind==0?CS_RFS_IO:(kind==1?CS_RFS_UNAVAILABLE:CS_RFS_PROVIDER);
            CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,32760,32767,out+1,32767,stage+1,32767,&r)==expected);
            CHECK(p.calls==call && r.position==e.data.offset+32760u+256u*(call-1u));
            for(uint32_t n=0;n<32769;++n) CHECK(out[n]==0xa5);
            CHECK(stage[0]==0x3c && stage[32768]==0x3c);
        }
    }
    p.fail_call=0;p.calls=0;memset(out,0xa5,32769);memset(stage,0x3c,32769);
    CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,70000,1,out+1,3,stage+1,3,&r)==CS_RFS_OK);
    CHECK(out[1]==payload(0,70000) && out[2]==0xa5 && stage[2]==0x3c);
    uint32_t calls=p.calls;
    for(unsigned k=0;k<6;++k) {
        memset(out,0xa5,32769);
        uint32_t offset=k==0?70001u:(k==1?70002u:(k==2?UINT32_MAX:0u));
        size_t amount=k==0?1u:(k==1?0u:2u),oc=k==3?1u:3u,sc=k==4?1u:3u;
        cs_rfs_result expected=k==3 || k==4?CS_RFS_CAPACITY:CS_RFS_OUT_OF_RANGE;
        cs_rf_entry forged=e;if(k==5) forged.data.offset=v.map.offset;
        CHECK(cs_rfs_read(&v,&forged,CS_RFS_PAYLOAD,offset,amount,out+1,oc,stage+1,sc,&r)==expected);
        CHECK(p.calls==calls && out[1]==0xa5);
    }
    for(unsigned k=0;k<11;++k) {
        memset(&r,0x6b,sizeof r);memcpy(&rb,&r,sizeof rb);memcpy(&eb,&e,sizeof eb);
        cs_rfs_result result;
        switch(k) {
        case 0:result=cs_rfs_read(NULL,&e,CS_RFS_PAYLOAD,0,1,out,3,stage,3,&r);break;
        case 1:result=cs_rfs_read(&v,NULL,CS_RFS_PAYLOAD,0,1,out,3,stage,3,&r);break;
        case 2:result=cs_rfs_read(&v,&e,(cs_rfs_part)9,0,1,out,3,stage,3,&r);break;
        case 3:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,32768,out,32767,stage,32767,&r);break;
        case 4:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,1,out,32768,stage,3,&r);break;
        case 5:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,1,out,3,stage,32768,&r);break;
        case 6:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,0,NULL,0,stage,0,&r);break;
        case 7:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,0,out,0,NULL,0,&r);break;
        case 8:result=cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,0,out,0,out,0,&r);break;
        case 9:result=cs_rfs_find(&v,&w,keys[0],32768,&e,&r);break;
        default:result=cs_rfs_find(&v,&w,keys[0],-32769,&e,&r);break;
        }
        CHECK(result==CS_RFS_ARGUMENT && memcmp(&r,&rb,sizeof r)==0);
        CHECK(memcmp(&e,&eb,sizeof e)==0 && p.calls==calls);
    }
    for(unsigned k=0;k<5;++k) {
        memcpy(&vb,&v,sizeof vb);memcpy(&wb,&w,sizeof wb);memset(&r,0x5c,sizeof r);memcpy(&rb,&r,sizeof rb);
        cs_rfs_source bad=s;bad.read=NULL;
        cs_rfs_result got=k==0?cs_rfs_open(NULL,&w,&v,&r):k==1?cs_rfs_open(&s,NULL,&v,&r):
            k==2?cs_rfs_open(&s,&w,NULL,&r):k==3?cs_rfs_open(&s,&w,&v,NULL):cs_rfs_open(&bad,&w,&v,&r);
        CHECK(got==CS_RFS_ARGUMENT && memcmp(&v,&vb,sizeof v)==0 && memcmp(&w,&wb,sizeof w)==0);
        CHECK(memcmp(&r,&rb,sizeof r)==0 && p.calls==calls);
    }
    free(stage);free(out);free(f.bytes);return 0;
}

static int high_offsets(void)
{
    fixture f;provider p;cs_rfs_source s;cs_rfs_workspace w;cs_rfs_view v;cs_rfs_report r;cs_rf_entry e;
    uint8_t out[520],stage[520];CHECK(make(&f,520,256,0)==0);source_for(&f,&p,&s);
    p.sparse=1;p.length=UINT32_MAX;s.length=UINT32_MAX;
    p.relocated_data=UINT32_C(0xfffefffe);p.relocated_map=UINT32_C(0xfffffe9c);
    memcpy(p.header,f.bytes,16);be32(p.header,0,p.relocated_data);be32(p.header,4,p.relocated_map);
    CHECK(cs_rfs_open(&s,&w,&v,&r)==CS_RFS_OK && v.source.length==UINT32_MAX);
    CHECK(v.map.offset==p.relocated_map && v.data.offset==p.relocated_data);
    CHECK(cs_rfs_find(&v,&w,keys[0],-32768,&e,&r)==CS_RFS_OK);
    CHECK(e.data.offset==p.relocated_data+4u && e.data.length==520 && e.reference==p.relocated_map+46u);
    CHECK(cs_rfs_read(&v,&e,CS_RFS_PAYLOAD,0,520,out,520,stage,520,&r)==CS_RFS_OK);
    for(uint32_t i=0;i<520;++i) CHECK(out[i]==payload(0,i));
    CHECK(p.bad==0 && p.maximum==256 && p.highest<=UINT32_MAX);
    /* The very last valid source byte can be part of a counted name. */
    CHECK(cs_rfs_find(&v,&w,keys[1],0,&e,&r)==CS_RFS_OK);
    CHECK(e.name.offset+e.name.length==UINT32_MAX);
    CHECK(cs_rfs_read(&v,&e,CS_RFS_RESOURCE_NAME,254,1,out,1,stage,1,&r)==CS_RFS_OK && out[0]==1);
    CHECK(p.bad==0 && p.highest==UINT32_MAX);
    free(f.bytes);return 0;
}

typedef struct {uint8_t bytes[20][512];uint32_t calls,fail;cs_block_io_result error;unsigned bad;} disk;
static cs_block_io_result sector(void *context,uint32_t lba,uint8_t *out)
{
    disk *d=context;++d->calls;
    if(lba>=20u) { ++d->bad;return CS_BLOCK_IO_ERROR; }
    if(d->calls==d->fail) { memset(out,0xde,91);return d->error; }
    memcpy(out,d->bytes[lba],512);return CS_BLOCK_IO_OK;
}
static int mfs_adapter(void)
{
    disk *d=calloc(1,sizeof *d);fixture f;cs_block_device device;cs_mfsv_volume volume;
    cs_mfsv_workspace ms;cs_mfs_entry entry;cs_rfs_mfs_adapter adapter,ab;
    cs_rfs_source source,sb;cs_rfs_workspace work;cs_rfs_view view;cs_rfs_report report;cs_rf_entry resource;
    uint8_t out[522],stage[522];CHECK(d!=NULL);CHECK(make(&f,520,256,0)==0);
    uint8_t *h=d->bytes[2];be16(h,0,0xd2d7);be16(h,12,0);be16(h,14,4);be16(h,16,2);
    be16(h,18,6);be32(h,20,1024);be32(h,24,1024);be16(h,28,8);be16(h,34,4);h[36]=1;h[37]='x';
    /* Blocks2 ->4 ->end. The other four allocation blocks are free. */
    h[64]=0;h[65]=0x40;h[66]=0;h[67]=0;h[68]=0x10;h[69]=0;
    for(uint32_t i=0;i<f.size;++i) {
        uint32_t lba=i<1024u?8u+(i>>9):12u+((i-1024u)>>9);
        d->bytes[lba][i&511u]=f.bytes[i];
    }
    device.blocks=20;device.read=sector;device.context=d;
    CHECK(cs_mfsv_mount(&device,&ms,&volume)==CS_MFSV_OK);
    memset(&entry,0,sizeof entry);entry.resource_start=2;entry.resource_logical=f.size;entry.resource_physical=2048;
    memset(&adapter,0xa5,sizeof adapter);memset(&source,0x5a,sizeof source);
    uint32_t calls=d->calls;CHECK(cs_rfs_mfs_bind(&volume,&entry,&ms,&adapter,&source)==CS_MFSV_OK);
    CHECK(d->calls==calls && source.length==f.size && source.context==&adapter);
    CHECK(cs_rfs_open(&source,&work,&view,&report)==CS_RFS_OK);
    CHECK(cs_rfs_find(&view,&work,keys[0],-32768,&resource,&report)==CS_RFS_OK);
    CHECK(cs_rfs_read(&view,&resource,CS_RFS_PAYLOAD,0,520,out+1,520,stage+1,520,&report)==CS_RFS_OK);
    for(uint32_t i=0;i<520;++i) CHECK(out[i+1u]==payload(0,i));
    for(unsigned k=0;k<4;++k) {
        memset(out,0xa5,sizeof out);memset(stage,0x3c,sizeof stage);
        d->calls=0;d->fail=3;d->error=k==0?CS_BLOCK_IO_UNAVAILABLE:k==1?CS_BLOCK_IO_ERROR:(cs_block_io_result)99;
        cs_rfs_result expected=k==0?CS_RFS_UNAVAILABLE:k==1?CS_RFS_IO:CS_RFS_PROVIDER;
        CHECK(cs_rfs_read(&view,&resource,CS_RFS_PAYLOAD,0,520,out+1,520,stage+1,520,&report)==expected);
        for(unsigned i=0;i<522;++i) CHECK(out[i]==0xa5);
        CHECK(d->calls==3 && out[0]==0xa5 && stage[0]==0x3c && stage[521]==0x3c);
    }
    d->fail=0;calls=d->calls;memcpy(&ab,&adapter,sizeof ab);memcpy(&sb,&source,sizeof sb);
    cs_mfs_entry bad=entry;bad.resource_physical=1024;
    CHECK(cs_rfs_mfs_bind(&volume,&bad,&ms,&adapter,&source)==CS_MFSV_CHAIN);
    CHECK(memcmp(&adapter,&ab,sizeof ab)==0 && memcmp(&source,&sb,sizeof sb)==0 && d->calls==calls);
    CHECK(cs_rfs_mfs_bind(&volume,&entry,NULL,&adapter,&source)==CS_MFSV_ARGUMENT);
    CHECK(memcmp(&adapter,&ab,sizeof ab)==0 && memcmp(&source,&sb,sizeof sb)==0 && d->calls==calls);
    CHECK(source.read(source.context,0,0,out)==CS_RFS_IO_PROVIDER);
    CHECK(source.read(source.context,0,257,out)==CS_RFS_IO_PROVIDER);
    CHECK(source.read(source.context,f.size,1,out)==CS_RFS_IO_PROVIDER);
    CHECK(d->calls==calls && d->bad==0);
    free(f.bytes);free(d);return 0;
}
int main(void)
{
    if(valid_and_alignment() || malformed() || lookup_special() || io_failures() ||
        ranges_and_arguments() || high_offsets() || mfs_adapter()) return 1;
    printf("Resource transport: %u checks passed\n",checks);return 0;
}
