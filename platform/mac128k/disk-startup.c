/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-floppy.h"
#include "../../core/storage/mac-boot-header.h"
#include "../../core/services/traps.h"
#include "../../personality/system1/startup-globals.h"

static union {
    struct {
        cs_mac_startup_floppy floppy;
        cs_mac_boot_workspace sectors;
        cs_mac_boot_header header;
    } load;
    cs_trap_gateway traps;
} phase;

extern void cs_mac_get_trap_address(void);
extern void cs_mac_set_trap_address(void);
extern void cs_mac_startup_stop(void);

uint32_t cs_mac_trap_target(uint32_t word)
{
    uint32_t target;
    if((word&0xfe00u)!=0xa000u ||
        cs_trap_get(&phase.traps,(uint16_t)(word&0xffu),&target)!=CS_TRAP_OK)
        return 0;
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
    return cs_trap_init(&phase.traps,&config)==CS_TRAP_OK;
}
