/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/store4-model.h"

static int fails;
#define CK(c) do { if (!(c)) { ++fails; printf("FAIL line %d\n", __LINE__); } } while (0)
#define REF(call, want) do { save(); CK((call) == (want)); CK(unchanged()); } while (0)

static uint8_t big0[32767], big1[32767], big2[1], big3[1];
static uint8_t t0[1], t1[3], t2[4], t3[8], t4[8];
static uint8_t *const bufs[9] = {big0, big1, big2, big3, t0, t1, t2, t3, t4};
static const uint32_t lens[9] = {32767, 32767, 1, 1, 1, 3, 4, 8, 8};

static uint8_t before0[32767],before1[32767],before2[1],before3[1];
static uint8_t before4[1],before5[3],before6[4],before7[8],before8[8];
static uint8_t *const before[9]={before0,before1,before2,before3,before4,before5,before6,before7,before8};
static void save(void){unsigned i;for(i=0;i<9u;i++)memcpy(before[i],bufs[i],lens[i]);}
static int unchanged(void){unsigned i;for(i=0;i<9u;i++)if(memcmp(before[i],bufs[i],lens[i]))return 0;return 1;}
static uint32_t nz(void)
{
    uint32_t n = 0u, i, j;
    for (i = 0; i < 9u; ++i) for (j = 0; j < lens[i]; ++j) n += bufs[i][j] != 0u;
    return n;
}
static void fill(int v)
{
    uint32_t i;
    for (i = 0; i < 9u; ++i) memset(bufs[i], v, lens[i]);
}

/* A: 65535-byte bank 32767+32767+1, then 1-byte bank, one 65536 region at 0x1000. */
static const cs_bm_bank ab[2] = {{1, 65535}, {2, 1}};
static const cs_bm_region ar[1] = {{0x1000, 65536, 3, 0, 2}};
static const cs_bm_map amap = {ar, ab, 1, 2};
static const cs_bc_bank acat[2] = {{1, 0x10000, 0}, {2, 0x30000, 0}};
static const cs_bs_span asp[4] = {{1, 0, 32767, big0}, {1, 32767, 32767, big1},
                                  {1, 65534, 1, big2}, {2, 0, 1, big3}};
/* B: unsorted ids, chunked bank, read-only region, final 24-bit region. */
static const cs_bm_bank bb[4] = {{5, 4}, {3, 4}, {7, 8}, {2, 8}};
static const cs_bm_region br[3] = {{0x100, 8, 3, 0, 2}, {0x108, 8, 1, 2, 1},
                                   {0xFFFFF8, 8, 3, 3, 1}};
static const cs_bm_map bmap = {br, bb, 3, 4};
static const cs_bm_map badmap = {br, bb, 0, 4};
static const cs_bc_bank bcat[4] = {{5, 0x1000, 0}, {3, 0x2000, 0}, {7, 0x3000, 0}, {2, 0x4000, 0}};
static const cs_bs_span bsp[5] = {{5, 0, 1, t0}, {5, 1, 3, t1}, {3, 0, 4, t2},
                                  {7, 0, 8, t3}, {2, 0, 8, t4}};

static cs_bs_result runA(const cs_bs_span *s, uint16_t n, uint32_t at, uint32_t v)
{
    return cs_bs_store4(&amap, acat, 2, 0, 0x100000UL, s, n, 3, 3, at, v);
}
static cs_bs_result runB(const cs_bs_span *s, uint16_t n, uint32_t at, uint32_t v)
{
    return cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, s, n, 3, 3, at, v);
}

