/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0069 section 5 capability + borrow bridge under the R2-1/R2-2 context
 * rules: fresh-only registry, one authoritative binding per owner lifetime,
 * rebind advances first, registry identity recorded per capability.
 * Author-written; fixed seed for the random drains. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-cap.h"
#include "../probes/pc286/span-build.h"

static int fails;
#define CK(c) do { if (!(c)) { ++fails; if (fails < 25) printf("FAIL line %d\n", __LINE__); } } while (0)

/* 128 KiB RWX region over six odd banks, plus an adjacent read-only region. */
static const cs_bm_bank mb[7] = {{1, 30001}, {2, 32767}, {3, 32768}, {4, 17535},
                                 {5, 17001}, {6, 1000}, {7, 16}};
static const cs_bm_region mr[2] = {{0x0, 131072, 7, 0, 6}, {0x20000, 16, 1, 6, 1}};
static const cs_bm_map map = {mr, mb, 2, 7};
static const cs_bm_map other = {mr, mb, 2, 7};          /* equal content, other identity */
static const cs_bm_region badr[1] = {{0x1, 16, 1, 0, 1}};
static const cs_bm_map badmap = {badr, mb, 1, 7};
/* Two placements of the SAME map: two different bindings. */
static const cs_bc_bank cat1[7] = {{1, 0x10000, 0}, {2, 0x20000, 0}, {3, 0x30000, 0},
                                   {4, 0x40000, 0}, {5, 0x50000, 0}, {6, 0x60000, 0}, {7, 0x70000, 0}};
static const cs_bc_bank cat2[7] = {{1, 0x80000, 0}, {2, 0x90000, 0}, {3, 0xA0000, 0},
                                   {4, 0xB0000, 0}, {5, 0xC0000, 0}, {6, 0xD0000, 0}, {7, 0xE0000, 0}};
static uint8_t sim[CS_BK_REAL_TOP];
static cs_bs_span sp1[16], sp2[16], spx[16];
static cs_bx_env env1, env2, envx;
static cs_bq_registry reg, regb, reg2, rega1, regb1, keep_reg;   /* static: zero-filled */
static cs_bq_owner own, own2, own3, keep_own;

static void where(uint32_t at, uint16_t *bi, uint32_t *off)
{
    uint16_t r = at >= 0x20000UL ? 1u : 0u, b = mr[r].first_bank;
    uint32_t o = at - mr[r].base;
    while (o >= mb[b].bytes) { o -= mb[b].bytes; ++b; }
    *bi = b; *off = o;
}
static int drain_ok(cs_bq_cap *k, uint32_t at, uint32_t n, uint32_t limit, uint32_t *chunks)
{
    cs_bq_descriptor c;
    uint16_t e = 0u, bi;
    uint32_t pos = at, left = n, off, guard = 0UL, cap = limit < 32767UL ? limit : 32767UL;
    for (;;) {
        uint32_t want;
        if (cs_bq_chunk(k, limit, &c, &e) != CS_BQ_OK || ++guard > 200000UL) return 0;
        if (!e) break;
        where(pos, &bi, &off);
        want = left < cap ? left : cap;
        if (mb[bi].bytes - off < want) want = mb[bi].bytes - off;
        if (c.bank != mb[bi].id || c.offset != off || c.length != want) return 0;
        pos += c.length; left -= c.length; ++*chunks;
    }
    return left == 0UL;
}
static int bind(cs_bx_env *e, const cs_bc_bank *cat, cs_bs_span *sp)
{
    cs_bk_chunk ch[16];
    uint16_t n = 0;
    if (cs_bk_plan(&map, cat, 7, 0x10000, 0xF0000UL, ch, 16, &n) != CS_BK_OK ||
        cs_bk_bind(ch, n, sim, sp, 16) != CS_BK_OK) return 0;
    e->map = &map; e->catalog = cat; e->catalog_count = 7; e->ram_first = 0x10000;
    e->ram_bytes = 0xF0000UL; e->spans = sp; e->span_count = n;
    e->current_epoch = e->expected_epoch = 4;               /* same number in both */
    return 1;
}
/* Fills [at, at+n) through write runs with byte f(i) = i ^ salt. */
static int fill(const cs_bq_owner *o, uint32_t at, uint32_t n, uint8_t salt)
{
    cs_bq_cap k;
    cs_bq_run r;
    uint16_t e = 0;
    uint32_t pos = 0, t;
    if (cs_bq_open(o, at, n, 3, &k) != CS_BQ_OK) return 0;
    while (cs_bq_borrow(&k, 3, 32767UL, &r, &e) == CS_BQ_OK && e) {
        for (t = 0; t < r.length; ++t) r.write[t] = (uint8_t)((pos + t) ^ salt);
        pos += r.length;
    }
    return pos == n;
}
static int check(const cs_bq_owner *o, uint32_t at, uint32_t n, uint8_t salt)
{
    cs_bq_cap k;
    cs_bq_run r;
    uint16_t e = 0;
    uint32_t pos = 0, t;
    if (cs_bq_open(o, at, n, 1, &k) != CS_BQ_OK) return 0;
    while (cs_bq_borrow(&k, 1, 4096UL, &r, &e) == CS_BQ_OK && e) {
        if (r.write != NULL) return 0;
        for (t = 0; t < r.length; ++t) if (r.read[t] != (uint8_t)((pos + t) ^ salt)) return 0;
        pos += r.length;
    }
    return pos == n;
}

