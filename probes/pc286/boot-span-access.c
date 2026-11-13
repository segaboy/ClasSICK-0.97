/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* BootHush-only experiment using the pinned Open Watcom compact memory
 * model. The fixture supplies 640 KiB RAM and CS=DS=SS=1000h. Its two far
 * pointers are known 2000:0000 and 3000:0000 byte windows. This does not
 * implement a production provider or establish physical ownership. */
#include "span-access.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This diagnostic requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_boot_data_pointer_is_far, sizeof(uint8_t *) == 4);

static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

void start(void)
{
    cs_bm_bank banks[2]; cs_bm_region region; cs_bm_map map;
    cs_bc_bank catalog[2]; cs_bs_span spans[2]; cs_bx_env env;
    uint32_t value = 0;
    uint8_t *a = (uint8_t *)0x20000000UL;
    uint8_t *b = (uint8_t *)0x30000000UL;
    unsigned i, expected = 0xddu;
    cs_bx_result stale, binding;

    banks[0].id = 1; banks[0].bytes = 6;
    banks[1].id = 2; banks[1].bytes = 6;
    region.base = 0x1000UL; region.bytes = 12; region.flags = 3;
    region.first_bank = 0; region.bank_count = 2;
    map.regions = &region; map.banks = banks; map.region_count = 1; map.bank_count = 2;
    catalog[0].id = 1; catalog[0].physical_base = 0x20000UL; catalog[0].native_offset = 0;
    catalog[1].id = 2; catalog[1].physical_base = 0x30000UL; catalog[1].native_offset = 0;
    spans[0].bank = 1; spans[0].offset = 0; spans[0].bytes = 6; spans[0].data = a;
    spans[1].bank = 2; spans[1].offset = 0; spans[1].bytes = 6; spans[1].data = b;
    env.map = &map; env.catalog = catalog; env.spans = spans;
    env.ram_first = 0x20000UL; env.ram_bytes = 0x10006UL;
    env.current_epoch = 1; env.expected_epoch = 1; env.catalog_count = 2; env.span_count = 2;
    for (i = 0; i < 6; ++i) { a[i] = 0; b[i] = 0; }
    a[3] = 0x44; a[4] = 0x44; a[5] = 0x55;
    b[0] = 0x66; b[1] = 0x77; b[2] = 0x88;

    if (cs_bx_write(&env, 0x1004UL, 4, 0xaabbccddUL) != CS_BX_OK) {
        post(0xe5); for (;;) { }
    }
#ifdef CS_P286_MUTATE_SPAN
    expected = 0xdcu;
#endif
    if (a[3] != 0x44u || a[4] != 0xaau || a[5] != 0xbbu ||
        b[0] != 0xccu || b[1] != expected || b[2] != 0x88u) {
        post(0xe6); for (;;) { }
    }
    if (cs_bx_read(&env, 0x1004UL, 4, 1, &value) != CS_BX_OK ||
        value != 0xaabbccddUL) {
        post(0xe7); for (;;) { }
    }
    env.expected_epoch = 2;
    stale = cs_bx_write(&env, 0x1004UL, 4, 0);
#ifdef CS_P286_MUTATE_EPOCH
    if (stale != CS_BX_OK) {
#else
    if (stale != CS_BX_STALE || a[4] != 0xaau || b[1] != 0xddu) {
#endif
        post(0xe8); for (;;) { }
    }
    if (cs_bx_read(&env, 0x1005UL, 2, 1, &value) != CS_BX_STALE ||
        value != 0xaabbccddUL) {
        post(0xe9); for (;;) { }
    }
    env.expected_epoch = 1;
    if (cs_bx_write(&env, 0x1005UL, 2, 0) != CS_BX_ADDRESS ||
        a[4] != 0xaau || b[1] != 0xddu) {
        post(0xea); for (;;) { }
    }
    if (cs_bx_read(&env, 0x1005UL, 2, 1, &value) != CS_BX_ADDRESS ||
        value != 0xaabbccddUL) {
        post(0xeb); for (;;) { }
    }
    spans[1].data = NULL;
    binding = cs_bx_write(&env, 0x1004UL, 4, 0);
#ifdef CS_P286_MUTATE_BINDING
    if (binding != CS_BX_OK) {
#else
    if (binding != CS_BX_BINDING || a[4] != 0xaau || a[5] != 0xbbu ||
        b[0] != 0xccu || b[1] != 0xddu) {
#endif
        post(0xec); for (;;) { }
    }
    post(0xa6);
    for (;;) { }
}
