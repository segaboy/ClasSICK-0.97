/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original generated format conformance. */
#include "../formats/resourcefork/reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {int32_t id;uint32_t attributes;const char *name,*data;uint32_t specification;} item;
typedef struct {uint32_t type;const item *items;uint32_t count;} group;
typedef struct {uint32_t base,type_pad,ref_pad,name_pad,data_pad,name_entry_pad,trailing,map_first;} layout;
typedef struct {uint8_t *bytes;uint32_t size,data,map,types,names;} fixture;
static void put16(uint8_t *b,uint32_t at,uint32_t v){b[at]=(uint8_t)(v>>8);b[at+1]=(uint8_t)v;}
static void put24(uint8_t *b,uint32_t at,uint32_t v){b[at]=(uint8_t)(v>>16);put16(b,at+1,v);}
static void put32(uint8_t *b,uint32_t at,uint32_t v){put16(b,at,v>>16);put16(b,at+2,v);}
static uint32_t get32(const uint8_t *b,uint32_t at)
{return ((uint32_t)b[at]<<24)|((uint32_t)b[at+1]<<16)|((uint32_t)b[at+2]<<8)|b[at+3];}
static uint32_t length(const char *s){return s==0?0u:(uint32_t)strlen(s);}
/* Declarative encoder: independent of reader functions and validation helpers. */
static int build(fixture *f,const group *g,uint32_t groups,const layout *l)
{
    uint32_t resources=0,data=l->data_pad,names=l->name_entry_pad;
    for(uint32_t i=0;i<groups;++i){resources+=g[i].count;for(uint32_t j=0;j<g[i].count;++j){
        const item *p=&g[i].items[j];if(!(p->attributes&128u))data+=4u+length(p->data);
        if(p->name!=0)names+=1u+length(p->name);
    }}
    uint32_t type_relative=28u+l->type_pad,ref_relative=type_relative+2u+8u*groups+l->ref_pad;
    uint32_t name_relative=ref_relative+12u*resources+l->name_pad,map_size=name_relative+names;
    REQUIRE(name_relative<=65535u);
    f->data=l->map_first?l->base+map_size:l->base;
    f->map=l->map_first?l->base:l->base+data;
    f->size=l->base+map_size+data+l->trailing;
    f->bytes=calloc(f->size,1);REQUIRE(f->bytes!=0);
    put32(f->bytes,0,f->data);put32(f->bytes,4,f->map);put32(f->bytes,8,data);put32(f->bytes,12,map_size);
    put16(f->bytes,f->map+24,type_relative);put16(f->bytes,f->map+26,name_relative);
    f->types=f->map+type_relative;f->names=f->map+name_relative;
    put16(f->bytes,f->types,groups==0?65535u:groups-1u);
    uint32_t ref=f->map+ref_relative,dp=l->data_pad,np=l->name_entry_pad;
    for(uint32_t i=0;i<groups;++i){
        uint32_t t=f->types+2u+8u*i;put32(f->bytes,t,g[i].type);
        put16(f->bytes,t+4,g[i].count==0?65535u:g[i].count-1u);put16(f->bytes,t+6,ref-f->types);
        for(uint32_t j=0;j<g[i].count;++j){
            const item *p=&g[i].items[j];put16(f->bytes,ref,(uint32_t)p->id&65535u);
            put16(f->bytes,ref+2,p->name==0?65535u:np);f->bytes[ref+4]=(uint8_t)p->attributes;
            if(p->attributes&128u){put24(f->bytes,ref+5,0xFFFFFF);put32(f->bytes,ref+8,p->specification);}
            else{put24(f->bytes,ref+5,dp);put32(f->bytes,f->data+dp,length(p->data));
                if(p->data!=0){memcpy(f->bytes+f->data+dp+4,p->data,length(p->data));}
                dp+=4u+length(p->data);}
            if(p->name!=0){f->bytes[f->names+np]=(uint8_t)length(p->name);
                memcpy(f->bytes+f->names+np+1,p->name,length(p->name));np+=1u+length(p->name);}
            ref+=12;
        }
    }return 0;
}
static const item first[]={{-32768,0,"ab","one!",0},{32767,0,"c","two!",0}};
static const item second[]={{0,0,"d","thr!",0},{1,0,"e","for!",0}};
static const item third[]={{-1,0,"f","five",0},{128,0,"gh","six!",0}};
static const group base_groups[]={{0x72303121,first,2},{0x0000FF80,second,2},{0x72503221,third,2}};
static int base(fixture *f){layout l={256,0,0,0,0,0,0,0};return build(f,base_groups,3,&l);}
static int failure(fixture *f,cs_rf_result expected,cs_rf_field field,uint32_t position)
{
    cs_rf_view v,before;cs_rf_report r;memset(&v,0xA5,sizeof v);before=v;
    uint8_t *snapshot=malloc(f->size);REQUIRE(snapshot!=0);memcpy(snapshot,f->bytes,f->size);
    cs_rf_result got=cs_rf_open(f->bytes,f->size,&v,&r);
    if(got!=expected || r.field!=field || r.position!=position)
        fprintf(stderr,"got %d/%d/%u expected %d/%d/%u\n",got,r.field,r.position,expected,field,position);
    REQUIRE(got==expected && r.field==field && r.position==position);
    REQUIRE(!memcmp(&v,&before,sizeof v) && !memcmp(snapshot,f->bytes,f->size));free(snapshot);return 0;
}
static int valid(void)
{
    for(unsigned k=0;k<14;++k){
        layout l={256,0,0,0,0,0,0,0};item p={-32768,0,0,"own",0};group g={0x0000FF80,&p,1};
        char name[256];memset(name,'q',255);name[255]=0;
        uint32_t groups=1;switch(k){case 0:groups=0;break;case 1:p.data="";break;
        case 2:p.name="";break;case 3:p.name=name;break;case 4:p.id=32767;break;
        case 5:l.base=16;break;case 6:l.base=128;break;case 7:l.map_first=1;break;
        case 8:l.type_pad=2;break;case 9:l.ref_pad=4;break;case 10:l.name_pad=6;break;
        case 11:l.trailing=5;break;case 12:g.count=0;break;default:p.attributes=128;p.specification=0x8000FFFF;break;}
        fixture f;REQUIRE(build(&f,&g,groups,&l)==0);cs_rf_view v;cs_rf_report r;cs_rf_entry e;
        REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK && r.types==groups && r.references==(groups?g.count:0));
        REQUIRE(v.reserved.length==(f.size>=128?112u:0u) && v.application.length==(f.size>=256?128u:0u));
        if(k>=5 && k<=11) REQUIRE(r.warnings&CS_RF_WARN_LAYOUT);
        if(k==12) REQUIRE(r.warnings&CS_RF_WARN_EMPTY_LIST);
        if(groups && g.count){
            REQUIRE(cs_rf_find(&v,g.type,p.id,&e,&r)==(k==13?CS_RF_REFERENCE:CS_RF_OK));
            REQUIRE(e.id==p.id && e.name_present==(p.name!=0));
            if(k==13) REQUIRE(e.specification==p.specification && e.data.length==0 && e.data.offset==0 && !(r.warnings&CS_RF_WARN_HANDLE));
            else REQUIRE(e.data.length==length(p.data) && !memcmp(v.bytes+e.data.offset,p.data,e.data.length));
        }
        /* Every truncated length of each small variant, exact native allocations. */
        for(uint32_t n=0;n<f.size;++n){
            uint8_t *b=malloc(n==0?1u:n);REQUIRE(b!=0);memcpy(b,f.bytes,n);
            cs_rf_view out,before;memset(&out,0x3C,sizeof out);before=out;
            uint32_t required=f.size-l.trailing;
            cs_rf_result result=cs_rf_open(b,n,&out,&r);
            if(n<required) REQUIRE(result!=CS_RF_OK && !memcmp(&out,&before,sizeof out));
            else REQUIRE(result==CS_RF_OK);
            free(b);
        }free(f.bytes);
    }
    fixture f;REQUIRE(base(&f)==0 && f.size==444);cs_rf_view v;cs_rf_report r;
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK && r.warnings==0 && r.types==3 && r.references==6);free(f.bytes);return 0;
}
static int malformed(void)
{
    for(unsigned k=0;k<27;++k){
        fixture f;REQUIRE(base(&f)==0);uint32_t t=f.types+2,ref=f.types+26;
        cs_rf_result status=CS_RF_OUT_OF_RANGE;cs_rf_field field=CS_RF_FIELD_REFERENCES;uint32_t pos=t+6;
        switch(k){
        case 0:f.size=15;status=CS_RF_TRUNCATED;field=CS_RF_FIELD_HEADER;pos=0;break;
        case 1:put32(f.bytes,0,UINT32_MAX);put32(f.bytes,8,2);status=CS_RF_OVERFLOW;field=CS_RF_FIELD_DATA;pos=8;break;
        case 2:put32(f.bytes,4,UINT32_MAX);put32(f.bytes,12,2);status=CS_RF_OVERFLOW;field=CS_RF_FIELD_MAP;pos=12;break;
        case 3:put32(f.bytes,8,f.size);status=CS_RF_TRUNCATED;field=CS_RF_FIELD_DATA;pos=8;break;
        case 4:put32(f.bytes,12,f.size);status=CS_RF_TRUNCATED;field=CS_RF_FIELD_MAP;pos=12;break;
        case 5:put32(f.bytes,0,0);status=CS_RF_OVERLAP;field=CS_RF_FIELD_DATA;pos=0;break;
        case 6:put32(f.bytes,4,0);status=CS_RF_OVERLAP;field=CS_RF_FIELD_MAP;pos=4;break;
        case 7:put32(f.bytes,0,f.map);status=CS_RF_OVERLAP;field=CS_RF_FIELD_MAP;pos=4;break;
        case 8:put32(f.bytes,12,29);field=CS_RF_FIELD_MAP;pos=12;break;
        case 9:put16(f.bytes,f.map+24,27);status=CS_RF_OVERLAP;field=CS_RF_FIELD_TYPE_LIST;pos=f.map+24;break;
        case 10:put16(f.bytes,f.map+24,140);field=CS_RF_FIELD_TYPE_LIST;pos=f.map+24;break;
        case 11:put16(f.bytes,f.map+26,27);status=CS_RF_OVERLAP;field=CS_RF_FIELD_NAME_LIST;pos=f.map+26;break;
        case 12:put16(f.bytes,f.map+26,141);field=CS_RF_FIELD_NAME_LIST;pos=f.map+26;break;
        case 13:put16(f.bytes,f.types,65534);field=CS_RF_FIELD_TYPE_LIST;pos=f.types;break;
        case 14:put16(f.bytes,f.map+26,40);status=CS_RF_OVERLAP;field=CS_RF_FIELD_TYPE_LIST;pos=f.types;break;
        case 15:put16(f.bytes,t+6,0);status=CS_RF_OVERLAP;break;
        case 16:put16(f.bytes,t+6,65535);break;
        case 17:put16(f.bytes,t+4,65534);pos=t+4;break;
        case 18:put16(f.bytes,t+6,75);status=CS_RF_OVERLAP;pos=t+4;break;
        case 19:put16(f.bytes,t+8+6,26);status=CS_RF_OVERLAP;pos=t+8+6;break;
        case 20:put16(f.bytes,ref+2,14);status=CS_RF_NAME;field=CS_RF_FIELD_NAME;pos=ref+2;break;
        case 21:f.bytes[f.names]=255;status=CS_RF_NAME;field=CS_RF_FIELD_NAME;pos=ref+2;break;
        case 22:put24(f.bytes,ref+5,48);field=CS_RF_FIELD_PAYLOAD;pos=ref+5;break;
        case 23:put24(f.bytes,ref+5,46);field=CS_RF_FIELD_PAYLOAD;pos=ref+5;break;
        case 24:put32(f.bytes,f.data,UINT32_MAX);status=CS_RF_OVERFLOW;field=CS_RF_FIELD_PAYLOAD;pos=f.data;break;
        case 25:put32(f.bytes,f.data,49);field=CS_RF_FIELD_PAYLOAD;pos=f.data;break;
        default:put16(f.bytes,ref+2,14);put16(f.bytes,t+8+6,0);status=CS_RF_OVERLAP;pos=t+8+6;break;
        }
        REQUIRE(failure(&f,status,field,pos)==0);free(f.bytes);
    }return 0;
}
static int lookup(void)
{
    fixture f;REQUIRE(base(&f)==0);cs_rf_view v;cs_rf_report r;cs_rf_entry e,before;
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK);
    memset(&e,0xA5,sizeof e);before=e;
    REQUIRE(cs_rf_find(&v,0x72303120,-32768,&e,&r)==CS_RF_NOT_FOUND && r.types==3 && r.references==0 && !memcmp(&e,&before,sizeof e));
    REQUIRE(cs_rf_find(&v,0x72303121,0,&e,&r)==CS_RF_NOT_FOUND && r.references==2 && !memcmp(&e,&before,sizeof e));
    REQUIRE(cs_rf_find(&v,0x72303121,-32768,&e,&r)==CS_RF_OK && e.name.length==2 && e.data.length==4);
    uint32_t ref=f.types+26;
    /* Shared individually valid names/data remain usable. */
    put16(f.bytes,ref+12+2,0);put24(f.bytes,ref+12+5,0);f.bytes[f.map]=1;f.bytes[ref+4]=3;put32(f.bytes,ref+8,0xFFFFFFFF);
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK && r.warnings==(CS_RF_WARN_MAP_RESERVED|CS_RF_WARN_ATTRIBUTES|CS_RF_WARN_HANDLE));
    REQUIRE(cs_rf_find(&v,0x72303121,32767,&e,&r)==CS_RF_OK && e.data.offset==f.data+4 && e.name.offset==f.names+1);
    put16(f.bytes,ref+12,0x8000);
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK);before=e;
    REQUIRE(cs_rf_find(&v,0x72303121,-32768,&e,&r)==CS_RF_DUPLICATE_ID && r.position==ref+12 && r.references==2 && !memcmp(&e,&before,sizeof e));
    REQUIRE(cs_rf_find(&v,0x0000FF80,0,&e,&r)==CS_RF_OK);
    put32(f.bytes,f.types+10,0x72303121);
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK);before=e;
    REQUIRE(cs_rf_find(&v,0x72303121,-32768,&e,&r)==CS_RF_DUPLICATE_TYPE && r.position==f.types+10 && r.types==3 && r.references==0 && !memcmp(&e,&before,sizeof e));
    REQUIRE(cs_rf_find(&v,0x72503221,128,&e,&r)==CS_RF_OK);
    free(f.bytes);
    /* Complete diagnostic prefix spans may overlap tolerated file regions. */
    char data[201];memset(data,'j',200);data[200]=0;
    item p={0,0,0,data,0};group g={0,&p,1};layout l={16,0,0,0,0,0,0,0};
    REQUIRE(build(&f,&g,1,&l)==0 && cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK);
    REQUIRE(v.reserved.offset==16 && v.reserved.length==112 && v.application.offset==128 && v.application.length==128);
    REQUIRE(cs_rf_find(&v,0,0,&e,&r)==CS_RF_OK && e.data.offset==20 && e.data.length==200);
    free(f.bytes);return 0;
}
static int arguments(void)
{
    fixture f;REQUIRE(base(&f)==0);cs_rf_view v,before;cs_rf_report r,report;cs_rf_entry e,entry;
    memset(&v,0xA5,sizeof v);before=v;memset(&r,0xB6,sizeof r);report=r;
    REQUIRE(cs_rf_open(0,0,&v,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r) && !memcmp(&v,&before,sizeof v));
    REQUIRE(cs_rf_open(f.bytes,f.size,0,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r));
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,0)==CS_RF_ARGUMENT && !memcmp(&v,&before,sizeof v));
#if SIZE_MAX > UINT32_MAX
    REQUIRE(cs_rf_open(f.bytes,(size_t)UINT32_MAX+1u,&v,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r) && !memcmp(&v,&before,sizeof v));
