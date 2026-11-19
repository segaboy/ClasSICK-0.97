/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../personality/system1/banked-services.h"
#include "../../platform/pc286/m68k-transfer.h"
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(uint8_t code){post(code);for(;;) { }}
static void zero(void *p,unsigned bytes)
{volatile uint8_t *d=p;for(unsigned i=0;i<bytes;++i)d[i]=0;}
static void copy(void *p,const void *q,unsigned bytes)
{volatile uint8_t *d=p;const volatile uint8_t *s=q;for(unsigned i=0;i<bytes;++i)d[i]=s[i];}
static int equal(const void *p,const void *q,unsigned bytes)
{const volatile uint8_t *a=p,*b=q;for(unsigned i=0;i<bytes;++i)if(a[i]!=b[i])return 0;return 1;}
static volatile uint8_t __far *far_byte(uint32_t encoded)
{return (volatile uint8_t __far *)encoded;}
typedef struct {cs_pc286_m68k_memory memory;cs_m68k_transfer_backend native;uint32_t fault;} owner;
typedef struct {unsigned deny;} policy;
static cs_guest_result validate(const void *p)
{const owner *o=p;return o->native.scalar.validate(o->native.scalar.context);}
static cs_guest_result span(const void *p,uint32_t at,uint32_t bytes,uint32_t access)
{const owner *o=p;return o->native.scalar.span(o->native.scalar.context,at,bytes,access);}
static cs_guest_result read_value(const void *p,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{const owner *o=p;return o->native.scalar.read(o->native.scalar.context,at,width,access,out);}
static cs_guest_result write_value(const void *p,uint32_t at,unsigned width,uint32_t value)
{
    const owner *o=p;
    if(o->fault>=at && o->fault-at<width)return CS_GUEST_PERMISSION;
    return o->native.scalar.write(o->native.scalar.context,at,width,value);
}
static cs_guest_result write_bytes(const void *p,uint32_t at,unsigned count,const uint8_t *bytes)
{
    const owner *o=p;
    if(o->fault>=at && o->fault-at<count)return CS_GUEST_PERMISSION;
    return o->native.write_bytes(o->native.scalar.context,at,count,bytes);
}
static int check(const void *p,const cs_m68k_guard_event *e)
{
    const policy *q=p;
    if(q->deny==1u && e->kind==CS_M68K_GUARD_OPCODE && e->before==e->proposed)return 0;
    if(q->deny==2u && e->kind==CS_M68K_GUARD_OPCODE && e->before!=e->proposed)return 0;
    if(q->deny==3u && e->kind==CS_M68K_GUARD_COMMIT)return 0;
    return 1;
}
static void put(owner *o,uint32_t at,unsigned width,uint32_t value)
{
    if(cs_pc286_guest_write(&o->memory.guest,o->memory.generation,at,width,value)!=CS_PG_OK)fail(0xd1);
}
static int scratch(owner *o,uint32_t high,uint32_t low)
{
    uint32_t a=0,b=0;
    if(cs_pc286_guest_read(&o->memory.guest,o->memory.generation,0x10ffcUL,4,1,&a)!=CS_PG_OK ||
        cs_pc286_guest_read(&o->memory.guest,o->memory.generation,0x11000UL,2,1,&b)!=CS_PG_OK)fail(0xd2);
    return a==high && b==low;
}
static void seed(cs_trap_frame *f)
{
    zero(f,sizeof *f);f->pc=0x9000UL;f->sr=0x271f;f->d[0]=23;
    f->a[0]=0x55667788UL;f->a[7]=0x11002UL;
}
void start(void)
{
    owner o;policy policy;cs_m68k_transfer_backend backend,bad;
    cs_pg_region region;cs_pg_bank banks[8];
    cs_trap_gateway gateway,saved_gateway;cs_trap_config config;
    cs_trap_frame frame,before;cs_m68k_bus_report report,saved_report;
    cs_m68k_stack_limits limits;cs_m68k_guard guard;
    zero(&o,sizeof o);zero(&report,sizeof report);o.fault=0xffffffffUL;
    region.base=0x1000UL;region.bytes=0x20000UL;region.flags=7;region.first_bank=0;region.bank_count=8;
    for(unsigned i=0;i<8u;++i){
        banks[i].id=(uint16_t)(i+1u);banks[i].offset=0;
        banks[i].segment=(uint16_t)(0x2000u+(i/2u)*0x1100u+((i&1u)?0x1000u:0u));
        banks[i].bytes=(i&1u)?1u:32767u;
    }
    if(cs_pc286_guest_bind(&o.memory.guest,&region,1,banks,8,0x20000UL,0x50000UL,&o.memory.generation)!=CS_PG_OK ||
        cs_pc286_m68k_transfer_backend(&o.native,&o.memory)!=CS_PG_OK)fail(0xd0);
    backend.scalar.context=&o;backend.scalar.context_bytes=sizeof o;backend.scalar.validate=validate;
    backend.scalar.span=span;backend.scalar.read=read_value;backend.scalar.write=write_value;backend.write_bytes=write_bytes;
    config.rom_base=0x400000UL;config.ram_begin=0x1000UL;config.ram_end=0x11000UL;
    config.get_token=0x400000UL;config.set_token=0x400002UL;config.unsupported_token=0x400020UL;
    if(cs_trap_init(&gateway,&config)!=CS_TRAP_OK || cs_trap_set(&gateway,23,0x401234UL)!=CS_TRAP_OK)fail(0xd3);
    copy(&saved_gateway,&gateway,sizeof gateway);
    limits.base=0x10000UL;limits.size=0x2000UL;policy.deny=0;
    guard.check=check;guard.context=&policy;guard.context_bytes=sizeof policy;
    put(&o,0x8ffeUL,2,0x7017UL);put(&o,0x9000UL,2,0xa146UL);put(&o,0x9002UL,2,0x2208UL);
    put(&o,0x9004UL,2,0x207cUL);put(&o,0x9006UL,4,0x55667788UL);put(&o,0x900aUL,2,0xa046UL);
    put(&o,0x900cUL,2,0x2408UL);put(&o,0x900eUL,2,0x4e71UL);
    *far_byte(0x31007ffbUL)=0x5a;*far_byte(0x31007fffUL)=0xa5;
    *far_byte(0x41000001UL)=0x69;*far_byte(0x42000002UL)=0x96;
    seed(&frame);frame.pc=0x8ffeUL;
    for(unsigned i=0;i<7u;++i)
        if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_OK)fail(0xd4);
    if(frame.pc!=0x9010UL || frame.d[1]!=0x401234UL || frame.d[2]!=0x55667788UL ||
        frame.a[0]!=0x55667788UL || frame.a[7]!=0x11002UL || !scratch(&o,0x27100000UL,0x900cUL) ||
        !equal(&gateway,&saved_gateway,sizeof gateway))fail(0xd5);
    if(*far_byte(0x31007ffcUL)!=0x27 || *far_byte(0x31007ffdUL)!=0x10 ||
        *far_byte(0x31007ffeUL)!=0 || *far_byte(0x41000000UL)!=0 ||
        *far_byte(0x42000000UL)!=0x90 || *far_byte(0x42000001UL)!=0x0c ||
        *far_byte(0x31007ffbUL)!=0x5a || *far_byte(0x31007fffUL)!=0xa5 ||
        *far_byte(0x41000001UL)!=0x69 || *far_byte(0x42000002UL)!=0x96)fail(0xd6);
    /* Last-byte callback failure distinguishes early frame and split stores. */
    put(&o,0x10ffcUL,4,0x11223344UL);put(&o,0x11000UL,2,0x5566UL);
    seed(&frame);copy(&before,&frame,sizeof frame);o.fault=0x11001UL;
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_PERMISSION)fail(0xd7);
    if(!equal(&frame,&before,sizeof frame) || !equal(&gateway,&saved_gateway,sizeof gateway))fail(0xe1);
    if(!scratch(&o,0x11223344UL,0x5566UL))fail(0xe2);
    o.fault=0xffffffffUL;policy.deny=3;
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        !equal(&frame,&before,sizeof frame) || !scratch(&o,0x11223344UL,0x5566UL))fail(0xd8);
    policy.deny=0;gateway.entries[CS_TRAP_GET]=config.set_token;
    copy(&saved_gateway,&gateway,sizeof gateway);
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        report.cpu.service_called || !equal(&frame,&before,sizeof frame) ||
        !equal(&gateway,&saved_gateway,sizeof gateway) || !scratch(&o,0x11223344UL,0x5566UL))fail(0xe3);
    gateway.entries[CS_TRAP_GET]=config.get_token;copy(&saved_gateway,&gateway,sizeof gateway);
    for(unsigned which=1;which<=2u;++which){
        policy.deny=which;frame.pc=0x900eUL;copy(&before,&frame,sizeof frame);
        if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
            !equal(&frame,&before,sizeof frame) || !scratch(&o,0x11223344UL,0x5566UL))fail(0xd9);
    }
    policy.deny=0;seed(&frame);copy(&before,&frame,sizeof frame);
    if(cs_m68k_bus_step_transfer(&backend,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        !equal(&frame,&before,sizeof frame) || !scratch(&o,0x11223344UL,0x5566UL))fail(0xda);
    copy(&saved_report,&report,sizeof report);copy(&bad,&backend,sizeof backend);
    bad.scalar.context=&gateway;bad.scalar.context_bytes=sizeof gateway;
    if(cs_s1_banked_step(&bad,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_ARGUMENT ||
        !equal(&report,&saved_report,sizeof report) || !equal(&frame,&before,sizeof frame))fail(0xdb);
    frame.pc=0xfffffeUL;copy(&before,&frame,sizeof frame);
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        !equal(&frame,&before,sizeof frame))fail(0xdc);
    seed(&frame);frame.d[0]=512;copy(&before,&frame,sizeof frame);
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_SERVICE ||
        report.cpu.service.status!=CS_TRAP_NUMBER || !equal(&frame,&before,sizeof frame) ||
        !equal(&gateway,&saved_gateway,sizeof gateway) || !scratch(&o,0x11223344UL,0x5566UL))fail(0xdd);
    seed(&frame);
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_OK ||
        frame.a[0]!=0x401234UL || !scratch(&o,0x27100000UL,0x9002UL))fail(0xde);
    /* Install an independently authored RAM routine, look it up, call it
     * with JSR (A0), and return to the caller on the split guest stack. */
    put(&o,0x9200,2,0x7017); /* MOVEQ #23,D0 */
    put(&o,0x9202,2,0x207c);put(&o,0x9204,4,0x9340);
    put(&o,0x9208,2,0xa047);
    put(&o,0x920a,2,0x207c);put(&o,0x920c,4,0);
    put(&o,0x9210,2,0xa146);
    put(&o,0x9212,2,0x4e90);put(&o,0x9214,2,0x4e71);
    put(&o,0x9340,2,0x762a);put(&o,0x9342,2,0x4e75); /* MOVEQ #42,D3; RTS */
    seed(&frame);frame.pc=0x9200;
    for(unsigned i=0;i<2u;++i)
        if(cs_s1_banked_trap_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_OK)fail(0xc0);
    if(frame.pc!=0x9208 || frame.a[0]!=0x9340 || frame.d[0]!=23)fail(0xc1);
    copy(&before,&frame,sizeof frame);
    put(&o,0x10ffcUL,4,0x11223344UL);put(&o,0x11000UL,2,0x5566);
    if(cs_s1_banked_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_POLICY)fail(0xc2);
    o.fault=0x11001UL;
    if(cs_s1_banked_trap_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_PERMISSION || !report.cpu.service_called)fail(0xc3);
    if(!equal(&gateway,&saved_gateway,sizeof gateway))fail(0xe4);
    if(!equal(&frame,&before,sizeof frame))fail(0xe5);
    if(!scratch(&o,0x11223344UL,0x5566))fail(0xe6);
    o.fault=0xffffffffUL;
    gateway.entries[CS_TRAP_SET]=config.get_token;
    if(cs_s1_banked_trap_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        report.cpu.service_called || !equal(&frame,&before,sizeof frame))fail(0xe7);
    gateway.entries[CS_TRAP_SET]=config.set_token;
    if(!equal(&gateway,&saved_gateway,sizeof gateway) || !scratch(&o,0x11223344UL,0x5566))fail(0xc4);
    if(cs_s1_banked_trap_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_OK ||
        !scratch(&o,0x27100000UL,0x920a))fail(0xc5);
    saved_gateway.entries[23]=0x9340;
    if(!equal(&gateway,&saved_gateway,sizeof gateway))fail(0xc6);
    for(unsigned i=0;i<6u;++i)
        if(cs_s1_banked_trap_step(&backend,&gateway,&frame,&report,&limits,&guard)!=CS_M68K_OK)fail(0xc7);
    if(frame.pc!=0x9216 || frame.a[7]!=0x11002UL || frame.a[0]!=0x9340 ||
        frame.d[3]!=42 || !scratch(&o,0x27100000UL,0x9214) ||
        !equal(&gateway,&saved_gateway,sizeof gateway))fail(0xc8);
    if(*far_byte(0x31007ffbUL)!=0x5a || *far_byte(0x31007fffUL)!=0xa5 ||
        *far_byte(0x41000001UL)!=0x69 || *far_byte(0x42000002UL)!=0x96)fail(0xc9);
    post(0xb8);for(;;) { }
}
