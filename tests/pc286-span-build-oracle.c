/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-build.h"

static int failures;
#define CHECK(c) do { if (!(c)) { failures++; \
    printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static uint8_t arena[CS_BK_REAL_TOP];
static uint8_t snap[CS_BK_REAL_TOP];
static cs_bm_bank B[64];
static cs_bc_bank C[64];
static cs_bm_region R;
static cs_bm_map M;
static cs_bk_chunk CH[200], CHS[200];
static cs_bs_span SP[200], SPS[200];
static uint16_t cnt, cnts;

/* One region over banks B[0..n-1]; sums bank bytes. */
static void mk(uint16_t n, uint32_t flags)
{
    uint32_t sum = 0;
    uint16_t i;
    for (i = 0; i < n; i++) sum += B[i].bytes;
    memset(&R, 0, sizeof R);
    R.base = 0; R.bytes = sum; R.flags = flags;
    R.first_bank = 0; R.bank_count = n;
    memset(&M, 0, sizeof M);
    M.regions = &R; M.banks = B;
    M.region_count = 1; M.bank_count = n;
}

static void sent(void)
{
    memset(CH, 0x5A, sizeof CH);
    memcpy(CHS, CH, sizeof CH);
    memset(SP, 0xA5, sizeof SP);
    memcpy(SPS, SP, sizeof SP);
    cnt = 0x1234; cnts = cnt;
}

static int plan_same(void)
{
    return memcmp(CH, CHS, sizeof CH) == 0 && cnt == cnts;
}

static int span_same(void)
{
    return memcmp(SP, SPS, sizeof SP) == 0;
}

static cs_bk_result plan(uint16_t n, uint16_t cap)
{
    return cs_bk_plan(&M, C, n, 0, CS_BK_REAL_TOP, CH, cap, &cnt);
}

static void one(uint32_t bytes, uint32_t phys, uint32_t off, uint32_t flags)
{
    B[0].id = 1; B[0].bytes = bytes;
    C[0].id = 1; C[0].physical_base = phys; C[0].native_offset = off;
    mk(1, flags);
}

static void t_null_tuple(void)
{
    cs_bk_chunk bad;
    one(4, 0, 0, 3);
    CHECK(cs_bc_validate(&M, C, 1, 0, CS_BK_REAL_TOP) == CS_BC_OK);
    sent();
    CHECK(plan(1, 4) == CS_BK_REPRESENTATION);
    CHECK(plan_same());
    memset(&bad, 0, sizeof bad);
    bad.bank = 1; bad.bytes = 4;
    CHECK(cs_bk_bind(&bad, 1, arena, SP, 4) == CS_BK_REPRESENTATION);
    CHECK(span_same());
}

static void t_segment0(void)
{
    one(4, 16, 16, 3);
    sent();
    CHECK(plan(1, 1) == CS_BK_OK);
    CHECK(cnt == 1 && CH[0].segment == 0 && CH[0].native_offset == 16);
    CHECK(CH[0].bank == 1 && CH[0].offset == 0 && CH[0].bytes == 4);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 4) == CS_BK_OK);
    CHECK(SP[0].data == arena + 16 && SP[0].bytes == 4);
}

static void t_max_bank(void)
{
    B[0].id = 1; B[0].bytes = 65535;
    B[1].id = 2; B[1].bytes = 1;
    C[0].id = 1; C[0].physical_base = 4096; C[0].native_offset = 0;
    C[1].id = 2; C[1].physical_base = 69632; C[1].native_offset = 0;
    mk(2, 3);
    CHECK(R.bytes == 65536);
    sent();
    CHECK(plan(2, 4) == CS_BK_OK && cnt == 4);
    CHECK(CH[0].offset == 0 && CH[0].bytes == 32767);
    CHECK(CH[1].offset == 32767 && CH[1].bytes == 32767);
    CHECK(CH[2].offset == 65534 && CH[2].bytes == 1);
    CHECK(CH[3].bank == 2 && CH[3].offset == 0 && CH[3].bytes == 1);
    CHECK(CH[0].segment == 256 && CH[0].native_offset == 0);
    CHECK(CH[1].native_offset == 32767 && CH[2].native_offset == 65534);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 4) == CS_BK_OK);
    CHECK(SP[2].data == arena + 4096 + 65534);
}

