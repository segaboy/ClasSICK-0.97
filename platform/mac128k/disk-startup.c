/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-floppy.h"
#include "../../core/storage/mac-boot-header.h"
#include "../../core/services/traps.h"
#include "../../personality/system1/startup-globals.h"
#include "../../personality/system1/startup-heap.h"

static union {
    struct {
        cs_mac_startup_floppy floppy;
        cs_mac_boot_workspace sectors;
        cs_mac_boot_header header;
    } load;
    cs_trap_gateway traps;
} phase;
static cs_startup_heap heap;

extern void cs_mac_get_trap_address(void);
extern void cs_mac_set_trap_address(void);
extern void cs_mac_startup_stop(void);
extern void cs_mac_new_ptr(void);
extern void cs_mac_new_ptr_clear(void);
extern void cs_mac_new_ptr_sys(void);
extern void cs_mac_new_ptr_sys_clear(void);
/* The assembly entry defines these as absolute guest RAM symbols. */
extern volatile uint32_t cs_mac_current_zone, cs_mac_application_limit;
extern volatile int16_t cs_mac_memory_error;

uint32_t cs_mac_trap_target(uint32_t word)
{
    uint32_t target;
    int new_ptr = (word & 0xf9ffu) == 0xa11eu;
    if(((word&0xfe00u)!=0xa000u && !new_ptr) ||
        cs_trap_get(&phase.traps,(uint16_t)(word&0xffu),&target)!=CS_TRAP_OK)
        return 0;
    if(target==(uint32_t)(uintptr_t)cs_mac_new_ptr) {
        if (!new_ptr) return 0;
        switch (word) {
            case 0xa31e: return (uint32_t)(uintptr_t)cs_mac_new_ptr_clear;
            case 0xa51e: return (uint32_t)(uintptr_t)cs_mac_new_ptr_sys;
            case 0xa71e: return (uint32_t)(uintptr_t)cs_mac_new_ptr_sys_clear;
            default: return target;
        }
    }
    if(target==phase.traps.config.get_token)
        return word==0xa146u || word==0xa046u?target:0;
    if(target==phase.traps.config.set_token)return word==0xa047u?target:0;
    return target>=phase.traps.config.ram_begin && target<phase.traps.config.ram_end?target:0;
}

uint32_t cs_mac_trap_get_value(uint32_t number)
{
    uint32_t target;
    return cs_trap_get(&phase.traps,(uint16_t)number,&target)==CS_TRAP_OK?target:0;
}

int cs_mac_trap_set_value(uint32_t number,uint32_t address)
{
    return cs_trap_set(&phase.traps,(uint16_t)number,address)==CS_TRAP_OK;
}

/* ROM bridge; all selected logical addresses are backed by physical RAM
 * in this 128K post-overlay configuration. UINT32_MAX is a contract fault. */
uint32_t cs_mac_allocate_startup_ptr(uint32_t size, uint32_t word)
{
    cs_mm_reply reply;
    if (!cs_startup_new_ptr(&heap, cs_mac_current_zone, cs_mac_application_limit,
            (uint16_t)word, size, &reply))
        return UINT32_MAX;
    if (reply.code == CS_MM_NO_ERROR && (word & 0x0200u)) {
        volatile uint8_t *payload = (volatile uint8_t *)(uintptr_t)reply.value;
        for (uint32_t i = 0; i < size; ++i)
            payload[i] = 0;
    }
    cs_mac_memory_error = reply.code;
    return reply.value;
}

int cs_mac_load_startup_disk(void)
{
    cs_block_device disk;
    volatile uint8_t *const boot = (volatile uint8_t *)(uintptr_t)0x8000;
    volatile uint8_t *const globals = (volatile uint8_t *)(uintptr_t)CS_MAC128K_GLOBALS_BASE;

    if (!cs_mac_startup_floppy_bind(&phase.load.floppy, &disk) ||
        cs_mac_boot_header_read(&disk, &phase.load.sectors, &phase.load.header) != CS_MAC_BOOT_OK ||
        !cs_mac128k_seed_startup_globals(globals, CS_MAC128K_GLOBALS_BYTES))
        return 0;

    for (unsigned i = 0; i < sizeof phase.load.sectors.area; ++i)
        boot[i] = phase.load.sectors.area[i];
    static const cs_trap_config config={0x400000UL,0x1000,0x11000UL,
        (uint32_t)(uintptr_t)cs_mac_get_trap_address,(uint32_t)(uintptr_t)cs_mac_set_trap_address,
        (uint32_t)(uintptr_t)cs_mac_startup_stop};
    cs_startup_heap_globals(globals);
    cs_startup_heap_init(&heap);
    return cs_trap_init(&phase.traps,&config)==CS_TRAP_OK &&
        cs_trap_set(&phase.traps,0x1e,(uint32_t)(uintptr_t)cs_mac_new_ptr)==CS_TRAP_OK;
}
