/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "boothush-floppy-read.h"
#include "mac128k-transfer.h"
#include "../../core/storage/mac-boot-header.h"
#include "../../personality/system1/banked-services.h"
#include "../../personality/system1/startup-globals.h"
#include "../../personality/system1/startup-heap.h"

static cs_pc286_m68k_memory memory;
static cs_m68k_transfer_backend backend;
static cs_trap_frame cpu;
/* The loader zeroes BSS. Only the active union member lives past startup. */
static union {
    struct {
        cs_pc286_floppy floppy;
        cs_block_device disk;
        cs_mac_boot_workspace workspace;
        cs_mac_boot_header header;
    } load;
    uint8_t globals[CS_MAC128K_GLOBALS_BYTES];
    struct { cs_trap_gateway traps; cs_s1_patch_state patches; cs_startup_heap heap; } running;
} phase;

static void post(uint8_t code);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void halt(void);
#pragma aux halt = "cli" "hlt" modify exact [];
static void fail(uint8_t code){post(code);for(;;)halt();}

static int allow(const void *context,const cs_m68k_guard_event *event)
{
    (void)context;(void)event;return 1;
}

static int bind_ram(void)
{
    static const cs_pg_region region={0,0x20000UL,7,0,4};
    cs_pg_bank banks[4];
    for(unsigned i=0;i<4;++i){
        banks[i].id=(uint16_t)(i+1);banks[i].segment=(uint16_t)(0x3000u+i*0x800u);
        banks[i].offset=0;banks[i].bytes=0x8000;
    }
    if(cs_pc286_guest_bind(&memory.guest,&region,1,banks,4,0x30000UL,0x20000UL,
            &memory.generation)!=CS_PG_OK || cs_pc286_mac128k_transfer_backend(&backend,&memory)!=CS_PG_OK)
        return 0;
    for(unsigned i=0;i<4;++i){
        volatile uint8_t __far *bytes=(volatile uint8_t __far *)((uint32_t)banks[i].segment<<16);
        for(unsigned n=0;n<0x8000u;++n)bytes[n]=0;
    }
    return 1;
}

static int seed(uint32_t address,const uint8_t *source,unsigned bytes)
{
    while(bytes){
        unsigned chunk=bytes>64?64:bytes;
        if(cs_pc286_guest_write_bytes(&memory.guest,memory.generation,address,chunk,source)!=CS_PG_OK)
            return 0;
        address+=chunk;source+=chunk;bytes-=chunk;
    }
    return 1;
}

/* The fixed, exclusive four-bank layout in bind_ram() is valid for the
 * entire native runtime. Call only after preflighting the full guest range. */
static volatile uint8_t __far *bank_address(uint32_t at)
{
    const cs_pg_bank *bank=&memory.guest.banks[(unsigned)(at>>15)];
    return (volatile uint8_t __far *)(((uint32_t)bank->segment<<16) |
                                      (bank->offset+(at&0x7fffUL)));
}

static void clear_ram(uint32_t address,uint32_t bytes)
{
    while(bytes){
        unsigned count=(unsigned)(0x8000UL-(address&0x7fffUL));
        if(bytes<count)count=(unsigned)bytes;
        volatile uint8_t __far *p=bank_address(address);
        address+=count;bytes-=count;
        while(count--)*p++=0;
    }
}

/* The callback is called only for a NewPtr entry resolved to our ROM token.
 * It receives a validated CPU/stack and the proposed, unpublished frame. */
