/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original data-only conformance. */
#include "../personality/system1/code-image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct {
    uint8_t low[1024],ram[131072],source[600000],staging[524816],sentinel[8];
    cs_guest_region regions[6];cs_guest_map map;cs_mm_manager manager;cs_life_controller life;
    cs_m68k_session session;cs_trap_gateway gateway;cs_res_service resource;
    cs_life_entry inventory[8];const cs_res_service *resources[2];cs_life_scrap scrap;
    cs_image_service image;cs_image_request request;cs_image_workspace work;
    cs_image_description description;cs_image_report report;
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
    x->request.source=x->source;x->request.length=x->length;
}
static void setup(fixture *x,uint32_t end)
{
    memset(x,0,sizeof *x);memset(x->low,0xB3,sizeof x->low);memset(x->ram,0xC7,sizeof x->ram);
    x->regions[0]=(cs_guest_region){x->low,x->low,0,1024,3};
    x->regions[1]=(cs_guest_region){x->ram,x->ram,0x1000,131072,3};x->map=(cs_guest_map){x->regions,2};
    cs_mm_config config={{0x400,0x1000,0x1800},{0x402,0x2000,end}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&config)==CS_MM_OK);
    REQUIRE(cs_life_init(&x->life,&x->manager)==CS_LIFE_OK);
    cs_image_profile profile={sizeof x->source,sizeof x->staging,65536,93};
    REQUIRE(cs_image_init(&x->image,&x->life,&profile)==CS_IMAGE_OK);
    x->inventory[0]=(cs_life_entry){CS_LIFE_FUTURE,CS_LIFE_REFUSE,93,0,sizeof x->image,0,&x->image};
    x->request.context=(cs_image_context){CS_LIFE_ORDINARY,0,0,0,0,0,x->inventory,1,0,0};
    x->request.b=0x19000;x->request.s=1024;x->request.staging=x->staging;x->request.staging_capacity=sizeof x->staging;
    x->request.operation=CS_IMAGE_PREPARE;source(x,4,2,9,0);
    memset(&x->description,0xA3,sizeof x->description);memset(&x->report,0xF1,sizeof x->report);
}
static cs_image_result run(fixture *x)
{return cs_image_execute(&x->image,&x->request,&x->work,sizeof x->work,&x->description,&x->report);}
static void next(fixture *x,cs_image_operation operation)
{x->request.operation=operation;x->request.generation=x->image.generation;x->request.epoch=x->image.epoch;
    x->request.source=0;x->request.length=0;x->request.staging=0;x->request.staging_capacity=0;}
static cs_mm_reply mm(fixture *x,cs_mm_operation op,uint32_t address,uint32_t bytes,unsigned flags)
{cs_mm_request q={op,address,0,bytes,flags,0};cs_mm_reply r;cs_mm_result result;
    REQUIRE(cs_life_manager_call(&x->life,&q,&r,&result)==CS_LIFE_OK && result==CS_MM_OK);return r;}
static uint32_t read32(fixture *x,uint32_t address)
{uint32_t v;REQUIRE(cs_guest_read(&x->map,address,4,CS_GUEST_READ,&v)==CS_GUEST_OK);return v;}
static void guest_same(const fixture *x,const fixture *s)
{REQUIRE(!memcmp(x->low,s->low,sizeof x->low));REQUIRE(!memcmp(x->ram,s->ram,sizeof x->ram));
    REQUIRE(!memcmp(&x->manager,&s->manager,sizeof x->manager));REQUIRE(!memcmp(&x->life,&s->life,sizeof x->life));
    REQUIRE(!memcmp(&x->session,&s->session,sizeof x->session));REQUIRE(!memcmp(&x->gateway,&s->gateway,sizeof x->gateway));}
static void refused(fixture *x,cs_image_result expected,int unsafe)
{fixture *s=make();memcpy(s,x,sizeof *s);cs_image_result actual=run(x);
    if(actual!=expected){fprintf(stderr,"expected %u got %u operation %u state %u\n",(unsigned)expected,(unsigned)actual,(unsigned)x->request.operation,(unsigned)x->image.state);exit(1);}
    guest_same(x,s);REQUIRE(!memcmp(&x->image,&s->image,sizeof x->image));REQUIRE(!memcmp(&x->description,&s->description,sizeof x->description));
    if(unsafe)REQUIRE(!memcmp(&x->report,&s->report,sizeof x->report));
    free(s);}
