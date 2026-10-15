/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original generated host protocol cases. */
#include "../personality/system1/resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
#define TYPE UINT32_C(0x6F776E21)
typedef struct {
    uint8_t low[1024],ram[131072],rom[512],source[70000],stage[8];
    cs_guest_region regions[3];cs_guest_map map;cs_mm_manager manager;
    cs_life_controller life;cs_res_service service;cs_res_profile profile;
    cs_trap_gateway gateway;cs_m68k_session session;cs_res_ticket input;
    cs_life_entry registration,inventory[7];cs_life_plan plan;cs_life_scrap scrap;
    cs_life_request reset;cs_life_report lr;cs_res_request request;cs_res_report report;
    size_t source_length;uint32_t data,map_at,refs,names;
} fixture;
static fixture *make(void){fixture *x=calloc(1,sizeof *x);REQUIRE(x);return x;}
static void put16(uint8_t *b,uint32_t at,uint32_t v){b[at]=(uint8_t)(v>>8);b[at+1]=(uint8_t)v;}
static void put32(uint8_t *b,uint32_t at,uint32_t v){put16(b,at,v>>16);put16(b,at+2,v);}
static uint32_t read_guest(const fixture *x,uint32_t at)
{uint32_t v=0;REQUIRE(cs_guest_read(&x->map,at,4,CS_GUEST_READ,&v)==CS_GUEST_OK);return v;}
static void write_guest(fixture *x,uint32_t at,uint32_t value)
{REQUIRE(cs_guest_write(&x->map,at,4,value)==CS_GUEST_OK);}
static uint8_t pattern(uint32_t id,uint32_t i){return (uint8_t)(id*17u+i*31u+5u);}
/* Independent declarative encoder, no reader/manager helper in its construction. */
static void source(fixture *x,uint32_t count,uint32_t length)
{
    uint32_t data_bytes=count*(4u+length);x->data=256;x->map_at=256+data_bytes;
    x->refs=x->map_at+38;x->names=x->refs+count*12u;
    x->source_length=x->names+4u;REQUIRE(x->source_length<=sizeof x->source);
    memset(x->source,0,sizeof x->source);
    put32(x->source,0,256);put32(x->source,4,x->map_at);put32(x->source,8,data_bytes);
    put32(x->source,12,42u+count*12u);put16(x->source,x->map_at+24,28);
    put16(x->source,x->map_at+26,38u+count*12u);put16(x->source,x->map_at+28,0);
    put32(x->source,x->map_at+30,TYPE);put16(x->source,x->map_at+34,count?count-1u:65535u);
    put16(x->source,x->map_at+36,10);
    x->source[x->names]=3;memcpy(x->source+x->names+1,"own",3);
    for(uint32_t i=0;i<count;++i){uint32_t ref=x->refs+i*12u,dp=i*(4u+length);
        put16(x->source,ref,i);put16(x->source,ref+2,i?65535u:0u);
        x->source[ref+5]=(uint8_t)(dp>>16);put16(x->source,ref+6,dp);
        put32(x->source,x->data+dp,length);
        for(uint32_t j=0;j<length;++j)x->source[x->data+dp+4u+j]=pattern(i,j);
    }
}
static void setup_profile(fixture *x,uint32_t count,uint32_t length,uint32_t maximum_source,uint32_t maximum_resource)
{
    memset(x,0,sizeof *x);memset(x->low,0xD3,sizeof x->low);memset(x->ram,0xB7,sizeof x->ram);
    x->regions[0]=(cs_guest_region){x->low,x->low,0,1024,3};
    x->regions[1]=(cs_guest_region){x->ram,x->ram,0x1000,131072,3};
    x->regions[2]=(cs_guest_region){x->rom,0,0x400000,512,5};x->map=(cs_guest_map){x->regions,3};
    cs_mm_config config={{0x400,0x1000,0x1800},{0x402,0x2000,0x12000}};
    REQUIRE(cs_mm_init(&x->manager,&x->map,&config)==CS_MM_OK);
    REQUIRE(cs_life_init(&x->life,&x->manager)==CS_LIFE_OK);
    cs_trap_config g={0x400000,0x1000,0x1800,0x400010,0x400012,0x400014};
    REQUIRE(cs_trap_init(&x->gateway,&g)==CS_TRAP_OK);
    for(unsigned i=0;i<4;++i){x->rom[0x100u+i*2u]=0x4E;x->rom[0x101u+i*2u]=0x71;}
    cs_m68k_bus_options options={CS_M68K_DATA24,0,0};cs_m68k_call_config call={0x18000,0x400,0x400100,2};
    REQUIRE(cs_m68k_session_init(&x->session,&x->map,&x->gateway,&options,&call)==CS_M68K_SESSION_OK);
    x->profile=(cs_res_profile){maximum_source,maximum_resource,77};
    REQUIRE(cs_res_init(&x->service,&x->life,&x->profile)==CS_RES_OK);
    x->registration=(cs_life_entry){CS_LIFE_FUTURE,CS_LIFE_REFUSE,77,0,sizeof x->service,0,&x->service};
    x->request.context=(cs_res_context){CS_LIFE_ORDINARY,0,0,0,0,&x->session,&x->registration};
    source(x,count,length);
}
static void setup(fixture *x,uint32_t count,uint32_t length)
{setup_profile(x,count,length,70000,65000);}
static cs_res_result op(fixture *x,cs_res_operation operation,int32_t id)
{
    x->request.operation=operation;x->request.type=TYPE;x->request.id=id;
    x->request.source=operation==CS_RES_OPEN?x->source:0;
    x->request.length=operation==CS_RES_OPEN?x->source_length:0;
    x->request.ticket=(operation==CS_RES_DESCRIBE || operation==CS_RES_RELEASE)?&x->input:0;
    return cs_res_execute(&x->service,&x->request,&x->report);
}
static void open_source(fixture *x){REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_OK);}
static cs_res_ticket acquire(fixture *x,int32_t id)
{REQUIRE(op(x,CS_RES_ACQUIRE,id)==CS_RES_OK && x->report.ticket_valid);return x->report.ticket;}
static void release(fixture *x,cs_res_ticket t){x->input=t;REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_OK);}
static fixture *save(const fixture *x){fixture *s=make();memcpy(s,x,sizeof *s);return s;}
static void unchanged(const fixture *x,const fixture *s)
{
    REQUIRE(!memcmp(x->low,s->low,sizeof x->low) && !memcmp(x->ram,s->ram,sizeof x->ram));
    REQUIRE(!memcmp(x->source,s->source,sizeof x->source) && !memcmp(&x->manager,&s->manager,sizeof x->manager));
    REQUIRE(!memcmp(&x->life,&s->life,sizeof x->life) && !memcmp(&x->service,&s->service,sizeof x->service));
    REQUIRE(!memcmp(&x->session,&s->session,sizeof x->session) && !memcmp(&x->gateway,&s->gateway,sizeof x->gateway));
}
static void check_payload(fixture *x,cs_res_ticket t,uint32_t id,uint32_t length)
{
    x->input=t;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK && x->report.span_valid && x->report.length==length);
    REQUIRE(x->report.span.writable==0 && x->report.empty==(length==0));
    if(!length)REQUIRE(x->report.span.bytes==0);else for(uint32_t i=0;i<length;++i)REQUIRE(x->report.span.bytes[i]==pattern(id,i));
    REQUIRE((read_guest(x,t.handle)>>24)==0x80);
}
static void success(void)
{
    fixture *x=make();setup(x,4,7);fixture *s=save(x);open_source(x);
    REQUIRE(x->report.completed==1 && x->report.native_committed && !x->report.guest_committed);
    REQUIRE(!memcmp(x->ram,s->ram,sizeof x->ram) && !memcmp(x->source,s->source,sizeof x->source));
    cs_res_ticket a=acquire(x,0);REQUIRE(x->report.completed==11 && x->report.allocated && x->report.guest_committed);
    cs_res_ticket b=acquire(x,0);REQUIRE(a.handle==b.handle && a.identity!=b.identity && x->report.completed==1 && !x->report.allocated);
    check_payload(x,a,0,7);free(s);s=save(x);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_IN_USE);unchanged(x,s);
    REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_IN_USE);unchanged(x,s);release(x,a);release(x,b);
    REQUIRE(x->service.cache[0].live && x->service.cache[0].leases==0);
    REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_OK && x->report.disposed_mask==1 && x->report.retired_mask==1 && x->report.completed==2);
    cs_res_ticket c=acquire(x,0);REQUIRE(c.handle==a.handle && c.identity>b.identity);check_payload(x,c,0,7);release(x,c);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK && x->service.source_borrow==0 && x->report.completed==3);
    REQUIRE(!x->report.generation_valid && !x->report.epoch_valid && x->report.heap_epoch_valid);
    REQUIRE(cs_res_external_admit(&x->service)==CS_RES_OK);uint32_t last=x->service.last_ticket_id;
    memset(x->source,0xCE,sizeof x->source);source(x,2,3);open_source(x);REQUIRE(x->service.generation==2 && x->service.last_ticket_id==last);
    c=acquire(x,1);check_payload(x,c,1,3);release(x,c);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK && !x->report.completed);
    setup(x,1,0);open_source(x);c=acquire(x,0);check_payload(x,c,0,0);REQUIRE(x->service.cache[0].extent==2);release(x,c);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);setup(x,1,33001);open_source(x);c=acquire(x,0);check_payload(x,c,0,33001);
    REQUIRE(x->report.entry.data.length==33001);release(x,c);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    setup(x,1,65000);open_source(x);c=acquire(x,0);check_payload(x,c,0,65000);
    release(x,c);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    free(s);free(x);
}
static cs_mm_reply mm(fixture *x,cs_mm_operation operation,uint32_t at,uint32_t size,unsigned flags)
{cs_mm_request q={operation,at,0,size,flags,0};cs_mm_reply r;
    REQUIRE(cs_mm_execute(&x->manager,&q,&r)==CS_MM_OK);return r;}
