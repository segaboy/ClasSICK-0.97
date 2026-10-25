/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/bank-catalog.h"

static int failures;
#define EXPECT(call, want) do { if ((call) != (want)) { \
    printf("FAIL line %d\n", __LINE__); failures++; } } while (0)

static const cs_bm_bank B[6] = {{10, 65535}, {11, 3}, {12, 30001},
                                {13, 20001}, {14, 10001}, {15, 5531}};
static const cs_bm_region R = {0, 131072, 3, 0, 6};
static const cs_bm_map M = {&R, B, 1, 6};
static const cs_bc_bank G[6] = {
    {10, 0xE0000UL, 1}, {11, 0xEFFFFUL, 65533}, {12, 0xF0002UL, 0},
    {13, 0xF7533UL, 0}, {14, 0xFC354UL, 0}, {15, 0xFEA65UL, 0}};

static cs_bc_result run(const cs_bc_bank *d, uint16_t n, uint32_t f, uint32_t b)
{
    return cs_bc_validate(&M, d, n, f, b);
}

int main(void)
{
    cs_bc_bank d[6], snap[6];
    cs_bm_bank bb[6];
    cs_bm_region rr;
    cs_bm_map m2;
    cs_bc_bank one;
    cs_bm_bank ob = {7, 2};
    cs_bm_region orr = {0, 2, 1, 0, 1};
    cs_bm_map m1;

    memcpy(d, G, sizeof d);
    memcpy(snap, G, sizeof snap);
    EXPECT(run(d, 6, 0xE0000UL, 131072UL), CS_BC_OK);
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_OK);
    EXPECT(memcmp(d, snap, sizeof d), 0);

    /* Two-byte bank at the final native offset boundary. */
    m1.regions = &orr; m1.banks = &ob; m1.region_count = 1; m1.bank_count = 1;
    one.id = 7; one.physical_base = 0xFFFFEUL; one.native_offset = 65534u;
    EXPECT(cs_bc_validate(&m1, &one, 1, 0xFFFFEUL, 2), CS_BC_OK);
    one.native_offset = 65535u;
    EXPECT(cs_bc_validate(&m1, &one, 1, 0xFFFFEUL, 2), CS_BC_BACKING);
    one.native_offset = 65536u;
    EXPECT(cs_bc_validate(&m1, &one, 1, 0xFFFFEUL, 2), CS_BC_BACKING);
    memcpy(d, G, sizeof d);
    d[0].native_offset = 2;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);
    d[0].native_offset = 1; d[1].native_offset = 65534u;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);

    /* counts, ids, duplicates */
    memcpy(d, G, sizeof d);
    EXPECT(run(d, 5, 0, 0x100000UL), CS_BC_BACKING);
    EXPECT(run(d, 7, 0, 0x100000UL), CS_BC_BACKING);
    d[5].id = 99;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);
    d[5].id = 10;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);

    /* overlaps, including differently based partial containment */
    memcpy(d, G, sizeof d);
    d[1].physical_base = 0xEFFFEUL;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);
    memcpy(d, G, sizeof d);
    d[3].physical_base = 0xF0003UL;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);
    memcpy(d, G, sizeof d);
    d[4].physical_base = 0xF7000UL;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);

    /* RAM interval gaps, outside, end overflow */
    memcpy(d, G, sizeof d);
    EXPECT(run(d, 6, 0xE0001UL, 131071UL), CS_BC_BACKING);
    EXPECT(run(d, 6, 0xE0000UL, 131071UL), CS_BC_BACKING);
    EXPECT(run(d, 6, 0xE0000UL, 0), CS_BC_BACKING);
    EXPECT(run(d, 6, 0x100000UL, 1), CS_BC_BACKING);
    EXPECT(run(d, 6, 0xE0000UL, 131073UL), CS_BC_BACKING);
    EXPECT(run(d, 6, 0xFFFFFFFFUL, 2), CS_BC_BACKING);
    EXPECT(run(d, 6, 1, 0xFFFFFFFFUL), CS_BC_BACKING);
    d[5].physical_base = 0xFFFFFFF0UL;
    EXPECT(run(d, 6, 0, 0x100000UL), CS_BC_BACKING);

    /* malformed late logical bank: MAP precedes backing errors */
    memcpy(bb, B, sizeof bb);
    bb[5].bytes = 5530;
    rr = R;
    m2.regions = &rr; m2.banks = bb; m2.region_count = 1; m2.bank_count = 6;
    memcpy(d, G, sizeof d);
    d[0].id = 99;
    EXPECT(cs_bc_validate(&m2, d, 6, 0, 0x100000UL), CS_BC_MAP);
    EXPECT(cs_bc_validate(&m2, d, 3, 0, 0x100000UL), CS_BC_MAP);

    /* null args */
    EXPECT(cs_bc_validate(&M, NULL, 6, 0, 0x100000UL), CS_BC_ARGUMENT);
    EXPECT(cs_bc_validate(NULL, G, 6, 0, 0x100000UL), CS_BC_ARGUMENT);
    EXPECT(cs_bc_validate(NULL, NULL, 0, 0, 0), CS_BC_ARGUMENT);

    /* Lead regression: subtracting base from an earlier RAM end must not wrap. */
    one = (cs_bc_bank){7, 0x20000UL, 0};
    EXPECT(cs_bc_validate(&m1, &one, 1, 0x10000UL, 2), CS_BC_BACKING);

    printf(failures ? "pc286-bank-catalog FAILED\n" : "pc286-bank-catalog ok\n");
    return failures != 0;
}