cs_m68k_result cs_pc286_startup_new_ptr(const cs_trap_frame *before,
    cs_trap_frame *next,cs_m68k_bus_report *report,uint32_t scratch,const cs_m68k_guard *guard)
{
    cs_startup_heap candidate=phase.running.heap;
    cs_mm_reply reply;
    uint32_t zone,limit;
    cs_m68k_guard_event event;
    event.kind=CS_M68K_GUARD_READ;event.address=0x118;event.bytes=4;event.access=CS_GUEST_READ;
    event.word=report->cpu.word;event.before=before;event.proposed=next;
    if(!guard->check(guard->context,&event))return CS_M68K_POLICY;
    report->memory=backend.scalar.read(backend.scalar.context,0x118,4,CS_GUEST_READ,&zone);
    if(report->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    event.address=0x130;
    if(!guard->check(guard->context,&event))return CS_M68K_POLICY;
    report->memory=backend.scalar.read(backend.scalar.context,0x130,4,CS_GUEST_READ,&limit);
    if(report->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    if(!cs_startup_new_ptr(&candidate,zone,limit,report->cpu.word,before->d[0],&reply))
        return CS_M68K_POLICY;

    uint32_t clear=reply.code==0 && (report->cpu.word&0x0200u)?before->d[0]:0;
    if(clear){
        /* The hardware frame is live until return and cannot be zeroed. */
        if(reply.value<scratch+6 && scratch<reply.value+clear)return CS_M68K_POLICY;
        event.kind=CS_M68K_GUARD_WRITE;event.address=reply.value;event.bytes=clear;event.access=CS_GUEST_WRITE;
        if(!guard->check(guard->context,&event))return CS_M68K_POLICY;
        report->memory=backend.scalar.span(backend.scalar.context,reply.value,clear,CS_GUEST_WRITE);
        if(report->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    }
    event.kind=CS_M68K_GUARD_WRITE;event.address=CS_STARTUP_MEM_ERR;event.bytes=2;event.access=CS_GUEST_WRITE;
    if(!guard->check(guard->context,&event))return CS_M68K_POLICY;
    report->memory=backend.scalar.span(backend.scalar.context,CS_STARTUP_MEM_ERR,2,CS_GUEST_WRITE);
    if(report->memory!=CS_GUEST_OK)return CS_M68K_ADDRESS;
    uint16_t code=(uint16_t)reply.code;
    next->d[0]=(before->d[0]&0xffff0000UL)|code;
    next->a[0]=reply.value;
    next->sr=(uint16_t)((before->sr&0xfff0u)|(code==0?4u:((code&0x8000u)?8u:0u)));
    next->pc=before->pc+2;
    event.kind=CS_M68K_GUARD_COMMIT;event.address=next->pc;event.bytes=0;event.access=0;
    if(!guard->check(guard->context,&event))return CS_M68K_POLICY;

    /* The outer dispatcher preflighted all six scratch bytes. Use only
     * stable ordinary RAM now; none of these stores can subsequently fail. */
    clear_ram(reply.value,clear);
    *bank_address(CS_STARTUP_MEM_ERR)=(uint8_t)(code>>8);
    *bank_address(CS_STARTUP_MEM_ERR+1)=(uint8_t)code;
    *bank_address(scratch)=(uint8_t)(next->sr>>8);*bank_address(scratch+1)=(uint8_t)next->sr;
    for(unsigned n=0;n<4;++n)*bank_address(scratch+2+n)=(uint8_t)(next->pc>>((3u-n)*8u));
    phase.running.heap=candidate;
    return CS_M68K_OK;
}

void start(void)
{
    static const cs_m68k_guard guard={allow,NULL,0};
    static const cs_m68k_stack_limits stack={0x828,0x1a700UL-0x828};
    static const cs_trap_config directory={0x400000UL,0x1000,0x11000UL,0x400000UL,0x400002UL,0x400020UL};
    cs_m68k_bus_report report;
    if(!bind_ram())fail(0xd1);
    if(!cs_pc286_floppy_bind(&phase.load.floppy,0,800,&phase.load.disk) ||
        cs_mac_boot_header_read(&phase.load.disk,&phase.load.workspace,&phase.load.header)!=CS_MAC_BOOT_OK)
        fail(0xd0);
    if(!seed(0x8000,phase.load.workspace.area,CS_MAC_BOOT_AREA_BYTES))fail(0xd1);
    /* Release the floppy provider and both native sector buffers. A clean
     * temporary page is needed because only four globals are initialized. */
    for(unsigned i=0;i<sizeof phase.globals;++i)phase.globals[i]=0;
    if(!cs_mac128k_seed_startup_globals(phase.globals,sizeof phase.globals))fail(0xd1);
    cs_startup_heap_globals(phase.globals);
    if(!seed(CS_MAC128K_GLOBALS_BASE,phase.globals,sizeof phase.globals))fail(0xd1);
    if(cs_trap_init(&phase.running.traps,&directory)!=CS_TRAP_OK ||
        cs_trap_set(&phase.running.traps,0x1e,CS_S1_STARTUP_NEW_PTR_TOKEN)!=CS_TRAP_OK)fail(0xd1);
    cs_startup_heap_init(&phase.running.heap);
    uint8_t *state=(uint8_t *)&phase.running.patches;
    for(unsigned i=0;i<sizeof phase.running.patches;++i)state[i]=0;
    cpu.pc=0x8002;cpu.a[0]=0x8000;cpu.a[7]=0x7800;cpu.sr=0x2700;
    for(;;)
        if(cs_s1_banked_startup_step(&backend,&phase.running.traps,&cpu,&report,&stack,
                &guard,&phase.running.patches,cs_pc286_startup_new_ptr)!=CS_M68K_OK)fail(0xd2);
}
