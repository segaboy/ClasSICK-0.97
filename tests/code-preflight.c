/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independently generated data, never executed. */
#include "../personality/system1/code-preflight.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct {
    uint8_t low[1024],ram[131072],source[600000];
    cs_guest_region regions[6];cs_guest_map map;cs_mm_manager manager;cs_life_controller life;
    cs_m68k_session session;cs_trap_gateway gateway;cs_code_context context;
    cs_life_plan plan;cs_life_request reset;cs_life_report lr;cs_life_scrap scrap;
    cs_life_entry inventory[6];uint8_t staging[128];
    cs_code_inspect_request inspect;cs_code_layout_request request;
    cs_code_manifest manifest;cs_code_layout layout;cs_code_report report;cs_code_workspace work;
    uint32_t data[18],ref[18],map_at,names;size_t length;
} fixture;
static fixture *make(void){fixture *x=calloc(1,sizeof *x);REQUIRE(x);return x;}
static void put16(uint8_t *b,uint32_t at,uint32_t v){b[at]=(uint8_t)(v>>8);b[at+1]=(uint8_t)v;}
static void put32(uint8_t *b,uint32_t at,uint32_t v){put16(b,at,v>>16);put16(b,at+2,v);}
/* Standalone declarative resource encoder. Group membership is constructed
 * from a mathematical partition, not from parser/manager helpers. */
