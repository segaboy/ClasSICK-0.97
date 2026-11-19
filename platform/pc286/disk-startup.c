/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "boothush-floppy-read.h"
#include "mac128k-transfer.h"
#include "../../core/storage/mac-boot-header.h"
#include "../../personality/system1/banked-services.h"
#include "../../personality/system1/startup-globals.h"

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
    struct { cs_trap_gateway traps; cs_s1_patch_state patches; } running;
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
    if(!cs_mac128k_seed_startup_globals(phase.globals,sizeof phase.globals) ||
        !seed(CS_MAC128K_GLOBALS_BASE,phase.globals,sizeof phase.globals))fail(0xd1);
    if(cs_trap_init(&phase.running.traps,&directory)!=CS_TRAP_OK)fail(0xd1);
    uint8_t *state=(uint8_t *)&phase.running.patches;
    for(unsigned i=0;i<sizeof phase.running.patches;++i)state[i]=0;
    cpu.pc=0x8002;cpu.a[0]=0x8000;cpu.a[7]=0x7800;cpu.sr=0x2700;
    for(;;)
        if(cs_s1_banked_patch_step(&backend,&phase.running.traps,&cpu,&report,&stack,
                &guard,&phase.running.patches)!=CS_M68K_OK)fail(0xd2);
}
