/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original synthetic coordinator cases. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/memory/lifecycle.h"
#define REQUIRE(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct {
    uint8_t low[1024],ram[20480],rom[512],stage[4096];
    cs_guest_region regions[3];cs_guest_map map;cs_mm_config config;
    cs_mm_manager manager;cs_trap_gateway gateway;cs_m68k_session session;
    cs_m68k_session_report saved;cs_life_controller controller;
    cs_life_plan plan;cs_life_scrap scrap;cs_life_entry inventory[65];
    cs_life_request request;cs_life_report report;
} fixture;
typedef struct {uint8_t low[1024],ram[20480];cs_mm_manager manager;
    cs_trap_gateway gateway;cs_m68k_session session;cs_m68k_session_report saved;
    cs_life_scrap scrap;uint32_t epoch;} snapshot;
static const uint32_t locations[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
static uint32_t read(const fixture *x,uint32_t at)
{uint32_t v=0;REQUIRE(cs_guest_read(&x->map,at,4,CS_GUEST_READ,&v)==CS_GUEST_OK);return v;}
static void write(fixture *x,uint32_t at,uint32_t v)
{REQUIRE(cs_guest_write(&x->map,at,4,v)==CS_GUEST_OK);}
static void inventory(fixture *x)
{
    for(unsigned i=0;i<6;++i)x->inventory[i]=(cs_life_entry){CS_LIFE_GLOBAL,
        (i==1 || i==2)?CS_LIFE_REBIND:CS_LIFE_PRESERVE,1,locations[i],4,read(x,locations[i]),0};
    x->inventory[6]=(cs_life_entry){CS_LIFE_NATIVE,CS_LIFE_PRESERVE,2,0,sizeof x->saved,0,&x->saved};
    x->inventory[7]=(cs_life_entry){CS_LIFE_GUEST,CS_LIFE_PRESERVE,3,0x4000,0x400,0,0};
    x->request.count=8;
}
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->low,0xD3,sizeof x->low);memset(x->ram,0xB7,sizeof x->ram);
    memset(x->stage,0xCE,sizeof x->stage);memset(&x->saved,0x5D,sizeof x->saved);
    x->regions[0]=(cs_guest_region){x->low,x->low,0,1024,3};
    x->regions[1]=(cs_guest_region){x->ram,x->ram,0x1000,20480,3};
    x->regions[2]=(cs_guest_region){x->rom,0,0x400000,512,5};x->map=(cs_guest_map){x->regions,3};
    x->config=(cs_mm_config){{0x400,0x1000,0x1800},{0x402,0x2000,0x3000}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&x->config)==CS_MM_OK);
    cs_trap_config g={0x400000,0x1000,0x1800,0x400010,0x400012,0x400014};
    REQUIRE(cs_trap_init(&x->gateway,&g)==CS_TRAP_OK);
    cs_m68k_bus_options options={CS_M68K_DATA24,0,0};
    for(unsigned i=0;i<4;++i){x->rom[0x100+i*2u]=0x4E;x->rom[0x101+i*2u]=0x71;}
    cs_m68k_call_config call={0x4000,0x400,0x400100,2};
    REQUIRE(cs_m68k_session_init(&x->session,&x->map,&x->gateway,&options,&call)==CS_M68K_SESSION_OK);
    REQUIRE(cs_life_init(&x->controller,&x->manager)==CS_LIFE_OK);
    x->scrap=(cs_life_scrap){0,CS_LIFE_NONE,0,0,0};
    x->request=(cs_life_request){CS_LIFE_RESET,CS_LIFE_ORDINARY,0,0,0,0,
        0x4000,0x400,0x4400,0x1800,0,&x->scrap,x->stage,sizeof x->stage,x->inventory,8,&x->gateway,&x->session};
    inventory(x);
}
static cs_mm_reply alloc(fixture *x,uint32_t bytes,unsigned flags,unsigned handle)
{
    cs_mm_request q={handle?CS_MM_NEW_HANDLE:CS_MM_NEW_PTR,0,0,bytes,flags,0};cs_mm_reply r;
    REQUIRE(cs_mm_execute(&x->manager,&q,&r)==CS_MM_OK && r.code==0);return r;
}
static void scrap(fixture *x,uint32_t size,unsigned system)
{
    cs_mm_reply r=alloc(x,size,system?CS_MM_SYSTEM:0,1);
    x->scrap=(cs_life_scrap){(int32_t)size,CS_LIFE_RESIDENT,r.value,0,0};
    uint32_t at=read(x,r.value)&0xFFFFFFu;
    for(uint32_t i=0;i<size;++i)REQUIRE(cs_guest_write(&x->map,at+i,1,(i*73u+19u)&255u)==CS_GUEST_OK);
}
static void save(const fixture *x,snapshot *s)
{
    memcpy(s->low,x->low,sizeof s->low);memcpy(s->ram,x->ram,sizeof s->ram);s->manager=x->manager;
    s->gateway=x->gateway;s->session=x->session;s->saved=x->saved;s->scrap=x->scrap;s->epoch=x->controller.epoch;
}
static void unchanged(const fixture *x,const snapshot *s)
{
    REQUIRE(memcmp(s->low,x->low,sizeof s->low)==0 && memcmp(s->ram,x->ram,sizeof s->ram)==0);
    REQUIRE(memcmp(&s->manager,&x->manager,sizeof s->manager)==0 && memcmp(&s->gateway,&x->gateway,sizeof s->gateway)==0);
    REQUIRE(memcmp(&s->session,&x->session,sizeof s->session)==0 && memcmp(&s->saved,&x->saved,sizeof s->saved)==0);
    REQUIRE(memcmp(&s->scrap,&x->scrap,sizeof s->scrap)==0 && s->epoch==x->controller.epoch);
}
static void persistent(const fixture *x,const snapshot *s)
{
    REQUIRE(memcmp(s->ram,x->ram,0x800)==0);
    REQUIRE(memcmp(&s->gateway,&x->gateway,sizeof s->gateway)==0 && memcmp(&s->session,&x->session,sizeof s->session)==0);
    REQUIRE(memcmp(&s->saved,&x->saved,sizeof s->saved)==0 && memcmp(s->ram+0x3000,x->ram+0x3000,0x400)==0);
    for(unsigned i=0;i<32;++i)if(s->manager.blocks[i].kind && s->manager.blocks[i].zone==0)
        REQUIRE(memcmp(&s->manager.blocks[i],&x->manager.blocks[i],sizeof(cs_mm_block))==0);
    for(unsigned i=0;i<1024;++i){int writable=0;
        for(unsigned j=1;j<=2;++j)if(i>=locations[j] && i<locations[j]+4u)writable=1;
        if(!writable)REQUIRE(x->low[i]==s->low[i]);}
}
static void run(fixture *x)
{REQUIRE(cs_life_prepare(&x->controller,&x->request,&x->plan,&x->report)==CS_LIFE_OK);
 REQUIRE(cs_life_commit(&x->controller,&x->plan,&x->report)==CS_LIFE_OK);}
