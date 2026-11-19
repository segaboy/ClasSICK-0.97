/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-globals.h"

static void store_pointer(volatile uint8_t *page, unsigned address, uint32_t value)
{
    volatile uint8_t *slot = page + (address - CS_MAC128K_GLOBALS_BASE);
    slot[0] = (uint8_t)(value >> 24);
    slot[1] = (uint8_t)(value >> 16);
    slot[2] = (uint8_t)(value >> 8);
    slot[3] = (uint8_t)value;
}

int cs_mac128k_seed_startup_globals(volatile uint8_t *page100, size_t bytes)
{
    if (page100 == NULL || bytes < CS_MAC128K_GLOBALS_BYTES)
        return 0;
    store_pointer(page100, 0x0108u, UINT32_C(0x020000)); /* MemTop */
    store_pointer(page100, 0x0266u, UINT32_C(0x01fd00)); /* SoundBase */
    store_pointer(page100, 0x02aeu, UINT32_C(0x400000)); /* ROMBase */
    store_pointer(page100, 0x0824u, UINT32_C(0x01a700)); /* ScrnBase */
    return 1;
}
