/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Host tests for the proposed span construction experiment. The 1 MiB array
 * simulates real-mode linear memory; it is not a native far-memory proof. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-build.h"

static int fails;
#define CK(c) do { if (!(c)) { ++fails; printf("FAIL line %d\n", __LINE__); } } while (0)

static uint8_t sim[CS_BK_REAL_TOP], snap[CS_BK_REAL_TOP];
static cs_bk_chunk ch[16], chs[16];
static cs_bs_span sp[16], sps[16];
static uint16_t n;

static void save(void)
{
    memcpy(snap, sim, sizeof sim);
    memcpy(chs, ch, sizeof ch);
    memcpy(sps, sp, sizeof sp);
}
static int same_sim(void) { return memcmp(snap, sim, sizeof sim) == 0; }
static int same_out(void)
{
    return memcmp(chs, ch, sizeof ch) == 0 && memcmp(sps, sp, sizeof sp) == 0;
}
static void sentinel(void)
{
    memset(ch, 0xA5, sizeof ch);
    memset(sp, 0x5A, sizeof sp);
    n = 0xBEEFu;
}

/* Independent oracle: every chunk's linear start equals catalog base + offset,
 * chunks tile each bank in map order from 0, and none crosses a 64 KiB window. */
static int oracle(const cs_bm_map *m, const cs_bc_bank *c, uint16_t cn,
                  const cs_bk_chunk *k, uint16_t kn)
{
    uint16_t i, j, at = 0u;
    for (i = 0u; i < m->bank_count; ++i) {
        uint32_t want = 0UL, base = 0UL, pieces = 0UL;
        for (j = 0u; j < cn; ++j) {
            if (c[j].id == m->banks[i].id) {
                base = c[j].physical_base;
            }
        }
        while (want < m->banks[i].bytes) {
            if (at >= kn || k[at].bank != m->banks[i].id || k[at].offset != want ||
                k[at].bytes == 0UL || k[at].bytes > 32767UL ||
                (uint32_t)k[at].segment * 16UL + k[at].native_offset != base + want ||
                (uint32_t)k[at].native_offset + k[at].bytes > 65536UL) {
                return 0;
            }
            want += k[at].bytes;
            ++at;
            ++pieces;
        }
        if (want != m->banks[i].bytes || pieces < 1UL || pieces > 3UL) {
            return 0;
        }
    }
    return at == kn;
}

/* A: 65535-byte bank (three chunks) and a 1-byte bank, one 64 KiB region. */
static const cs_bm_bank ab[2] = {{1, 65535}, {2, 1}};
static const cs_bm_region ar[1] = {{0x1000, 65536, 3, 0, 2}};
static const cs_bm_map amap = {ar, ab, 1, 2};
static const cs_bc_bank acat[2] = {{1, 0x10000, 0}, {2, 0x30000, 0}};
/* B: unsorted ids, read-only region, nonzero native offsets, final 24-bit region,
 * a bank ending exactly at its segment window and one at the 1 MiB top. */
static const cs_bm_bank bb[5] = {{5, 4}, {3, 4}, {7, 16}, {2, 8}, {9, 16}};
static const cs_bm_region br[3] = {{0x100, 8, 3, 0, 2}, {0x108, 16, 1, 2, 1},
                                   {0xFFFFE8, 24, 3, 3, 2}};
static const cs_bm_map bmap = {br, bb, 3, 5};
static const cs_bc_bank bcat[5] = {{5, 0x20010, 0x10}, {3, 0x20020, 0x20},
                                   {7, 0x30000, 0}, {2, 0x4FFF8, 0xFFF8},
                                   {9, 0xFFFF0, 0}};
