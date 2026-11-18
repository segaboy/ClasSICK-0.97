/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/pc286/m68k-transfer.h"
#include "../cpu/m68k-transfer.h"
static void post(uint8_t value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void stop(unsigned value){post((uint8_t)value);for(;;) { }}
static void fill(void *object,unsigned n,uint8_t v){uint8_t *p=object;unsigned i;for(i=0;i<n;++i)p[i]=v;}
static int equal(const void *p,const void *q,unsigned n){const uint8_t *a=p,*b=q;unsigned i;for(i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static void copy(void *p,const void *q,unsigned n){uint8_t *d=p;const uint8_t *s=q;unsigned i;for(i=0;i<n;++i)d[i]=s[i];}
typedef struct {cs_pc286_m68k_memory memory;cs_m68k_transfer_backend native;unsigned refuse;uint32_t fail_read;} owner;
typedef struct {unsigned deny,bytes;cs_m68k_guard_kind kind;} policy;
static owner backing;
static policy rules;
static cs_m68k_transfer_backend bus;
static cs_m68k_guard guard;
static cs_m68k_stack_limits limits;
static cs_trap_frame frame,before;
static cs_m68k_bus_report report;
static uint8_t bytes[64];
static int allow(const void *context,const cs_m68k_guard_event *event)
{const policy *p=context;return !p->deny || p->kind!=event->kind || p->bytes!=event->bytes;}
static cs_guest_result valid(const void *context)
{const owner *o=context;return o->native.scalar.validate(o->native.scalar.context);}
static cs_guest_result span(const void *context,uint32_t at,uint32_t n,uint32_t access)
{const owner *o=context;return o->native.scalar.span(o->native.scalar.context,at,n,access);}
static cs_guest_result read_scalar(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out)
{const owner *o=context;if(at==o->fail_read)return CS_GUEST_PERMISSION;return o->native.scalar.read(o->native.scalar.context,at,width,access,out);}
static cs_guest_result write_scalar(const void *context,uint32_t at,unsigned width,uint32_t value)
{const owner *o=context;return o->native.scalar.write(o->native.scalar.context,at,width,value);}
static cs_guest_result write_bytes(const void *context,uint32_t at,unsigned n,const uint8_t *source)
{const owner *o=context;if(o->refuse)return CS_GUEST_PERMISSION;return o->native.write_bytes(o->native.scalar.context,at,n,source);}
static void put(uint32_t at,unsigned width,uint32_t value)
{if(cs_pc286_guest_write(&backing.memory.guest,backing.memory.generation,at,width,value)!=CS_PG_OK)stop(0xc1);}
static uint32_t get(uint32_t at,unsigned width)
{uint32_t value=0;if(cs_pc286_guest_read(&backing.memory.guest,backing.memory.generation,at,width,CS_PG_READ,&value)!=CS_PG_OK)stop(0xc1);return value;}
static cs_m68k_result step(void){return cs_m68k_bus_step_transfer(&bus,&frame,&report,&limits,&guard);}
static int unchanged_block(void){unsigned i;for(i=0;i<64u;++i)if(get(0x18fe0UL+i,1)!=0xa5UL)return 0;return 1;}
static void seed(void)
{unsigned i;fill(&frame,sizeof frame,0);for(i=0;i<8u;++i){frame.d[i]=0x81000000UL+i;frame.a[i]=0x52000000UL+i;}frame.pc=0x9002UL;frame.a[7]=0x19020UL;frame.sr=0x271f;}
void start(void)
{
    cs_pg_region region;cs_pg_bank banks[8];unsigned i,result=cs_m68k_transfer_probe();
    if(result!=0xba)stop(result);
    fill(&backing,sizeof backing,0);fill(&rules,sizeof rules,0);backing.fail_read=UINT32_MAX;
    region.base=0x1000UL;region.bytes=0x20000UL;region.flags=7;region.first_bank=0;region.bank_count=8;
    for(i=0;i<8u;++i){banks[i].id=(uint16_t)(i+1u);banks[i].offset=0;
        banks[i].segment=(uint16_t)(0x2000u+(i/2u)*0x1100u+((i&1u)?0x1000u:0u));banks[i].bytes=(i&1u)?1u:32767u;}
    if(cs_pc286_guest_bind(&backing.memory.guest,&region,1,banks,8,0x20000UL,0x50000UL,
        &backing.memory.generation)!=CS_PG_OK ||
        cs_pc286_m68k_transfer_backend(&backing.native,&backing.memory)!=CS_PG_OK)stop(0xc0);
    bus.scalar.context=&backing;bus.scalar.context_bytes=sizeof backing;bus.scalar.validate=valid;
    bus.scalar.span=span;bus.scalar.read=read_scalar;bus.scalar.write=write_scalar;bus.write_bytes=write_bytes;
    guard.check=allow;guard.context=&rules;guard.context_bytes=sizeof rules;
    limits.base=0x18000UL;limits.size=0x2000UL;
    put(0x8ffeUL,2,0x51c8);put(0x9000UL,2,0xfffe);
    put(0x9002UL,2,0x48e7);put(0x9004UL,2,0xffff);put(0x9006UL,2,0x4cdf);put(0x9008UL,2,0xffff);
    put(0x900aUL,2,0x4ef9);put(0x900cUL,4,0x19080UL);
    put(0x19080UL,2,0x51cd);put(0x19082UL,2,0xfffe);put(0x19084UL,2,0x4e71);
    put(0x18fdfUL,1,0x96);put(0x19020UL,1,0x69);
    seed();frame.pc=0x8ffeUL;frame.d[0]=0x12340002UL;
    for(i=0;i<3u;++i)if(step()!=CS_M68K_OK)stop(0xc2);
    if(frame.pc!=0x9002UL || frame.d[0]!=0x1234ffffUL)stop(0xc2);
    copy(&before,&frame,sizeof frame);
    if(step()!=CS_M68K_OK || frame.a[7]!=0x18fe0UL)stop(0xc3);
    for(i=0;i<16u;++i)if(get(0x18fe0UL+(uint32_t)i*4UL,4)!=(i<8u?before.d[i]:before.a[i-8u]))stop(0xc3);
    for(i=0;i<8u;++i){frame.d[i]=0;if(i<7u)frame.a[i]=0;}
    if(step()!=CS_M68K_OK || frame.a[7]!=0x19020UL)stop(0xc4);
    for(i=0;i<8u;++i)if(frame.d[i]!=before.d[i] || frame.a[i]!=before.a[i])stop(0xc4);
    frame.d[5]=0xa5000001UL;
    if(step()!=CS_M68K_OK || frame.pc!=0x19080UL || step()!=CS_M68K_OK ||
        frame.pc!=0x19080UL || step()!=CS_M68K_OK || frame.pc!=0x19084UL || frame.d[5]!=0xa500ffffUL)stop(0xc5);
    fill(bytes,sizeof bytes,0xa5);
    if(cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,64,bytes)!=CS_PG_OK)stop(0xc6);
    seed();copy(&before,&frame,sizeof frame);backing.refuse=1;
    if(step()!=CS_M68K_ADDRESS || report.memory!=CS_GUEST_PERMISSION || report.bytes!=64u ||
        !equal(&frame,&before,sizeof frame) || !unchanged_block())stop(0xc7);
    backing.refuse=0;rules.deny=1;rules.kind=CS_M68K_GUARD_WRITE;rules.bytes=64;
    if(step()!=CS_M68K_POLICY || !equal(&frame,&before,sizeof frame) || !unchanged_block())stop(0xc8);
    rules.kind=CS_M68K_GUARD_COMMIT;rules.bytes=0;
    if(step()!=CS_M68K_POLICY || !equal(&frame,&before,sizeof frame) || !unchanged_block())stop(0xc8);
    rules.deny=0;frame.pc=0x9006UL;frame.a[7]=0x18fe0UL;backing.fail_read=0x1901cUL;copy(&before,&frame,sizeof frame);
    if(step()!=CS_M68K_ADDRESS || !equal(&frame,&before,sizeof frame) || !unchanged_block())stop(0xc9);
    backing.fail_read=UINT32_MAX;
    if(cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation-1UL,0x18fe0UL,64,bytes)!=CS_PG_STALE ||
        cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,0,bytes)!=CS_PG_WIDTH ||
        cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,65,bytes)!=CS_PG_WIDTH ||
        cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,64,(const uint8_t *)(uintptr_t)0xffe0u)!=CS_PG_ARGUMENT ||
        cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x20fe0UL,64,bytes)!=CS_PG_ADDRESS ||
        !unchanged_block())stop(0xca);
    /* Rebind read-only between steps; permission refusal must preserve64 bytes. */
    if(cs_pc286_guest_retire(&backing.memory.guest,backing.memory.generation)!=CS_PG_OK)stop(0xcb);
    region.flags=5;
    if(cs_pc286_guest_bind(&backing.memory.guest,&region,1,banks,8,0x20000UL,0x50000UL,&backing.memory.generation)!=CS_PG_OK ||
        cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,64,bytes)!=CS_PG_PERMISSION ||
        !unchanged_block())stop(0xcb);
    if(cs_pc286_guest_retire(&backing.memory.guest,backing.memory.generation)!=CS_PG_OK)stop(0xcc);
    region.flags=7;
    if(cs_pc286_guest_bind(&backing.memory.guest,&region,1,banks,8,0x20000UL,0x50000UL,&backing.memory.generation)!=CS_PG_OK)stop(0xcc);
    for(i=0;i<64u;++i)bytes[i]=(uint8_t)(i+1u);
    if(cs_pc286_guest_write_bytes(&backing.memory.guest,backing.memory.generation,0x18fe0UL,64,bytes)!=CS_PG_OK)stop(0xcd);
    for(i=0;i<64u;++i)if(get(0x18fe0UL+i,1)!=(uint32_t)i+1UL)stop(0xcd);
    if(get(0x18fdfUL,1)!=0x96UL || get(0x19020UL,1)!=0x69UL)stop(0xcd);
    stop(0xba);
}