static void t_window_end(void)
{
    one(16, 131056, 65520, 3);
    sent();
    CHECK(plan(1, 1) == CS_BK_OK && cnt == 1);
    CHECK(CH[0].segment == 4096 && CH[0].native_offset == 65520);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 1) == CS_BK_OK);
    CHECK(SP[0].data == arena + 131056);
}

static void t_last4(void)
{
    uint32_t i;
    one(4, 1048572, 12, 3);
    sent();
    CHECK(plan(1, 1) == CS_BK_OK && CH[0].segment == 65535);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 1) == CS_BK_OK);
    CHECK(SP[0].data == arena + 1048572);
    memset(arena, 0xEE, sizeof arena);
    memcpy(snap, arena, sizeof snap);
    CHECK(cs_bs_store4(&M, C, 1, 0, CS_BK_REAL_TOP, SP, cnt, 7, 7, 0,
                       0x01020304UL) == CS_BS_OK);
    for (i = 0; i < 4; i++) snap[1048572 + i] = (uint8_t)(i + 1);
    CHECK(memcmp(arena, snap, sizeof arena) == 0);
}

static void t_95(void)
{
    uint16_t i;
    uint32_t phys = 16;
    for (i = 0; i < 64; i++) {
        B[i].id = 64u - i;
        B[i].bytes = i < 31 ? 32768u : (i < 63 ? 1u : 2u);
    }
    for (i = 0; i < 64; i++) {
        cs_bc_bank *c = &C[63 - i];
        c->id = B[i].id;
        c->physical_base = phys;
        c->native_offset = phys & 15u;
        phys += B[i].bytes;
    }
    mk(64, 3);
    CHECK(R.bytes == 1015842UL && phys <= CS_BK_REAL_TOP);
    sent();
    CHECK(plan(64, 94) == CS_BK_CAPACITY);
    CHECK(plan_same());
    CHECK(plan(64, 95) == CS_BK_OK && cnt == 95);
    CHECK(CH[0].bank == 64 && CH[0].bytes == 32767);
    CHECK(CH[1].bank == 64 && CH[1].offset == 32767 && CH[1].bytes == 1);
    CHECK(CH[93].bank == 2 && CH[93].bytes == 1);
    CHECK(CH[94].bank == 1 && CH[94].bytes == 2);
    for (i = 0; i < cnt; i++) {
        uint32_t lin = (uint32_t)CH[i].segment * 16u + CH[i].native_offset;
        CHECK(lin + CH[i].bytes <= CS_BK_REAL_TOP);
    }
    memcpy(CHS, CH, sizeof CH);
    cnts = cnt;
    memset(SP, 0xA5, sizeof SP);
    memcpy(SPS, SP, sizeof SP);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 94) == CS_BK_CAPACITY);
    CHECK(span_same());
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 95) == CS_BK_OK);
}

static void t_rep_before_capacity(void)
{
    B[0].id = 1; B[0].bytes = 2;
    B[1].id = 2; B[1].bytes = 2;
    C[0].id = 1; C[0].physical_base = 16; C[0].native_offset = 0;
    C[1].id = 2; C[1].physical_base = 33; C[1].native_offset = 0;
    mk(2, 3);
    sent();
    CHECK(plan(2, 0) == CS_BK_REPRESENTATION && plan_same());
    C[1].physical_base = 48; C[1].native_offset = 1;
    sent();
    CHECK(plan(2, 0) == CS_BK_REPRESENTATION && plan_same());
    C[1].physical_base = 16 + 32; C[1].native_offset = 49;
    sent();
    CHECK(plan(2, 0) == CS_BK_REPRESENTATION && plan_same());
}

static void t_bind_precedence(void)
{
    cs_bk_chunk two[2];
    memset(two, 0, sizeof two);
    two[0].bank = 1; two[0].bytes = 4; two[0].segment = 1;
    two[1].bank = 1; two[1].bytes = 0; two[1].segment = 1;
    sent();
    CHECK(cs_bk_bind(two, 2, arena, SP, 1) == CS_BK_CAPACITY);
    CHECK(span_same());
}