/* Bad representations that the numeric catalog still admits. */
static const cs_bm_bank xb[1] = {{1, 16}};
static const cs_bm_region xr[1] = {{0, 16, 3, 0, 1}};
static const cs_bm_map xmap = {xr, xb, 1, 1};
static const cs_bc_bank odd[1] = {{1, 0x10008, 0}};   /* not a paragraph */
static const cs_bc_bank low[1] = {{1, 0x8, 0x10}};    /* negative segment */
static const cs_bc_bank zero[1] = {{1, 0x10, 0x10}};  /* segment 0 is valid */
static const cs_bc_bank dup[2] = {{1, 0x10000, 0}, {1, 0x30000, 0}};
static const cs_bm_map badmap = {br, bb, 0, 5};

static cs_bk_result planA(uint16_t cap) { return cs_bk_plan(&amap, acat, 2, 0, CS_BK_REAL_TOP, ch, cap, &n); }
static cs_bk_result planB(void) { return cs_bk_plan(&bmap, bcat, 5, 0x1000, CS_BK_REAL_TOP - 0x1000, ch, 16, &n); }
static cs_bs_result putA(uint32_t at, uint32_t v)
{
    return cs_bs_store4(&amap, acat, 2, 0, CS_BK_REAL_TOP, sp, n, 1, 1, at, v);
}
static cs_bs_result putB(uint32_t at, uint32_t v)
{
    return cs_bs_store4(&bmap, bcat, 5, 0x1000, CS_BK_REAL_TOP - 0x1000, sp, n, 1, 1, at, v);
}
/* Checks k expected bytes at linear a and folds them into the snapshot, so a
 * following same_sim() proves that nothing else in the 1 MiB space moved. */
static int mark(uint32_t a, const uint8_t *v, uint32_t k)
{
    uint32_t i;
    int ok = 1;
    for (i = 0; i < k; ++i) {
        ok &= sim[a + i] == v[i];
        snap[a + i] = v[i];
    }
    return ok;
}