#endif
    REQUIRE(cs_rf_find(&v,0,0,&e,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r));
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK);memset(&r,0xB6,sizeof r);report=r;
    memset(&e,0xC7,sizeof e);entry=e;
    REQUIRE(cs_rf_find(&v,0,32768,&e,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r) && !memcmp(&e,&entry,sizeof e));
    REQUIRE(cs_rf_find(&v,0,-32769,&e,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r));
    REQUIRE(cs_rf_find(0,0,0,&e,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r));
    REQUIRE(cs_rf_find(&v,0,0,0,&r)==CS_RF_ARGUMENT && !memcmp(&r,&report,sizeof r));
    REQUIRE(cs_rf_find(&v,0,0,&e,0)==CS_RF_ARGUMENT && !memcmp(&e,&entry,sizeof e));
    fixture other;REQUIRE(base(&other)==0);cs_rf_view two;REQUIRE(cs_rf_open(other.bytes,other.size,&two,&r)==CS_RF_OK);
    for(unsigned n=0;n<8;++n){REQUIRE(cs_rf_find(n&1u?&v:&two,0x72303121,-32768,&e,&r)==CS_RF_OK);}
    cs_rf_view saved=v;put32(other.bytes,12,UINT32_MAX);
    REQUIRE(cs_rf_open(other.bytes,other.size,&v,&r)==CS_RF_OVERFLOW && !memcmp(&v,&saved,sizeof v));
    REQUIRE(cs_rf_find(&v,0x72303121,-32768,&e,&r)==CS_RF_OK);
    uint8_t odd[445];memcpy(odd+1,f.bytes,444);
    REQUIRE(cs_rf_open(odd+1,444,&two,&r)==CS_RF_OK && cs_rf_find(&two,0x72303121,-32768,&e,&r)==CS_RF_OK);
    free(f.bytes);free(other.bytes);return 0;
}
static int large(void)
{
    for(unsigned k=0;k<5;++k){
        layout l={256,0,0,0,0,0,0,0};item p={7,0,"u","z",0};group g={0x72303121,&p,1};
        char *data=0;switch(k){case 0:l.type_pad=0x8000-28;break;case 1:l.ref_pad=0x8000-10;break;
        case 2:l.name_entry_pad=0x8000;break;case 3:l.data_pad=0xFFFFFF;break;
        default:data=malloc(32770);REQUIRE(data!=0);memset(data,'Z',32769);data[32769]=0;p.data=data;break;}
        fixture f;REQUIRE(build(&f,&g,1,&l)==0);cs_rf_view v;cs_rf_entry e;cs_rf_report r;
        REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK && cs_rf_find(&v,g.type,7,&e,&r)==CS_RF_OK);
        if(k==3) REQUIRE(e.data.offset-f.data==0x1000003 && e.data.length==1 && v.data.length>0xFFFFFF);
        if(k==4) REQUIRE(e.data.length==32769 && (r.warnings&CS_RF_WARN_LARGE));
        uint32_t cuts[]={15,f.data,f.map,f.size-1};
        for(unsigned i=0;i<4;++i)REQUIRE(cs_rf_open(f.bytes,cuts[i],&v,&r)!=CS_RF_OK);
        free(f.bytes);free(data);
    }
    item *items=calloc(5000,sizeof *items);REQUIRE(items!=0);
    for(unsigned i=0;i<5000;++i){items[i].id=(int32_t)i;items[i].data="";}
    group g={0x72303121,items,5000};layout l={256,0,0,0,0,0,0,0};fixture f;
    REQUIRE(build(&f,&g,1,&l)==0);cs_rf_view v;cs_rf_report r;cs_rf_entry e;
    REQUIRE(cs_rf_open(f.bytes,f.size,&v,&r)==CS_RF_OK && r.types==1 && r.references==5000);
    REQUIRE(cs_rf_find(&v,g.type,4999,&e,&r)==CS_RF_OK && r.references==5000 && e.data.length==0);
    free(f.bytes);free(items);return 0;
}
static int expected(FILE *in,uint32_t *values,unsigned n)
{uint8_t bytes[4];for(unsigned i=0;i<n;++i){if(fread(bytes,1,4,in)!=4)return 1;values[i]=get32(bytes,0);}return 0;}
static int same_report(const cs_rf_report *r,const uint32_t *v)
{return (uint32_t)r->status==v[0] && (uint32_t)r->field==v[1] && r->position==v[2] && r->warnings==v[3] && r->types==v[4] && r->references==v[5];}
static int oracle(const char *raw,const char *model)
{
    fixture f;REQUIRE(base(&f)==0);FILE *input=fopen(raw,"rb"),*in=fopen(model,"rb");REQUIRE(input!=0 && in!=0);
    uint8_t original[444],snapshot[444];REQUIRE(fread(original,1,444,input)==444 && fgetc(input)==EOF && !memcmp(original,f.bytes,444));fclose(input);
    for(uint32_t at=0;at<444;++at){for(unsigned value=0;value<256;++value){
        f.bytes[at]=(uint8_t)value;memcpy(snapshot,f.bytes,444);uint32_t v[22];REQUIRE(expected(in,v,22)==0);
        cs_rf_view view,before;cs_rf_report report;memset(&view,0xA5,sizeof view);before=view;
        cs_rf_result result=cs_rf_open(f.bytes,444,&view,&report);
        if(!same_report(&report,v)){fprintf(stderr,"mutation %u/%u open got %d/%d/%u/%u/%u/%u expected %u/%u/%u/%u/%u/%u\n",at,value,result,report.field,report.position,report.warnings,report.types,report.references,v[0],v[1],v[2],v[3],v[4],v[5]);return 1;}
        REQUIRE(!memcmp(f.bytes,snapshot,444));
        if(result!=CS_RF_OK)REQUIRE(!memcmp(&view,&before,sizeof view));
        else{
            cs_rf_entry e,old;memset(&e,0xB6,sizeof e);old=e;
            result=cs_rf_find(&view,0x72303121,-32768,&e,&report);REQUIRE(same_report(&report,v+6));
            if(result==CS_RF_OK || result==CS_RF_REFERENCE){
                uint32_t actual[]={e.type,(uint32_t)e.id,e.reference,e.attributes,e.name_present,e.specification,e.name.offset,e.name.length,e.data.offset,e.data.length};
                for(unsigned i=0;i<10;++i)REQUIRE(actual[i]==v[i+12]);
                REQUIRE(e.name.offset<=444 && e.name.length<=444-e.name.offset && e.data.offset<=444 && e.data.length<=444-e.data.offset);
            }else REQUIRE(!memcmp(&e,&old,sizeof e));
        }
    }f.bytes[at]=original[at];}
    REQUIRE(fgetc(in)==EOF);fclose(in);free(f.bytes);puts("113664 original mutations match independent integer model; outputs/input preserved.");return 0;
}
int main(int argc,char **argv)
{
    if(argc==4 && !strcmp(argv[1],"oracle"))return oracle(argv[2],argv[3]);
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"valid"))return valid();
    if(!strcmp(argv[1],"malformed"))return malformed();
    if(!strcmp(argv[1],"lookup"))return lookup();
    if(!strcmp(argv[1],"arguments"))return arguments();
    if(!strcmp(argv[1],"large"))return large();
    return 2;
}
