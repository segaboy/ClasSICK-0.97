/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../core/storage/mac-boot-image.h"
#include "../../platform/mac128k/iwm-read.h"

volatile uint32_t cs_mac_diag[7];
static cs_mac_boot_workspace workspace;
cs_mac_boot_image cs_boot_image;
static cs_mac_iwm provider;
static cs_block_device device;

void cs_mac_main(void)
{
    uint32_t marker;
    cs_mac_boot_result result = CS_MAC_BOOT_ARGUMENT;
    if (cs_mac_iwm_bind(&provider, &device))
        result = cs_mac_boot_image_load(&device, &workspace, &cs_boot_image);
    cs_mac_diag[1] = (uint32_t)result;
    if (result == CS_MAC_BOOT_OK) {
        /* The independently generated fixture follows the C scalar return ABI. */
        typedef uint32_t (*fixture_entry)(void);
        fixture_entry entry = (fixture_entry)(cs_boot_image.blocks + 2);
        marker = entry();
        cs_mac_diag[2] = marker;
        cs_mac_diag[3] = 1;
    }
    cs_mac_diag[0] = 0xb1;
    for (;;) { }
}

void cs_mac_line_a_handler(void)
{
    cs_mac_diag[6] = 0xf002;
    for (;;) { }
}