static uint32_t rng = 0xC0FFEE11u;
static uint32_t next(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

int main(void)
{
    static const uint32_t spans[5] = {32767, 32768, 65535, 65536, 131072};
    cs_bq_cap k, k2, keep;
    cs_bq_descriptor c;
    cs_bq_run r, r0;
    uint16_t e = 0u, e0;
    uint32_t i, j, a, runs, chunks = 0UL, acc = 0UL;

    setvbuf(stdout, NULL, _IONBF, 0);   /* failures visible even if a mutant crashes */
    CK(bind(&env1, cat1, sp1) && bind(&env2, cat2, sp2));

    /* R2-1: fresh-only registry. */
    CK(cs_bq_registry_init(&reg, 0xFFFFFFFFUL) == CS_BQ_OK);
    CK(cs_bq_owner_open(&reg, &env1, 0xFFFFFFFFUL, &own) == CS_BQ_OK && own.lifetime == 1UL);
    keep_reg = reg;
    CK(cs_bq_registry_init(&reg, 5) == CS_BQ_STATE && memcmp(&reg, &keep_reg, sizeof reg) == 0);
    CK(cs_bq_registry_init(NULL, 1) == CS_BQ_ARGUMENT);

    /* Descriptor conformance: SPEC lengths from three starts, random drains. */
    for (i = 0; i < 5u; ++i) {
        for (j = 0; j < 3u; ++j) {
            uint32_t at = j == 0u ? 0UL : j == 1u ? 29999UL : 131072UL - spans[i];
            if (at + spans[i] > 131072UL) continue;
            CK(cs_bq_open(&own, at, spans[i], 1, &k) == CS_BQ_OK && drain_ok(&k, at, spans[i], 32767UL, &chunks));
            CK(cs_bq_open(&own, at, spans[i], 3, &k) == CS_BQ_OK && drain_ok(&k, at, spans[i], 1000UL, &chunks));
        }
    }
    for (i = 0; i < 20000UL; ++i) {
        uint32_t r0b = next() & 1u, base = r0b ? 0x20000UL : 0UL, size = r0b ? 16UL : 131072UL;
        uint32_t at = base + next() % size, len = 1UL + next() % (base + size - at);
        uint32_t lim = (next() & 3u) ? 32767UL : 1UL + next() % 32767UL;
        CK(cs_bq_open(&own, at, len, 1, &k) == CS_BQ_OK && drain_ok(&k, at, len, lim, &chunks));
    }

    /* Open preflight refusals create nothing. */
    memset(&k, 0xA5, sizeof k);
    keep = k;
    CK(cs_bq_open(&own, 131070, 4, 1, &k) == CS_BQ_ADDRESS);
    CK(cs_bq_open(&own, 0x20000, 16, 2, &k) == CS_BQ_PERMISSION);
    CK(cs_bq_open(&own, 0, 16, 0, &k) == CS_BQ_ARGUMENT && cs_bq_open(&own, 0, 16, 9, &k) == CS_BQ_ARGUMENT);
    CK(cs_bq_open(&own, 0, 0, 1, &k) == CS_BQ_ADDRESS && cs_bq_open(NULL, 0, 1, 1, &k) == CS_BQ_ARGUMENT);
    CK(memcmp(&k, &keep, sizeof k) == 0);

    /* Rights, release, limit bounds. */
    CK(cs_bq_open(&own, 0, 100, 5, &k) == CS_BQ_OK && cs_bq_rights(&k, &acc) == CS_BQ_OK && acc == 5UL);
    CK(cs_bq_chunk(&k, 0, &c, &e) == CS_BQ_ARGUMENT && cs_bq_chunk(&k, 32768UL, &c, &e) == CS_BQ_ARGUMENT);
    CK(cs_bq_release(&k) == CS_BQ_OK && cs_bq_chunk(&k, 10, &c, &e) == CS_BQ_CAPABILITY);
    CK(cs_bq_borrow(&k, 1, 10, &r, &e) == CS_BQ_CAPABILITY && cs_bq_release(NULL) == CS_BQ_ARGUMENT);

    /* Generation advance: STALE without cursor movement. */
    CK(cs_bq_open(&own, 30000, 10, 1, &k) == CS_BQ_OK && cs_bq_chunk(&k, 32767UL, &c, &e) == CS_BQ_OK);
    CK(e == 1u && c.bank == 1UL && c.offset == 30000UL && c.length == 1UL);
    CK(cs_bq_owner_advance(&own) == CS_BQ_OK);
    keep = k;
    CK(cs_bq_chunk(&k, 32767UL, &c, &e) == CS_BQ_STALE && cs_bq_borrow(&k, 1, 10, &r, &e) == CS_BQ_STALE);
    CK(memcmp(&k, &keep, sizeof k) == 0);

    /* Owner open refuses an unusable binding without registry effect. */
    keep_reg = reg;
    envx = env1; memcpy(spx, sp1, sizeof spx); envx.spans = spx; spx[7].data = NULL;
    CK(cs_bq_owner_open(&reg, &envx, 10, &own2) == CS_BQ_BINDING);
    envx = env1; envx.expected_epoch = 3;
    CK(cs_bq_owner_open(&reg, &envx, 10, &own2) == CS_BQ_STALE);
    envx = env1; envx.map = &badmap;
    CK(cs_bq_owner_open(&reg, &envx, 10, &own2) == CS_BQ_MAP);
    envx = env1; envx.ram_first = 0x20000;
    CK(cs_bq_owner_open(&reg, &envx, 10, &own2) == CS_BQ_BACKING);
    CK(cs_bq_owner_open(&reg, NULL, 10, &own2) == CS_BQ_ARGUMENT);
    CK(memcmp(&reg, &keep_reg, sizeof reg) == 0);

    /* New lifetime retires the old owner and its capabilities. */
    CK(cs_bq_open(&own, 0, 2, 1, &k2) == CS_BQ_OK);
    CK(cs_bq_owner_open(&reg, &env1, 10, &own2) == CS_BQ_OK && own2.lifetime == 2UL);
    CK(cs_bq_borrow(&k2, 1, 10, &r, &e) == CS_BQ_STALE && cs_bq_open(&own, 0, 2, 1, &k) == CS_BQ_STALE);

    /* R2-1: an owner object re-opened in ANOTHER fresh registry gets lifetime 1
     * again, but capabilities opened under the first registry stay STALE. */
    CK(cs_bq_registry_init(&regb, 10) == CS_BQ_OK);
    CK(cs_bq_open(&own2, 0, 8, 1, &k) == CS_BQ_OK);
    CK(cs_bq_registry_init(&reg2, 10) == CS_BQ_OK && cs_bq_owner_open(&regb, &env1, 10, &own2) == CS_BQ_OK);
    CK(own2.lifetime == 1UL && cs_bq_borrow(&k, 1, 10, &r, &e) == CS_BQ_STALE);

    /* R2-1, numbers equal: lifetime 1 / generation 0 in two fresh registries.
     * Only the recorded registry identity tells the capabilities apart. */
    CK(cs_bq_registry_init(&rega1, 10) == CS_BQ_OK && cs_bq_registry_init(&regb1, 10) == CS_BQ_OK);
    CK(cs_bq_owner_open(&rega1, &env1, 10, &own3) == CS_BQ_OK && cs_bq_open(&own3, 0, 8, 1, &k) == CS_BQ_OK);
    CK(cs_bq_owner_open(&regb1, &env1, 10, &own3) == CS_BQ_OK && own3.lifetime == 1UL && own3.generation == 0UL);
    CK(rega1.current == 1UL && k.lifetime == 1UL && k.generation == 0UL);
    CK(cs_bq_borrow(&k, 1, 10, &r, &e) == CS_BQ_STALE && cs_bq_chunk(&k, 10, &c, &e) == CS_BQ_STALE);
    CK(cs_bq_open(&own3, 0, 8, 1, &k) == CS_BQ_OK && cs_bq_borrow(&k, 1, 10, &r, &e) == CS_BQ_OK);

    /* Exhaustion: lifetimes and generations never wrap. */
    CK(cs_bq_owner_open(&reg2, &env1, 1UL, &own) == CS_BQ_OK);
    reg2.limit = 1UL;
    keep_reg = reg2;
    CK(cs_bq_owner_open(&reg2, &env1, 1UL, &own2) == CS_BQ_EXHAUSTED && memcmp(&reg2, &keep_reg, sizeof reg2) == 0);
    CK(cs_bq_owner_advance(&own) == CS_BQ_OK && cs_bq_owner_advance(&own) == CS_BQ_EXHAUSTED && own.generation == 1UL);
    keep_own = own;
    CK(cs_bq_owner_rebind(&own, &env2) == CS_BQ_EXHAUSTED && memcmp(&own, &keep_own, sizeof own) == 0);
    CK(cs_bq_owner_advance(NULL) == CS_BQ_ARGUMENT);

    /* R2-2: two bindings, same map pointer, same epoch numbers. A capability
     * only ever reaches its owner's CURRENT binding; rebind advances first. */
    memset(&reg, 0, sizeof reg);  /* fresh storage: nothing references it now */
    CK(cs_bq_registry_init(&reg, 100) == CS_BQ_OK && cs_bq_owner_open(&reg, &env1, 100, &own) == CS_BQ_OK);
    CK(fill(&own, 0, 131072UL, 0x5A) && check(&own, 0, 131072UL, 0x5A));
    CK(sim[0x10000] == 0x5A && sim[0x80000] == 0x00);            /* env1 memory only */
    CK(cs_bq_open(&own, 0, 64, 1, &k) == CS_BQ_OK);
    keep_own = own;
    CK(cs_bq_owner_rebind(&own, NULL) == CS_BQ_ARGUMENT);
    envx = env2; envx.map = &other;
    CK(cs_bq_owner_rebind(&own, &envx) == CS_BQ_MAP);            /* other map identity */
    envx = env2; memcpy(spx, sp2, sizeof spx); envx.spans = spx; spx[2].bank = 9;
    CK(cs_bq_owner_rebind(&own, &envx) == CS_BQ_BINDING);
    CK(memcmp(&own, &keep_own, sizeof own) == 0 && cs_bq_borrow(&k, 1, 64, &r, &e) == CS_BQ_OK);
    CK(r.read == sim + 0x10000);                                 /* still env1 */
    CK(cs_bq_owner_rebind(&own, &env2) == CS_BQ_OK && own.generation == keep_own.generation + 1UL);
    CK(cs_bq_borrow(&k, 1, 64, &r, &e) == CS_BQ_STALE);          /* opened on env1 */
    CK(fill(&own, 0, 131072UL, 0x33) && check(&own, 0, 131072UL, 0x33));
    CK(sim[0x80000] == 0x33 && sim[0x10000] == 0x5A);            /* env2 written, env1 kept */

    /* Borrow runs: whole-region count, native split capping, refusal ladder. */
    CK(cs_bq_open(&own, 0, 131072UL, 3, &k) == CS_BQ_OK);
    for (a = 0UL, runs = 0UL;;) {
        CK(cs_bq_borrow(&k, 3, 32767UL, &r, &e) == CS_BQ_OK);
        if (!e) break;
        a += r.length; ++runs;
    }
    CK(a == 131072UL && runs == 7UL);
    CK(cs_bq_open(&own, 30001UL + 32767UL + 32760UL, 20, 1, &k) == CS_BQ_OK);
    CK(cs_bq_borrow(&k, 1, 32767UL, &r, &e) == CS_BQ_OK && r.length == 7UL);
    CK(cs_bq_borrow(&k, 1, 32767UL, &r, &e) == CS_BQ_OK && r.length == 1UL && r.offset == 32767UL);
    CK(cs_bq_borrow(&k, 1, 32767UL, &r, &e) == CS_BQ_OK && r.bank == 4UL && r.length == 12UL);
    CK(cs_bq_borrow(&k, 1, 32767UL, &r, &e) == CS_BQ_OK && e == 0u);
    CK(cs_bq_open(&own, 100, 50, 1, &k) == CS_BQ_OK);
    memset(&r, 0xA5, sizeof r);
    r0 = r; e = 0x77; e0 = e; keep = k;
    CK(cs_bq_borrow(&k, 3, 100, &r, &e) == CS_BQ_PERMISSION);
    CK(cs_bq_borrow(&k, 2, 100, &r, &e) == CS_BQ_ARGUMENT && cs_bq_borrow(&k, 1, 0, &r, &e) == CS_BQ_ARGUMENT);
    CK(cs_bq_borrow(&k, 1, 100, NULL, &e) == CS_BQ_ARGUMENT && cs_bq_borrow(NULL, 1, 100, &r, &e) == CS_BQ_ARGUMENT);
    sp2[7].data = NULL;                                           /* contract breach: revalidation catches it */
    CK(cs_bq_borrow(&k, 1, 100, &r, &e) == CS_BQ_BINDING);       /* mid-span revalidation */
    sp2[7].data = sim + 0xE0000;
    env2.map = &other;                                            /* contract breach: binding re-pointed */
    CK(cs_bq_borrow(&k, 1, 100, &r, &e) == CS_BQ_STALE);         /* owner map identity */
    env2.map = &map;
    CK(memcmp(&r, &r0, sizeof r) == 0 && e == e0 && memcmp(&k, &keep, sizeof k) == 0);
    CK(cs_bq_open(&own, 100, 1, 1, &k) == CS_BQ_OK && cs_bq_borrow(&k, 1, 100, &r, &e) == CS_BQ_OK);
    sp2[7].data = NULL;                                           /* end-of-span still validates */
    CK(cs_bq_borrow(&k, 1, 100, &r, &e) == CS_BQ_BINDING);       /* end after binding check */
    sp2[7].data = sim + 0xE0000;
    CK(cs_bq_borrow(&k, 1, 100, &r, &e) == CS_BQ_OK && e == 0u);

    if (fails) {
        printf("%d failure(s)\n", fails);
        return 1;
    }
    printf("PASS pc286 span-cap: %lu chunks checked\n", (unsigned long)chunks);
    return 0;
}