int main(void)
{
    cs_bs_span w[7], a[4];
    uint32_t o = 7u;

    fill(0);
    CK(runA(asp, 4, 0x10FFC, 0xA1B2C3D4UL) == CS_BS_OK);
    CK(big1[32765] == 0xA1 && big1[32766] == 0xB2 && big2[0] == 0xC3 && big3[0] == 0xD4 && nz() == 4u);
    fill(0);
    CK(runA(asp, 4, 0x8FFE, 0x01020304UL) == CS_BS_OK);
    CK(big0[32766] == 1 && big1[0] == 2 && big1[1] == 3 && big1[2] == 4 && nz() == 4u);
    fill(0);
    CK(runA(asp, 4, 0x1000, 0x80000001UL) == CS_BS_OK);
    CK(big0[0] == 0x80 && big0[3] == 1 && nz() == 2u);
    fill(0);
    CK(runB(bsp, 5, 0x100, 0x11223344UL) == CS_BS_OK);
    CK(t0[0] == 0x11 && t1[0] == 0x22 && t1[1] == 0x33 && t1[2] == 0x44 && nz() == 4u);
    fill(0);
    CK(runB(bsp, 5, 0x102, 0x11223344UL) == CS_BS_OK);
    CK(t1[1] == 0x11 && t1[2] == 0x22 && t2[0] == 0x33 && t2[1] == 0x44 && nz() == 4u);
    fill(0);
    CK(runB(bsp, 5, 0xFFFFFC, 0x55667788UL) == CS_BS_OK);
    CK(t4[4] == 0x55 && t4[5] == 0x66 && t4[6] == 0x77 && t4[7] == 0x88 && nz() == 4u);

    fill(0xEE);
    REF(runB(bsp, 5, 0x106, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0x101, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0xFFFFFE, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0x2000, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0x80000000UL, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0xFFFFFFFEUL, 1), CS_BS_ADDRESS);
    REF(runB(bsp, 5, 0x108, 1), CS_BS_PERMISSION);
    REF(cs_bs_store4(NULL, bcat, 4, 0x1000, 0x4000UL, bsp, 5, 3, 3, 0x100, 1), CS_BS_ARGUMENT);
    REF(cs_bs_store4(&bmap, NULL, 4, 0x1000, 0x4000UL, bsp, 5, 3, 3, 0x100, 1), CS_BS_ARGUMENT);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, NULL, 5, 3, 3, 0x100, 1), CS_BS_ARGUMENT);
    REF(runB(bsp, 0, 0x100, 1), CS_BS_ARGUMENT);
    REF(runB(bsp, 193, 0x100, 1), CS_BS_ARGUMENT);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, bsp, 5, 0, 3, 0x100, 1), CS_BS_ARGUMENT);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, bsp, 5, 3, 0, 0x100, 1), CS_BS_ARGUMENT);

    /* precedence: invalid map/catalog beat STALE; STALE beats address/binding */
    memcpy(w, bsp, sizeof bsp);
    w[4].data = NULL;
    REF(cs_bs_store4(&badmap, bcat, 4, 0x1000, 0x4000UL, bsp, 5, 1, 2, 0x100, 1), CS_BS_MAP);
    REF(cs_bs_store4(&bmap, bcat, 3, 0x1000, 0x4000UL, bsp, 5, 1, 2, 0x100, 1), CS_BS_BACKING);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x80000000UL, bsp, 5, 1, 2, 0x100, 1), CS_BS_BACKING);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x80000000UL, 0x4000UL, bsp, 5, 1, 2, 0x100, 1), CS_BS_BACKING);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, w, 5, 4, 3, 0x101, 1), CS_BS_STALE);
    REF(cs_bs_store4(&bmap, bcat, 4, 0x1000, 0x4000UL, w, 5, 3, 4, 0x100, 1), CS_BS_STALE);

    /* whole-binding validation, including late irrelevant spans */
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[4].bytes = 0;
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[1].offset = 2;
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[1].offset = 0;
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[3].bank = 9;
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[5] = bsp[4];
    REF(runB(w, 6, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp);
    REF(runB(w, 4, 0x100, 1), CS_BS_BINDING);
    memcpy(w, bsp, sizeof bsp); w[2] = bsp[3]; w[3] = bsp[2];
    REF(runB(w, 5, 0x100, 1), CS_BS_BINDING);
    w[0].bank = 5; w[0].offset = 0; w[0].bytes = 1; w[0].data = t0;
    w[1].bank = 5; w[1].offset = 1; w[1].bytes = 1; w[1].data = t1;
    w[2].bank = 5; w[2].offset = 2; w[2].bytes = 1; w[2].data = t1 + 1;
    w[3].bank = 5; w[3].offset = 3; w[3].bytes = 1; w[3].data = t1 + 2;
    w[4] = bsp[2]; w[5] = bsp[3]; w[6] = bsp[4];
    REF(runB(w, 7, 0x100, 1), CS_BS_BINDING);
    memcpy(a, asp, sizeof asp); a[3].data = NULL;
    REF(runA(a, 4, 0x1000, 1), CS_BS_BINDING);
    memcpy(a, asp, sizeof asp); a[0].bytes = 32768UL;
    REF(runA(a, 4, 0x1000, 1), CS_BS_BINDING);
    memcpy(a, asp, sizeof asp); a[0].bytes = 0x80000000UL;
    REF(runA(a, 4, 0x1000, 1), CS_BS_BINDING);

    CK(cs_bs_epoch_next(UINT32_MAX, &o) == CS_BS_EXHAUSTED && o == 7u);
    CK(cs_bs_epoch_next(0, &o) == CS_BS_OK && o == 1u);
    CK(cs_bs_epoch_next(0xFFFFFFFEUL, &o) == CS_BS_OK && o == 0xFFFFFFFFUL);
    CK(cs_bs_epoch_next(5, NULL) == CS_BS_ARGUMENT);

    puts(fails ? "pc286-store4-model: FAIL" : "pc286-store4-model: PASS");
    return fails != 0;
}
