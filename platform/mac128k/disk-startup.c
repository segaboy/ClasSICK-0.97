/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-floppy.h"
#include "../../core/storage/mac-boot-header.h"
#include "../../personality/system1/startup-globals.h"

static cs_mac_startup_floppy floppy;
static cs_mac_boot_workspace sectors;
static cs_mac_boot_header header;

int cs_mac_load_startup_disk(void)
{
    cs_block_device disk;
    volatile uint8_t *const boot = (volatile uint8_t *)(uintptr_t)0x8000;
    volatile uint8_t *const globals = (volatile uint8_t *)(uintptr_t)CS_MAC128K_GLOBALS_BASE;

    if (!cs_mac_startup_floppy_bind(&floppy, &disk) ||
        cs_mac_boot_header_read(&disk, &sectors, &header) != CS_MAC_BOOT_OK ||
        !cs_mac128k_seed_startup_globals(globals, CS_MAC128K_GLOBALS_BYTES))
        return 0;

    for (unsigned i = 0; i < sizeof sectors.area; ++i)
        boot[i] = sectors.area[i];
    return 1;
}
