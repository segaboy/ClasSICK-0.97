/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/startup-globals.h"
#include <string.h>

int main(void)
{
    /* Absolute guest offsets in the oracle catch a page-relative API mistake. */
    uint8_t ram[0x900], expected[sizeof ram];
    memset(ram, 0xa5, sizeof ram);
    memcpy(expected, ram, sizeof ram);
    if (cs_mac128k_seed_startup_globals(NULL, 0x728) ||
        cs_mac128k_seed_startup_globals(ram+0x100, 0x727) ||
        memcmp(ram, expected, sizeof ram))
        return 1;
    memcpy(expected+0x108, "\x00\x02\x00\x00", 4);
    memcpy(expected+0x266, "\x00\x01\xfd\x00", 4);
    memcpy(expected+0x2ae, "\x00\x40\x00\x00", 4);
    memcpy(expected+0x824, "\x00\x01\xa7\x00", 4);
    if (!cs_mac128k_seed_startup_globals(ram+0x100, 0x728) ||
        memcmp(ram, expected, sizeof ram))
        return 2;
    return 0;
}