static void prepared(fixture *x){REQUIRE(run(x)==CS_IMAGE_OK);REQUIRE(x->image.state==CS_IMAGE_PREPARED);next(x,CS_IMAGE_LOAD);}
static void loaded(fixture *x){prepared(x);REQUIRE(run(x)==CS_IMAGE_OK);REQUIRE(x->image.state==CS_IMAGE_LOADED);}
static void success(void)
{
    fixture *x=make(),*saved=make();setup(x,0x12000);memcpy(saved,x,sizeof *x);
    /* A separate allocation is actually freed before LOAD, under ASan too. */
    uint8_t *source_copy=malloc(x->length);REQUIRE(source_copy);memcpy(source_copy,x->source,x->length);x->request.source=source_copy;
    prepared(x);guest_same(x,saved);free(source_copy);memset(x->source,0,sizeof x->source);
    REQUIRE(x->image.staging_bytes==66 && !x->description.execution_authorized && cs_image_external_admit(&x->image)==CS_IMAGE_IN_USE);
    REQUIRE(!x->work.inspect.source && !x->work.inspect.length && !x->work.parser.view.bytes);
    REQUIRE(run(x)==CS_IMAGE_OK);REQUIRE(x->report.guest_committed && !x->report.execution_authorized);
    REQUIRE(x->image.state==CS_IMAGE_LOADED && !x->image.staging_borrow && !x->image.staging && x->image.live==3);
    for(uint32_t i=0;i<2;++i){const cs_image_segment *p=&x->image.segments[i];REQUIRE(p->length==9 && p->extent==10);
        REQUIRE(read32(x,p->handle)==(CS_MM_LOCK|p->address));
        for(uint32_t j=0;j<9;++j)REQUIRE(x->ram[p->address-0x1000u+j]==saved->source[saved->data[i+1u]+j]);
        REQUIRE(x->ram[p->address-0x1000u+9u]==0xC7);}
    uint32_t table=x->image.layout.table_begin;
    for(uint32_t i=0;i<4;++i){REQUIRE(read32(x,table+i*8u)==((i<2?1u:2u)<<16|0x4EF9u));
        REQUIRE(read32(x,table+i*8u+4u)==x->image.segments[i/2u].address+4u);}
    REQUIRE(read32(x,x->image.layout.a5)==0 && read32(x,x->image.layout.a5+16u)==0);
    REQUIRE(!memcmp(x->ram+x->image.layout.stack_base-0x1000u,saved->ram+x->image.layout.stack_base-0x1000u,x->request.s));
    next(x,CS_IMAGE_DESCRIBE);REQUIRE(run(x)==CS_IMAGE_OK && x->description.generation==1 && x->description.live==3);
    next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK && x->image.state==CS_IMAGE_EMPTY && cs_image_external_admit(&x->image)==CS_IMAGE_OK);
    REQUIRE(x->report.generation_valid && x->report.epoch_valid && x->report.disposed==3 && x->report.retired==3 && !x->report.live);
    REQUIRE(!memcmp(x->low,saved->low,sizeof x->low));REQUIRE(!memcmp(&x->manager,&saved->manager,sizeof x->manager));
    for(uint32_t i=0;i<80;++i)REQUIRE(x->ram[0x18FB0u-0x1000u+i]==0);
    source(x,4,2,9,0);x->request.operation=CS_IMAGE_PREPARE;x->request.staging=x->staging;x->request.staging_capacity=sizeof x->staging;
    REQUIRE(run(x)==CS_IMAGE_OK && x->image.generation==2);next(x,CS_IMAGE_CANCEL);REQUIRE(run(x)==CS_IMAGE_OK);
    free(saved);free(x);
}
static void fragmented(fixture *x,uint32_t first)
{
    setup(x,0x2100);source(x,4,2,24,0);put32(x->source,x->data[1]-4u,first);
    uint32_t handles[7];for(unsigned i=0;i<7;++i){cs_mm_reply r=mm(x,CS_MM_NEW_HANDLE,0,i==6?32u:16u,0);REQUIRE(!r.code);handles[i]=r.value;
        REQUIRE(!mm(x,CS_MM_HLOCK,r.value,0,0).code);}
    for(unsigned i=0;i<6;i+=2u)REQUIRE(mm(x,CS_MM_DISPOS_HANDLE,handles[i],0,0).code==0);
}
static void capacity_cases(void)
{
    fixture *x=make(),*saved=make();fragmented(x,24);prepared(x);memcpy(saved,x,sizeof *x);
    REQUIRE(run(x)==CS_IMAGE_CAPACITY && x->image.state==CS_IMAGE_PREPARED && x->report.reply_valid && x->report.reply.code==-108 && !x->report.guest_committed);
    REQUIRE(x->image.generation==1 && x->image.last_generation==1 && cs_image_external_admit(&x->image)==CS_IMAGE_IN_USE);
    REQUIRE(x->report.attempted==1 && !x->report.completed);
    guest_same(x,saved);next(x,CS_IMAGE_CANCEL);REQUIRE(run(x)==CS_IMAGE_OK);
    fragmented(x,16);prepared(x);memcpy(saved,x,sizeof *x);
    REQUIRE(run(x)==CS_IMAGE_CAPACITY && x->image.state==CS_IMAGE_PARTIAL && x->image.live==1 && x->image.copied==1 && !x->image.world_written);
    REQUIRE(x->report.reply_valid && x->report.reply.code==-108 && x->report.guest_committed && x->image.staging_borrow);
    REQUIRE(x->report.attempted==19 && x->report.completed==18);
    REQUIRE(x->image.generation==1 && cs_image_external_admit(&x->image)==CS_IMAGE_IN_USE);
    REQUIRE(!memcmp(x->ram+0x18BB0u-0x1000u,saved->ram+0x18BB0u-0x1000u,1104));
    next(x,CS_IMAGE_LOAD);refused(x,CS_IMAGE_STATE,0);next(x,CS_IMAGE_CANCEL);refused(x,CS_IMAGE_STATE,0);
    uint32_t partial_handle=x->image.segments[0].handle,address=x->image.segments[0].address;
    REQUIRE(cs_guest_write(&x->map,partial_handle,4,address)==CS_GUEST_OK);next(x,CS_IMAGE_RELEASE);refused(x,CS_IMAGE_CHANGED,0);
    REQUIRE(cs_guest_write(&x->map,partial_handle,4,address|CS_MM_LOCK)==CS_GUEST_OK);
    next(x,CS_IMAGE_DESCRIBE);REQUIRE(run(x)==CS_IMAGE_OK && x->description.live==1);
    next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK && !x->report.world_bytes && !x->image.staging_borrow);
    REQUIRE(!memcmp(&x->manager,&saved->manager,sizeof x->manager));REQUIRE(!memcmp(x->low,saved->low,sizeof x->low));
    /* Genuine successful compaction followed by later ordinary OOM. Native
     * pointers are immovable; only this preexisting unlocked handle moves. */
    setup(x,0x2100);source(x,4,2,32,0);put32(x->source,x->data[2]-4u,24);
    uint32_t objects[8];for(unsigned i=0;i<8;++i){cs_mm_reply r=mm(x,i==1?CS_MM_NEW_HANDLE:CS_MM_NEW_PTR,0,16,0);REQUIRE(!r.code);objects[i]=r.value;}
    for(unsigned i=0;i<8;i+=2u)REQUIRE(!mm(x,CS_MM_DISPOS_PTR,objects[i],0,0).code);
    uint32_t old=read32(x,objects[1]);memset(x->ram+old-0x1000u,0x6D,16);prepared(x);
    REQUIRE(run(x)==CS_IMAGE_CAPACITY && x->image.state==CS_IMAGE_PARTIAL && x->report.moved==1);
    uint32_t moved=read32(x,objects[1]);REQUIRE(moved!=old);
    next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK && !x->report.world_bytes);
    REQUIRE(read32(x,objects[1])==moved);for(unsigned i=0;i<16;++i)REQUIRE(x->ram[moved-0x1000u+i]==0x6D);
    /* Exactly enough shared slots, including both system/application handles. */
    setup(x,0x12000);for(unsigned i=0;i<30;++i)REQUIRE(!mm(x,CS_MM_NEW_HANDLE,0,2,i&1u?CS_MM_SYSTEM:0).code);
    loaded(x);next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK);
    setup(x,0x12000);for(unsigned i=0;i<31;++i)REQUIRE(!mm(x,CS_MM_NEW_PTR,0,2,CS_MM_SYSTEM).code);
    refused(x,CS_IMAGE_CAPACITY,0);setup(x,0x2100);source(x,4,2,80,0);refused(x,CS_IMAGE_CAPACITY,0);
    free(saved);free(x);
}
static void ownership(void)
{
    fixture *x=make(),*saved=make();setup(x,0x12000);
    cs_mm_reply sys=mm(x,CS_MM_NEW_HANDLE,0,10,CS_MM_SYSTEM),app=mm(x,CS_MM_NEW_HANDLE,0,12,0);
    REQUIRE(!sys.code && !app.code);REQUIRE(!mm(x,CS_MM_HLOCK,app.value,0,0).code);
    cs_mm_reply scrap=mm(x,CS_MM_NEW_HANDLE,0,14,0);REQUIRE(!scrap.code);
    x->scrap=(cs_life_scrap){14,CS_LIFE_RESIDENT,scrap.value,0,0};
    x->inventory[1]=(cs_life_entry){CS_LIFE_NATIVE,CS_LIFE_PRESERVE,9,0,sizeof x->scrap,0,&x->scrap};x->request.context.count=2;
    memcpy(saved,x,sizeof *x);loaded(x);next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK);
    REQUIRE(!memcmp(&x->manager,&saved->manager,sizeof x->manager));REQUIRE(read32(x,sys.value)==read32(saved,sys.value));
    REQUIRE(!memcmp(x->ram,saved->ram,0x800));REQUIRE(!memcmp(x->low,saved->low,sizeof x->low));
    REQUIRE(!memcmp(&x->scrap,&saved->scrap,sizeof x->scrap));
    setup(x,0x12000);loaded(x);next(x,CS_IMAGE_RELEASE);x->ram[x->image.layout.a5+16u-0x1000u]=1;refused(x,CS_IMAGE_CHANGED,0);
    x->ram[x->image.layout.a5+16u-0x1000u]=0;uint32_t h=x->image.segments[0].handle;
    REQUIRE(cs_guest_write(&x->map,h,4,x->image.segments[0].address)==CS_GUEST_OK);refused(x,CS_IMAGE_CHANGED,0);
    REQUIRE(cs_guest_write(&x->map,h,4,x->image.segments[0].address|CS_MM_LOCK)==CS_GUEST_OK);
    x->request.generation++;refused(x,CS_IMAGE_CHANGED,0);x->request.generation--;x->life.epoch++;refused(x,CS_IMAGE_CHANGED,0);
    x->life.epoch--;REQUIRE(run(x)==CS_IMAGE_OK);free(saved);free(x);
}
static void cancel(void)
{
    fixture *x=make(),*saved=make();setup(x,0x12000);prepared(x);
    x->manager.magic=0;x->life.epoch=9;x->life.state=CS_LIFE_FAULTED;x->regions[1].flags|=CS_GUEST_EXECUTE;
    next(x,CS_IMAGE_CANCEL);memcpy(saved,x,sizeof *x);REQUIRE(run(x)==CS_IMAGE_OK);guest_same(x,saved);
    REQUIRE(x->image.state==CS_IMAGE_EMPTY && !x->image.staging_borrow && x->image.last_generation==1);
    REQUIRE(cs_image_external_admit(&x->image)==CS_IMAGE_CHANGED);
    setup(x,0x12000);prepared(x);next(x,CS_IMAGE_CANCEL);x->inventory[0].action=CS_LIFE_PRESERVE;refused(x,CS_IMAGE_IN_USE,0);
    x->inventory[0].action=CS_LIFE_REFUSE;x->request.context.active_root=1;refused(x,CS_IMAGE_CONTEXT,0);
    free(saved);free(x);
}
static void aliases(void)
{
    fixture *x=make(),*saved=make();setup(x,0x12000);memcpy(saved,x,sizeof *x);
    REQUIRE(cs_image_execute(&x->image,&x->request,&x->work,sizeof x->work,&x->description,(cs_image_report *)(void *)x->ram)==CS_IMAGE_ARGUMENT);
    REQUIRE(cs_image_execute(&x->image,&x->request,&x->work,sizeof x->work,&x->description,(cs_image_report *)(void *)x->source)==CS_IMAGE_ARGUMENT);
    REQUIRE(cs_image_execute(&x->image,&x->request,&x->work,sizeof x->work,(cs_image_description *)(void *)&x->image,&x->report)==CS_IMAGE_ARGUMENT);
    REQUIRE(!memcmp(x,saved,sizeof *x));
    x->request.staging=x->source;refused(x,CS_IMAGE_ARGUMENT,1);
    setup(x,0x12000);x->request.context.count=65;refused(x,CS_IMAGE_ARGUMENT,1);
    setup(x,0x12000);x->request.context.resource_count=9;refused(x,CS_IMAGE_ARGUMENT,1);
    setup(x,0x12000);prepared(x);next(x,CS_IMAGE_DESCRIBE);
    REQUIRE(cs_image_execute(&x->image,&x->request,(cs_image_workspace *)(void *)x->staging,sizeof x->work,&x->description,&x->report)==CS_IMAGE_ARGUMENT);
    memcpy(saved,x,sizeof *x);x->map.regions=x->regions+2;x->map.count=2;x->regions[2]=x->regions[0];x->regions[3]=x->regions[1];
    x->regions[3].bytes=(const uint8_t *)&x->description;x->regions[3].writable=(uint8_t *)&x->description;x->regions[3].size=sizeof x->description;
    refused(x,CS_IMAGE_ARGUMENT,1);free(saved);free(x);
}
static void seal(void)
{
    fixture *x=make();for(unsigned i=0;i<11;++i){setup(x,0x12000);prepared(x);
        switch(i){case 0:x->life.epoch++;break;case 1:x->regions[1].flags|=CS_GUEST_EXECUTE;break;
            case 2:x->staging[0]^=1;break;case 3:x->staging[18]^=1;break;case 4:x->staging[x->image.offsets[0]]^=1;break;
            case 5:REQUIRE(!mm(x,CS_MM_NEW_PTR,0,2,CS_MM_SYSTEM).code);break;
            case 6:x->request.b-=2;break;case 7:x->request.s-=2;break;case 8:x->image.manifest.entry_count=65;break;
            case 9:x->inventory[1]=(cs_life_entry){CS_LIFE_NATIVE,CS_LIFE_PRESERVE,42,0,1,0,&x->names};x->request.context.count=2;break;
            default:x->request.generation++;break;}
        refused(x,i==8?CS_IMAGE_CODE:CS_IMAGE_CHANGED,0);
        next(x,CS_IMAGE_CANCEL);REQUIRE(run(x)==CS_IMAGE_OK);}
    setup(x,0x12000);x->regions[1].flags|=CS_GUEST_EXECUTE;refused(x,CS_IMAGE_CHANGED,0);
    setup(x,0x12000);x->regions[2]=(cs_guest_region){x->ram+0x1080,0,0x30000,2,CS_GUEST_READ|CS_GUEST_EXECUTE};x->map.count=3;
    /* Rebind a newly initialized owner to the truthful three-region map. */
    memset(&x->image,0,sizeof x->image);cs_image_profile p={600000,524816,65536,93};REQUIRE(cs_image_init(&x->image,&x->life,&p)==CS_IMAGE_OK);
    refused(x,CS_IMAGE_CHANGED,0);
    /* Stable-looking outside-heap guest registrations cannot hide a physical
     * alias to an application allocation. Native registrations cannot hide it
     * either. Neither case requires searching opaque object contents. */
    setup(x,0x12000);x->regions[2]=(cs_guest_region){x->ram+0x1080,0,0x30000,2,CS_GUEST_READ};x->map.count=3;
    memset(&x->image,0,sizeof x->image);REQUIRE(cs_image_init(&x->image,&x->life,&p)==CS_IMAGE_OK);
    x->inventory[1]=(cs_life_entry){CS_LIFE_GUEST,CS_LIFE_PRESERVE,19,0x30000,2,0,0};x->request.context.count=2;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);x->inventory[1]=(cs_life_entry){CS_LIFE_NATIVE,CS_LIFE_PRESERVE,19,0,2,0,x->ram+0x1080};x->request.context.count=2;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);x->regions[1].size=0x800;x->regions[2]=(cs_guest_region){x->ram,x->ram,0x2000,0x1F000,3};x->map.count=3;
    cs_mm_config cfg={{0x400,0x1000,0x1800},{0x402,0x2000,0x12000}};REQUIRE(cs_mm_init(&x->manager,&x->map,&cfg)==CS_MM_OK);
    memset(&x->image,0,sizeof x->image);REQUIRE(cs_image_init(&x->image,&x->life,&p)==CS_IMAGE_OK);refused(x,CS_IMAGE_CODE,0);free(x);
}
static void resources(void)
{
    fixture *x=make();setup(x,0x12000);cs_res_profile p={600000,32768,19};REQUIRE(cs_res_init(&x->resource,&x->life,&p)==CS_RES_OK);
    x->resources[0]=&x->resource;x->request.context.resources=x->resources;x->request.context.resource_count=1;
    refused(x,CS_IMAGE_IN_USE,0);
    x->inventory[1]=(cs_life_entry){CS_LIFE_NATIVE,CS_LIFE_PRESERVE,19,0,sizeof x->resource,0,&x->resource};x->request.context.count=2;
    loaded(x);next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK);
    x->request.operation=CS_IMAGE_PREPARE;source(x,4,2,9,0);x->request.staging=x->staging;x->request.staging_capacity=sizeof x->staging;
    x->resource.state=CS_RES_FAULTED;refused(x,CS_IMAGE_IN_USE,0);
    x->resource.state=CS_RES_OPENED;refused(x,CS_IMAGE_IN_USE,0);
    x->resource.state=CS_RES_CLOSED;x->resource.source_borrow=1;x->resource.source=x->source;x->resource.source_length=2;refused(x,CS_IMAGE_ARGUMENT,1);
    x->resource.source_borrow=0;x->resource.manager=&x->image.sealed_manager;refused(x,CS_IMAGE_ARGUMENT,1);free(x);
}
static void profile(void)
{
    fixture *x=make();setup(x,0x12000);x->request.staging_capacity=65;refused(x,CS_IMAGE_CAPACITY,0);
    setup(x,0x12000);x->image.last_generation=UINT32_MAX;refused(x,CS_IMAGE_COUNTER_EXHAUSTED,0);
    setup(x,0x12000);x->image.last_generation=UINT32_MAX-1u;prepared(x);REQUIRE(x->image.generation==UINT32_MAX);next(x,CS_IMAGE_CANCEL);REQUIRE(run(x)==CS_IMAGE_OK);
    x->request.operation=CS_IMAGE_PREPARE;x->request.source=x->source;x->request.length=x->length;x->request.staging=x->staging;x->request.staging_capacity=sizeof x->staging;
    refused(x,CS_IMAGE_COUNTER_EXHAUSTED,0);
    setup(x,0x12000);x->request.length=15;refused(x,CS_IMAGE_CODE,0);REQUIRE(x->report.preflight_valid && !x->report.execution_authorized);
    REQUIRE(!x->work.inspect.source && !x->work.parser.view.bytes);
    setup(x,0x12000);source(x,64,16,32768,0);refused(x,CS_IMAGE_CAPACITY,0);
    setup(x,0x12000);x->request.context.context=CS_LIFE_INTERRUPT;refused(x,CS_IMAGE_CONTEXT,0);
    setup(x,0x12000);x->request.context.inventory=0;x->request.context.count=0;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);x->inventory[0].owner++;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);x->inventory[1]=(cs_life_entry){CS_LIFE_GUEST,CS_LIFE_PRESERVE,19,0x2080,2,0,0};x->request.context.count=2;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);x->inventory[1]=(cs_life_entry){CS_LIFE_FUTURE,CS_LIFE_REFUSE,19,0,4,0,&x->names};x->request.context.count=2;refused(x,CS_IMAGE_IN_USE,0);
    setup(x,0x12000);for(unsigned i=0;i<4;++i)put16(x->sentinel,i*2u,0x4E71);
    x->regions[2]=(cs_guest_region){x->sentinel,0,0x30000,8,5};x->map.count=3;
    memset(&x->image,0,sizeof x->image);cs_image_profile session_profile={600000,524816,65536,93};
    REQUIRE(cs_image_init(&x->image,&x->life,&session_profile)==CS_IMAGE_OK);
    cs_m68k_call_config call_config={0x18BB0,1024,0x30000,4};
    REQUIRE(cs_m68k_session_init(&x->session,&x->map,&x->gateway,0,&call_config)==CS_M68K_SESSION_OK);
    x->request.context.session=&x->session;x->session.active=1;refused(x,CS_IMAGE_CONTEXT,0);
    x->session.active=0;x->session.calls.depth=1;refused(x,CS_IMAGE_CONTEXT,0);
    x->session.calls.depth=0;x->session.exit_pending=1;refused(x,CS_IMAGE_CONTEXT,0);
    x->session.exit_pending=0;prepared(x);x->session.calls.config.max_depth=3;refused(x,CS_IMAGE_CHANGED,0);
    next(x,CS_IMAGE_CANCEL);REQUIRE(run(x)==CS_IMAGE_OK);
    /* Full 64/16/staging upper bound on an explicitly larger hosted map. This
     * has no physical128K or ROM fit implication. */
    setup(x,0x12000);uint8_t *large=malloc(0x100000);REQUIRE(large);memset(large,0xC7,0x100000);
    x->regions[1]=(cs_guest_region){large,large,0x1000,0x100000,3};
    cs_mm_config cfg={{0x400,0x1000,0x1800},{0x402,0x2000,0x90000}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&cfg)==CS_MM_OK);memset(&x->life,0,sizeof x->life);REQUIRE(cs_life_init(&x->life,&x->manager)==CS_LIFE_OK);
    memset(&x->image,0,sizeof x->image);cs_image_profile p={600000,524816,65536,93};REQUIRE(cs_image_init(&x->image,&x->life,&p)==CS_IMAGE_OK);
    source(x,64,16,32768,0);x->request.b=0x100000;prepared(x);REQUIRE(x->image.staging_bytes==524816);
    REQUIRE(run(x)==CS_IMAGE_OK && x->image.live==65535);next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK);free(large);free(x);
}
static void model(void)
{
    fixture *x=make();for(uint32_t entries=1;entries<=16;++entries)for(uint32_t segments=1;segments<=entries;++segments){
        setup(x,0x12000);source(x,entries,segments,7u+segments,entries&1u);
        put32(x->source,x->data[0],38u+entries*8u);put32(x->source,x->data[0]+12u,34);
        if(entries&1u)x->source[x->data[0]+16u+entries*8u]=0xD1;
        loaded(x);
        REQUIRE(x->image.staging_bytes==16u+8u*entries+segments*(7u+segments));
        uint8_t expected[192];uint32_t world=54u+entries*8u;memset(expected,0,sizeof expected);
        REQUIRE(x->image.live==(uint16_t)((UINT32_C(1)<<segments)-1u));
        for(uint32_t i=0;i<entries;++i){uint32_t sid=1u+(i*segments)/entries;
            REQUIRE(read32(x,x->image.layout.table_begin+i*8u)==(sid<<16|0x4EF9u));
            REQUIRE(read32(x,x->image.layout.table_begin+i*8u+4u)==x->image.segments[sid-1u].address+4u);
            put32(expected,50u+i*8u,sid<<16|0x4EF9u);put32(expected,54u+i*8u,x->image.segments[sid-1u].address+4u);}
        REQUIRE(!memcmp(x->ram+x->image.layout.globals_begin-0x1000u,expected,world));
        REQUIRE(x->ram[x->image.layout.globals_begin-0x1000u-1u]==0xC7 && x->ram[x->request.b-0x1000u]==0xC7);
        for(uint32_t i=0;i<segments;++i){const cs_image_segment *p=&x->image.segments[i];
            REQUIRE(!memcmp(x->ram+p->address-0x1000u,x->source+x->data[i+1u],p->length));}
        next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK && cs_image_external_admit(&x->image)==CS_IMAGE_OK);}
    free(x);
}
#ifdef CLASSICK_IMAGE_TEST_FAULTS
static void fault_retained(fixture *x)
{
    REQUIRE(x->image.state==CS_IMAGE_FAULTED && x->report.retained_fault && !x->report.execution_authorized);
    cs_image_report first=x->image.first_fault;fixture *saved=make();memcpy(saved,x,sizeof *x);
    REQUIRE(cs_image_external_admit(&x->image)==CS_IMAGE_FAULT);
    for(unsigned i=1;i<=5;++i){next(x,(cs_image_operation)i);REQUIRE(run(x)==CS_IMAGE_FAULT);guest_same(x,saved);
        REQUIRE(!memcmp(&first,&x->image.first_fault,sizeof first));REQUIRE(!x->report.masks_valid && !x->report.reply_valid && !x->report.retained_fault);}
    free(saved);
}
static void faults(void)
{
    fixture *x=make(),*saved=make();setup(x,0x12000);loaded(x);uint32_t loads=x->report.completed;REQUIRE(loads==135);
    next(x,CS_IMAGE_RELEASE);REQUIRE(run(x)==CS_IMAGE_OK);uint32_t releases=x->report.completed;REQUIRE(releases==85);
    for(uint32_t n=1;n<=loads;++n){setup(x,0x12000);prepared(x);REQUIRE(cs_image_test_fault(&x->image,n,0,0,0)==CS_IMAGE_OK);
        memcpy(saved,x,sizeof *x);
        REQUIRE(run(x)==CS_IMAGE_FAULT && x->report.completed==n && x->report.guest_committed && x->image.staging_borrow);
        REQUIRE(!memcmp(&x->description,&saved->description,sizeof x->description));
        REQUIRE(!memcmp(x->low,saved->low,sizeof x->low) && !memcmp(x->ram,saved->ram,0x800));
        REQUIRE(!memcmp(x->ram+0x18BB0u-0x1000u,saved->ram+0x18BB0u-0x1000u,1024));
        unsigned live=0;for(unsigned i=0;i<CS_MM_BLOCKS;++i)if(x->manager.blocks[i].kind)++live;
        REQUIRE(live==((x->image.live&1u)?1u:0u)+((x->image.live&2u)?1u:0u));
        REQUIRE(x->image.allocated==(uint16_t)(1u+(n>=12?2u:0u)));
        REQUIRE(x->image.locked==(uint16_t)((n>=2?1u:0u)+(n>=13?2u:0u)));
        REQUIRE(x->image.copied==(uint16_t)((n>=11?1u:0u)+(n>=22?2u:0u)));
        REQUIRE(x->report.world_bytes==(n>22?(n>102?80u:n-22u):0u));
        REQUIRE(x->report.table_bytes==(n>102?(n>134?32u:n-102u):0u));
        if(n==1 || n==12)REQUIRE(x->report.phase==CS_IMAGE_ALLOCATION);
        if(n==2 || n==13)REQUIRE(x->report.phase==CS_IMAGE_LOCKING);
        if(n==135)REQUIRE(x->report.phase==CS_IMAGE_DONE);
        fault_retained(x);}
    for(uint32_t n=1;n<=releases;++n){setup(x,0x12000);loaded(x);next(x,CS_IMAGE_RELEASE);
        REQUIRE(cs_image_test_fault(&x->image,n,0,0,0)==CS_IMAGE_OK);memcpy(saved,x,sizeof *x);
        REQUIRE(run(x)==CS_IMAGE_FAULT && x->report.completed==n && x->image.reservation);
        REQUIRE(!memcmp(&x->description,&saved->description,sizeof x->description));
        REQUIRE(!memcmp(x->low,saved->low,sizeof x->low) && !memcmp(x->ram,saved->ram,0x800));
        REQUIRE((x->image.live&x->image.disposed)==0 && (x->image.retired&~x->image.disposed)==0);
        REQUIRE(x->image.disposed==(uint16_t)((n>=81?2u:0u)+(n>=83?1u:0u)));
        REQUIRE(x->image.retired==(uint16_t)((n>=82?2u:0u)+(n>=84?1u:0u)));fault_retained(x);}
    for(uint32_t kind=1;kind<=3;++kind)for(uint32_t index=1;index<=2;++index){setup(x,0x12000);prepared(x);
        if(kind==3){REQUIRE(run(x)==CS_IMAGE_OK);next(x,CS_IMAGE_RELEASE);}
        REQUIRE(cs_image_test_fault(&x->image,0,kind,index,0)==CS_IMAGE_OK);REQUIRE(run(x)==CS_IMAGE_FAULT);
        REQUIRE(x->report.attempted==x->report.completed+1u);
        REQUIRE(!x->report.manager_valid && !x->report.reply_valid);if(kind==1 && index==1)REQUIRE(!x->report.guest_committed);
        if(kind==2)REQUIRE(x->image.allocated&(UINT32_C(1)<<(index-1u)));
        fault_retained(x);}
    setup(x,0x12000);REQUIRE(cs_image_test_fault(&x->image,0,0,0,1)==CS_IMAGE_OK);memcpy(saved,x,sizeof *x);
    refused(x,CS_IMAGE_CHANGED,0);REQUIRE(x->image.last_generation==0 && !x->image.staging_borrow);guest_same(x,saved);
    free(saved);free(x);
}
#endif
int main(int argc,char **argv)
{
    REQUIRE(argc==2);const char *name=argv[1];
    if(!strcmp(name,"success"))success();else if(!strcmp(name,"capacity"))capacity_cases();else if(!strcmp(name,"ownership"))ownership();
    else if(!strcmp(name,"cancel"))cancel();else if(!strcmp(name,"aliases"))aliases();else if(!strcmp(name,"seal"))seal();
    else if(!strcmp(name,"resources"))resources();else if(!strcmp(name,"profile"))profile();else if(!strcmp(name,"model"))model();
#ifdef CLASSICK_IMAGE_TEST_FAULTS
    else if(!strcmp(name,"faults"))faults();
#endif
    else REQUIRE(0);
    printf("CODE image %s PASS; original data only, no guest execution.\n",name);return 0;
}
