/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Checked width-1/2/4 access over spans built by span-build, compared with an
 * independent flat reference written from SPEC-0069 section 4 and guest.c
 * semantics. The 1 MiB arrays simulate real-mode memory only. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-access.h"
#include "../probes/pc286/span-build.h"

static int fails;
#define CK(c) do { if (!(c)) { ++fails; if (fails < 20) printf("FAIL line %d\n", __LINE__); } } while (0)

static uint8_t sim[CS_BK_REAL_TOP], ref[CS_BK_REAL_TOP];

/* R0 128 KiB RW over four banks (65535,1,32768,32768); R1 read-only; R2 RWX
 * adjacent to R1; R3 at the top of the 24-bit space. */
static const cs_bm_bank mb[7] = {{1, 65535}, {2, 1}, {3, 32768}, {4, 32768},
                                 {5, 16}, {6, 2}, {7, 16}};
static const cs_bm_region mr[4] = {{0x000000, 0x20000, 3, 0, 4}, {0x400000, 16, 1, 4, 1},
                                   {0x400010, 2, 7, 5, 1}, {0xFFFFF0, 16, 3, 6, 1}};
static const cs_bm_map map = {mr, mb, 4, 7};
static const cs_bc_bank cat[7] = {{1, 0x10000, 0}, {2, 0x30000, 0}, {3, 0x40008, 8},
                                  {4, 0x50000, 0x7FF0}, {5, 0x60000, 0},
                                  {6, 0x60010, 0}, {7, 0xFFFF0, 0}};
static cs_bk_chunk ch[16];
static cs_bs_span sp[16], sp2[16];
static cs_bx_env env;
static uint8_t c0[32767], c1[1]; /* bank 3 as two separate native objects */

/* ---- independent reference ------------------------------------------- */
static long phys(uint32_t at)
{
    uint16_t i, k, c;
    for (i = 0u; i < map.region_count; ++i) {
        uint32_t off;
        if (at < mr[i].base || at - mr[i].base >= mr[i].bytes) continue;
        off = at - mr[i].base;
        for (k = mr[i].first_bank; ; ++k) {
            if (off < mb[k].bytes) {
                for (c = 0u; c < 7u; ++c) {
                    if (cat[c].id == mb[k].id) return (long)(cat[c].physical_base + off);
                }
                return -1;
            }
            off -= mb[k].bytes;
        }
    }
    return -1;
}
static int region_of(uint32_t at, uint32_t n)
{
    int i;
    for (i = 0; i < 4; ++i) {
        if (at >= mr[i].base && n <= mr[i].bytes && at - mr[i].base <= mr[i].bytes - n) return i;
    }
    return -1;
}
static cs_bx_result ref_op(int write, uint32_t at, unsigned w, uint32_t access,
                           uint32_t value, uint32_t *out)
{
    int r;
    unsigned j;
    if (!write && (access & 1UL) == 0UL) return CS_BX_ARGUMENT;
    if (write) access = 2UL;
    if (w != 1u && w != 2u && w != 4u) return CS_BX_WIDTH;
    if (w != 1u && (at & 1UL)) return CS_BX_ADDRESS;
    if (access == 0UL || (access & ~7UL)) return CS_BX_ARGUMENT;
    if (at >= 0x1000000UL || w > 0x1000000UL - at) return CS_BX_ADDRESS;
    r = region_of(at, w);
    if (r < 0) return CS_BX_ADDRESS;
    if ((mr[r].flags & access) != access) return CS_BX_PERMISSION;
    if (write) {
        for (j = 0u; j < w; ++j) ref[phys(at + j)] = (uint8_t)(value >> (8u * (w - 1u - j)));
    } else {
        uint32_t v = 0UL;
        for (j = 0u; j < w; ++j) v = (v << 8) | ref[phys(at + j)];
        *out = v;
    }
    return CS_BX_OK;
}

/* ---- deterministic generator ------------------------------------------ */
static uint32_t rng = 0x2860C16Bu;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
static const uint32_t anchors[] = {0x0, 0x7FFE, 0x8000, 0xFFFC, 0xFFFE, 0x10000, 0x17FFE,
    0x18000, 0x1FFFC, 0x20000, 0x3FFFFE, 0x400000, 0x40000E, 0x400010, 0x400012,
    0xFFFFEE, 0xFFFFF0, 0xFFFFFC, 0xFFFFFE, 0xFFFFFF, 0x1000000};
