/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Checked access vs the UNCHANGED production guest.c (SPEC-0022): same four
 * regions, same bytes, same operations. With a valid environment and complete
 * binding (BACKING/STALE/BINDING unreachable), result codes, read values and
 * resulting bytes must be identical. Fixed seed; author-generated cases. */
#include <stdio.h>
#include <string.h>
#include "../core/memory/guest.h"
#include "../probes/pc286/span-access.h"
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

static uint32_t rng = 0xACCE55u;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

/* Logical byte -> physical byte in the arena (independent of the backend). */
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
static uint8_t *flat(uint32_t at)
{
    uint16_t r;
    for (r = 0; r < 4u; ++r)
        if (at >= GR[r].base && at - GR[r].base < GR[r].size)
            return (uint8_t *)(uintptr_t)GR[r].bytes + (at - GR[r].base);
    return NULL;
}
static int code(cs_guest_result g)
{
    static const int map[6] = {CS_BX_OK, CS_BX_ARGUMENT, CS_BX_MAP, CS_BX_WIDTH, CS_BX_ADDRESS, CS_BX_PERMISSION};
    return map[g];
}
static int equal_all(void)
{
    uint16_t r;
    uint32_t j;
    for (r = 0; r < 4u; ++r)
        for (j = 0; j < GR[r].size; ++j)
            if (*phys(GR[r].base + j) != *flat(GR[r].base + j)) return 0;
    return 1;
}
static const uint32_t anchors[] = {0x0, 0x7FFE, 0x8000, 0xFFFC, 0xFFFE, 0x10000, 0x17FFE,
    0x18000, 0x1FFFC, 0x20000, 0x3FFFFE, 0x400000, 0x40000E, 0x400010, 0x400012,
    0xFFFFEE, 0xFFFFF0, 0xFFFFFC, 0xFFFFFE, 0xFFFFFF, 0x1000000};
static const uint32_t accesses[] = {1, 3, 5, 7, 2, 0, 9, 0x81, 0x80000001UL, 4, 6};

int main(void)
{
    cs_bk_chunk ch[16];
    cs_bs_span sp[16];
    cs_bx_env e;
    uint16_t n = 0, r;
    uint32_t i, ok = 0, seen[9] = {0};

    CK(cs_bk_plan(&MAP, CAT, 7, 0x10000, 0xF0000UL, ch, 16, &n) == CS_BK_OK);
    CK(cs_bk_bind(ch, n, arena, sp, 16) == CS_BK_OK);
    e.map = &MAP; e.catalog = CAT; e.catalog_count = 7; e.ram_first = 0x10000;
    e.ram_bytes = 0xF0000UL; e.spans = sp; e.span_count = n; e.current_epoch = e.expected_epoch = 1;
    CK(cs_guest_validate(&GMAP) == CS_GUEST_OK);
    for (r = 0; r < 4u; ++r) {
        uint32_t j;
        for (j = 0; j < GR[r].size; ++j) {
            uint8_t v = (uint8_t)next();
            *phys(GR[r].base + j) = v;
            *flat(GR[r].base + j) = v;
        }
    }
    for (i = 0; i < 300000UL; ++i) {
        uint32_t pick = next(), at = anchors[pick % 21u] + (next() % 9u) - 4u;
        unsigned w = (unsigned)(next() % 6u);
        uint32_t acc = accesses[next() % 11u], v = next(), o1 = 0x2468ACE0UL, o2 = 0x2468ACE0UL;
        int g, s;
        if ((next() & 7u) == 0u) at = next() & 0x1FFFFUL;
        if (pick >> 31) {
            g = code(cs_guest_write(&GMAP, at, w, v));
            s = (int)cs_bx_write(&e, at, w, v);
        } else {
            g = code(cs_guest_read(&GMAP, at, w, acc, &o1));
            s = (int)cs_bx_read(&e, at, w, acc, &o2);
        }
        CK(g == s && o1 == o2);
        if (s >= 0 && s < 9) ++seen[s];
        if (s == CS_BX_OK) {
            unsigned j;
            ++ok;
            for (j = 0; j < w; ++j) CK(*phys(at + j) == *flat(at + j));
        }
        if ((i & 16383u) == 0u) CK(equal_all());
    }
    CK(equal_all());
    CK(seen[CS_BX_OK] && seen[CS_BX_ARGUMENT] && seen[CS_BX_WIDTH] && seen[CS_BX_ADDRESS] &&
       seen[CS_BX_PERMISSION] && !seen[CS_BX_BACKING] && !seen[CS_BX_STALE] && !seen[CS_BX_BINDING]);
    if (fails) {
        printf("%d failure(s)\n", fails);
        return 1;
    }
    printf("PASS pc286 span-access parity with guest.c: 300000 ops, %lu accepted\n", (unsigned long)ok);
    return 0;
}