int main(void)
{
    static const uint8_t v1[4] = {0xA1, 0xB2, 0xC3, 0xD4}, v2[4] = {1, 2, 3, 4};
    static const uint8_t v3[4] = {0x11, 0x22, 0x33, 0x44}, v4[4] = {0x55, 0x66, 0x77, 0x88};
    cs_bk_chunk bad;

    /* Plan A: three chunks for bank 1, one for bank 2, exact parts. */
    sentinel();
    CK(planA(16) == CS_BK_OK && n == 4u);
    CK(ch[0].segment == 0x1000u && ch[0].native_offset == 0u && ch[0].bytes == 32767UL);
    CK(ch[1].native_offset == 32767u && ch[1].offset == 32767UL && ch[1].bytes == 32767UL);
    CK(ch[2].native_offset == 65534u && ch[2].offset == 65534UL && ch[2].bytes == 1UL);
    CK(ch[3].bank == 2UL && ch[3].segment == 0x3000u && ch[3].native_offset == 0u);
    CK(oracle(&amap, acat, 2, ch, n));
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_OK);
    CK(sp[1].data == sim + 0x10000 + 32767 && sp[3].data == sim + 0x30000);

    /* End-to-end through the unchanged store4: bytes land at linear addresses. */
    memset(sim, 0, sizeof sim);
    save();
    CK(putA(0x10FFC, 0xA1B2C3D4UL) == CS_BS_OK);
    CK(mark(0x1FFFC, v1, 3) && mark(0x30000, v1 + 3, 1) && same_sim());
    save();
    CK(putA(0x8FFE, 0x01020304UL) == CS_BS_OK); /* crosses chunk 0 -> 1 */
    CK(mark(0x17FFE, v2, 4) && same_sim());

    /* Capacity: needs 4, offered 3. Nothing written. */
    sentinel();
    save();
    CK(planA(3) == CS_BK_CAPACITY && n == 0xBEEFu && same_out());

    /* Plan B: map bank-array order, native offsets, segment-window edge. */
    sentinel();
    CK(planB() == CS_BK_OK && n == 5u && oracle(&bmap, bcat, 5, ch, n));
    CK(ch[0].bank == 5UL && ch[1].bank == 3UL && ch[2].bank == 7UL && ch[3].bank == 2UL && ch[4].bank == 9UL);
    CK(ch[0].segment == 0x2000u && ch[0].native_offset == 0x10u);
    CK(ch[3].segment == 0x4000u && ch[3].native_offset == 0xFFF8u);
    CK(ch[4].segment == 0xFFFFu && ch[4].native_offset == 0u);
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_OK);
    memset(sim, 0, sizeof sim);
    save();
    CK(putB(0x102, 0x11223344UL) == CS_BS_OK); /* bank 5 -> bank 3 */
    CK(mark(0x20012, v3, 2) && mark(0x20020, v3 + 2, 2) && same_sim());
    save();
    CK(putB(0xFFFFEC, 0x55667788UL) == CS_BS_OK); /* last long at window top */
    CK(mark(0x4FFFC, v4, 4) && same_sim());
    save();
    CK(putB(0xFFFFFC, 0x01020304UL) == CS_BS_OK); /* last byte of 1 MiB */
    CK(mark(0xFFFFC, v2, 4) && same_sim());
    save();
    CK(putB(0x108, 1UL) == CS_BS_PERMISSION && same_sim());

    /* Representation refusals the numeric catalog admits. */
    sentinel();
    save();
    CK(cs_bk_plan(&xmap, odd, 1, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_REPRESENTATION && same_out() && n == 0xBEEFu);
    CK(cs_bk_plan(&xmap, low, 1, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_REPRESENTATION && same_out());
    CK(cs_bk_plan(&xmap, zero, 1, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_OK && n == 1u && ch[0].segment == 0u);

    /* Error precedence: NULL outputs, then catalog ARGUMENT/MAP/BACKING. */
    sentinel();
    save();
    CK(cs_bk_plan(&badmap, bcat, 5, 0, CS_BK_REAL_TOP, NULL, 16, &n) == CS_BK_ARGUMENT);
    CK(cs_bk_plan(&amap, acat, 2, 0, CS_BK_REAL_TOP, ch, 16, NULL) == CS_BK_ARGUMENT);
    CK(cs_bk_plan(NULL, acat, 2, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_ARGUMENT);
    CK(cs_bk_plan(&badmap, bcat, 5, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_MAP);
    CK(cs_bk_plan(&amap, dup, 2, 0, CS_BK_REAL_TOP, ch, 16, &n) == CS_BK_BACKING);
    CK(cs_bk_plan(&xmap, odd, 1, 0x20000, 0x1000, ch, 16, &n) == CS_BK_BACKING);
    CK(same_out() && n == 0xBEEFu);

    /* Bind refusals: every chunk is checked before any output. */
    CK(planA(16) == CS_BK_OK);
    memset(sp, 0x5A, sizeof sp);
    save();
    CK(cs_bk_bind(ch, n, NULL, sp, 16) == CS_BK_ARGUMENT);
    CK(cs_bk_bind(NULL, n, sim, sp, 16) == CS_BK_ARGUMENT);
    CK(cs_bk_bind(ch, 0, sim, sp, 16) == CS_BK_ARGUMENT);
    CK(cs_bk_bind(ch, n, sim, sp, 3) == CS_BK_CAPACITY);
    bad = ch[3];
    ch[3].bytes = 0UL;                                   /* late empty chunk */
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_REPRESENTATION);
    ch[3] = bad;
    ch[3].native_offset = 0xFFFFu; ch[3].bytes = 2UL;    /* wraps its window */
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_REPRESENTATION);
    ch[3] = bad;
    ch[3].segment = 0xFFFFu; ch[3].native_offset = 0x10u; /* above 1 MiB */
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_REPRESENTATION);
    ch[3] = bad;
    ch[3].bytes = 32768UL;                               /* over one object */
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_REPRESENTATION);
    ch[3] = bad;
    CK(memcmp(sps, sp, sizeof sp) == 0 && same_sim());

    if (fails) {
        printf("%d failure(s)\n", fails);
        return 1;
    }
    printf("PASS pc286 span-build\n");
    return 0;
}
