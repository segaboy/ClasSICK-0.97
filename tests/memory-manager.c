/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/heap-probe/probe.h"
#define REQUIRE(e) do {if(!(e)){fprintf(stderr,"line %d: %s\n",__LINE__,#e);exit(1);}} while(0)
typedef struct {
    uint8_t low[1032],heap[8200];cs_guest_region regions[2];cs_guest_map map;
    cs_mm_config config;cs_mm_manager manager;
} fixture;
_Static_assert(sizeof(cs_mm_manager)<=704,"Bounded caller-owned metadata budget");
static void setup(fixture *x)
{
    memset(x,0,sizeof *x);memset(x->low,0xDB,sizeof x->low);memset(x->heap,0xDB,sizeof x->heap);
    x->regions[0]=(cs_guest_region){x->low+4,x->low+4,0,1024,3};
    x->regions[1]=(cs_guest_region){x->heap+4,x->heap+4,0x1000,8192,3};x->map=(cs_guest_map){x->regions,2};
    x->config=(cs_mm_config){{0x400,0x1000,0x1800},{0x402,0x2000,0x3000}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&x->config)==CS_MM_OK);
}
static cs_mm_reply execute(fixture *x,cs_mm_operation op,uint32_t address,uint32_t size,unsigned flags)
{
    cs_mm_request q={op,address,0,size,flags,0};cs_mm_reply reply;
    REQUIRE(cs_mm_execute(&x->manager,&q,&reply)==CS_MM_OK);return reply;
}
static uint32_t get(const fixture *x,uint32_t at,unsigned width)
{uint32_t out=0;REQUIRE(cs_guest_read(&x->map,at,width,1,&out)==CS_GUEST_OK);return out;}
static void put(fixture *x,uint32_t at,unsigned width,uint32_t v)
{REQUIRE(cs_guest_write(&x->map,at,width,v)==CS_GUEST_OK);}
static void guards(const fixture *x)
{
    for(unsigned i=0;i<4;++i) REQUIRE(x->low[i]==0xDB && x->low[1028+i]==0xDB && x->heap[i]==0xDB && x->heap[8196+i]==0xDB);
    for(unsigned i=0x800;i<0x1000;++i) REQUIRE(x->heap[4+i]==0xDB);
    const unsigned locations[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
    for(unsigned i=0;i<1024;++i) {
        int special=0;for(unsigned j=0;j<6;++j) if(i>=locations[j] && i<locations[j]+4) special=1;
        if(!special) REQUIRE(x->low[4+i]==0xDB);
    }
}
static void oracle(const fixture *x)
{
    for(unsigned i=0;i<32;++i) {
        const cs_mm_block *a=&x->manager.blocks[i];if(!a->kind) continue;
        REQUIRE(a->address%2==0 && a->size<=a->extent && a->extent>=2 && a->extent%2==0);
        const cs_mm_zone *z=a->zone?&x->config.application:&x->config.system;
        REQUIRE(a->address>=z->begin+128 && a->address+a->extent<=z->end);
        if(a->kind==2) REQUIRE((get(x,z->begin+i*4u,4)&0xFFFFFFu)==a->address);
        for(unsigned j=0;j<i;++j) {
            const cs_mm_block *b=&x->manager.blocks[j];if(!b->kind) continue;
            REQUIRE(a->address+a->extent<=b->address || b->address+b->extent<=a->address);
        }
    }
    guards(x);
}
static void code_failure(fixture *x,cs_mm_request *q,int code)
{
    cs_mm_manager m=x->manager;uint8_t low[1032],heap[8200];memcpy(low,x->low,sizeof low);memcpy(heap,x->heap,sizeof heap);
    cs_mm_reply out;REQUIRE(cs_mm_execute(&x->manager,q,&out)==CS_MM_OK && out.code==code && out.moved==0);
    REQUIRE(memcmp(&m,&x->manager,sizeof m)==0 && memcmp(low,x->low,sizeof low)==0 && memcmp(heap,x->heap,sizeof heap)==0);
}
static void contract_failure(fixture *x,cs_mm_request *q,cs_mm_result expected)
{
    cs_mm_manager m=x->manager;uint8_t low[1032],heap[8200];memcpy(low,x->low,sizeof low);memcpy(heap,x->heap,sizeof heap);
    cs_mm_reply out,old;memset(&out,0xCC,sizeof out);old=out;
    REQUIRE(cs_mm_execute(&x->manager,q,&out)==expected && memcmp(&old,&out,sizeof old)==0);
    REQUIRE(memcmp(&m,&x->manager,sizeof m)==0 && memcmp(low,x->low,sizeof low)==0 && memcmp(heap,x->heap,sizeof heap)==0);
}
static void allocation(void)
{
    fixture x;setup(&x);REQUIRE(get(&x,0x114,4)==0x3000 && get(&x,0x118,4)==0x402 && get(&x,0x130,4)==0x3000 &&
        get(&x,0x2A6,4)==0x400 && get(&x,0x2AA,4)==0x402 && get(&x,0x31A,4)==0xFFFFFF);
    for(unsigned kind=0;kind<2;++kind) for(uint32_t size=0;size<17;++size) {
        cs_mm_operation newop=kind?CS_MM_NEW_HANDLE:CS_MM_NEW_PTR;
        cs_mm_reply r=execute(&x,newop,0,size,CS_MM_CLEAR);REQUIRE(r.code==0 && r.value && r.value%2==0);
        uint32_t address=kind?get(&x,r.value,4):r.value;
        REQUIRE(execute(&x,kind?CS_MM_GET_HANDLE_SIZE:CS_MM_GET_PTR_SIZE,r.value,0,0).value==size);
        for(uint32_t i=0;i<size;++i) REQUIRE(get(&x,address+i,1)==0);
        oracle(&x);REQUIRE(execute(&x,kind?CS_MM_DISPOS_HANDLE:CS_MM_DISPOS_PTR,r.value,0,0).code==0);
    }
    cs_mm_request q={CS_MM_NEW_PTR,0,0,0xFFFFFFFF,0,0};code_failure(&x,&q,CS_MM_FULL);
    q.size=0x80000000;code_failure(&x,&q,CS_MM_FULL);q.size=1000000;code_failure(&x,&q,CS_MM_FULL);
    uint32_t ids[32];
    for(unsigned i=0;i<32;++i) {cs_mm_reply r=execute(&x,CS_MM_NEW_HANDLE,0,1,0);REQUIRE(r.code==0);ids[i]=r.value;oracle(&x);}
    q.operation=CS_MM_NEW_HANDLE;q.size=1;code_failure(&x,&q,CS_MM_FULL);
    for(unsigned i=0;i<32;++i) REQUIRE(execute(&x,CS_MM_DISPOS_HANDLE,ids[i],0,0).code==0);
    q.size=1;REQUIRE(cs_mm_execute(&x.manager,&q,&(cs_mm_reply){0})==CS_MM_OK);oracle(&x);
}
static void zones(void)
{
    fixture x;setup(&x);cs_mm_reply app=execute(&x,CS_MM_NEW_PTR,0,8,0),sys=execute(&x,CS_MM_NEW_PTR,0,8,CS_MM_SYSTEM);
    REQUIRE(app.value>=0x2080 && sys.value<0x1800 && get(&x,0x118,4)==0x402);
    put(&x,0x118,4,0x400);cs_mm_reply another=execute(&x,CS_MM_NEW_HANDLE,0,8,0);
    REQUIRE(another.code==0 && another.value>=0x1000 && another.value<0x1080 && get(&x,0x118,4)==0x400);
    put(&x,0x118,4,0x402);put(&x,0x130,4,0x2080);
    REQUIRE(get(&x,0x114,4)==0x3000 && execute(&x,CS_MM_GET_PTR_SIZE,app.value,0,0).value==8);
    cs_mm_request q={CS_MM_NEW_PTR,0,0,0,0,0};code_failure(&x,&q,CS_MM_FULL);
    REQUIRE(execute(&x,CS_MM_SET_PTR_SIZE,app.value,4,0).code==0);
    q.operation=CS_MM_SET_PTR_SIZE;q.address=app.value;q.size=10;code_failure(&x,&q,CS_MM_FULL);
    REQUIRE(execute(&x,CS_MM_NEW_PTR,0,8,CS_MM_SYSTEM).code==0);
    REQUIRE(execute(&x,CS_MM_DISPOS_PTR,app.value,0,0).code==0);oracle(&x);
}
static void lifetime(void)
{
    fixture x;setup(&x);uint32_t h=execute(&x,CS_MM_NEW_HANDLE,0,16,0).value,p=execute(&x,CS_MM_NEW_PTR,0,16,0).value;
    cs_mm_request q={CS_MM_GET_HANDLE_SIZE,0,0,0,0,0};code_failure(&x,&q,CS_MM_NIL_HANDLE);
    q.address=h+1;code_failure(&x,&q,CS_MM_WRONG_ZONE);q.address=p;code_failure(&x,&q,CS_MM_WRONG_ZONE);
    q.operation=CS_MM_GET_PTR_SIZE;q.address=p+2;code_failure(&x,&q,CS_MM_WRONG_ZONE);
    q.address=0x1000000;code_failure(&x,&q,CS_MM_WRONG_ZONE);
    REQUIRE(execute(&x,CS_MM_DISPOS_PTR,p,0,0).code==0);q.operation=CS_MM_DISPOS_PTR;q.address=p;code_failure(&x,&q,CS_MM_WRONG_ZONE);
    REQUIRE(execute(&x,CS_MM_DISPOS_HANDLE,h,0,0).code==0 && get(&x,h,4)==0);
    q.operation=CS_MM_DISPOS_HANDLE;q.address=h;code_failure(&x,&q,CS_MM_WRONG_ZONE);guards(&x);
    setup(&x);h=execute(&x,CS_MM_NEW_HANDLE,0,16,0).value;put(&x,h,4,0);
    for(unsigned i=0;i<3;++i) {q.operation=i==0?CS_MM_GET_HANDLE_SIZE:i==1?CS_MM_SET_HANDLE_SIZE:CS_MM_HLOCK;q.address=h;q.size=8;code_failure(&x,&q,CS_MM_NIL_HANDLE);}
    REQUIRE(execute(&x,CS_MM_DISPOS_HANDLE,h,0,0).code==0);
}
static void resize(void)
{
    fixture x;setup(&x);uint32_t h=execute(&x,CS_MM_NEW_HANDLE,0,16,0).value,old=get(&x,h,4);
    for(unsigned i=0;i<16;++i) put(&x,old+i,1,17u+i);
    uint32_t p=execute(&x,CS_MM_NEW_PTR,0,16,0).value;
    cs_mm_reply r=execute(&x,CS_MM_SET_HANDLE_SIZE,h,32,0);uint32_t now=get(&x,h,4);
    REQUIRE(r.code==0 && r.moved==1 && now!=old && execute(&x,CS_MM_GET_HANDLE_SIZE,h,0,0).value==32);
    for(unsigned i=0;i<16;++i) REQUIRE(get(&x,now+i,1)==17u+i);
    REQUIRE(execute(&x,CS_MM_HLOCK,h,0,0).code==0);cs_mm_request q={CS_MM_SET_HANDLE_SIZE,h,0,64,0,0};code_failure(&x,&q,CS_MM_FULL);
    REQUIRE((get(&x,h,4)&0xFFFFFFu)==now);
    REQUIRE(execute(&x,CS_MM_HUNLOCK,h,0,0).code==0 && execute(&x,CS_MM_SET_HANDLE_SIZE,h,4,0).code==0);
    REQUIRE(execute(&x,CS_MM_SET_HANDLE_SIZE,h,12,0).code==0 && get(&x,h,4)==now);
    for(unsigned i=0;i<4;++i) REQUIRE(get(&x,now+i,1)==17u+i);
    q.size=0xFFFFFFFF;code_failure(&x,&q,CS_MM_FULL);oracle(&x);
    REQUIRE(execute(&x,CS_MM_SET_PTR_SIZE,p,24,0).code==0 && execute(&x,CS_MM_GET_PTR_SIZE,p,0,0).value==24);
    setup(&x);uint32_t top=execute(&x,CS_MM_NEW_PTR,0,16,0).value,below=execute(&x,CS_MM_NEW_PTR,0,16,0).value;
    q=(cs_mm_request){CS_MM_SET_PTR_SIZE,below,0,32,0,0};code_failure(&x,&q,CS_MM_FULL);
    REQUIRE(execute(&x,CS_MM_DISPOS_PTR,top,0,0).code==0 && execute(&x,CS_MM_SET_PTR_SIZE,below,32,0).code==0);
    REQUIRE(execute(&x,CS_MM_SET_PTR_SIZE,below,0,0).code==0 && execute(&x,CS_MM_GET_PTR_SIZE,below,0,0).value==0);oracle(&x);
}
static void compaction(void)
{
    fixture x;setup(&x);uint32_t ids[16],old[16];
    for(unsigned i=0;i<16;++i) {ids[i]=execute(&x,CS_MM_NEW_HANDLE,0,160,0).value;old[i]=get(&x,ids[i],4);
        for(unsigned j=0;j<160;++j) put(&x,old[i]+j,1,i+1u);}
    for(unsigned i=0;i<16;i+=2) REQUIRE(execute(&x,CS_MM_DISPOS_HANDLE,ids[i],0,0).code==0);
    cs_mm_reply r=execute(&x,CS_MM_NEW_PTR,0,1800,0);REQUIRE(r.code==0 && r.moved>0);
    for(unsigned i=1;i<16;i+=2) {uint32_t at=get(&x,ids[i],4);REQUIRE(at<=old[i]);
        for(unsigned j=0;j<160;++j) REQUIRE(get(&x,at+j,1)==i+1u);}
    oracle(&x);
    /* A failed planned compaction cannot change any surviving payload or cell. */
    cs_mm_request q={CS_MM_NEW_PTR,0,0,3000,0,0};code_failure(&x,&q,CS_MM_FULL);
}
static void locks(void)
{
    fixture x;setup(&x);uint32_t h=execute(&x,CS_MM_NEW_HANDLE,0,16,0).value,at=get(&x,h,4);
    put(&x,h,4,at|0x60000000u);REQUIRE(execute(&x,CS_MM_HLOCK,h,0,0).code==0 && get(&x,h,4)==(at|0xE0000000u));
    REQUIRE(execute(&x,CS_MM_HLOCK,h,0,0).code==0 && get(&x,h,4)==(at|0xE0000000u));
    REQUIRE(execute(&x,CS_MM_HUNLOCK,h,0,0).code==0 && get(&x,h,4)==(at|0x60000000u));
    REQUIRE(execute(&x,CS_MM_HUNLOCK,h,0,0).code==0 && get(&x,h,4)==(at|0x60000000u));
    put(&x,h,4,at|CS_MM_LOCK);cs_mm_request q={CS_MM_SET_HANDLE_SIZE,h,0,32,0,0};code_failure(&x,&q,CS_MM_FULL);
    uint32_t p=execute(&x,CS_MM_NEW_PTR,0,20,0).value;
    REQUIRE(p!=0 && get(&x,h,4)==(at|CS_MM_LOCK));
    REQUIRE(execute(&x,CS_MM_GET_HANDLE_SIZE,h,0,0).value==16 && get(&x,h,4)==(at|CS_MM_LOCK));oracle(&x);
}
static void copies(void)
{
    fixture x;setup(&x);uint32_t p=execute(&x,CS_MM_NEW_PTR,0,128,0).value;
    for(unsigned direction=0;direction<2;++direction) {
        uint8_t expected[128];for(unsigned i=0;i<128;++i) {expected[i]=(uint8_t)i;put(&x,p+i,1,i);}
        unsigned src=direction?10u:0u,dst=direction?0u:10u;memmove(expected+dst,expected+src,50);
        cs_mm_request q={CS_MM_BLOCK_MOVE,p+src,p+dst,50,0,0};cs_mm_reply r;
        REQUIRE(cs_mm_execute(&x.manager,&q,&r)==CS_MM_OK && r.code==0 && r.moved==0);
        for(unsigned i=0;i<128;++i) REQUIRE(get(&x,p+i,1)==expected[i]);
    }
    cs_mm_request q={CS_MM_BLOCK_MOVE,UINT32_MAX,UINT32_MAX,0,0,0};cs_mm_reply r;
    REQUIRE(cs_mm_execute(&x.manager,&q,&r)==CS_MM_OK && r.code==0);
    q.size=0xFFFFFFFF;code_failure(&x,&q,CS_MM_WRONG_ZONE);
    q.address=p;q.other=0xFFFFFF;q.size=4;contract_failure(&x,&q,CS_MM_MEMORY);guards(&x);
}
static void failures(void)
{
    fixture x;cs_mm_request q={CS_MM_NEW_PTR,0,0,8,0,0};
    for(unsigned n=0;n<11;++n) {
        setup(&x);uint32_t h=execute(&x,CS_MM_NEW_HANDLE,0,16,0).value;
        switch(n) {case 0:x.manager.magic=0;break;case 1:x.manager.blocks[0].kind=99;break;
            case 2:x.manager.blocks[0].extent=18;break;case 3:put(&x,h,4,0x1234);break;
            case 4:put(&x,0x118,4,0x999);break;case 5:put(&x,0x130,4,0x3001);break;
            case 6:put(&x,0x31A,4,0);break;case 7:put(&x,0x114,4,0x2000);break;
            case 8:x.manager.blocks[1].size=1;break;
            case 9:x.regions[1].flags=1;break;default:x.manager.config.system.begin=0x2000;break;}
        contract_failure(&x,&q,n==9?CS_MM_MEMORY:CS_MM_STATE);
    }
    setup(&x);q.flags=4;contract_failure(&x,&q,CS_MM_FLAGS);q.flags=0;q.interrupt_context=1;contract_failure(&x,&q,CS_MM_INTERRUPT);
    q.interrupt_context=0;q.operation=(cs_mm_operation)99;contract_failure(&x,&q,CS_MM_OPERATION);
    q.operation=CS_MM_HLOCK;q.flags=1;contract_failure(&x,&q,CS_MM_FLAGS);
    cs_mm_reply r;REQUIRE(cs_mm_execute(NULL,&q,&r)==CS_MM_ARGUMENT && cs_mm_execute(&x.manager,NULL,&r)==CS_MM_ARGUMENT &&
        cs_mm_execute(&x.manager,&q,NULL)==CS_MM_ARGUMENT);
    for(unsigned n=0;n<7;++n) {
        setup(&x);cs_mm_config c=x.config;cs_mm_manager out,before;memset(&out,0xCC,sizeof out);before=out;
        uint8_t low[1032],heap[8200];memcpy(low,x.low,sizeof low);memcpy(heap,x.heap,sizeof heap);
        switch(n) {case 0:c.system.begin++;break;case 1:c.system.end=c.system.begin+128;break;
            case 2:c.application.begin=0x1400;break;case 3:c.application.identity=c.system.identity;break;
            case 4:c.system.identity=c.system.begin;break;case 5:c.application.end=0x1000002;break;default:x.regions[0].flags=1;break;}
        REQUIRE(cs_mm_init(&out,&x.map,&c)==(n==6?CS_MM_MEMORY:CS_MM_STATE));
        REQUIRE(memcmp(&out,&before,sizeof out)==0 && memcmp(low,x.low,sizeof low)==0 && memcmp(heap,x.heap,sizeof heap)==0);
    }
}
static void registers(void)
{
    fixture x;setup(&x);cs_trap_frame f;memset(&f,0xCC,sizeof f);
    for(unsigned i=0;i<8;++i){f.d[i]=0x11110000u+i;f.a[i]=0x22220000u+i;}f.pc=0x401000;f.sr=0x271F;
    f.d[0]=16;cs_trap_frame before=f;cs_mm_register_report r;
    REQUIRE(cs_mm_register_call(&x.manager,0xA322,0,&f,&r)==CS_MM_OK && f.d[0]==0 && f.a[0]>=0x2000 && f.sr==0x2714);
    uint32_t h=f.a[0];f.d[0]=before.d[0];f.a[0]=before.a[0];f.sr=before.sr;REQUIRE(memcmp(&f,&before,sizeof f)==0);
    f.a[0]=h;before=f;REQUIRE(cs_mm_register_call(&x.manager,0xA023,0,&f,&r)==CS_MM_OK && f.a[0]==h && f.d[0]==0);
    f.d[0]=0xABCD1234;before=f;REQUIRE(cs_mm_register_call(&x.manager,0xA021,0,&f,&r)==CS_MM_OK &&
        f.d[0]==0xFFFFFF91 && f.a[0]==h && f.sr==0x2718);
    f.d[0]=8;before=f;REQUIRE(cs_mm_register_call(&x.manager,0xA01E,0,&f,&r)==CS_MM_OK && f.a[0]==before.a[0]);
    for(unsigned word=0;word<65536;++word) {
        unsigned number=word&255u;int memory=number==30 || number==31 || number==32 || number==33 || number==34 || number==35 ||
            number==36 || number==37 || number==41 || number==42 || number==46;
        if(word>=0xA000 && word<=0xA7FF && memory) continue;
        cs_trap_frame saved=f;REQUIRE(cs_mm_register_call(&x.manager,(uint16_t)word,0,&f,&r)==CS_MM_OPERATION && memcmp(&f,&saved,sizeof f)==0);
    }
    before=f;REQUIRE(cs_mm_register_call(&x.manager,0xA229,0,&f,&r)==CS_MM_FLAGS && memcmp(&f,&before,sizeof f)==0);
    REQUIRE(cs_mm_register_call(&x.manager,0xA11E,1,&f,&r)==CS_MM_INTERRUPT && memcmp(&f,&before,sizeof f)==0);guards(&x);
}
static void long_sizes(void)
{
    static uint8_t low[1024],heap[0x21000];cs_guest_region r[2]={{low,low,0,1024,3},{heap,heap,0x1000,sizeof heap,3}};
    cs_guest_map map={r,2};cs_mm_config c={{0x400,0x1000,0x1800},{0x402,0x2000,0x22000}};cs_mm_manager m;
    REQUIRE(cs_mm_init(&m,&map,&c)==CS_MM_OK);
    cs_trap_frame f={0};f.sr=0x271F;f.d[0]=65536;cs_mm_register_report report;
    REQUIRE(cs_mm_register_call(&m,0xA11E,0,&f,&report)==CS_MM_OK && report.reply.code==0);
    REQUIRE(cs_mm_register_call(&m,0xA021,0,&f,&report)==CS_MM_OK && f.d[0]==65536 && f.sr==0x2714);
    REQUIRE(cs_mm_register_call(&m,0xA01F,0,&f,&report)==CS_MM_OK && report.reply.code==0);
    f.d[0]=65536;REQUIRE(cs_mm_register_call(&m,0xA122,0,&f,&report)==CS_MM_OK && report.reply.code==0);
    REQUIRE(cs_mm_register_call(&m,0xA025,0,&f,&report)==CS_MM_OK && f.d[0]==65536 && f.sr==0x2714);
}
static void probe(void)
{
    fixture x,y;setup(&x);setup(&y);cs_mm_manager before=y.manager;cs_heap_probe_result r;
    REQUIRE(cs_heap_probe(&x.manager,&r) && r.checks==20 && r.status==0 && r.signature==0x48454131);
    REQUIRE(memcmp(&before,&y.manager,sizeof before)==0);oracle(&x);oracle(&y);
    REQUIRE(!cs_heap_probe(NULL,&r) && !cs_heap_probe(&x.manager,NULL));
}
static void sequences(void)
{
    typedef struct {uint32_t identity,size,seed,last;unsigned kind,locked;} model;
    model objects[32]={0};fixture x;setup(&x);uint32_t random=0xC51C0097;
    for(unsigned step=0;step<1500;++step) {
        random=random*1664525u+1013904223u;unsigned slot=(random>>16)%32u,action=(random>>8)%6u;
        model *a=&objects[slot];cs_mm_reply r;
        if(a->kind==0) {
            unsigned kind=(random&1u)?2u:1u;uint32_t size=(random>>20)%201u;
            unsigned flags=(random>>4)&3u;put(&x,0x118,4,(random&4u)?0x400u:0x402u);
            r=execute(&x,kind==2?CS_MM_NEW_HANDLE:CS_MM_NEW_PTR,0,size,flags);
            if(r.code==0) {
                a->identity=r.value;a->size=size;a->seed=(random>>12)%251u;a->kind=kind;a->locked=0;
                uint32_t at=kind==2?get(&x,r.value,4):r.value;
                for(uint32_t i=0;i<size;++i) {if(flags&CS_MM_CLEAR) REQUIRE(get(&x,at+i,1)==0);put(&x,at+i,1,(a->seed+i)%251u);}
            } else REQUIRE(r.code==CS_MM_FULL);
        } else if(action==0) {
            r=execute(&x,a->kind==2?CS_MM_DISPOS_HANDLE:CS_MM_DISPOS_PTR,a->identity,0,0);
            REQUIRE(r.code==0);a->kind=0;
        } else if(action<=2) {
            uint32_t at=a->kind==2?get(&x,a->identity,4)&0xFFFFFFu:a->identity,size=(random>>17)%601u;
            cs_mm_manager before=x.manager;uint8_t heap[8200];memcpy(heap,x.heap,sizeof heap);
            r=execute(&x,a->kind==2?CS_MM_SET_HANDLE_SIZE:CS_MM_SET_PTR_SIZE,a->identity,size,0);
            if(r.code==0) {
                uint32_t next=a->kind==2?get(&x,a->identity,4)&0xFFFFFFu:a->identity;
                if(a->kind==1 || a->locked) REQUIRE(next==at);
                uint32_t prefix=a->size<size?a->size:size;
                for(uint32_t i=0;i<prefix;++i) REQUIRE(get(&x,next+i,1)==(a->seed+i)%251u);
                a->size=size;for(uint32_t i=0;i<size;++i) put(&x,next+i,1,(a->seed+i)%251u);
            } else REQUIRE(r.code==CS_MM_FULL && memcmp(heap,x.heap,sizeof heap)==0 && memcmp(&before,&x.manager,sizeof before)==0);
        } else if(action<=4 && a->kind==2) {
            a->locked=action==3?1u:0u;
            if(random&2u) put(&x,a->identity,4,(get(&x,a->identity,4)&0x7FFFFFFFu)|(a->locked?CS_MM_LOCK:0u));
            r=execute(&x,a->locked?CS_MM_HLOCK:CS_MM_HUNLOCK,a->identity,0,0);REQUIRE(r.code==0);
        } else {
            r=execute(&x,a->kind==2?CS_MM_GET_HANDLE_SIZE:CS_MM_GET_PTR_SIZE,a->identity,0,0);
            REQUIRE(r.code==0 && r.value==a->size && r.moved==0);
        }
        for(unsigned i=0;i<32;++i) {
            model *b=&objects[i];if(!b->kind) continue;
            uint32_t cell=b->kind==2?get(&x,b->identity,4):b->identity,at=cell&0xFFFFFFu;
            if(b->kind==2) REQUIRE(((cell&CS_MM_LOCK)!=0)==(b->locked!=0));
            if(b->last && (b->kind==1 || b->locked)) REQUIRE(at==b->last);
            for(uint32_t j=0;j<b->size;++j) REQUIRE(get(&x,at+j,1)==(b->seed+j)%251u);
            b->last=at;
        }
        if(!a->kind) a->last=0;
        oracle(&x);
    }
}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"allocation")) allocation();else if(!strcmp(argv[1],"zones")) zones();
    else if(!strcmp(argv[1],"lifetime")) lifetime();else if(!strcmp(argv[1],"resize")) resize();
    else if(!strcmp(argv[1],"compaction")) compaction();else if(!strcmp(argv[1],"locks")) locks();
    else if(!strcmp(argv[1],"copies")) copies();else if(!strcmp(argv[1],"failures")) failures();
    else if(!strcmp(argv[1],"registers")) registers();else if(!strcmp(argv[1],"long-sizes")) long_sizes();
    else if(!strcmp(argv[1],"probe")) probe();else if(!strcmp(argv[1],"sequences")) sequences();else return 2;
    return 0;
}
