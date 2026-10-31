/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Capability windows vs the UNCHANGED production cs_guest_window: identical
 * result codes for the same request; on success every byte borrowed through
 * read runs equals the flat view, and bytes written through write runs are
 * then seen by production cs_guest_read. Fixed seed; author-generated cases. */
#include <stdio.h>
#include <string.h>
#include "../core/memory/guest.h"
#include "../probes/pc286/span-cap.h"
#include "../probes/pc286/span-build.h"

static int fails;
#define CK(c) do { if (!(c)) { ++fails; if (fails < 25) printf("FAIL line %d\n", __LINE__); } } while (0)

static uint8_t arena[CS_BK_REAL_TOP];
static uint8_t f0[0x20000], f1[16], f2[2], f3[16];
static const cs_bm_bank MB[7] = {{1, 65535}, {2, 1}, {3, 32768}, {4, 32768}, {5, 16}, {6, 2}, {7, 16}};
static const cs_bm_region MR[4] = {{0x000000, 0x20000, 3, 0, 4}, {0x400000, 16, 1, 4, 1},
                                   {0x400010, 2, 7, 5, 1}, {0xFFFFF0, 16, 3, 6, 1}};
static const cs_bm_map MAP = {MR, MB, 4, 7};
static const cs_bc_bank CAT[7] = {{1, 0x10000, 0}, {2, 0x30000, 0}, {3, 0x40008, 8},
                                  {4, 0x50000, 0x7FF0}, {5, 0x60000, 0}, {6, 0x60010, 0},
                                  {7, 0xFFFF0, 0}};
static const cs_guest_region GR[4] = {{f0, f0, 0x000000, 0x20000, 3}, {f1, NULL, 0x400000, 16, 1},
                                      {f2, f2, 0x400010, 2, 7}, {f3, f3, 0xFFFFF0, 16, 3}};
static const cs_guest_map GMAP = {GR, 4};

static uint32_t rng = 0xCAB0057u;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

static uint8_t *phys(uint32_t at)
{
    uint16_t r, b, c;
    for (r = 0; r < 4u; ++r) {
        uint32_t off;
        if (at < MR[r].base || at - MR[r].base >= MR[r].bytes) continue;
        off = at - MR[r].base;
        for (b = MR[r].first_bank; off >= MB[b].bytes; ++b) off -= MB[b].bytes;
        for (c = 0; c < 7u; ++c)
            if (CAT[c].id == MB[b].id) return arena + CAT[c].physical_base + off;
    }
    return NULL;
}
static int code(cs_guest_result g)
{
    switch (g) {
    case CS_GUEST_OK: return CS_BQ_OK;
    case CS_GUEST_ARGUMENT: return CS_BQ_ARGUMENT;
    case CS_GUEST_MAP: return CS_BQ_MAP;
    case CS_GUEST_ADDRESS: return CS_BQ_ADDRESS;
    case CS_GUEST_PERMISSION: return CS_BQ_PERMISSION;
    default: return -1;
    }
}
static const uint32_t anchors[] = {0x0, 0x7FFE, 0x8000, 0xFFFC, 0xFFFE, 0x10000, 0x17FFE,
    0x18000, 0x1FFFC, 0x20000, 0x3FFFFE, 0x400000, 0x40000E, 0x400010, 0x400012,
    0xFFFFEE, 0xFFFFF0, 0xFFFFFC, 0xFFFFFE, 0xFFFFFF, 0x1000000};
static const uint32_t accesses[] = {1, 3, 5, 7, 2, 0, 9, 0x81, 0x80000001UL, 4, 6};

int main(void)
{
    cs_bk_chunk ch[16];
    cs_bs_span sp[16];
    cs_bx_env env;
    static cs_bq_registry reg;   /* fresh-only init needs zero-filled storage */
    cs_bq_owner own;
    uint16_t n = 0, r;
    uint32_t i, accepted = 0, compared = 0, written = 0;

    setvbuf(stdout, NULL, _IONBF, 0);   /* failures visible even if a mutant crashes */
    CK(cs_bk_plan(&MAP, CAT, 7, 0x10000, 0xF0000UL, ch, 16, &n) == CS_BK_OK);
    CK(cs_bk_bind(ch, n, arena, sp, 16) == CS_BK_OK);
    env.map = &MAP; env.catalog = CAT; env.catalog_count = 7; env.ram_first = 0x10000;
    env.ram_bytes = 0xF0000UL; env.spans = sp; env.span_count = n; env.current_epoch = env.expected_epoch = 9;
    CK(cs_bq_registry_init(&reg, 10) == CS_BQ_OK && cs_bq_owner_open(&reg, &env, 10, &own) == CS_BQ_OK);
    for (r = 0; r < 4u; ++r) {
        uint32_t j;
        for (j = 0; j < GR[r].size; ++j) {
            uint8_t v = (uint8_t)next();
            *phys(GR[r].base + j) = v;
            ((uint8_t *)(uintptr_t)GR[r].bytes)[j] = v;
        }
    }
    for (i = 0; i < 8000UL; ++i) {
        uint32_t at = anchors[next() % 21u] + (next() % 9u) - 4u, len, acc = accesses[next() % 11u];
        uint32_t bias = next() & 1u, salt = next();
        cs_guest_view view;
        cs_bq_cap k;
        int g, q;
        if (bias) acc = (next() & 1u) ? 1UL : 3UL;
        if ((next() & 3u) == 0u) at = next() & 0x1FFFFUL;
        len = (next() & 1u) ? next() % 40u : next() % 0x22000UL;
        g = code(cs_guest_window(&GMAP, at, len, acc, &view));
        q = (int)cs_bq_open(&own, at, len, acc, &k);
        CK(g == q);
        if (q != CS_BQ_OK || (acc & 1u) == 0u) continue;   /* borrow needs READ */
        ++accepted;
        {
            uint32_t need = (acc & 2u) ? 3UL : 1UL, pos = 0;
            cs_bq_run run;
            uint16_t em = 0;
            CK(((acc & 2u) != 0u) == (view.writable != NULL));
            while (cs_bq_borrow(&k, need, 32767UL, &run, &em) == CS_BQ_OK && em) {
                CK(memcmp(run.read, view.bytes + pos, run.length) == 0);
                compared += run.length;
                if (run.write != NULL) {                /* write through, flat in step */
                    uint32_t t;
                    for (t = 0; t < run.length; ++t) {
                        uint8_t v = (uint8_t)(salt + pos + t);
                        run.write[t] = v;
                        view.writable[pos + t] = v;
                    }
                    written += run.length;
                }
                pos += run.length;
            }
            CK(pos == len);
            if (need == 3UL && len >= 1u) {             /* production read sees it */
                uint32_t v = 0;
                CK(cs_guest_read(&GMAP, at, 1, 1, &v) == CS_GUEST_OK && v == *phys(at));
            }
        }
    }
    for (r = 0; r < 4u; ++r) {
        uint32_t j;
        for (j = 0; j < GR[r].size; ++j) CK(*phys(GR[r].base + j) == GR[r].bytes[j]);
    }
    CK(accepted > 500u && written > 0u);
    if (fails) {
        printf("%d failure(s)\n", fails);
        return 1;
    }
    printf("PASS pc286 span-cap parity with guest.c: 8000 windows, %lu borrowed, %lu bytes compared, %lu written\n",
           (unsigned long)accepted, (unsigned long)compared, (unsigned long)written);
    return 0;
}
