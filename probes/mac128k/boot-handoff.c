/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../core/storage/mac-boot-image.h"
#include "../../platform/mac128k/iwm-read.h"
#include "../../personality/system1/startup-globals.h"

volatile uint32_t cs_mac_diag[7];
static cs_mac_boot_workspace workspace;
static cs_mac_boot_image acquired;
static cs_mac_iwm provider;
static cs_block_device device;

void cs_mac_enter_boot_image(void);
void cs_mac_main(void)
{
    volatile uint8_t *disk_ram = (volatile uint8_t *)(uintptr_t)0x8000;
    if (cs_mac_iwm_bind(&provider, &device) &&
        cs_mac_boot_image_load(&device, &workspace, &acquired) == CS_MAC_BOOT_OK &&
        cs_mac128k_seed_startup_globals((volatile uint8_t *)(uintptr_t)0x100, 0x728)) {
        for (unsigned i = 0; i < sizeof acquired.blocks; ++i)
            disk_ram[i] = acquired.blocks[i];
        cs_mac_enter_boot_image();
    }
    cs_mac_diag[0] = 0xd0;
    for (;;) { }
}

void cs_mac_line_a_handler(void)
{
    cs_mac_diag[0] = 0xd1;
    for (;;) { }
}
