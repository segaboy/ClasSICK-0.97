/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/bank-plan.h"

static cs_bm_bank B[5];
static cs_bm_region R[3];
static cs_bm_map M;
static int fails;

#define EQ(a, b) do { unsigned long x_ = (unsigned long)(a), y_ = (unsigned long)(b); \
    if (x_ != y_) { printf("line %d: %lu != %lu\n", __LINE__, x_, y_); ++fails; } } while (0)

static void init(void)
{
    static const cs_bm_bank b[5] = {{1, 6}, {2, 6}, {3, 4}, {4, 4}, {5, 2}};
    static const cs_bm_region r[3] = {
        {0x1000UL, 12UL, 3UL, 0, 2},
        {0x100CUL, 4UL, 1UL, 2, 1},
        {0xFFFFFCUL, 4UL, 7UL, 3, 1}};
    memcpy(B, b, sizeof B);
    memcpy(R, r, sizeof R);
    M.regions = R;
    M.banks = B;
    M.region_count = 3;
    M.bank_count = 4;
}

/* Refusal must leave a sentinel plan byte-for-byte unchanged. */
static void refuse(const cs_bm_map *m, uint32_t at, cs_bm_result want, int line)
{
    cs_bm_plan out, snap;
    memset(&out, 0xA5, sizeof out);
    snap = out;
    if (cs_bm_plan4(m, at, 0x01020304UL, &out) != want || memcmp(&out, &snap, sizeof out) != 0) {
        printf("line %d: refusal mismatch\n", line);
        ++fails;
    }
}
#define REFUSE(m, at, want) refuse(m, at, want, __LINE__)

int main(void)
{
    static const cs_bm_plan cross = {2, {{1, 4, 2, {0xAA, 0xBB}}, {2, 0, 2, {0xCC, 0xDD}}}};
    static const cs_bm_plan flat = {1, {{1, 0, 4, {1, 2, 3, 4}}}};
    static const cs_bm_plan tail = {1, {{4, 0, 4, {9, 8, 7, 6}}}};
    cs_bm_plan out;

    init();
    EQ(cs_bm_validate(&M), CS_BM_OK);
    memset(&out, 0xA5, sizeof out);
    EQ(cs_bm_plan4(&M, 0x1004UL, 0xAABBCCDDUL, &out), CS_BM_OK);
    EQ(memcmp(&out, &cross, sizeof out), 0);
    EQ(cs_bm_plan4(&M, 0x1000UL, 0x01020304UL, &out), CS_BM_OK);
    EQ(memcmp(&out, &flat, sizeof out), 0);
    EQ(cs_bm_plan4(&M, 0xFFFFFCUL, 0x09080706UL, &out), CS_BM_OK);
    EQ(memcmp(&out, &tail, sizeof out), 0);

    /* Null arguments and access bits. */
    EQ(cs_bm_validate(NULL), CS_BM_ARGUMENT);
    EQ(cs_bm_plan4(&M, 0x1000UL, 0, NULL), CS_BM_ARGUMENT);
    EQ(cs_bm_plan4(NULL, 0x1000UL, 0, &out), CS_BM_ARGUMENT);
    EQ(cs_bm_check(&M, 0x1000UL, 1, 0), CS_BM_ARGUMENT);
    EQ(cs_bm_check(&M, 0x1000UL, 1, 8UL), CS_BM_ARGUMENT);
    EQ(cs_bm_check(&M, 0x1000UL, 1, 0x80000001UL), CS_BM_ARGUMENT);

    /* Address, adjacency, boundary and permission. */
    REFUSE(&M, 0x1001UL, CS_BM_ADDRESS);
    REFUSE(&M, 0x100AUL, CS_BM_ADDRESS);
    REFUSE(&M, 0xFFFFFEUL, CS_BM_ADDRESS);
    REFUSE(&M, 0x1000000UL, CS_BM_ADDRESS);
    REFUSE(&M, 0x100CUL, CS_BM_PERMISSION);
    EQ(cs_bm_check(&M, 0x100AUL, 4, 1UL), CS_BM_ADDRESS);
    EQ(cs_bm_check(&M, 0x1000UL, 12, 3UL), CS_BM_OK);
    EQ(cs_bm_check(&M, 0x1000UL, 0, 1UL), CS_BM_ADDRESS);
    EQ(cs_bm_check(&M, 0xFFFFFFUL, 1, 1UL), CS_BM_OK);
    EQ(cs_bm_check(&M, 0xFFFFFCUL, 4, 4UL), CS_BM_OK);
    EQ(cs_bm_check(&M, 0xFFFFFCUL, 5, 1UL), CS_BM_ADDRESS);
    EQ(cs_bm_check(&M, 0x100CUL, 2, 2UL), CS_BM_PERMISSION);

    /* Whole-map invalidity wins even when the touched bank is valid. */
    init(); B[3].bytes = 0;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    REFUSE(&M, 0x1000UL, CS_BM_MAP);
    init(); R[0].flags = 0x103UL;
    REFUSE(&M, 0x1000UL, CS_BM_MAP);
    init(); R[0].flags = 0x8UL | 1UL;
    REFUSE(&M, 0x1000UL, CS_BM_MAP);
    init(); R[0].flags = 2UL;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); B[1].id = 1;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); B[0].id = 65;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); R[1].first_bank = 1;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); M.bank_count = 5;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); B[3].bytes = 6;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); B[3].bytes = 2;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); R[1].base = 0x100AUL;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); R[2].base = 0xFFFFFEUL;
    EQ(cs_bm_validate(&M), CS_BM_MAP);
    init(); M.region_count = 0;
    EQ(cs_bm_validate(&M), CS_BM_MAP);

    if (fails == 0) {
        puts("pc286-bank-plan: all checks passed");
    }
    return fails != 0;
}