static void success(void)
{
    fixture x;snapshot s;
    for(unsigned operation=1;operation<=2;++operation)for(uint32_t n=0;n<35;++n){
        setup(&x);(void)alloc(&x,17,CS_MM_SYSTEM,0);(void)alloc(&x,19,CS_MM_SYSTEM,1);
        (void)alloc(&x,31,0,0);scrap(&x,n,0);x.request.operation=(cs_life_operation)operation;
        write(&x,0x118,0x400);inventory(&x);save(&x,&s);uint32_t old=x.scrap.handle;
        run(&x);persistent(&x,&s);REQUIRE(x.controller.epoch==2 && x.controller.state==CS_LIFE_STABLE);
        REQUIRE(read(&x,0x118)==0x402 && read(&x,0x130)==0x2C00 && read(&x,0x114)==0x3000);
        uint32_t cell=read(&x,x.scrap.handle);REQUIRE((cell>>24)==0x80 && (cell&0xFFFFFFu)==0x2C00-(n?((n+1u)&~1u):2u));
        for(uint32_t i=0;i<n;++i)REQUIRE(x.ram[(cell&0xFFFFFFu)-0x1000+i]==(uint8_t)(i*73u+19u));
        unsigned app=0;for(unsigned i=0;i<32;++i){const cs_mm_block *b=&x.manager.blocks[i];
            if(b->kind && b->zone==1){++app;REQUIRE(b->kind==2 && b->size==n && b->extent==(n?((n+1u)&~1u):2u));}
            uint32_t expected=x.scrap.handle==0x2000+i*4u?cell:0;REQUIRE(read(&x,0x2000+i*4u)==expected);}
        REQUIRE(app==1 && x.report.prepared && x.report.committed && x.report.scrap_valid && x.report.new_handle==x.scrap.handle);
        REQUIRE(x.report.old_handle==old && x.report.new_handle_valid && x.report.globals_valid==63 && x.report.state==CS_LIFE_STABLE);
        REQUIRE(cs_life_admit(&x.controller)==CS_LIFE_OK);
        inventory(&x);run(&x);REQUIRE(x.controller.epoch==3);persistent(&x,&s);
        cell=read(&x,x.scrap.handle);
        for(uint32_t i=0;i<n;++i)REQUIRE(x.ram[(cell&0xFFFFFFu)-0x1000+i]==(uint8_t)(i*73u+19u));
    }
    setup(&x);save(&x,&s);run(&x);persistent(&x,&s);REQUIRE(!x.scrap.handle);
    setup(&x);scrap(&x,13,1);write(&x,x.scrap.handle,read(&x,x.scrap.handle)|0x45000000u);
    save(&x,&s);run(&x);persistent(&x,&s);REQUIRE(memcmp(&s.scrap,&x.scrap,sizeof s.scrap)==0);
    setup(&x);scrap(&x,0,1);save(&x,&s);run(&x);persistent(&x,&s);
}
static void refused(fixture *x,cs_life_result result)
{
    snapshot s;save(x,&s);uint8_t stage[4096];memcpy(stage,x->stage,sizeof stage);
    REQUIRE(cs_life_prepare(&x->controller,&x->request,&x->plan,&x->report)==result);
    unchanged(x,&s);REQUIRE(memcmp(stage,x->stage,sizeof stage)==0 && x->controller.state==CS_LIFE_STABLE);
}
static void matrix(void)
{
    fixture x;
    for(int size=-1;size<=1;++size)for(unsigned tag=0;tag<=3;++tag)for(unsigned handle=0;handle<2;++handle){
        setup(&x);if(handle)scrap(&x,0,0);x.scrap.size=size;x.scrap.residency=(cs_life_residency)tag;
        int valid=size==0 && tag<2 && ((tag==0 && !handle) || (tag==1 && handle));
        if(valid)run(&x);else refused(&x,CS_LIFE_SCRAP);
    }
    setup(&x);scrap(&x,0,0);write(&x,x.scrap.handle,0x80000000);refused(&x,CS_LIFE_SCRAP);
    setup(&x);scrap(&x,8,0);x.scrap.size=7;refused(&x,CS_LIFE_SCRAP);
    setup(&x);x.scrap=(cs_life_scrap){0,CS_LIFE_RESIDENT,0x2001,0,0};refused(&x,CS_LIFE_SCRAP);
    setup(&x);scrap(&x,8,0);x.scrap.handle=0x207C;refused(&x,CS_LIFE_SCRAP);
    setup(&x);scrap(&x,3,0);x.scrap.name_address=0x2400;x.scrap.name_bytes=4;refused(&x,CS_LIFE_SCRAP);
    setup(&x);x.scrap.name_address=0x114;x.scrap.name_bytes=4;refused(&x,CS_LIFE_SCRAP);
    setup(&x);x.scrap.name_address=0x1800;x.scrap.name_bytes=7;run(&x);
    setup(&x);x.scrap.name_address=0x1800;refused(&x,CS_LIFE_SCRAP);
}
static void limits(void)
{
    fixture x;
    setup(&x);scrap(&x,17,0);x.request.stack_reference=0x4000;x.request.reserve=0x1F6E;
    x.request.minimum=18;run(&x);REQUIRE(read(&x,0x130)==0x2092);
    setup(&x);scrap(&x,17,0);x.request.stack_reference=0x4000;x.request.reserve=0x1F70;
    refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.minimum=0xB82;refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.minimum=0xB80;run(&x);
    setup(&x);scrap(&x,7,0);x.request.staging_bytes=6;refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.reserve=0x4402;refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.stack_reference=0x4402;refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.reserve=1;refused(&x,CS_LIFE_CAPACITY);
    setup(&x);x.request.stack_base=0x2800;refused(&x,CS_LIFE_CONTEXT);
    setup(&x);x.request.session=0;x.request.stack_base=0x2800;x.request.stack_size=0x400;x.request.stack_reference=0x2C00;
    refused(&x,CS_LIFE_ALIAS);
    setup(&x);for(unsigned i=0;i<31;++i)(void)alloc(&x,0,CS_MM_SYSTEM,1);scrap(&x,0,0);
    run(&x);REQUIRE(x.scrap.handle==0x207C);
    setup(&x);for(unsigned i=0;i<32;++i)(void)alloc(&x,0,CS_MM_SYSTEM,1);run(&x);
    /* A synthetic forged descriptor cannot invent a33rd application object. */
    setup(&x);x.scrap=(cs_life_scrap){0,CS_LIFE_RESIDENT,0x207C,0,0};refused(&x,CS_LIFE_SCRAP);
    setup(&x);x.controller.epoch=UINT32_MAX;refused(&x,CS_LIFE_EPOCH);
    setup(&x);x.controller.epoch=UINT32_MAX-1u;run(&x);REQUIRE(x.controller.epoch==UINT32_MAX);
    inventory(&x);refused(&x,CS_LIFE_EPOCH);
}
static void plans(void)
{
    fixture x,y;snapshot s;setup(&x);scrap(&x,23,0);save(&x,&s);
    memset(&x.controller,0,sizeof x.controller);
    REQUIRE(cs_life_init(&x.controller,&x.manager)==CS_LIFE_OK && x.controller.epoch==1);unchanged(&x,&s);
    REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);unchanged(&x,&s);
    REQUIRE(cs_life_admit(&x.controller)==CS_LIFE_STATE);
    REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_STATE);
    REQUIRE(x.controller.active==&x.plan && x.plan.ready);
    setup(&y);REQUIRE(cs_life_prepare(&y.controller,&y.request,&y.plan,&y.report)==CS_LIFE_OK);
    REQUIRE(cs_life_commit(&y.controller,&x.plan,&y.report)==CS_LIFE_STATE);
    REQUIRE(y.controller.active==&y.plan && y.plan.ready);
    REQUIRE(cs_life_cancel(&x.controller,&x.plan,&x.report)==CS_LIFE_OK);unchanged(&x,&s);
    REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_STATE);unchanged(&x,&s);
    cs_life_plan held=y.plan;
    REQUIRE(cs_life_prepare(&x.controller,&x.request,&y.plan,&x.report)==CS_LIFE_STATE);
    REQUIRE(memcmp(&held,&y.plan,sizeof held)==0 && y.controller.active==&y.plan && x.controller.state==CS_LIFE_STABLE);
    unchanged(&x,&s);
    run(&x);REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_STATE);
    REQUIRE(cs_life_init(&x.controller,&x.manager)==CS_LIFE_STATE);
    REQUIRE(cs_life_cancel(&y.controller,&y.plan,&y.report)==CS_LIFE_OK);
}
static void changes(void)
{
    fixture x;snapshot s;
    for(unsigned change=0;change<15;++change){setup(&x);scrap(&x,23,0);
        REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);
        switch(change){
        case 0:x.request.minimum=2;break;case 1:x.scrap.size=22;break;
        case 2:x.inventory[6].owner=9;break;case 3:write(&x,0x130,0x2F00);break;
        case 4:write(&x,0x1000,0x12345678);break;case 5:write(&x,0x2000,read(&x,0x2000)|CS_MM_LOCK);break;
        case 6:x.manager.blocks[0].size=22;break;case 7:x.regions[1].flags=1;break;
        case 8:x.gateway.entries[3]=0x400020;break;case 9:x.stage[1]^=1;break;
        case 10:x.ram[(read(&x,x.scrap.handle)&0xFFFFFFu)-0x1000+1]^=1;break;
        case 11:x.plan.address-=2;break;case 12:x.request.count=0;break;
        case 13:x.request.staging=(uint8_t *)(void *)&x.saved;break;
        default:x.regions[2].bytes=x.low;break;}
        save(&x,&s);REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_CHANGED);
        unchanged(&x,&s);REQUIRE(x.controller.state==CS_LIFE_STABLE && !x.plan.ready && !x.controller.active);
        REQUIRE(!x.report.committed && x.report.epoch_after==1);
    }
    setup(&x);REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);
    x.session.active=1;save(&x,&s);REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_CHANGED);unchanged(&x,&s);
    setup(&x);REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);
    cs_life_report prior=x.report;x.regions[0].bytes=(const uint8_t *)(const void *)&x.report;x.regions[0].flags=0;
    save(&x,&s);REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_ALIAS);unchanged(&x,&s);
    REQUIRE(memcmp(&prior,&x.report,sizeof prior)==0 && x.plan.ready && x.controller.state==CS_LIFE_PREPARED);
}
static void references(void)
{
    fixture x;setup(&x);x.inventory[6].kind=CS_LIFE_FUTURE;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.inventory[0].action=CS_LIFE_CLEAR;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.inventory[0].value=1;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.inventory[5]=x.inventory[0];refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.request.count=65;refused(&x,CS_LIFE_ARGUMENT);
    setup(&x);x.gateway.entries[5]=0x1000;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.gateway.entries[5]=0x400020;
    x.inventory[8]=(cs_life_entry){CS_LIFE_BUILTIN,CS_LIFE_PRESERVE,4,5,4,0x400020,0};x.request.count=9;
    run(&x);REQUIRE(x.gateway.entries[5]==0x400020);
    setup(&x);x.inventory[8]=(cs_life_entry){CS_LIFE_GUEST,CS_LIFE_PRESERVE,4,0x2400,8,0,0};x.request.count=9;
    refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.manager.magic=0;refused(&x,CS_LIFE_MEMORY);
    setup(&x);x.inventory[0].owner=0;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.inventory[0].action=CS_LIFE_REFUSE;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);x.inventory[0].address=0x120;refused(&x,CS_LIFE_REFERENCE);
    setup(&x);scrap(&x,4,0);write(&x,x.scrap.handle,0x2900);refused(&x,CS_LIFE_MEMORY);
    setup(&x);write(&x,0x114,1);inventory(&x);refused(&x,CS_LIFE_MEMORY);
    setup(&x);x.manager.config.application.end=0x2FFF;refused(&x,CS_LIFE_MEMORY);
    for(unsigned context=0;context<3;++context)if(context!=1){setup(&x);x.request.context=(cs_life_context)context;refused(&x,CS_LIFE_CONTEXT);}
    for(unsigned active=0;active<5;++active){setup(&x);
        if(active==0)x.request.active_root=1;else if(active==1)x.request.active_callback=1;
        else if(active==2)x.request.active_patch=1;else if(active==3)x.request.active_manager=1;else x.session.calls.depth=1;
        refused(&x,CS_LIFE_CONTEXT);}
}
static void aliases(void)
{
    fixture x;snapshot s;setup(&x);save(&x,&s);cs_life_report old=x.report;
    REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,(cs_life_report *)(void *)x.ram)==CS_LIFE_ALIAS);unchanged(&x,&s);
    REQUIRE(memcmp(&old,&x.report,sizeof old)==0);
    setup(&x);x.request.staging=x.ram;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.request.staging=(uint8_t *)(void *)&x.saved;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.request.staging=(uint8_t *)(void *)&x.plan;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.inventory[6].native=&x.manager;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.inventory[7].address=0x114;x.inventory[7].bytes=4;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.regions[2].bytes=x.low;refused(&x,CS_LIFE_ALIAS);
    setup(&x);x.regions[1].writable=x.stage;refused(&x,CS_LIFE_MEMORY);
    setup(&x);save(&x,&s);memset(&x.controller,0,sizeof x.controller);x.manager.magic=0;save(&x,&s);
    REQUIRE(cs_life_init(&x.controller,&x.manager)==CS_LIFE_MEMORY);unchanged(&x,&s);REQUIRE(x.controller.tag==0);
    setup(&x);save(&x,&s);
    REQUIRE(cs_life_init((cs_life_controller *)(void *)x.ram,&x.manager)==CS_LIFE_ALIAS);unchanged(&x,&s);
    setup(&x);x.request.staging=(uint8_t *)(uintptr_t)(UINTPTR_MAX-2u);x.request.staging_bytes=8;refused(&x,CS_LIFE_ARGUMENT);
    setup(&x);save(&x,&s);REQUIRE(cs_life_prepare(&x.controller,0,&x.plan,&x.report)==CS_LIFE_ARGUMENT);unchanged(&x,&s);
}
static void gates(void)
{
    fixture x;setup(&x);cs_mm_request q={CS_MM_NEW_PTR,0,0,4,0,0};cs_mm_reply r;cs_mm_result result;
    REQUIRE(cs_life_manager_call(&x.controller,&q,&r,&result)==CS_LIFE_OK && result==CS_MM_OK && r.code==0);
    REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);
    r=(cs_mm_reply){123,456,789};result=CS_MM_FLAGS;
    REQUIRE(cs_life_manager_call(&x.controller,&q,&r,&result)==CS_LIFE_STATE && r.value==456 && result==CS_MM_FLAGS);
    REQUIRE(cs_life_cancel(&x.controller,&x.plan,&x.report)==CS_LIFE_OK);
    REQUIRE(cs_life_manager_call(&x.controller,&q,&r,&result)==CS_LIFE_OK);
}
static void reuse(void)
{
    fixture x;setup(&x);scrap(&x,8,0);uint32_t old=x.scrap.handle;
    x.request.reserve=0;run(&x);REQUIRE(x.scrap.handle==old);
    REQUIRE(x.controller.epoch==2);inventory(&x);x.request.operation=CS_LIFE_TEARDOWN;run(&x);
    REQUIRE(x.controller.epoch==3 && x.scrap.handle==old);
    /* Numerical reuse is admitted, not called a stale-pointer detector. */
}
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
static void faults(void)
{
    fixture x;snapshot before;
    for(unsigned operation=1;operation<=2;++operation){setup(&x);scrap(&x,5,0);x.request.operation=(cs_life_operation)operation;
        run(&x);uint32_t writes=x.report.completed;
        for(uint32_t n=1;n<=writes;++n){setup(&x);scrap(&x,5,0);x.request.operation=(cs_life_operation)operation;save(&x,&before);
            REQUIRE(cs_life_test_fault_after(&x.controller,n)==CS_LIFE_OK);
            REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_OK);
            REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_INVARIANT);
            REQUIRE(x.controller.state==CS_LIFE_FAULTED && x.controller.epoch==2 && x.report.committed &&
                x.report.completed==n && x.report.attempted==n && !x.report.scrap_valid);
            REQUIRE(x.report.new_handle_valid==(n==writes));
            REQUIRE(x.report.new_handle==(n==writes?x.scrap.handle:0));
            persistent(&x,&before);cs_life_report first=x.controller.first_fault;uint8_t stage[4096];memcpy(stage,x.stage,sizeof stage);
            REQUIRE(cs_life_prepare(&x.controller,&x.request,&x.plan,&x.report)==CS_LIFE_FAULT);
            REQUIRE(cs_life_commit(&x.controller,&x.plan,&x.report)==CS_LIFE_FAULT);
            REQUIRE(cs_life_cancel(&x.controller,&x.plan,&x.report)==CS_LIFE_FAULT);
            REQUIRE(cs_life_admit(&x.controller)==CS_LIFE_FAULT && cs_life_init(&x.controller,&x.manager)==CS_LIFE_STATE);
            cs_mm_request q={CS_MM_NEW_PTR,0,0,4,0,0};cs_mm_reply reply={123,456,789};cs_mm_result result=CS_MM_FLAGS;
            REQUIRE(cs_life_manager_call(&x.controller,&q,&reply,&result)==CS_LIFE_FAULT && reply.value==456 && result==CS_MM_FLAGS);
            REQUIRE(memcmp(&first,&x.controller.first_fault,sizeof first)==0 && memcmp(stage,x.stage,sizeof stage)==0);
            if(n<=36)REQUIRE(first.phase==CS_LIFE_COMMIT);
            else REQUIRE(first.phase==(operation==1?CS_LIFE_POPULATION:CS_LIFE_SCRAP_PRESERVE));
        }
    }
}
#endif
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    if(strcmp(argv[1],"success")==0)success();else if(strcmp(argv[1],"matrix")==0)matrix();
    else if(strcmp(argv[1],"limits")==0)limits();else if(strcmp(argv[1],"plans")==0)plans();
    else if(strcmp(argv[1],"changes")==0)changes();else if(strcmp(argv[1],"references")==0)references();
    else if(strcmp(argv[1],"aliases")==0)aliases();else if(strcmp(argv[1],"gates")==0)gates();
    else if(strcmp(argv[1],"reuse")==0)reuse();
#ifdef CLASSICK_LIFECYCLE_TEST_FAULTS
    else if(strcmp(argv[1],"faults")==0)faults();
#endif
    else return 2;
    printf("lifecycle %s PASS; controller=%zu plan=%zu entry=%zu report=%zu\n",argv[1],
        sizeof(cs_life_controller),sizeof(cs_life_plan),sizeof(cs_life_entry),sizeof(cs_life_report));return 0;
}
