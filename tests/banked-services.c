/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../personality/system1/banked-services.h"
static unsigned checks;
#define REQUIRE(x) do {++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct {uint8_t *ram;uint32_t base,size,fault;unsigned invalid,readonly;} context;
typedef struct {unsigned deny;} policy;
typedef struct {
    context memory;policy policy;cs_m68k_transfer_backend backend;
    cs_trap_gateway gateway;cs_trap_frame frame;cs_m68k_bus_report report;
    cs_m68k_stack_limits limits;cs_m68k_guard guard;uint8_t ram[256];
} fixture;
typedef struct {cs_trap_gateway gateway;cs_trap_frame frame;uint8_t ram[256];} saved;
static cs_guest_result validate(const void *p)
{const context *c=p;return c->invalid?CS_GUEST_MAP:CS_GUEST_OK;}
static cs_guest_result span(const void *p,uint32_t at,uint32_t bytes,uint32_t access)
{
    const context *c=p;
    if(validate(p)!=CS_GUEST_OK)return CS_GUEST_MAP;
    if(!bytes || at<c->base || bytes>c->size || at-c->base>c->size-bytes)return CS_GUEST_ADDRESS;
    if((access&2u)!=0 && c->readonly && bytes<=6u)return CS_GUEST_PERMISSION;
    return CS_GUEST_OK;
}
static cs_guest_result read_value(const void *p,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{
    const context *c=p;cs_guest_result r=span(p,at,width,access);uint32_t value=0;
    if(r!=CS_GUEST_OK)return r;
    if(width!=1u && (at&1u)!=0)return CS_GUEST_ADDRESS;
    for(unsigned i=0;i<width;++i)value=(value<<8)|c->ram[at-c->base+i];
    *out=value;return CS_GUEST_OK;
}
static cs_guest_result write_bytes(const void *p,uint32_t at,unsigned count,const uint8_t *bytes)
{
    const context *c=p;cs_guest_result r=span(p,at,count,2);
    if(r!=CS_GUEST_OK)return r;
    if(c->fault>=at && c->fault-at<count)return CS_GUEST_PERMISSION;
    for(unsigned i=0;i<count;++i)c->ram[at-c->base+i]=bytes[i];
    return CS_GUEST_OK;
}
static cs_guest_result write_value(const void *p,uint32_t at,unsigned width,uint32_t value)
{
    uint8_t bytes[4];for(unsigned i=0;i<width;++i)bytes[i]=(uint8_t)(value>>((width-1u-i)*8u));
    return write_bytes(p,at,width,bytes);
}
static int guard(const void *p,const cs_m68k_guard_event *e)
{
    const policy *q=p;
    if(q->deny==1u && e->kind==CS_M68K_GUARD_OPCODE && e->before==e->proposed)return 0;
    if(q->deny==2u && e->kind==CS_M68K_GUARD_OPCODE && e->before!=e->proposed)return 0;
    if(q->deny==3u && e->kind==CS_M68K_GUARD_INSTRUCTION)return 0;
    if(q->deny==4u && e->kind==CS_M68K_GUARD_WRITE)return 0;
    if(q->deny==5u && e->kind==CS_M68K_GUARD_COMMIT)return 0;
    return 1;
}
static void init(fixture *f,uint16_t word)
{
    memset(f,0,sizeof *f);memset(f->ram,0x69,sizeof f->ram);
    f->memory.ram=f->ram;f->memory.base=0x1000;f->memory.size=sizeof f->ram;f->memory.fault=UINT32_MAX;
    f->backend.scalar.context=&f->memory;f->backend.scalar.context_bytes=sizeof f->memory;
    f->backend.scalar.validate=validate;f->backend.scalar.span=span;f->backend.scalar.read=read_value;
    f->backend.scalar.write=write_value;f->backend.write_bytes=write_bytes;
    cs_trap_config config={0x400000,0x1000,0x1100,0x400000,0x400002,0x400020};
    REQUIRE(cs_trap_init(&f->gateway,&config)==CS_TRAP_OK);
    for(unsigned i=0;i<CS_TRAP_ENTRIES;++i)f->gateway.entries[i]=0x400100u+i*2u;
    f->gateway.entries[CS_TRAP_GET]=config.get_token;f->gateway.entries[CS_TRAP_SET]=config.set_token;
    for(unsigned i=0;i<8u;++i){f->frame.d[i]=0x12340000u+i;f->frame.a[i]=0x87650000u+i;}
    f->frame.pc=0x1000;f->frame.a[7]=0x1100;f->frame.sr=0x271f;
    f->limits.base=0x1080;f->limits.size=0x80;
    f->guard.check=guard;f->guard.context=&f->policy;f->guard.context_bytes=sizeof f->policy;
    f->ram[0]=(uint8_t)(word>>8);f->ram[1]=(uint8_t)word;
}
static void snapshot(const fixture *f,saved *s)
{memcpy(&s->gateway,&f->gateway,sizeof s->gateway);memcpy(&s->frame,&f->frame,sizeof s->frame);memcpy(s->ram,f->ram,sizeof s->ram);}
static void unchanged(const fixture *f,const saved *s)
{REQUIRE(!memcmp(&s->gateway,&f->gateway,sizeof s->gateway));REQUIRE(!memcmp(&s->frame,&f->frame,sizeof s->frame));REQUIRE(!memcmp(s->ram,f->ram,sizeof s->ram));}
static cs_m68k_result step(fixture *f)
{return cs_s1_banked_step(&f->backend,&f->gateway,&f->frame,&f->report,&f->limits,&f->guard);}
static void successes(void)
{
    fixture f;saved s;
    for(unsigned flag=0;flag<2u;++flag)for(unsigned index=0;index<512u;++index)for(unsigned ccr=0;ccr<32u;++ccr){
        init(&f,(uint16_t)(flag?0xa146u:0xa046u));f.frame.d[0]=0xdead0000u|index;f.frame.sr=(uint16_t)(0x2700u|ccr);
        snapshot(&f,&s);cs_trap_frame expected=s.frame;expected.pc+=2;
        if(flag)expected.a[0]=f.gateway.entries[index];
        expected.sr=(uint16_t)((expected.sr&0xfff0u)|(index==0?4u:0u));
        REQUIRE(step(&f)==CS_M68K_OK);REQUIRE(!memcmp(&f.frame,&expected,sizeof expected));
        REQUIRE(!memcmp(&f.gateway,&s.gateway,sizeof f.gateway));
        s.ram[250]=(uint8_t)(expected.sr>>8);s.ram[251]=(uint8_t)expected.sr;
        s.ram[252]=0;s.ram[253]=0;s.ram[254]=0x10;s.ram[255]=2;
        REQUIRE(!memcmp(f.ram,s.ram,sizeof f.ram));REQUIRE(f.report.cpu.service_called==1);
        REQUIRE(f.report.cpu.service.target==f.gateway.config.get_token);
    }
}
static void failures(void)
{
    fixture f;saved s;
    for(unsigned i=512;i<65536u;++i){init(&f,0xa146);f.frame.d[0]=i;snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_SERVICE);REQUIRE(f.report.cpu.service.status==CS_TRAP_NUMBER);unchanged(&f,&s);}
    for(unsigned deny=1;deny<=5u;++deny){
        init(&f,deny==2u?0x4e71:0xa146);f.policy.deny=deny;snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_POLICY);unchanged(&f,&s);
    }
    init(&f,0xa146);f.memory.fault=0x10ff;snapshot(&f,&s);
    REQUIRE(step(&f)==CS_M68K_ADDRESS);REQUIRE(f.report.memory==CS_GUEST_PERMISSION);unchanged(&f,&s);
    init(&f,0xa146);f.memory.readonly=1;snapshot(&f,&s);
    REQUIRE(step(&f)==CS_M68K_ADDRESS);unchanged(&f,&s);
    for(unsigned route=0;route<3u;++route){
        init(&f,0xa146);f.gateway.entries[CS_TRAP_GET]=route==0?f.gateway.config.set_token:
            route==1?f.gateway.config.unsupported_token:0x1000u;f.frame.d[0]=20;f.frame.a[0]=0x1000;
        snapshot(&f,&s);REQUIRE(step(&f)==CS_M68K_POLICY);REQUIRE(!f.report.cpu.service_called);unchanged(&f,&s);
    }
    const uint16_t words[]={0xa047,0xa147,0xa346,0xa946,0xa9f0,0xafff};
    for(unsigned i=0;i<sizeof words/sizeof words[0];++i){init(&f,words[i]);snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_POLICY);REQUIRE(!f.report.cpu.service_called);unchanged(&f,&s);}
    for(unsigned which=0;which<8u;++which){
        init(&f,0xa146);
        if(which==0)f.frame.pc=0x1001;
        if(which==1)f.frame.pc=0xfffffe;
        if(which==2)f.frame.sr=0x2000;
        if(which==3)f.frame.a[7]=0x10ff;
        if(which==4)f.frame.a[7]=f.limits.base+4u;
        if(which==5)f.memory.invalid=1;
        if(which==6)f.limits.size=0;
        if(which==7)f.gateway.magic=0;
        snapshot(&f,&s);REQUIRE(step(&f)!=CS_M68K_OK);unchanged(&f,&s);
    }
    init(&f,0xa146);snapshot(&f,&s);
    REQUIRE(cs_m68k_bus_step_transfer(&f.backend,&f.frame,&f.report,&f.limits,&f.guard)==CS_M68K_POLICY);
    unchanged(&f,&s);
}
static void envelopes_and_delegation(void)
{
    fixture f;saved s;cs_m68k_bus_report report;
    union {cs_m68k_transfer_backend alignment;uint8_t bytes[sizeof(cs_m68k_transfer_backend)+1u];} misaligned;
    for(unsigned which=0;which<5u;++which){
        init(&f,0xa146);memset(&f.report,0x37,sizeof f.report);memcpy(&report,&f.report,sizeof report);snapshot(&f,&s);
        if(which==0){f.backend.scalar.context=&f.gateway;f.backend.scalar.context_bytes=sizeof f.gateway;}
        if(which==1){f.guard.context=&f.memory;f.guard.context_bytes=sizeof f.memory;}
        if(which==2)f.backend.write_bytes=0;
        if(which==3)f.guard.check=0;
        if(which==4){f.backend.scalar.context=&f.frame;f.backend.scalar.context_bytes=sizeof f.frame;}
        REQUIRE(step(&f)==CS_M68K_ARGUMENT);REQUIRE(!memcmp(&report,&f.report,sizeof report));unchanged(&f,&s);
    }
    init(&f,0xa146);snapshot(&f,&s);memcpy(&report,&f.report,sizeof report);
    memcpy(misaligned.bytes+1,&f.backend,sizeof f.backend);
    REQUIRE(cs_s1_banked_step((const cs_m68k_transfer_backend *)(const void *)(misaligned.bytes+1),
        &f.gateway,&f.frame,&f.report,&f.limits,&f.guard)==CS_M68K_ARGUMENT);
    REQUIRE(!memcmp(&report,&f.report,sizeof report));unchanged(&f,&s);
    init(&f,0x7011);REQUIRE(step(&f)==CS_M68K_OK);REQUIRE(f.frame.pc==0x1002 && f.frame.d[0]==17);
    init(&f,0x4afc);snapshot(&f,&s);REQUIRE(step(&f)==CS_M68K_OPCODE);unchanged(&f,&s);
    init(&f,0xa146);f.memory.base=0xffff00;f.frame.pc=0xfffffc;f.frame.a[7]=0x1000000;
    f.limits.base=0xffff80;f.limits.size=0x80;f.ram[252]=0xa1;f.ram[253]=0x46;
    REQUIRE(step(&f)==CS_M68K_OK);REQUIRE(f.frame.pc==0xfffffe);
    init(&f,0xa146);f.memory.base=0x300;f.frame.pc=0x300;f.frame.a[7]=0x400;
    f.limits.base=0x380;f.limits.size=0x80;snapshot(&f,&s);
    REQUIRE(step(&f)==CS_M68K_ADDRESS);unchanged(&f,&s);
}
int main(void)
{successes();failures();envelopes_and_delegation();printf("%u banked service checks passed\n",checks);return 0;}