static const uint32_t accesses[] = {1, 3, 5, 7, 2, 0, 9, 0x81, 0x80000001UL, 4, 6};

int main(void)
{
    uint32_t i, a, o1, o2, ops = 0UL, okc = 0UL, seen[9] = {0};
    uint16_t n = 0u;
    static uint8_t keep[CS_BK_REAL_TOP];

    for (i = 0; i < CS_BK_REAL_TOP; ++i) sim[i] = ref[i] = (uint8_t)next();
    CK(cs_bk_plan(&map, cat, 7, 0x10000, 0xF0000, ch, 16, &n) == CS_BK_OK && n == 11u);
    CK(cs_bk_bind(ch, n, sim, sp, 16) == CS_BK_OK);
    env.map = &map; env.catalog = cat; env.catalog_count = 7;
    env.ram_first = 0x10000; env.ram_bytes = 0xF0000;
    env.spans = sp; env.span_count = n; env.current_epoch = env.expected_epoch = 5;

    /* Directed boundary writes: bank1->2->3, bank3 chunk->bank4, 24-bit top. */
    CK(cs_bx_write(&env, 0xFFFE, 4, 0xA1B2C3D4UL) == CS_BX_OK);
    CK(sim[0x1FFFE] == 0xA1 && sim[0x30000] == 0xB2 && sim[0x40008] == 0xC3 && sim[0x40009] == 0xD4);
    CK(cs_bx_write(&env, 0x17FFE, 4, 0x01020304UL) == CS_BX_OK);
    CK(sim[0x48006] == 1 && sim[0x48007] == 2 && sim[0x50000] == 3 && sim[0x50001] == 4);
    CK(cs_bx_read(&env, 0x17FFE, 4, 1, &o1) == CS_BX_OK && o1 == 0x01020304UL);
    CK(cs_bx_write(&env, 0xFFFFFF, 1, 0x5AUL) == CS_BX_OK && sim[0xFFFFF] == 0x5A);
    CK(cs_bx_read(&env, 0x400010, 2, 5, &o1) == CS_BX_OK); /* RWX region, R|X */
    memcpy(ref, sim, sizeof sim);

    /* Precedence and no-effect refusals. */
    memcpy(keep, sim, sizeof sim);
    o1 = 0xDEADBEEFUL;
    CK(cs_bx_read(&env, 0, 4, 1, NULL) == CS_BX_ARGUMENT);
    CK(cs_bx_read(NULL, 0, 4, 1, &o1) == CS_BX_ARGUMENT);
    CK(cs_bx_read(&env, 0, 3, 2, &o1) == CS_BX_ARGUMENT);      /* no READ beats WIDTH */
    env.current_epoch = 6;
    CK(cs_bx_read(&env, 1, 3, 1, &o1) == CS_BX_STALE);         /* STALE beats WIDTH */
    CK(cs_bx_write(&env, 0, 4, 0) == CS_BX_STALE);
    env.current_epoch = 5;
    env.span_count = 0;
    CK(cs_bx_write(&env, 0, 4, 0) == CS_BX_ARGUMENT);
    env.span_count = n;
    env.ram_first = 0x20000;                                    /* bank1 outside RAM */
    CK(cs_bx_read(&env, 1, 3, 1, &o1) == CS_BX_BACKING);
    env.ram_first = 0x10000;
    CK(cs_bx_read(&env, 1, 3, 1, &o1) == CS_BX_WIDTH);         /* WIDTH beats ADDRESS */
    CK(cs_bx_read(&env, 1, 2, 9, &o1) == CS_BX_ADDRESS);       /* alignment beats access bits */
    CK(cs_bx_read(&env, 0x1000000, 1, 9, &o1) == CS_BX_ARGUMENT); /* access bits beat span */
    CK(cs_bx_write(&env, 0x40000E, 4, 0) == CS_BX_ADDRESS);    /* adjacent regions */
    CK(cs_bx_write(&env, 0x400000, 2, 0) == CS_BX_PERMISSION);
    CK(cs_bx_read(&env, 0x400000, 2, 5, &o1) == CS_BX_PERMISSION);
    sp[10].data = NULL;                                         /* late bank unbound */
    CK(cs_bx_write(&env, 0x0, 4, 0x11111111UL) == CS_BX_BINDING);
    CK(cs_bx_read(&env, 0x0, 4, 1, &o1) == CS_BX_BINDING);
    sp[10].data = sim + 0xFFFF0;
    env.span_count = (uint16_t)(n - 1u);                        /* missing final span */
    CK(cs_bx_write(&env, 0x0, 4, 0x11111111UL) == CS_BX_BINDING);
    env.span_count = n;
    CK(o1 == 0xDEADBEEFUL && memcmp(keep, sim, sizeof sim) == 0);

    /* Spans are separate native objects: bank 3's second chunk need not follow
     * its first. The byte at bank offset 32767 must use the second object. */
    memcpy(sp2, sp, sizeof sp);
    sp2[4].data = c0;
    sp2[5].data = c1;
    env.spans = sp2;
    CK(cs_bx_write(&env, 0x17FFE, 4, 0x0A0B0C0DUL) == CS_BX_OK);
    CK(c0[32766] == 0x0A && c1[0] == 0x0B && sim[0x50000] == 0x0C && sim[0x50001] == 0x0D);
    CK(cs_bx_read(&env, 0x17FFE, 2, 1, &o1) == CS_BX_OK && o1 == 0x0A0BUL);
    /* One 32768-byte object for bank 3 exceeds the C16 object limit. */
    memcpy(sp2, sp, 4u * sizeof sp[0]);
    sp2[4] = sp[4];
    sp2[4].bytes = 32768UL;
    memcpy(&sp2[5], &sp[6], 5u * sizeof sp[0]);
    env.span_count = (uint16_t)(n - 1u);
    memcpy(keep, sim, sizeof sim);
    CK(cs_bx_write(&env, 0x17FFE, 4, 0UL) == CS_BX_BINDING && memcmp(keep, sim, sizeof sim) == 0);
    env.spans = sp;
    env.span_count = n;
    sim[0x50000] = ref[0x50000];
    sim[0x50001] = ref[0x50001];

    /* Differential run: 200000 operations near every boundary. */
    for (i = 0; i < 200000UL; ++i) {
        uint32_t r = next(), at = anchors[r % 21u] + (next() % 9u) - 4u;
        unsigned w = (unsigned)(next() % 6u);
        uint32_t acc = accesses[next() % 11u], v = next();
        int write = (int)(r >> 31);
        cs_bx_result got, want;

        if (anchors[r % 21u] < 4u && at > 0xFFFFFFF0UL) at = 0x40000000UL; /* huge: ADDRESS */
        if ((next() & 7u) == 0u) at = next() & 0x1FFFFUL;                  /* anywhere in R0 */
        o1 = o2 = 0x13579BDFUL;
        want = ref_op(write, at, w, acc, v, &o2);
        got = write ? cs_bx_write(&env, at, w, v) : cs_bx_read(&env, at, w, acc, &o1);
        CK(got == want && o1 == o2);
        if (got == CS_BX_OK) ++okc;
        if ((unsigned)got < 9u) ++seen[got];
        if (write && got == CS_BX_OK) {
            for (a = 0; a < w; ++a) CK(sim[phys(at + a)] == ref[phys(at + a)]);
        }
        if ((i & 4095u) == 0u) CK(memcmp(sim, ref, sizeof sim) == 0);
        ++ops;
    }
    CK(memcmp(sim, ref, sizeof sim) == 0);
    /* Every operand-level outcome must occur in the random run. */
    CK(seen[CS_BX_OK] && seen[CS_BX_ARGUMENT] && seen[CS_BX_WIDTH] &&
       seen[CS_BX_ADDRESS] && seen[CS_BX_PERMISSION]);
    printf("outcomes OK=%lu ARG=%lu WIDTH=%lu ADDR=%lu PERM=%lu\n",
           (unsigned long)seen[0], (unsigned long)seen[1], (unsigned long)seen[5],
           (unsigned long)seen[6], (unsigned long)seen[7]);

    if (fails) {
        printf("%d failure(s)\n", fails);
        return 1;
    }
    printf("PASS pc286 span-access: %lu differential ops, %lu accepted\n",
           (unsigned long)ops, (unsigned long)okc);
    return 0;
}
