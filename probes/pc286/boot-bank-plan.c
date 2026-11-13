/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Freestanding 286 diagnostic: BootHush runs our separately linked bank
 * planner. The boot harness supplies CS=DS=SS and a valid stack. */
#include "bank-plan.h"

static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];

void start(void)
{
    cs_bm_bank banks[2];
    cs_bm_region region;
    cs_bm_map map;
    cs_bm_plan plan;
    cs_bm_result odd;
    unsigned expected_byte = 0xaau;

    banks[0].id = 1; banks[0].bytes = 6;
    banks[1].id = 2; banks[1].bytes = 6;
    region.base = 0x1000UL; region.bytes = 12; region.flags = 3;
    region.first_bank = 0; region.bank_count = 2;
    map.regions = &region; map.banks = banks;
    map.region_count = 1; map.bank_count = 2;

    if (cs_bm_validate(&map) != CS_BM_OK) {
        post(0xe1); for (;;) { }
    }
    if (cs_bm_plan4(&map, 0x1004UL, 0xaabbccddUL, &plan) != CS_BM_OK) {
        post(0xe2); for (;;) { }
    }
#ifdef CS_P286_MUTATE_PLAN
    expected_byte = 0xabu; /* diagnostic negative control */
#endif
    if (plan.count != 2 || plan.pieces[0].bank != 1 ||
        plan.pieces[0].offset != 4 || plan.pieces[0].length != 2 ||
        plan.pieces[0].data[0] != expected_byte || plan.pieces[0].data[1] != 0xbbu ||
        plan.pieces[1].bank != 2 || plan.pieces[1].offset != 0 ||
        plan.pieces[1].length != 2 || plan.pieces[1].data[0] != 0xccu ||
        plan.pieces[1].data[1] != 0xddu) {
        post(0xe3); for (;;) { }
    }
    odd = cs_bm_plan4(&map, 0x1005UL, 0x12345678UL, &plan);
#ifdef CS_P286_MUTATE_ODD
    if (odd != CS_BM_OK) {
#else
    if (odd != CS_BM_ADDRESS || plan.count != 2 ||
        plan.pieces[0].data[0] != 0xaau || plan.pieces[1].data[1] != 0xddu) {
#endif
        post(0xe4); for (;;) { }
    }
    post(0xa5);
    for (;;) { }
}
