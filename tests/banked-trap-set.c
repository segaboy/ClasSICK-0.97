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
    if(q->deny==6u && e->kind==CS_M68K_GUARD_TARGET)return 0;
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
{return cs_s1_banked_trap_step(&f->backend,&f->gateway,&f->frame,&f->report,&f->limits,&f->guard);}
static void expect_set(fixture *f,const saved *s)
{
    saved expected=*s;uint16_t index=(uint16_t)s->frame.d[0];
    expected.gateway.entries[index]=s->frame.a[0];expected.frame.pc+=2;
    expected.frame.sr=(uint16_t)((s->frame.sr&0xfff0u)|(index==0?4u:0u));
    expected.ram[250]=(uint8_t)(expected.frame.sr>>8);expected.ram[251]=(uint8_t)expected.frame.sr;
    expected.ram[252]=(uint8_t)(expected.frame.pc>>24);expected.ram[253]=(uint8_t)(expected.frame.pc>>16);
    expected.ram[254]=(uint8_t)(expected.frame.pc>>8);expected.ram[255]=(uint8_t)expected.frame.pc;
    REQUIRE(step(f)==CS_M68K_OK);unchanged(f,&expected);
    REQUIRE(f->report.cpu.service_called==1 && f->report.cpu.service.status==CS_TRAP_OK);
    REQUIRE(f->report.cpu.service.target==s->gateway.config.set_token);
}
static void successes(void)
{
    fixture f;saved s;
    for(unsigned index=0;index<512u;++index)for(unsigned ccr=0;ccr<32u;++ccr){
        init(&f,0xa047);f.frame.d[0]=0xdead0000u|index;f.frame.a[0]=0x400800u+index*2u;
        f.frame.sr=(uint16_t)(0x2700u|ccr);snapshot(&f,&s);expect_set(&f,&s);
    }
    const uint32_t addresses[]={0x1000,0x10fe,0x400000,0x40fffe};
    for(unsigned i=0;i<sizeof addresses/sizeof addresses[0];++i){
        init(&f,0xa047);f.frame.d[0]=42;f.frame.a[0]=addresses[i];snapshot(&f,&s);expect_set(&f,&s);
    }
    init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=0x401234;f.gateway.entries[23]=0xffffffffu;
    snapshot(&f,&s);expect_set(&f,&s); /* Set can repair an invalid old destination. */
    f.frame.pc=0x1000;f.ram[0]=0xa1;f.ram[1]=0x46;f.frame.a[0]=0;
    REQUIRE(step(&f)==CS_M68K_OK && f.frame.a[0]==0x401234);
    /* Self-patch affects the next call. Trusted native restoration is explicit. */
    init(&f,0xa047);f.frame.d[0]=CS_TRAP_SET;f.frame.a[0]=0x401234;
    snapshot(&f,&s);expect_set(&f,&s);f.frame.pc=0x1000;f.frame.d[0]=23;snapshot(&f,&s);
    REQUIRE(step(&f)==CS_M68K_POLICY);REQUIRE(!f.report.cpu.service_called);unchanged(&f,&s);
    REQUIRE(cs_trap_set(&f.gateway,CS_TRAP_SET,f.gateway.config.set_token)==CS_TRAP_OK);
    snapshot(&f,&s);expect_set(&f,&s);
    /* The still-admitted Set slot repairs a guest-remapped Get slot. */
    init(&f,0xa047);f.gateway.entries[CS_TRAP_GET]=0x1000;f.frame.d[0]=CS_TRAP_GET;
    f.frame.a[0]=f.gateway.config.get_token;snapshot(&f,&s);expect_set(&f,&s);
    f.frame.pc=0x1000;f.ram[0]=0xa1;f.ram[1]=0x46;
    REQUIRE(step(&f)==CS_M68K_OK && f.frame.a[0]==f.gateway.config.get_token);
}
static void pure_validation(void)
{
    fixture f;cs_trap_gateway before,applied;
    const uint32_t addresses[]={0,1,0x1000,0x10fe,0x10ff,0x1100,0x3ffffe,0x400000,
        0x40fffe,0x410000,0x1000000,0xffffffffu};
    init(&f,0xa047);f.gateway.entries[42]=1;before=f.gateway;
    REQUIRE(cs_trap_check_set(NULL,0,0)==CS_TRAP_ARGUMENT);
    for(unsigned index=0;index<65536u;++index){
        uint32_t address=addresses[index%(sizeof addresses/sizeof addresses[0])];
        applied=before;cs_trap_result check=cs_trap_check_set(&f.gateway,(uint16_t)index,address);
        REQUIRE(check==cs_trap_set(&applied,(uint16_t)index,address));
        REQUIRE(!memcmp(&f.gateway,&before,sizeof before));
        if(check==CS_TRAP_OK){cs_trap_gateway expected=before;expected.entries[index]=address;
            REQUIRE(!memcmp(&applied,&expected,sizeof applied));}
        else REQUIRE(!memcmp(&applied,&before,sizeof applied));
    }
    REQUIRE(cs_trap_check_set(&f.gateway,42,0x1000)==CS_TRAP_OK);
    f.gateway.magic=0;before=f.gateway;
    REQUIRE(cs_trap_check_set(&f.gateway,65535,1)==CS_TRAP_STATE);
    REQUIRE(!memcmp(&f.gateway,&before,sizeof before));
    init(&f,0xa047);f.gateway.config.set_token=f.gateway.config.get_token;before=f.gateway;
    REQUIRE(cs_trap_check_set(&f.gateway,0,0x1000)==CS_TRAP_STATE);
    REQUIRE(!memcmp(&f.gateway,&before,sizeof before));
}
static void failures(void)
{
    fixture f;saved s;
    for(unsigned index=512;index<65536u;++index){
        init(&f,0xa047);f.frame.d[0]=index;f.frame.a[0]=0x1000;snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_SERVICE);REQUIRE(f.report.cpu.service.status==CS_TRAP_NUMBER);unchanged(&f,&s);
    }
    const uint32_t addresses[]={0,1,0xffe,0x1001,0x10ff,0x1100,0x3ffffe,0x400001,0x410000,0x1000000,0xffffffffu};
    for(unsigned i=0;i<sizeof addresses/sizeof addresses[0];++i){
        init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=addresses[i];snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_SERVICE);REQUIRE(f.report.cpu.service.status==CS_TRAP_ADDRESS);unchanged(&f,&s);
    }
    for(unsigned deny=1;deny<=5u;++deny){
        init(&f,deny==2?0x4e71:0xa047);f.frame.d[0]=23;f.frame.a[0]=0x1000;f.policy.deny=deny;
        snapshot(&f,&s);REQUIRE(step(&f)==CS_M68K_POLICY);unchanged(&f,&s);
    }
    init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=0x401234;f.memory.fault=0x10ff;snapshot(&f,&s);
    REQUIRE(step(&f)==CS_M68K_ADDRESS);REQUIRE(f.report.memory==CS_GUEST_PERMISSION && f.report.cpu.service_called==1);
    unchanged(&f,&s);f.memory.fault=UINT32_MAX;expect_set(&f,&s);
    const uint32_t remaps[]={0x400000,0x400020,0x1000,1};
    for(unsigned i=0;i<sizeof remaps/sizeof remaps[0];++i){
        init(&f,0xa047);f.gateway.entries[CS_TRAP_SET]=remaps[i];f.frame.d[0]=23;f.frame.a[0]=0x1000;
        snapshot(&f,&s);REQUIRE(step(&f)==(i==3?CS_M68K_SERVICE:CS_M68K_POLICY));
        REQUIRE(!f.report.cpu.service_called);unchanged(&f,&s);
    }
    const uint16_t words[]={0xa147,0xa247,0xa347,0xa647,0xa847,0xafff};
    for(unsigned i=0;i<sizeof words/sizeof words[0];++i){
        init(&f,words[i]);f.frame.d[0]=23;f.frame.a[0]=0x1000;snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_POLICY);REQUIRE(!f.report.cpu.service_called);unchanged(&f,&s);
    }
    for(unsigned which=0;which<10u;++which){
        init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=0x1000;
        if(which==0)f.frame.pc=0x1001;if(which==1)f.frame.pc=0xfffffe;
        if(which==2)f.frame.sr=0x2000;if(which==3)f.frame.a[7]=0x10ff;
        if(which==4)f.frame.a[7]=f.limits.base+4u;if(which==5)f.memory.invalid=1;
        if(which==6)f.limits.size=0;if(which==7)f.gateway.magic=0;
        if(which==8)f.memory.readonly=1;if(which==9)f.gateway.config.ram_end=0;
        snapshot(&f,&s);REQUIRE(step(&f)!=CS_M68K_OK);unchanged(&f,&s);
    }
    init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=0x1000;snapshot(&f,&s);
    REQUIRE(cs_s1_banked_step(&f.backend,&f.gateway,&f.frame,&f.report,&f.limits,&f.guard)==CS_M68K_POLICY);
    unchanged(&f,&s);
    REQUIRE(cs_m68k_bus_step_transfer(&f.backend,&f.frame,&f.report,&f.limits,&f.guard)==CS_M68K_POLICY);
    unchanged(&f,&s);
}
static void envelopes(void)
{
    fixture f;saved s;cs_m68k_bus_report report;
    union {cs_m68k_transfer_backend align;uint8_t bytes[sizeof(cs_m68k_transfer_backend)+1u];} unaligned;
    for(unsigned which=0;which<5u;++which){
        init(&f,0xa047);f.frame.d[0]=23;f.frame.a[0]=0x1000;
        if(which==0){f.backend.scalar.context=&f.gateway;f.backend.scalar.context_bytes=sizeof f.gateway;}
        if(which==1){f.guard.context=&f.gateway;f.guard.context_bytes=sizeof f.gateway;}
        if(which==2)f.backend.write_bytes=0;
        if(which==3){f.backend.scalar.context=&f.frame;f.backend.scalar.context_bytes=sizeof f.frame;}
        if(which==4)f.guard.check=0;
        memset(&f.report,0x37,sizeof f.report);memcpy(&report,&f.report,sizeof report);snapshot(&f,&s);
        REQUIRE(step(&f)==CS_M68K_ARGUMENT);REQUIRE(!memcmp(&report,&f.report,sizeof report));unchanged(&f,&s);
    }
    init(&f,0xa047);snapshot(&f,&s);memcpy(&report,&f.report,sizeof report);
    memcpy(unaligned.bytes+1,&f.backend,sizeof f.backend);
    REQUIRE(cs_s1_banked_trap_step((const cs_m68k_transfer_backend *)(const void *)(unaligned.bytes+1),
        &f.gateway,&f.frame,&f.report,&f.limits,&f.guard)==CS_M68K_ARGUMENT);
    REQUIRE(!memcmp(&report,&f.report,sizeof report));unchanged(&f,&s);
}
static cs_m68k_result patched(fixture *f,cs_s1_patch_state *state)
{
    return cs_s1_banked_patch_step(&f->backend,&f->gateway,&f->frame,&f->report,&f->limits,&f->guard,state);
}
static void patch_dispatch(void)
{
    fixture f;
    saved before;
    cs_s1_patch_state state,previous;
    memset(&state,0,sizeof state);
    init(&f,0xa017);
    f.gateway.entries[23]=0x1040;
    f.gateway.entries[24]=0x1060;
    f.ram[0x40]=0xa1;f.ram[0x41]=0x18; /* inner call returns A0 */
    f.ram[0x42]=0x4e;f.ram[0x43]=0x75;
    f.ram[0x60]=0x4e;f.ram[0x61]=0x75;
    uint32_t original_a0=f.frame.a[0],original_a1=f.frame.a[1];
    uint32_t original_d1=f.frame.d[1],original_d2=f.frame.d[2];
    snapshot(&f,&before);previous=state;
    f.memory.fault=0x10ff;
    REQUIRE(patched(&f,&state)==CS_M68K_ADDRESS);
    unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    f.memory.fault=UINT32_MAX;
    for(unsigned deny=3;deny<=6u;++deny){
        f.policy.deny=deny;REQUIRE(patched(&f,&state)==CS_M68K_POLICY);
        unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    }
    f.policy.deny=0;
    REQUIRE(patched(&f,&state)==CS_M68K_OK && f.frame.pc==0x1040 && state.depth==1);
    REQUIRE(f.frame.a[7]==0x10fc && f.ram[254]==0x10 && f.ram[255]==0x02);
    REQUIRE(patched(&f,&state)==CS_M68K_OK && f.frame.pc==0x1060 && state.depth==2);
    REQUIRE(f.frame.a[7]==0x10f8 && f.ram[250]==0x10 && f.ram[251]==0x42);
    f.frame.d[0]=0x12340000;f.frame.d[1]=1;f.frame.d[2]=2;
    f.frame.a[0]=0x5555;f.frame.a[1]=0xaaaa;
    f.ram[251]=0x46;previous=state;snapshot(&f,&before);
    REQUIRE(patched(&f,&state)==CS_M68K_POLICY);
    unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    f.ram[251]=0x42;f.policy.deny=5;snapshot(&f,&before);
    REQUIRE(patched(&f,&state)==CS_M68K_POLICY);
    unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    f.policy.deny=0;
    REQUIRE(patched(&f,&state)==CS_M68K_OK && state.depth==1 && f.frame.pc==0x1042);
    REQUIRE(f.frame.sr==0x2714 && f.frame.a[0]==0x5555 && f.frame.a[7]==0x10fc);
    REQUIRE(f.frame.d[1]==original_d1 && f.frame.d[2]==original_d2 && f.frame.a[1]==original_a1);
    REQUIRE(patched(&f,&state)==CS_M68K_OK && state.depth==0 && f.frame.pc==0x1002);
    REQUIRE(f.frame.a[0]==original_a0 && f.frame.a[7]==0x1100);
    /* Recursing by the same trap word hits the native stack bound. */
    init(&f,0xa017);f.gateway.entries[23]=0x1000;
    for(unsigned i=0;i<CS_S1_PATCH_DEPTH;++i)REQUIRE(patched(&f,&state)==CS_M68K_OK);
    previous=state;snapshot(&f,&before);REQUIRE(patched(&f,&state)==CS_M68K_POLICY);
    unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    /* State is an admitted native control; it cannot double as a frame. */
    cs_m68k_bus_report saved_report=f.report;
    REQUIRE(patched(&f,(cs_s1_patch_state *)(void *)&f.frame)==CS_M68K_ARGUMENT);
    REQUIRE(memcmp(&saved_report,&f.report,sizeof saved_report)==0);
    memset(&state,0,sizeof state);
    init(&f,0xa146);f.gateway.entries[CS_TRAP_GET]=0x1040;
    REQUIRE(patched(&f,&state)==CS_M68K_OK && f.frame.pc==0x1040 && state.depth==1);
    state.calls[0].slot=UINT32_MAX-1;snapshot(&f,&before);previous=state;
    REQUIRE(patched(&f,&state)==CS_M68K_POLICY);
    unchanged(&f,&before);REQUIRE(memcmp(&state,&previous,sizeof state)==0);
    memset(&state,0,sizeof state);
    for(unsigned i=0;i<4u;++i){
        const uint16_t rejected[]={0xa217,0xa417,0xa817,0xac17};
        init(&f,rejected[i]);f.gateway.entries[23]=0x1040;snapshot(&f,&before);
        REQUIRE(patched(&f,&state)==CS_M68K_POLICY && state.depth==0);unchanged(&f,&before);
    }
    init(&f,0xa017);f.gateway.entries[23]=0x10fe;snapshot(&f,&before);
    REQUIRE(patched(&f,&state)==CS_M68K_POLICY && state.depth==0);unchanged(&f,&before);
}
/* Only the standalone startup profile binds this token. Stopping at the
 * callback deliberately checks routing independently of allocation results. */