static void source(fixture *x,uint32_t entries,uint32_t segments,uint32_t segment_bytes,uint32_t trailing)
{
    REQUIRE(entries && entries<=65 && segments && segments<=17 && segments<=entries);
    memset(x->source,0,sizeof x->source);uint32_t at=256,n=segments+1u;
    for(uint32_t s=0;s<n;++s){uint32_t bytes=s?segment_bytes:16u+entries*8u+trailing;
        put32(x->source,at,bytes);x->data[s]=at+4u;at+=4u+bytes;}
    x->map_at=at;x->names=at+38u+n*12u;x->length=x->names+4u;REQUIRE(x->length<=sizeof x->source);
    put32(x->source,0,256);put32(x->source,4,at);put32(x->source,8,at-256u);put32(x->source,12,42u+n*12u);
    put16(x->source,at+24u,28);put16(x->source,at+26u,38u+n*12u);put16(x->source,at+28u,0);
    put32(x->source,at+30u,CS_CODE_TYPE);put16(x->source,at+34u,n-1u);put16(x->source,at+36u,10);
    x->source[x->names]=3;memcpy(x->source+x->names+1u,"own",3);
    for(uint32_t s=0;s<n;++s){uint32_t ref=at+38u+s*12u,offset=x->data[s]-260u;x->ref[s]=ref;
        put16(x->source,ref,s);put16(x->source,ref+2u,s?65535u:0u);
        x->source[ref+5u]=(uint8_t)(offset>>16);put16(x->source,ref+6u,offset);}
    put32(x->source,x->data[0],32u+entries*8u);put32(x->source,x->data[0]+4u,16);
    put32(x->source,x->data[0]+8u,entries*8u);put32(x->source,x->data[0]+12u,32);
    for(uint32_t i=0;i<entries;++i){uint32_t sid=1u+(i*segments)/entries,entry=x->data[0]+16u+i*8u;
        put16(x->source,entry,4);put16(x->source,entry+2u,0x3F3C);put16(x->source,entry+4u,sid);put16(x->source,entry+6u,0xA9F0);
        if(!i || ((i-1u)*segments)/entries!=sid-1u)put16(x->source,x->data[sid],i*8u);
        uint32_t count=0;for(uint32_t j=0;j<entries;++j)if(1u+(j*segments)/entries==sid)++count;
        put16(x->source,x->data[sid]+2u,count);
        for(uint32_t j=4;j<segment_bytes;++j)x->source[x->data[sid]+j]=(uint8_t)(j*37u+sid*11u);
    }
    x->inspect.source=x->source;x->inspect.length=x->length;
}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->low,0xB3,sizeof x->low);memset(x->ram,0xC7,sizeof x->ram);
    x->regions[0]=(cs_guest_region){x->low,x->low,0,1024,3};
    x->regions[1]=(cs_guest_region){x->ram,x->ram,0x1000,131072,3};x->map=(cs_guest_map){x->regions,2};
    cs_mm_config config={{0x400,0x1000,0x1800},{0x402,0x2000,0x12000}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&config)==CS_MM_OK);
    REQUIRE(cs_life_init(&x->life,&x->manager)==CS_LIFE_OK);
    x->context=(cs_code_context){CS_LIFE_ORDINARY,0,0,0,0,&x->map,&x->life,0};
    x->inspect.context=x->context;x->inspect.maximum_source=sizeof x->source;
    x->request.context=x->context;x->request.manifest=&x->manifest;
    x->request.b=0x19000;x->request.s=1024;x->request.h_min=0;
    memset(&x->manifest,0xA9,sizeof x->manifest);memset(&x->layout,0xDB,sizeof x->layout);
    source(x,4,2,8,0);
}
static cs_code_result inspect(fixture *x)
{return cs_code_inspect(&x->inspect,&x->work,sizeof x->work,&x->manifest,&x->report);}
static cs_code_result layout(fixture *x)
{return cs_code_check_layout(&x->request,&x->work,sizeof x->work,&x->layout,&x->report);}
static void unchanged(fixture *x,const fixture *saved,int output)
{
    REQUIRE(!memcmp(x->low,saved->low,sizeof x->low));REQUIRE(!memcmp(x->ram,saved->ram,sizeof x->ram));
    REQUIRE(!memcmp(x->source,saved->source,sizeof x->source));REQUIRE(!memcmp(&x->manager,&saved->manager,sizeof x->manager));
    REQUIRE(!memcmp(&x->life,&saved->life,sizeof x->life));REQUIRE(!memcmp(&x->session,&saved->session,sizeof x->session));
    REQUIRE(!memcmp(&x->gateway,&saved->gateway,sizeof x->gateway));REQUIRE(!memcmp(x->regions,saved->regions,sizeof x->regions));
    REQUIRE(!memcmp(&x->plan,&saved->plan,sizeof x->plan) && !memcmp(x->staging,saved->staging,sizeof x->staging));
    if(output==1)REQUIRE(!memcmp(&x->manifest,&saved->manifest,sizeof x->manifest));
    if(output==2)REQUIRE(!memcmp(&x->layout,&saved->layout,sizeof x->layout));
}
static void refused(fixture *x,cs_code_result expected,int check_layout)
{
    fixture *s=make();memcpy(s,x,sizeof *s);cs_code_result result=check_layout?layout(x):inspect(x);
    if(result!=expected){fprintf(stderr,"expected %u got %u\n",(unsigned)expected,(unsigned)result);exit(1);}
    unchanged(x,s,check_layout?2:1);REQUIRE(!x->report.execution_authorized);free(s);
}
static void success(void)
{
    fixture *x=make(),*s=make();setup(x);memcpy(s,x,sizeof *s);REQUIRE(inspect(x)==CS_CODE_OK);unchanged(x,s,0);
    REQUIRE(x->manifest.entry_count==4 && x->manifest.segment_count==2 && !x->manifest.execution_authorized);
    REQUIRE(x->manifest.offset_policy==CS_CODE_RESOURCE_START_LITERAL && x->manifest.historical_offset_parity_unverified);
    REQUIRE(x->manifest.code0.entry.name_present && x->manifest.code0.entry.name.length==3);
    REQUIRE(x->manifest.segments[0].first_entry==0 && x->manifest.segments[1].first_entry==2);
    REQUIRE(layout(x)==CS_CODE_OK);unchanged(x,s,0);
    REQUIRE(x->layout.a5==0x18FC0 && x->layout.globals_begin==0x18FB0 && x->layout.stack_base==0x18BB0);
    REQUIRE(x->layout.table_begin==0x18FE0 && x->layout.table_end==x->request.b && x->layout.candidate_appl_limit==0x12000);
    REQUIRE(x->layout.heap_epoch==1 && x->layout.epoch_observed && !x->layout.source_revalidated_for_layout && !x->layout.execution_authorized);
    /* Source can go away after inspection: layout is explicitly snapshot-only. */
    memset(x->source,0,sizeof x->source);REQUIRE(layout(x)==CS_CODE_OK && !x->layout.source_revalidated_for_layout);
    free(s);free(x);
}
static void code0(void)
{
    fixture *x=make();
    for(uint32_t field=0;field<4;++field){setup(x);put32(x->source,x->data[0]+field*4u,33);refused(x,CS_CODE_CODE0_FORMAT,0);REQUIRE((uint32_t)x->report.field==field+1u);}
    const uint32_t values[]={0,2,10,520,UINT32_C(0xFFFFFFF8)};
    for(unsigned i=0;i<sizeof values/sizeof *values;++i){setup(x);put32(x->source,x->data[0]+8u,values[i]);refused(x,CS_CODE_CODE0_FORMAT,0);}
    for(uint32_t n=0;n<16;++n){setup(x);put32(x->source,x->data[0]-4u,n);refused(x,CS_CODE_CODE0_FORMAT,0);}
    setup(x);put32(x->source,x->data[0]-4u,47);refused(x,CS_CODE_CODE0_FORMAT,0);
    setup(x);put32(x->source,x->data[0]+12u,30);refused(x,CS_CODE_CODE0_FORMAT,0);
    setup(x);put32(x->source,x->data[0]+12u,0x8000);refused(x,CS_CODE_CODE0_FORMAT,0);
    setup(x);source(x,4,2,8,2);put32(x->source,x->data[0]+12u,34);put32(x->source,x->data[0],68);
    REQUIRE(inspect(x)==CS_CODE_OK);REQUIRE(x->manifest.warnings==7);REQUIRE(layout(x)==CS_CODE_OK && x->layout.table_end==x->request.b-2u);
    /* Padding is ignored: it cannot become a spurious entry. */
    x->source[x->data[0]+48u]=255;REQUIRE(inspect(x)==CS_CODE_OK);
    for(uint32_t bit=1;bit<=4;bit*=2u){setup(x);if(bit==4)source(x,4,2,8,2);
        if(bit==1){put32(x->source,x->data[0]+12u,34);put32(x->source,x->data[0],66);}
        if(bit==2)put32(x->source,x->data[0],66);
        REQUIRE(inspect(x)==CS_CODE_OK && x->manifest.warnings==bit && layout(x)==CS_CODE_OK);
        REQUIRE(x->layout.warnings==bit && !x->report.execution_authorized);}
    free(x);
}
static void entries(void)
{
    fixture *x=make();const uint32_t words[]={0,0x4EF9,0x3F3D,0xA800};
    for(unsigned i=0;i<4;++i){setup(x);put16(x->source,x->data[0]+18u,words[i]);refused(x,CS_CODE_ENTRY_FORM,0);REQUIRE(x->report.slot_valid && !x->report.slot);}
    setup(x);put16(x->source,x->data[0]+22u,0xADF0);refused(x,CS_CODE_ENTRY_FORM,0);
    const uint32_t ids[]={0,2,32768,65535};
    for(unsigned i=0;i<4;++i){setup(x);put16(x->source,x->data[0]+20u,ids[i]);refused(x,CS_CODE_ENTRY_FORM,0);}
    setup(x);put16(x->source,x->data[0]+44u,0);refused(x,CS_CODE_ENTRY_FORM,0);REQUIRE(x->report.slot==3);
    setup(x);put16(x->source,x->data[0]+42u,0);refused(x,CS_CODE_ENTRY_FORM,0);REQUIRE(x->report.slot==3);
    free(x);
}
static void segments(void)
{
    fixture *x=make();const uint32_t count[]={0,3,65,65535};
    for(unsigned i=0;i<4;++i){setup(x);put16(x->source,x->data[1]+2u,count[i]);refused(x,CS_CODE_SEGMENT_HEADER,0);}
    const uint32_t offset[]={1,8,32,65528};
    for(unsigned i=0;i<4;++i){setup(x);put16(x->source,x->data[1],offset[i]);refused(x,CS_CODE_SEGMENT_HEADER,0);}
    setup(x);put16(x->source,x->data[0]+28u,2);refused(x,CS_CODE_SEGMENT_HEADER,0);
    setup(x);put32(x->source,x->data[1]-4u,3);refused(x,CS_CODE_SEGMENT_HEADER,0);
    const uint32_t routine[]={0,2,3,5,8,65534};
    for(unsigned i=0;i<6;++i){setup(x);put16(x->source,x->data[0]+16u,routine[i]);refused(x,CS_CODE_ROUTINE_RANGE,0);}
    setup(x);put16(x->source,x->data[0]+16u,6);REQUIRE(inspect(x)==CS_CODE_OK);
    /* 4 succeeds only for the selected literal base in a six-byte resource;
     * adding the header as an alternative base would be out of range. */
    setup(x);source(x,4,2,6,0);REQUIRE(inspect(x)==CS_CODE_OK);REQUIRE(x->manifest.offset_policy==CS_CODE_RESOURCE_START_LITERAL);
    setup(x);source(x,4,2,4,0);refused(x,CS_CODE_ROUTINE_RANGE,0);
    setup(x);source(x,4,2,32768,0);put16(x->source,x->data[0]+16u,32766);REQUIRE(inspect(x)==CS_CODE_OK);
    setup(x);put16(x->source,x->ref[2],32767);put16(x->source,x->data[0]+36u,32767);put16(x->source,x->data[0]+44u,32767);
    REQUIRE(inspect(x)==CS_CODE_OK && x->manifest.segments[1].resource.entry.id==32767);
    setup(x);put16(x->source,x->data[0]+28u,2);put16(x->source,x->data[0]+36u,1);refused(x,CS_CODE_SEGMENT_HEADER,0);
    /* Reader tolerance does not waive independent segment group checks. */
    setup(x);memcpy(x->source+x->ref[2]+5u,x->source+x->ref[1]+5u,3);refused(x,CS_CODE_SEGMENT_HEADER,0);
    free(x);
}
static void reader(void)
{
    fixture *x=make();setup(x);x->inspect.length=15;refused(x,CS_CODE_SOURCE,0);REQUIRE(x->report.reader_valid);
    setup(x);put16(x->source,x->ref[0],9);refused(x,CS_CODE_NOT_FOUND,0);
    setup(x);put16(x->source,x->ref[1],0);refused(x,CS_CODE_AMBIGUOUS,0);
    setup(x);put16(x->source,x->ref[2],9);refused(x,CS_CODE_NOT_FOUND,0);REQUIRE(x->report.id==2 && x->report.field==CS_CODE_FIELD_NONE && !x->report.position_valid);
    for(uint32_t s=0;s<3;++s){setup(x);x->source[x->ref[s]+4u]=0x80;memset(x->source+x->ref[s]+5u,255,7);
        refused(x,CS_CODE_UNRESOLVED_REFERENCE,0);REQUIRE(x->report.id==(int32_t)s);
        setup(x);x->source[x->ref[s]+4u]=0x40;refused(x,CS_CODE_UNSUPPORTED_STORAGE,0);}
    setup(x);x->source[x->ref[1]+4u]=0xA;REQUIRE(inspect(x)==CS_CODE_OK);
    REQUIRE(x->manifest.segments[0].resource.entry.attributes==0xA && (x->manifest.segments[0].resource.reader_warnings&CS_RF_WARN_ATTRIBUTES));
    for(uint32_t a=0;a<64;++a){setup(x);for(uint32_t s=0;s<3;++s)x->source[x->ref[s]+4u]=(uint8_t)a;
        REQUIRE(inspect(x)==CS_CODE_OK && layout(x)==CS_CODE_OK);}
    setup(x);x->source[x->ref[0]+4u]=0xC0;refused(x,CS_CODE_UNRESOLVED_REFERENCE,0);
    setup(x);uint32_t old_refs=x->ref[0];memmove(x->source+old_refs+8u,x->source+old_refs,x->length-old_refs);
    memmove(x->source+x->names+20u,x->source+x->names+8u,4);
    memcpy(x->source+x->names+8u,x->source+old_refs+8u,12);put16(x->source,x->names+8u,7);x->inspect.length+=20u;
    put32(x->source,12,(uint32_t)x->inspect.length-x->map_at);put16(x->source,x->map_at+26u,x->names+20u-x->map_at);
    put16(x->source,x->map_at+28u,1);put16(x->source,x->map_at+36u,18);
    put32(x->source,x->map_at+38u,CS_CODE_TYPE);put16(x->source,x->map_at+42u,0);put16(x->source,x->map_at+44u,54);
    refused(x,CS_CODE_AMBIGUOUS,0);
    free(x);
}
static void limits(void)
{
    fixture *x=make();setup(x);source(x,64,16,32768,0);x->inspect.maximum_source=(uint32_t)x->length;
    REQUIRE(inspect(x)==CS_CODE_OK && x->manifest.entry_count==64 && x->manifest.segment_count==16);
    setup(x);source(x,65,1,8,0);refused(x,CS_CODE_PROFILE_LIMIT,0);
    setup(x);source(x,17,17,8,0);refused(x,CS_CODE_PROFILE_LIMIT,0);REQUIRE(x->report.slot==16);
    setup(x);source(x,4,2,32769,0);refused(x,CS_CODE_PROFILE_LIMIT,0);
    setup(x);x->inspect.maximum_source=(uint32_t)x->length-1u;refused(x,CS_CODE_PROFILE_LIMIT,0);
    setup(x);x->inspect.maximum_source=0;refused(x,CS_CODE_ARGUMENT,0);
    /* A falsely oversized span in this compact fixture aliases protected
     * objects. Mandatory native safety takes precedence over profile length. */
    if(SIZE_MAX>UINT32_MAX){setup(x);x->inspect.length=(size_t)UINT32_MAX+1u;refused(x,CS_CODE_ARGUMENT,0);}
    setup(x);REQUIRE(cs_code_inspect(&x->inspect,&x->work,sizeof x->work-1u,&x->manifest,&x->report)==CS_CODE_ARGUMENT);
    free(x);
}
static void aliases(void)
{
    fixture *x=make(),*s=make();setup(x);memcpy(s,x,sizeof *s);
    REQUIRE(cs_code_inspect(&x->inspect,&x->work,sizeof x->work,&x->work.manifest,&x->report)==CS_CODE_ARGUMENT);
    REQUIRE(cs_code_inspect(&x->inspect,&x->work,sizeof x->work,&x->manifest,(cs_code_report *)(void *)x->source)==CS_CODE_ARGUMENT);
    REQUIRE(cs_code_inspect(&x->inspect,&x->work,sizeof x->work,(cs_code_manifest *)(void *)x->ram,&x->report)==CS_CODE_ARGUMENT);
    REQUIRE(cs_code_inspect(&x->inspect,(cs_code_workspace *)(void *)x->source,sizeof x->work,&x->manifest,&x->report)==CS_CODE_ARGUMENT);
    unchanged(x,s,1);REQUIRE(!memcmp(&x->report,&s->report,sizeof x->report));
    x->inspect.source=x->ram;x->inspect.length=200;memcpy(s,x,sizeof *s);refused(x,CS_CODE_ARGUMENT,0);REQUIRE(!memcmp(&x->report,&s->report,sizeof x->report));
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);memcpy(s,x,sizeof *s);
    REQUIRE(cs_code_check_layout(&x->request,&x->work,sizeof x->work,(cs_code_layout *)(void *)&x->manifest,&x->report)==CS_CODE_ARGUMENT);
    REQUIRE(cs_code_check_layout(&x->request,&x->work,sizeof x->work,&x->layout,(cs_code_report *)(void *)&x->life)==CS_CODE_ARGUMENT);
    unchanged(x,s,2);free(s);free(x);
}
static void state(void)
{
    fixture *x=make();
    for(uint32_t i=0;i<6;++i){setup(x);REQUIRE(inspect(x)==CS_CODE_OK);
        if(i==0)x->request.context.context=CS_LIFE_UNKNOWN_CONTEXT;
        if(i==1)x->request.context.context=CS_LIFE_INTERRUPT;
        if(i==2)x->request.context.active_root=1;
        if(i==3)x->request.context.active_callback=1;
        if(i==4)x->request.context.active_patch=1;
        if(i==5)x->request.context.active_manager=1;
        refused(x,CS_CODE_CONTEXT,1);x->inspect.context=x->request.context;refused(x,CS_CODE_CONTEXT,0);}
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->life.state=CS_LIFE_PREPARED;x->request.s=1;refused(x,CS_CODE_STATE,1);REQUIRE(x->report.lifecycle_valid);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->life.state=CS_LIFE_FAULTED;refused(x,CS_CODE_STATE,1);REQUIRE(x->report.lifecycle==CS_LIFE_FAULT);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->low[0x31A]=1;refused(x,CS_CODE_STATE,1);REQUIRE(x->report.lifecycle==CS_LIFE_MEMORY);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->life.epoch=0;refused(x,CS_CODE_STATE,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->life.epoch=UINT32_MAX;REQUIRE(layout(x)==CS_CODE_OK && x->layout.heap_epoch==UINT32_MAX);
    setup(x);x->session.tag=0x53455331;x->session.calls.map=&x->map;x->session.active=1;
    x->inspect.context.session=&x->session;refused(x,CS_CODE_CONTEXT,0);
    x->session.active=0;REQUIRE(inspect(x)==CS_CODE_OK);x->request.context.session=&x->session;REQUIRE(layout(x)==CS_CODE_OK);
    x->session.stepping=1;refused(x,CS_CODE_CONTEXT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);
    const uint32_t globals[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
    for(unsigned i=0;i<6;++i){uint32_t v=0;REQUIRE(cs_guest_read(&x->map,globals[i],4,1,&v)==CS_GUEST_OK);
        x->inventory[i]=(cs_life_entry){CS_LIFE_GLOBAL,(i==1 || i==2)?CS_LIFE_REBIND:CS_LIFE_PRESERVE,77,globals[i],4,v,0};}
    x->reset.operation=CS_LIFE_RESET;x->reset.context=CS_LIFE_ORDINARY;
    x->reset.stack_base=0x18000;x->reset.stack_size=1024;x->reset.stack_reference=0x18400;x->reset.reserve=1024;
    x->reset.scrap=&x->scrap;x->reset.staging=x->staging;x->reset.staging_bytes=sizeof x->staging;
    x->reset.inventory=x->inventory;x->reset.count=6;
    REQUIRE(cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr)==CS_LIFE_OK);
    refused(x,CS_CODE_STATE,1);
    REQUIRE(inspect(x)==CS_CODE_OK); /* Inspection has no lifecycle admission gate. */
    fixture *saved=make();memcpy(saved,x,sizeof *saved);
    REQUIRE(cs_code_check_layout(&x->request,&x->work,sizeof x->work,&x->layout,(cs_code_report *)(void *)x->staging)==CS_CODE_ARGUMENT);
    unchanged(x,saved,2);free(saved);free(x);
}
static void layouts(void)
{
    fixture *x=make();const uint32_t b[]={0,32,0x400,0x1000002,UINT32_MAX};
    for(unsigned i=0;i<5;++i){setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.b=b[i];refused(x,CS_CODE_LAYOUT,1);}
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.s=0;refused(x,CS_CODE_LAYOUT,1);
    x->request.s=1;refused(x,CS_CODE_LAYOUT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.h_min=UINT32_MAX-1u;refused(x,CS_CODE_LAYOUT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.b=0x11000;refused(x,CS_CODE_LAYOUT_UNSUPPORTED,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.b=0x19000;x->request.s=0x18000;refused(x,CS_CODE_LAYOUT_UNSUPPORTED,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.h_min=0x10000;REQUIRE(layout(x)==CS_CODE_OK);
    x->request.h_min=0x10002;refused(x,CS_CODE_LAYOUT,1);
    x->request.h_min=1;refused(x,CS_CODE_LAYOUT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.s=UINT32_MAX-1u;refused(x,CS_CODE_LAYOUT,1);
    setup(x);put32(x->source,x->data[0]+4u,0);REQUIRE(inspect(x)==CS_CODE_OK && layout(x)==CS_CODE_OK);
    setup(x);put32(x->source,x->data[0]+4u,0x18000);REQUIRE(inspect(x)==CS_CODE_OK);refused(x,CS_CODE_LAYOUT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->request.b=0x30000;refused(x,CS_CODE_LAYOUT,1);
    /* Map a separate logical world onto application backing bytes. Manager
     * integrity still passes, but destinations physically collide. */
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->regions[1].size=0x12000;
    x->regions[2]=(cs_guest_region){x->ram+0x1000,x->ram+0x1000,0x18000,0x2000,3};x->map.count=3;
    refused(x,CS_CODE_LAYOUT,1);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->regions[1].size=0x12000;
    x->regions[2]=(cs_guest_region){x->ram+0x13000,0,0x18000,0x2000,1};x->map.count=3;refused(x,CS_CODE_LAYOUT,1);
    /* Exclusive world end at the strict24 boundary is admitted. No execute bit
     * is present, and no zero-length below-A5 window is requested. */
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->regions[2]=(cs_guest_region){x->source,x->source,0xFF0000,65536,3};
    x->map.count=3;x->request.b=0x1000000;REQUIRE(layout(x)==CS_CODE_OK && x->layout.table_end==0x1000000);
    setup(x);REQUIRE(inspect(x)==CS_CODE_OK);x->regions[1].size=0x12000;
    x->regions[2]=(cs_guest_region){x->ram+0x13000,x->ram+0x13000,0x18BB0,1024,3};
    x->regions[3]=(cs_guest_region){x->source,x->source,0x18FB0,16,3};
    x->regions[4]=(cs_guest_region){x->source+16,x->source+16,0x18FC0,64,3};x->map.count=5;
    REQUIRE(layout(x)==CS_CODE_OK);
    x->regions[4].bytes=x->source;x->regions[4].writable=x->source;refused(x,CS_CODE_LAYOUT,1);
    free(x);
}
static void snapshot(void)
{
    fixture *x=make();
    for(unsigned i=0;i<16;++i){setup(x);REQUIRE(inspect(x)==CS_CODE_OK);
        switch(i){case 0:x->manifest.entry_count=65;break;case 1:x->manifest.segment_count=17;break;
        case 2:x->manifest.entries[0].segment_index=16;break;case 3:x->manifest.segments[0].first_entry=UINT32_MAX;break;
        case 4:x->manifest.segments[0].entry_count=UINT32_MAX;break;case 5:x->manifest.execution_authorized=1;break;
        case 6:x->manifest.offset_policy=(cs_code_offset_policy)2;break;case 7:x->manifest.table_offset=UINT32_MAX;break;
        case 8:x->manifest.code0.entry.data.offset=UINT32_MAX;break;case 9:x->manifest.segments[1].resource.entry.id=1;break;
        case 10:x->manifest.entries[1].routine_offset=65534;break;case 11:x->manifest.warnings=7;break;
        case 12:x->manifest.historical_offset_parity_unverified=0;break;case 13:x->manifest.tag=0;break;
        case 14:x->manifest.maximum_source=0;break;default:x->manifest.entries[0].segment_index=1;break;}
        refused(x,CS_CODE_ARGUMENT,1);
    }
    free(x);
}
static void model(void)
{
    fixture *x=make();setup(x);uint32_t cases=0;
    for(uint32_t e=1;e<=64;++e)for(uint32_t s=1;s<=16 && s<=e;++s){source(x,e,s,6,0);
        REQUIRE(inspect(x)==CS_CODE_OK);REQUIRE(x->manifest.entry_count==e && x->manifest.segment_count==s);
        for(uint32_t i=0;i<e;++i)REQUIRE(x->manifest.entries[i].segment_index==(i*s)/e);
        REQUIRE(layout(x)==CS_CODE_OK);uint32_t expected_a5=x->request.b-(32u+e*8u);
        REQUIRE(x->layout.a5==expected_a5 && x->layout.stack_base==expected_a5-16u-x->request.s);++cases;
    }
    REQUIRE(cases==904);
    /* Sweep truncations with an independent oracle for required source length.
     * Each refusal preserves outputs and all protected data. */
    source(x,4,2,8,0);size_t length=x->length;
    for(size_t n=0;n<length;++n){x->inspect.length=n;refused(x,CS_CODE_SOURCE,0);}
    printf("904 independent partitions and %zu truncated lengths PASS\n",length);free(x);
}
int main(int argc,char **argv)
{
    REQUIRE(argc==2);
    if(!strcmp(argv[1],"success"))success();else if(!strcmp(argv[1],"code0"))code0();
    else if(!strcmp(argv[1],"entries"))entries();else if(!strcmp(argv[1],"segments"))segments();
    else if(!strcmp(argv[1],"reader"))reader();else if(!strcmp(argv[1],"limits"))limits();
    else if(!strcmp(argv[1],"aliases"))aliases();else if(!strcmp(argv[1],"state"))state();
    else if(!strcmp(argv[1],"layout"))layouts();else if(!strcmp(argv[1],"snapshot"))snapshot();
    else if(!strcmp(argv[1],"model"))model();else REQUIRE(0);
    puts("SPEC-0032 original data-only conformance PASS");return 0;
}
