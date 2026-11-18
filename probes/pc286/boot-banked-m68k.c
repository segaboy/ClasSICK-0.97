/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original 68000 instruction words and native 286 ownership fixture. */
#include "../../platform/pc286/m68k-banked.h"

static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(uint8_t code) { post(code); for (;;) { } }
static void zero(void *p,unsigned bytes)
{ volatile uint8_t *d=p;unsigned i;for(i=0;i<bytes;++i)d[i]=0; }
static void copy(void *p,const void *q,unsigned bytes)
{ volatile uint8_t *d=p;const volatile uint8_t *s=q;unsigned i;for(i=0;i<bytes;++i)d[i]=s[i]; }
static int equal(const void *p,const void *q,unsigned bytes)
{ const volatile uint8_t *a=p,*b=q;unsigned i;for(i=0;i<bytes;++i)if(a[i]!=b[i])return 0;return 1; }
static volatile uint8_t __far *far_byte(uint32_t encoded)
{return (volatile uint8_t __far *)encoded;}

typedef struct {
    cs_pc286_m68k_memory memory;
    cs_m68k_bus_backend native;
    unsigned refuse_write;
} owner;
typedef struct {unsigned deny;cs_m68k_guard_kind kind;} policy;
static int check(const void *context,const cs_m68k_guard_event *event)
{const policy *p=context;return !p->deny || p->kind!=event->kind;}
static cs_guest_result validate(const void *context)
{const owner *o=context;return o->native.validate(o->native.context);}
static cs_guest_result span(const void *context,uint32_t at,uint32_t bytes,uint32_t access)
{
    const owner *o=context;
    if(o->refuse_write==2u && access==CS_GUEST_WRITE && bytes<=4UL)return CS_GUEST_PERMISSION;
    return o->native.span(o->native.context,at,bytes,access);
}
static cs_guest_result read_value(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{const owner *o=context;return o->native.read(o->native.context,at,width,access,out);}
static cs_guest_result write_value(const void *context,uint32_t at,unsigned width,uint32_t value)
{
    const owner *o=context;
    if(o->refuse_write==1u)return CS_GUEST_PERMISSION;
    return o->native.write(o->native.context,at,width,value);
}
static void put(owner *o,uint32_t at,unsigned width,uint32_t value)
{
    if(cs_pc286_guest_write(&o->memory.guest,o->memory.generation,at,width,value)!=CS_PG_OK)fail(0xd1);
}
static uint32_t get(owner *o,uint32_t at)
{
    uint32_t value=0;
    if(cs_pc286_guest_read(&o->memory.guest,o->memory.generation,at,4,1,&value)!=CS_PG_OK)fail(0xd2);
    return value;
}
static void seed_frame(cs_trap_frame *f)
{
    zero(f,sizeof *f);f->pc=0x8ffeUL;f->sr=0x2700;
    f->a[0]=0x10ffeUL;f->a[6]=0x12345678UL;f->a[7]=0x19002UL;
}
static int exact_data(void)
{
    return *far_byte(0x31007ffeUL)==0x11 && *far_byte(0x41000000UL)==0x22 &&
        *far_byte(0x42000000UL)==0x33 && *far_byte(0x42000001UL)==0x44 &&
        *far_byte(0x31007ffdUL)==0x5a && *far_byte(0x31007fffUL)==0xa5 &&
        *far_byte(0x41000001UL)==0x69 && *far_byte(0x42000002UL)==0x96;
}
void start(void)
{
    owner o;policy p;cs_m68k_bus_backend bus,bad;
    cs_pg_region region;cs_pg_bank banks[8];
    cs_m68k_guard guard;cs_m68k_stack_limits limits;
    cs_trap_frame frame,before;cs_m68k_bus_report report,saved_report;
    unsigned i;uint32_t start;
    zero(&o,sizeof o);zero(&report,sizeof report);
    region.base=0x1000UL;region.bytes=0x20000UL;region.flags=7;
    region.first_bank=0;region.bank_count=8;
    for(i=0;i<8u;++i){
        banks[i].id=(uint16_t)(i+1u);banks[i].offset=0;
        banks[i].segment=(uint16_t)(0x2000u+(i/2u)*0x1100u+((i&1u)?0x1000u:0u));
        banks[i].bytes=(i&1u)?1u:32767u;
    }
    if(cs_pc286_guest_bind(&o.memory.guest,&region,1,banks,8,0x20000UL,0x50000UL,
            &o.memory.generation)!=CS_PG_OK || cs_pc286_m68k_backend(&o.native,&o.memory)!=CS_PG_OK)fail(0xd0);
    if(cs_pc286_guest_validate(&o.memory.guest,o.memory.generation)!=CS_PG_OK ||
        cs_pc286_guest_span(&o.memory.guest,o.memory.generation,0x1000UL,0x20000UL,7)!=CS_PG_OK ||
        cs_pc286_guest_span(&o.memory.guest,o.memory.generation,0x20fffUL,1,1)!=CS_PG_OK ||
        cs_pc286_guest_span(&o.memory.guest,o.memory.generation,0x1000UL,0x20001UL,1)!=CS_PG_ADDRESS ||
        cs_pc286_guest_span(&o.memory.guest,o.memory.generation,0x1000UL,4,0x10001UL)!=CS_PG_ARGUMENT)
        fail(0xc4);
    bus.context=&o;bus.context_bytes=sizeof o;bus.validate=validate;bus.span=span;
    bus.read=read_value;bus.write=write_value;
    p.deny=0;p.kind=CS_M68K_GUARD_COMMIT;guard.check=check;guard.context=&p;guard.context_bytes=sizeof p;
    limits.base=0x18000UL;limits.size=0x2000UL;
    /* A split fetch, absolute jump above 64K, split data and split stack. */
    put(&o,0x8ffeUL,2,0x7011UL);put(&o,0x9000UL,2,0x4ef9UL);put(&o,0x9002UL,4,0x19020UL);
    put(&o,0x9006UL,2,0x4e71UL);
    put(&o,0x19020UL,2,0x20bcUL);put(&o,0x19022UL,4,0x11223344UL);
    put(&o,0x19026UL,2,0x2210UL);put(&o,0x19028UL,2,0x4850UL);put(&o,0x1902aUL,2,0x2417UL);
    put(&o,0x1902cUL,2,0x4e56UL);put(&o,0x1902eUL,2,0xfffcUL);put(&o,0x19030UL,2,0x4e5eUL);
    put(&o,0x19032UL,2,0x2ebcUL);put(&o,0x19034UL,4,0x9006UL);put(&o,0x19038UL,2,0x4e75UL);
    put(&o,0x19040UL,2,0x4e56UL);put(&o,0x19042UL,2,0xfffcUL);put(&o,0x19044UL,2,0xa146UL);
    *far_byte(0x31007ffdUL)=0x5a;*far_byte(0x31007fffUL)=0xa5;
    *far_byte(0x41000001UL)=0x69;*far_byte(0x42000002UL)=0x96;
    seed_frame(&frame);
    for(i=0;i<11u;++i){
        if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_OK)fail(0xd3);
        if(i==0u && (frame.pc!=0x9000UL || frame.d[0]!=17UL))fail(0xd4);
        if(i==1u && frame.pc!=0x19020UL)fail(0xd5);
        if(i==2u && !exact_data())fail(0xd6);
        if(i==4u && (frame.a[7]!=0x18ffeUL || get(&o,0x18ffeUL)!=0x10ffeUL))fail(0xd7);
    }
    if(frame.pc!=0x9008UL || frame.d[0]!=17UL || frame.d[1]!=0x11223344UL ||
        frame.d[2]!=0x10ffeUL || frame.a[6]!=0x12345678UL || frame.a[7]!=0x19002UL ||
        !exact_data() || get(&o,0x18ffeUL)!=0x9006UL)fail(0xd8);
    /* A successful preflight must not publish before COMMIT permission. */
    put(&o,0x10ffeUL,4,0x55667788UL);seed_frame(&frame);frame.pc=0x19020UL;
    copy(&before,&frame,sizeof frame);p.deny=1;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        !equal(&before,&frame,sizeof frame) || get(&o,0x10ffeUL)!=0x55667788UL)fail(0xe1);
    p.deny=0;o.refuse_write=1;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_PERMISSION || report.address!=0x10ffeUL || report.bytes!=4u ||
        report.access!=CS_GUEST_WRITE || !equal(&before,&frame,sizeof frame) ||
        get(&o,0x10ffeUL)!=0x55667788UL)fail(0xe2);
    o.refuse_write=2;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_PERMISSION || !equal(&before,&frame,sizeof frame) ||
        get(&o,0x10ffeUL)!=0x55667788UL)fail(0xd9);
    o.refuse_write=0;
    /* LINK preflights a valid push but its final SP is below the bound. */
    frame.pc=0x19040UL;copy(&before,&frame,sizeof frame);limits.base=0x18ffeUL;limits.size=4;
    start=get(&o,0x18ffeUL);
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        !equal(&before,&frame,sizeof frame) || get(&o,0x18ffeUL)!=start)fail(0xe4);
    limits.base=0x18000UL;limits.size=0x2000UL;
    frame.pc=0x19044UL;copy(&before,&frame,sizeof frame);
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_POLICY ||
        report.cpu.service_called!=0 || !equal(&before,&frame,sizeof frame) ||
        get(&o,0x18ffeUL)!=start)fail(0xe3);
    /* Native envelopes refuse before even publishing a new report. */
    copy(&saved_report,&report,sizeof report);copy(&bad,&bus,sizeof bus);bad.context=&frame;
    if(cs_m68k_bus_step_backend(&bad,&frame,&report,&limits,&guard)!=CS_M68K_ARGUMENT ||
        !equal(&saved_report,&report,sizeof report) || !equal(&before,&frame,sizeof frame))fail(0xda);
    guard.context=&o;guard.context_bytes=sizeof o;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ARGUMENT ||
        !equal(&saved_report,&report,sizeof report))fail(0xdb);
    guard.context=&p;guard.context_bytes=sizeof p;copy(&bad,&bus,sizeof bus);bad.write=0;
    if(cs_m68k_bus_step_backend(&bad,&frame,&report,&limits,&guard)!=CS_M68K_ARGUMENT ||
        !equal(&saved_report,&report,sizeof report))fail(0xdc);
    /* Lifetime and whole-arena faults are refused before opcode fetch. */
    --o.memory.generation;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_MAP || report.cpu.word!=0 || !equal(&before,&frame,sizeof frame))fail(0xdd);
    ++o.memory.generation;o.memory.guest.arena_first=0x10000UL;o.memory.guest.arena_bytes=0x60000UL;
    if(cs_m68k_bus_step_backend(&bus,&frame,&report,&limits,&guard)!=CS_M68K_ADDRESS ||
        report.memory!=CS_GUEST_MAP || report.cpu.word!=0 || !equal(&before,&frame,sizeof frame))fail(0xde);
    o.memory.guest.arena_first=0x20000UL;o.memory.guest.arena_bytes=0x50000UL;
    /* Restore successful exact physical result for independent host inspection. */
    put(&o,0x10ffeUL,4,0x11223344UL);
    if(!exact_data() || get(&o,0x18ffeUL)!=0x9006UL)fail(0xdf);
    post(0xb8);for(;;) { }
}