static void limits(void)
{
    fixture *x=make();setup(x,20,1);open_source(x);
    for(int32_t i=0;i<16;++i){cs_res_ticket t=acquire(x,i);release(x,t);}fixture *s=save(x);
    REQUIRE(op(x,CS_RES_ACQUIRE,16)==CS_RES_CACHE_FULL);unchanged(x,s);
    cs_res_ticket t=acquire(x,0);release(x,t);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    setup(x,1,1);open_source(x);cs_res_ticket tickets[32];for(unsigned i=0;i<32;++i)tickets[i]=acquire(x,0);
    free(s);s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_TICKETS_FULL);unchanged(x,s);
    for(unsigned i=0;i<32;++i)release(x,tickets[i]);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    setup(x,1,1);open_source(x);x->service.last_ticket_id=UINT32_MAX-1u;t=acquire(x,0);
    REQUIRE(t.identity==UINT32_MAX);free(s);s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_COUNTER_EXHAUSTED);unchanged(x,s);
    release(x,t);REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_OK && op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    x->service.last_open_generation=UINT32_MAX-1u;open_source(x);REQUIRE(x->service.generation==UINT32_MAX);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);free(s);s=save(x);REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_COUNTER_EXHAUSTED);unchanged(x,s);
    setup(x,1,9);open_source(x);write_guest(x,0x130,0x2088);free(s);s=save(x);
    REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_MANAGER_CAPACITY && x->report.reply_valid && x->report.reply.code==CS_MM_FULL);
    REQUIRE(!x->report.guest_committed && !x->report.native_committed && !x->report.completed);unchanged(x,s);
    setup(x,1,1);open_source(x);for(unsigned i=0;i<32;++i)REQUIRE(mm(x,CS_MM_NEW_PTR,0,2,i<10?CS_MM_SYSTEM:0).code==0);
    free(s);s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_MANAGER_CAPACITY);unchanged(x,s);
    /* Fragmented application compaction succeeds; system payload/record stays. */
    setup(x,1,48);open_source(x);cs_mm_reply sy=mm(x,CS_MM_NEW_HANDLE,0,6,CS_MM_SYSTEM);REQUIRE(sy.code==0);
    uint32_t system_at=read_guest(x,sy.value);uint8_t sysbytes[0x800];memcpy(sysbytes,x->ram,sizeof sysbytes);
    write_guest(x,0x130,0x2100);cs_mm_reply a=mm(x,CS_MM_NEW_HANDLE,0,32,0),b=mm(x,CS_MM_NEW_HANDLE,0,32,0),c=mm(x,CS_MM_NEW_HANDLE,0,32,0);
    REQUIRE(a.code==0 && b.code==0 && c.code==0 && mm(x,CS_MM_DISPOS_HANDLE,b.value,0,0).code==0);
    t=acquire(x,0);REQUIRE(x->report.moved==2 && read_guest(x,sy.value)==system_at && !memcmp(sysbytes,x->ram,sizeof sysbytes));
    check_payload(x,t,0,48);free(s);free(x);
}
static void tickets(void)
{
    fixture *x=make(),*y=make();setup(x,2,5);setup(y,2,5);open_source(x);open_source(y);
    cs_res_ticket t=acquire(x,0),foreign=acquire(y,0);x->input=foreign;fixture *s=save(x);
    REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_INVALID_TICKET);unchanged(x,s);
    for(unsigned i=0;i<9;++i){x->input=t;switch(i){case 0:x->input.owner=0;break;case 1:++x->input.generation;break;
        case 2:++x->input.epoch;break;case 3:++x->input.identity;break;case 4:x->input.slot=16;break;
        case 5:++x->input.type;break;case 6:++x->input.id;break;case 7:++x->input.handle;break;default:++x->input.length;break;}
        REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_INVALID_TICKET);unchanged(x,s);
        REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_INVALID_TICKET);unchanged(x,s);}
    x->input=t;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK && !x->report.completed);release(x,t);
    free(s);s=save(x);x->input=t;REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_INVALID_TICKET);unchanged(x,s);
    REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_OK);cs_res_ticket n=acquire(x,1);REQUIRE(n.handle==t.handle && n.identity!=t.identity);
    x->input=t;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_INVALID_TICKET);check_payload(x,n,1,5);release(x,n);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);open_source(x);x->input=t;REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_INVALID_TICKET);
    free(s);free(x);free(y);
}
static void sources(void)
{
    fixture *x=make();setup(x,2,5);x->source[x->map_at+24]=0xFF;fixture *s=save(x);
    REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_SOURCE && x->report.reader_valid && !x->service.source_borrow);unchanged(x,s);
    setup(x,2,5);open_source(x);free(s);s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,9)==CS_RES_NOT_FOUND);unchanged(x,s);
    REQUIRE(op(x,CS_RES_ACQUIRE,32768)==CS_RES_ARGUMENT);unchanged(x,s);
    REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_NOT_RESIDENT);unchanged(x,s);
    setup(x,2,5);put16(x->source,x->refs+12,0);open_source(x);free(s);s=save(x);
    REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_AMBIGUOUS);unchanged(x,s);
    setup(x,1,5);x->source[x->refs+4]=0x80;put32(x->source,x->refs+8,0xFFFF1234);memset(x->source+x->refs+5,0xFF,3);
    open_source(x);free(s);s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_UNRESOLVED_REFERENCE && x->report.entry_valid);
    REQUIRE(x->report.entry.specification==0xFFFF1234 && !x->report.handle_valid && !x->report.span_valid);unchanged(x,s);
    setup(x,1,5);x->source[x->refs+4]=0x40;open_source(x);free(s);s=save(x);
    REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_SYSTEM_HEAP_UNSUPPORTED);unchanged(x,s);
    setup(x,2,5);x->source[x->refs+4]=0x3C;put32(x->source,x->refs+8,0x12345678);
    memcpy(x->source+x->refs+17,x->source+x->refs+5,3);open_source(x);cs_res_ticket a=acquire(x,0),b=acquire(x,1);
    REQUIRE(a.handle!=b.handle);x->input=b;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK);
    for(unsigned i=0;i<5;++i)REQUIRE(x->report.span.bytes[i]==pattern(0,i));
    x->input=a;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK && x->report.entry.attributes==0x3C);
    REQUIRE(x->report.reader_valid && (x->report.reader.warnings&CS_RF_WARN_HANDLE));
    REQUIRE(x->report.entry.name_present && x->report.entry.name.length==3);
    free(s);s=save(x);release(x,a);REQUIRE(op(x,CS_RES_EVICT,0)==CS_RES_OK);
    x->input=b;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK);
    for(unsigned i=0;i<5;++i)REQUIRE(x->report.span.bytes[i]==pattern(0,i));
    REQUIRE(!memcmp(x->source,s->source,sizeof x->source));release(x,b);
    setup_profile(x,1,5,70000,4);open_source(x);free(s);s=save(x);
    REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_PROFILE_LIMIT);unchanged(x,s);
    setup_profile(x,1,5,1,65000);free(s);s=save(x);
    REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_PROFILE_LIMIT);unchanged(x,s);
    setup(x,0,0);open_source(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_NOT_FOUND);
    free(s);free(x);
}
static void integrity(void)
{
    fixture *x=make();for(unsigned k=0;k<14;++k){setup(x,2,5);open_source(x);cs_res_ticket t=acquire(x,0);release(x,t);
        unsigned slot=x->service.cache[0].manager_slot;switch(k){case 0:++x->life.epoch;break;
        case 1:x->regions[1].flags=1;break;case 2:++x->manager.blocks[slot].size;break;
        case 3:++x->service.cache[0].address;break;case 4:write_guest(x,t.handle,read_guest(x,t.handle)&0xFFFFFFu);break;
        case 5:write_guest(x,t.handle,0);break;case 6:write_guest(x,t.handle,read_guest(x,t.handle)|0x01000000u);break;
        case 7:write_guest(x,0x118,x->manager.config.system.identity);break;
        case 8:REQUIRE(mm(x,CS_MM_DISPOS_HANDLE,t.handle,0,0).code==0);break;
        case 9:REQUIRE(mm(x,CS_MM_SET_HANDLE_SIZE,t.handle,4,0).code==0);break;
        case 10:x->service.cache[0].leases=1;break;case 11:x->registration.kind=CS_LIFE_NATIVE;break;
        case 12:x->manager.config.application.identity=0x404;break;default:x->service.map=&x->session.calls.map[0];x->manager.map=0;break;}
        fixture *s=save(x);cs_res_result expected=k==13?CS_RES_ARGUMENT:CS_RES_CONFLICT;
        REQUIRE(op(x,CS_RES_CLOSE,0)==expected);unchanged(x,s);
        if(k==0)REQUIRE(x->report.epoch_valid && x->report.epoch==1 && x->report.heap_epoch_valid && x->report.heap_epoch==2);
        free(s);
    }
    setup(x,1,5);open_source(x);cs_res_ticket t=acquire(x,0);uint32_t at=read_guest(x,t.handle)&0xFFFFFFu;
    write_guest(x,at&~1u,0xDEADBEEF); /* Payload authentication is expressly absent. */
    x->input=t;REQUIRE(op(x,CS_RES_DESCRIBE,0)==CS_RES_OK && x->report.span.bytes[0]==0xDE);
    setup(x,1,5);fixture *s=save(x);REQUIRE(cs_res_init(&x->service,&x->life,&x->profile)==CS_RES_STATE);unchanged(x,s);
    x->profile.maximum_source=0;REQUIRE(cs_res_init(&x->service,&x->life,&x->profile)==CS_RES_STATE);unchanged(x,s);
    free(s);free(x);
}
static void aliases(void)
{
    fixture *x=make();setup(x,1,5);open_source(x);x->request.operation=CS_RES_ACQUIRE;x->request.type=TYPE;
    void *targets[]={x->source,x->low,x->ram,&x->service,&x->life,&x->manager,&x->map,&x->regions,&x->session,&x->gateway,&x->registration,&x->request};
    fixture *s=save(x);for(unsigned i=0;i<sizeof targets/sizeof targets[0];++i){
        REQUIRE(cs_res_execute(&x->service,&x->request,(cs_res_report *)targets[i])==CS_RES_ARGUMENT);unchanged(x,s);}
    cs_res_ticket t=acquire(x,0);x->input=t;x->request.operation=CS_RES_RELEASE;x->request.ticket=&x->report.ticket;
    memset(&x->report,0x55,sizeof x->report);cs_res_report before=x->report;
    REQUIRE(cs_res_execute(&x->service,&x->request,&x->report)==CS_RES_ARGUMENT && !memcmp(&before,&x->report,sizeof before));
    setup(x,1,5);x->request.operation=CS_RES_OPEN;x->request.source=x->ram;x->request.length=400;
    free(s);s=save(x);REQUIRE(cs_res_execute(&x->service,&x->request,&x->report)==CS_RES_ARGUMENT);unchanged(x,s);
    x->request.source=(const uint8_t *)&x->service;x->request.length=sizeof x->service;
    REQUIRE(cs_res_execute(&x->service,&x->request,&x->report)==CS_RES_ARGUMENT);unchanged(x,s);
    x->request.source=x->source;x->request.length=0;REQUIRE(cs_res_execute(&x->service,&x->request,&x->report)==CS_RES_ARGUMENT);
    unchanged(x,s);
    /* Misaligned typed outputs refuse before dereference. */
    REQUIRE(cs_res_execute(&x->service,&x->request,(cs_res_report *)(x->source+1))==CS_RES_ARGUMENT);
    unchanged(x,s);
    /* Changed map descriptors cannot make an old guest backing buffer a safe
     * error-report destination. Both saved and current backing spans protect it. */
    setup(x,1,5);cs_guest_region replacement[3];memcpy(replacement,x->regions,sizeof replacement);
    uint8_t *new_ram=malloc(sizeof x->ram);REQUIRE(new_ram);memcpy(new_ram,x->ram,sizeof x->ram);
    replacement[1].bytes=new_ram;replacement[1].writable=new_ram;x->map.regions=replacement;
    x->request.operation=CS_RES_OPEN;free(s);s=save(x);
    REQUIRE(cs_res_execute(&x->service,&x->request,(cs_res_report *)x->ram)==CS_RES_ARGUMENT);unchanged(x,s);
    REQUIRE(cs_res_execute(&x->service,&x->request,&x->report)==CS_RES_CONFLICT);unchanged(x,s);free(new_ram);
    free(s);free(x);
}
static void reset_request(fixture *x,int future)
{
    static const uint32_t locations[]={0x114,0x118,0x130,0x2A6,0x2AA,0x31A};
    for(unsigned i=0;i<6;++i)x->inventory[i]=(cs_life_entry){CS_LIFE_GLOBAL,
        (i==1 || i==2)?CS_LIFE_REBIND:CS_LIFE_PRESERVE,1,locations[i],4,read_guest(x,locations[i]),0};
    x->inventory[6]=(cs_life_entry){future?CS_LIFE_FUTURE:CS_LIFE_NATIVE,future?CS_LIFE_REFUSE:CS_LIFE_PRESERVE,
        77,0,sizeof x->service,0,&x->service};
    x->scrap=(cs_life_scrap){0,CS_LIFE_NONE,0,0,0};
    x->reset=(cs_life_request){CS_LIFE_RESET,CS_LIFE_ORDINARY,0,0,0,0,
        0x18000,0x400,0x18400,0x6400,0,&x->scrap,x->stage,sizeof x->stage,x->inventory,7,&x->gateway,&x->session};
}
static void lifecycle(void)
{
    fixture *x=make();setup(x,2,5);REQUIRE(cs_res_external_admit(&x->service)==CS_RES_OK);
    open_source(x);REQUIRE(cs_res_external_admit(&x->service)==CS_RES_STATE);
    reset_request(x,1);fixture *s=save(x);
    REQUIRE(cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr)==CS_LIFE_REFERENCE);unchanged(x,s);
    cs_res_ticket t=acquire(x,0);release(x,t);reset_request(x,1);
    REQUIRE(cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr)==CS_LIFE_REFERENCE);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);reset_request(x,0);
    REQUIRE(cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr)==CS_LIFE_OK);
    REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_CONTEXT);REQUIRE(cs_life_commit(&x->life,&x->plan,&x->lr)==CS_LIFE_OK);
    open_source(x);REQUIRE(x->service.epoch==2 && x->service.generation==2);t=acquire(x,0);check_payload(x,t,0,5);release(x,t);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);
    for(unsigned k=0;k<9;++k){setup(x,1,5);switch(k){case 0:x->request.context.context=CS_LIFE_UNKNOWN_CONTEXT;break;
        case 1:x->request.context.context=CS_LIFE_INTERRUPT;break;case 2:x->request.context.active_root=1;break;
        case 3:x->request.context.active_callback=1;break;case 4:x->request.context.active_patch=1;break;
        case 5:x->request.context.active_manager=1;break;case 6:x->session.active=1;break;
        case 7:x->session.calls.depth=1;break;default:x->session.tag=0;break;}
        free(s);s=save(x);REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_CONTEXT);unchanged(x,s);}
    setup(x,1,5);x->request.context.registration=0;REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_CONFLICT);
    setup(x,1,5);x->registration.kind=CS_LIFE_NATIVE;x->request.context.context=CS_LIFE_INTERRUPT;
    free(s);s=save(x);REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_CONTEXT);unchanged(x,s);
    x->request.context.context=CS_LIFE_ORDINARY;REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_CONFLICT);unchanged(x,s);
    free(s);free(x);
}
static void model(void)
{
    fixture *x=make();setup(x,8,7);open_source(x);cs_res_ticket live[32];unsigned used[32]={0},resident[8]={0},counts[8]={0};uint32_t random=31337,last_id=0;
    for(unsigned n=0;n<2500;++n){random=random*1664525u+1013904223u;unsigned action=(random>>12)%4u,id=(random>>20)%8u;
        if(action<2){unsigned slot=0;while(slot<32 && used[slot])++slot;
            cs_res_result r=op(x,CS_RES_ACQUIRE,(int32_t)id);if(slot==32)REQUIRE(r==CS_RES_TICKETS_FULL);
            else{REQUIRE(r==CS_RES_OK && x->report.ticket.identity>last_id);last_id=x->report.ticket.identity;
                live[slot]=x->report.ticket;used[slot]=1;resident[id]=1;++counts[id];check_payload(x,live[slot],id,7);}
        }else if(action==2){unsigned slot=(random>>4)%32u;if(used[slot]){unsigned key=(unsigned)live[slot].id;
            release(x,live[slot]);used[slot]=0;--counts[key];x->input=live[slot];REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_INVALID_TICKET);}
        }else{cs_res_result r=op(x,CS_RES_EVICT,(int32_t)id);
            REQUIRE(r==(!resident[id]?CS_RES_NOT_RESIDENT:counts[id]?CS_RES_IN_USE:CS_RES_OK));if(r==CS_RES_OK)resident[id]=0;}
        for(unsigned i=0;i<8;++i){unsigned found=0;for(unsigned j=0;j<16;++j)if(x->service.cache[j].live && x->service.cache[j].entry.id==(int32_t)i){
            REQUIRE(x->service.cache[j].leases==counts[i]);++found;}REQUIRE(found==resident[i]);}
    }
    for(unsigned i=0;i<32;++i)if(used[i])release(x,live[i]);
    REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_OK);free(x);
}
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
static void quarantine(fixture *x)
{
    fixture *s=save(x);cs_res_report first=x->report;
    for(unsigned i=1;i<=6;++i){REQUIRE(op(x,(cs_res_operation)i,0)==CS_RES_FAULT);
        REQUIRE(x->report.operation==(cs_res_operation)i && x->report.retained_fault && !x->report.completed &&
            !x->report.guest_committed && !x->report.native_committed && !x->report.handle_valid && !x->report.ticket_valid && !x->report.span_valid);
        REQUIRE(!memcmp(&first,&x->service.first_fault,sizeof first));unchanged(x,s);}
    REQUIRE(cs_res_external_admit(&x->service)==CS_RES_FAULT);
    x->request.operation=CS_RES_CLOSE;REQUIRE(cs_res_execute(&x->service,&x->request,(cs_res_report *)x->source)==CS_RES_ARGUMENT);
    unchanged(x,s);REQUIRE(cs_res_init(&x->service,&x->life,&x->profile)==CS_RES_STATE);
    reset_request(x,1);cs_life_result refused=cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr);
    REQUIRE(refused==CS_LIFE_REFERENCE || refused==CS_LIFE_MEMORY);unchanged(x,s);
    free(s);
}
static void faults(void)
{
    fixture *x=make();for(uint32_t stop=1;stop<=11;++stop){setup(x,3,7);open_source(x);
        REQUIRE(cs_res_test_fault(&x->service,stop,0,0)==CS_RES_OK);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_FAULT);
        REQUIRE(x->report.completed==stop && x->service.state==CS_RES_FAULTED && x->service.source_borrow);
        REQUIRE(x->report.allocated && x->report.handle_valid && x->report.guest_committed);
        REQUIRE(x->report.cache_valid==(stop>=10) && x->report.ticket_valid==(stop==11));
        REQUIRE(x->service.last_ticket_id==(stop==11));if(stop>=2)REQUIRE((read_guest(x,x->report.handle)>>24)==0x80);
        uint32_t bytes=stop<3?0:stop<9?stop-2u:7u;
        uint32_t at=read_guest(x,x->report.handle)&0xFFFFFFu;for(uint32_t i=0;i<bytes;++i)REQUIRE(x->ram[at-0x1000u+i]==pattern(0,i));
        fixture *s=save(x);cs_res_report first=x->report;
        for(unsigned opn=1;opn<=6;++opn){REQUIRE(op(x,(cs_res_operation)opn,0)==CS_RES_FAULT);
            REQUIRE(x->report.operation==(cs_res_operation)opn && x->report.retained_fault && !x->report.completed &&
                !x->report.guest_committed && !x->report.native_committed && !x->report.ticket_valid && !x->report.span_valid);
            REQUIRE(!memcmp(&first,&x->service.first_fault,sizeof first));unchanged(x,s);}
        REQUIRE(cs_res_external_admit(&x->service)==CS_RES_FAULT);
        REQUIRE(cs_res_init(&x->service,&x->life,&x->profile)==CS_RES_STATE);reset_request(x,1);
        REQUIRE(cs_life_prepare(&x->life,&x->reset,&x->plan,&x->lr)==CS_LIFE_REFERENCE);free(s);
    }
    for(uint32_t stop=1;stop<=7;++stop){setup(x,3,1);open_source(x);for(int32_t i=0;i<3;++i){cs_res_ticket t=acquire(x,i);release(x,t);}
        REQUIRE(cs_res_test_fault(&x->service,stop,0,0)==CS_RES_OK);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_FAULT);
        unsigned disposed=(stop+1u)/2u;if(disposed>3)disposed=3;unsigned retired=stop/2u;if(retired>3)retired=3;
        REQUIRE(x->report.completed==stop && x->report.disposed_mask==(1u<<disposed)-1u && x->report.retired_mask==(1u<<retired)-1u);
        REQUIRE(x->service.source_borrow && x->service.state==CS_RES_FAULTED);
        for(unsigned i=0;i<3;++i){const cs_res_cached *c=&x->service.cache[i];
            if(i<disposed)REQUIRE(x->manager.blocks[c->manager_slot].kind==0);
            else{REQUIRE(c->live && !c->leases && x->manager.blocks[c->manager_slot].kind==2);
                REQUIRE((read_guest(x,c->handle)>>24)==0x80 && x->ram[c->address-0x1000u]==pattern(i,0));}}
        quarantine(x);
    }
    setup(x,1,1);REQUIRE(cs_res_test_fault(&x->service,1,0,0)==CS_RES_OK);REQUIRE(op(x,CS_RES_OPEN,0)==CS_RES_FAULT);
    REQUIRE(!x->report.guest_committed && x->report.native_committed && x->service.source_borrow && x->service.generation==1);
    quarantine(x);
    for(unsigned kind=1;kind<=2;++kind){setup(x,1,7);open_source(x);REQUIRE(cs_res_test_fault(&x->service,0,kind,1)==CS_RES_OK);
        fixture *s=save(x);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_FAULT && !x->report.manager_valid && !x->report.lifecycle_valid && !x->report.reply_valid);
        REQUIRE(x->report.completed==kind-1u && x->report.guest_committed==(kind==2) && x->report.allocated==(kind==2));
        if(kind==1)REQUIRE(!memcmp(x->ram,s->ram,sizeof x->ram) && !memcmp(&x->manager,&s->manager,sizeof x->manager));
        free(s);quarantine(x);}
    setup(x,3,1);open_source(x);for(int32_t i=0;i<3;++i){cs_res_ticket t=acquire(x,i);release(x,t);}
    REQUIRE(cs_res_test_fault(&x->service,0,3,2)==CS_RES_OK);REQUIRE(op(x,CS_RES_CLOSE,0)==CS_RES_FAULT);
    REQUIRE(x->report.disposed_mask==1 && x->report.retired_mask==1 && x->report.completed==2 && x->service.source_borrow);
    quarantine(x);
    setup(x,1,1);open_source(x);cs_res_ticket t=acquire(x,0);REQUIRE(cs_res_test_fault(&x->service,1,0,0)==CS_RES_OK);
    x->input=t;REQUIRE(op(x,CS_RES_RELEASE,0)==CS_RES_FAULT && !x->report.guest_committed && x->report.native_committed);
    REQUIRE(!x->service.leases[0].live && !x->service.cache[0].leases);quarantine(x);
    for(unsigned kind=4;kind<=5;++kind){setup(x,1,7);open_source(x);
        REQUIRE(cs_res_test_fault(&x->service,0,kind,1)==CS_RES_OK);REQUIRE(op(x,CS_RES_ACQUIRE,0)==CS_RES_FAULT);
        REQUIRE(x->report.phase==CS_RES_LOCKING && x->report.completed==2 && x->report.allocated && !x->report.cache_valid);
        quarantine(x);}
    free(x);
}
#endif
int main(int argc,char **argv)
{
    REQUIRE(argc==2);
    if(!strcmp(argv[1],"success"))success();else if(!strcmp(argv[1],"limits"))limits();
    else if(!strcmp(argv[1],"tickets"))tickets();else if(!strcmp(argv[1],"source"))sources();
    else if(!strcmp(argv[1],"integrity"))integrity();else if(!strcmp(argv[1],"aliases"))aliases();
    else if(!strcmp(argv[1],"lifecycle"))lifecycle();else if(!strcmp(argv[1],"model"))model();
#ifdef CLASSICK_RESOURCES_TEST_FAULTS
    else if(!strcmp(argv[1],"faults"))faults();
#endif
    else REQUIRE(0);
    printf("Resource residency %s PASS\n",argv[1]);return 0;
}