static void t_cross_bank(void)
{
    uint32_t i;
    B[0].id = 1; B[0].bytes = 65535;
    B[1].id = 2; B[1].bytes = 3;
    C[0].id = 1; C[0].physical_base = 4096; C[0].native_offset = 0;
    C[1].id = 2; C[1].physical_base = 131072; C[1].native_offset = 0;
    mk(2, 3);
    CHECK(R.bytes == 65538);
    sent();
    CHECK(plan(2, 4) == CS_BK_OK && cnt == 4);
    CHECK(cs_bk_bind(CH, cnt, arena, SP, 4) == CS_BK_OK);
    memset(arena, 0x11, sizeof arena);
    memcpy(snap, arena, sizeof snap);
    CHECK(cs_bs_store4(&M, C, 2, 0, CS_BK_REAL_TOP, SP, cnt, 1, 1, 65534,
                       0xA1B2C3D4UL) == CS_BS_OK);
    snap[69630] = 0xA1; snap[131072] = 0xB2;
    snap[131073] = 0xC3; snap[131074] = 0xD4;
    CHECK(memcmp(arena, snap, sizeof arena) == 0);
    memcpy(snap, arena, sizeof snap);
    mk(2, 1);
    CHECK(cs_bs_store4(&M, C, 2, 0, CS_BK_REAL_TOP, SP, cnt, 1, 1, 65534,
                       0x01010101UL) == CS_BS_PERMISSION);
    for (i = 0; i < 4; i++) CHECK(arena[131072 + i] == snap[131072 + i]);
    CHECK(memcmp(arena, snap, sizeof arena) == 0);
}

static void late(uint32_t bytes, uint32_t off, uint16_t seg, uint16_t no)
{
    cs_bk_chunk two[2];
    memset(two, 0, sizeof two);
    two[0].bank = 1; two[0].bytes = 4; two[0].segment = 16;
    two[1].bank = 1; two[1].bytes = bytes;
    two[1].segment = seg; two[1].native_offset = no;
    (void)off;
    sent();
    CHECK(cs_bk_bind(two, 2, arena, SP, 2) == CS_BK_REPRESENTATION);
    CHECK(span_same());
}

static void t_late_invalid(void)
{
    late(0, 0, 16, 0);
    late(32768, 0, 16, 0);
    late(2, 0, 16, 65535);
    late(1, 0, 65535, 16);
    late(16, 0, 65535, 8);
}

static void t_arguments(void)
{
    cs_bk_chunk bad;
    memset(&bad, 0, sizeof bad);
    bad.bytes = 0;
    one(4, 16, 0, 3);
    sent();
    CHECK(cs_bk_plan(NULL, C, 1, 0, CS_BK_REAL_TOP, CH, 0, &cnt)
          == CS_BK_ARGUMENT);
    CHECK(cs_bk_plan(&M, NULL, 1, 0, CS_BK_REAL_TOP, CH, 0, &cnt)
          == CS_BK_ARGUMENT);
    CHECK(cs_bk_plan(&M, C, 1, 0, CS_BK_REAL_TOP, NULL, 0, &cnt)
          == CS_BK_ARGUMENT);
    CHECK(cs_bk_plan(&M, C, 1, 0, CS_BK_REAL_TOP, CH, 0, NULL)
          == CS_BK_ARGUMENT);
    CHECK(plan_same());
    CHECK(cs_bk_bind(NULL, 1, arena, SP, 0) == CS_BK_ARGUMENT);
    CHECK(cs_bk_bind(&bad, 1, NULL, SP, 0) == CS_BK_ARGUMENT);
    CHECK(cs_bk_bind(&bad, 1, arena, NULL, 0) == CS_BK_ARGUMENT);
    CHECK(cs_bk_bind(&bad, 0, arena, SP, 0) == CS_BK_ARGUMENT);
    CHECK(span_same());
}

int main(void)
{
    t_null_tuple();
    t_segment0();
    t_max_bank();
    t_window_end();
    t_last4();
    t_95();
    t_rep_before_capacity();
    t_bind_precedence();
    t_cross_bank();
    t_late_invalid();
    t_arguments();
    if (failures) {
        printf("pc286-span-build-oracle: %d failure(s)\n", failures);
        return 1;
    }
    printf("pc286-span-build-oracle: ok\n");
    return 0;
}