static unsigned newptr_calls;
static cs_m68k_result observe_newptr(const cs_trap_frame *before,cs_trap_frame *next,
    cs_m68k_bus_report *report,uint32_t scratch,const cs_m68k_guard *guard)
{
    REQUIRE(before!=next && guard!=NULL && scratch==before->a[7]-6);
    REQUIRE(report->cpu.service.target==CS_S1_STARTUP_NEW_PTR_TOKEN);
    ++newptr_calls;
    return CS_M68K_POLICY;
}
static cs_m68k_result startup(fixture *f,cs_s1_patch_state *state)
{
    return cs_s1_banked_startup_step(&f->backend,&f->gateway,&f->frame,&f->report,
                                      &f->limits,&f->guard,state,observe_newptr);
}
static void newptr_routing(void)
{
    fixture f;
    cs_s1_patch_state state={0};
    saved before;
    for(unsigned flags=0;flags<4;++flags){
        init(&f,(uint16_t)(0xa11eu|(flags<<9)));
        f.gateway.entries[0x1e]=CS_S1_STARTUP_NEW_PTR_TOKEN;
        snapshot(&f,&before);newptr_calls=0;
        REQUIRE(patched(&f,&state)==CS_M68K_POLICY && newptr_calls==0);
        unchanged(&f,&before);
        REQUIRE(startup(&f,&state)==CS_M68K_POLICY && newptr_calls==1);
        unchanged(&f,&before);
        f.policy.deny=4;
        REQUIRE(startup(&f,&state)==CS_M68K_POLICY && newptr_calls==1);
        unchanged(&f,&before);
        f.policy.deny=0;
        f.gateway.entries[0x1e]=f.gateway.config.unsupported_token;
        REQUIRE(startup(&f,&state)==CS_M68K_POLICY && newptr_calls==1);
        f.gateway.entries[0x1e]=0x1040;
        f.ram[0x40]=0x4e;f.ram[0x41]=0x75;
        REQUIRE(startup(&f,&state)==CS_M68K_OK && state.depth==1 && f.frame.pc==0x1040);
        f.frame.a[0]=0x1234;
        REQUIRE(startup(&f,&state)==CS_M68K_OK && state.depth==0 && f.frame.pc==0x1002);
        REQUIRE(f.frame.a[0]==0x1234 && newptr_calls==1);
    }
}
int main(void)
{
    successes();pure_validation();failures();envelopes();patch_dispatch();newptr_routing();
    printf("banked SetTrapAddress: %u checks\n",checks);return 0;
}
