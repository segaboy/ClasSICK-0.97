/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent per-pixel oracle for the SPEC-0052 cursor contract. Public API only. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/graphics/cursor.h"

#define MAXB 1229100u
#define CK(c) do { if (!(c)) { fails++; printf("FAIL line %d\n", __LINE__); } } while (0)
#define BIND() CK(cs_cursor_bind(&st, &sf, pay, gcap, &t) == CS_CURSOR_OK)
#define REFUSE(call, code) do { snap(); CK((call) == (code)); CK(unch()); } while (0)

typedef struct { cs_surface_format f; int32_t w, h; size_t stride; } lay;

static _Alignas(8) uint8_t store[MAXB];
static uint8_t base[MAXB], expd[MAXB], pay[1100], spay[1100];
static cs_surface sf;
static cs_cursor_state st, sst;
static size_t total, gcap;
static cs_rect exr;
static int fails;
static uint32_t rng = 12345u;

static const lay M = { CS_SURFACE_MONO1_MSB, 512, 342, 64 };
static const lay LS[] = {
    { CS_SURFACE_MONO1_MSB, 512, 342, 64 }, { CS_SURFACE_MONO1_MSB, 520, 350, 66 },
    { CS_SURFACE_MONO1_MSB, 509, 342, 66 }, { CS_SURFACE_MONO1_MSB, 300, 200, 38 },
    { CS_SURFACE_MONO1_MSB, 37, 5, 5 }, { CS_SURFACE_MONO1_MSB, 1, 1, 1 },
    { CS_SURFACE_RGBA8, 640, 480, 2560 }, { CS_SURFACE_RGBA8, 512, 342, 2052 },
    { CS_SURFACE_RGBA8, 100, 80, 404 }, { CS_SURFACE_RGBA8, 513, 343, 2056 } };

static uint32_t rnd(void) { rng = rng * 1664525u + 1013904223u; return rng >> 8; }
static int32_t gb(const uint8_t *a, int32_t r, int32_t c) { return (a[2 * r + c / 8] >> (7 - c % 8)) & 1; }
static void sb(uint8_t *a, int32_t r, int32_t c) { a[2 * r + c / 8] = (uint8_t)(a[2 * r + c / 8] | (0x80 >> (c % 8))); }
static int32_t org(int32_t d, int32_t n) { return d > n ? (d - n) / 2 : 0; }
static int32_t mn(int32_t a, int32_t b) { return a < b ? a : b; }
static int32_t mx(int32_t a, int32_t b) { return a > b ? a : b; }

static void setup(const lay *L) {
    size_t i;
    total = L->stride * (size_t)L->h + 16u;
    for (i = 0; i < total; i++) store[i] = (uint8_t)(rnd() >> 3);
    memcpy(base, store, total);
    memset(pay, 0xA5, sizeof pay);
    gcap = L->f == CS_SURFACE_MONO1_MSB ? 48u : 1024u;
    CK(cs_surface_init(&sf, store, total, L->w, L->h, L->stride, L->f) == CS_SURFACE_OK);
    CK(cs_cursor_init(&st) == CS_CURSOR_OK);
}

/* Fresh per-pixel write, derived only from the SPEC art/format rules. */
static void wpix(uint8_t *m, const lay *L, int32_t x, int32_t y, int32_t one) {
    size_t o = (size_t)y * L->stride;
    if (L->f == CS_SURFACE_MONO1_MSB) {
        uint8_t b = (uint8_t)(0x80u >> (unsigned)(x % 8));
        o += (size_t)x / 8u;
        if (one) m[o] = (uint8_t)(m[o] | b); else m[o] = (uint8_t)(m[o] & (uint8_t)~b);
    } else {
        uint8_t v = one ? 0x00 : 0xFF;
        o += (size_t)x * 4u;
        m[o] = v; m[o + 1] = v; m[o + 2] = v; m[o + 3] = 0xFF;
    }
}

static void ref(const lay *L, const cs_cursor_art *a, int32_t x, int32_t y) {
    int32_t ox = org(L->w, 512), oy = org(L->h, 342), i, j;
    int32_t l = x - a->hot_x, t = y - a->hot_y, r = l + a->width, b = t + a->height;
    memcpy(expd, base, total);
    for (j = 0; j < a->height; j++)
        for (i = 0; i < a->width; i++) {
            int32_t lx = l + i, ly = t + j;
            if (!gb(a->bits->mask, j, i)) continue;
            if (lx < 0 || lx >= 512 || ly < 0 || ly >= 342) continue;
            if (ox + lx >= L->w || oy + ly >= L->h) continue;
            wpix(expd, L, ox + lx, oy + ly, gb(a->bits->image, j, i));
        }
    l = mx(l, 0) + ox; r = mn(mn(r, 512) + ox, L->w);
    t = mx(t, 0) + oy; b = mn(mn(b, 342) + oy, L->h);
    if (l >= r || t >= b) { l = 0; t = 0; r = 0; b = 0; }
    exr.left = l; exr.top = t; exr.right = r; exr.bottom = b;
}

static cs_cursor_info qi(void) {
    cs_cursor_info i;
    memset(&i, 0, sizeof i);
    CK(cs_cursor_query(&st, &i) == CS_CURSOR_OK);
    return i;
}

static int rect_is(cs_rect r, int32_t l, int32_t t, int32_t rr, int32_t b) {
    return r.left == l && r.top == t && r.right == rr && r.bottom == b;
}

/* Whole-storage compare (incl. padding/tail), saved rect, phase, payload guard. */
static void expect_vis(const lay *L, const cs_cursor_art *a, int32_t x, int32_t y) {
    cs_cursor_info in;
    size_t i;
    ref(L, a, x, y);
    CK(memcmp(store, expd, total) == 0);
    in = qi();
    CK(in.phase == CS_CURSOR_VISIBLE);
    CK(rect_is(in.saved, exr.left, exr.top, exr.right, exr.bottom));
    for (i = gcap; i < sizeof pay; i++) if (pay[i] != 0xA5) { CK(0); break; }
}

static void mk(cs_cursor_bits *b, cs_cursor_art *a, int32_t w, int32_t h, int32_t hx, int32_t hy, int mode) {
    int32_t r, c;
    memset(b, 0, sizeof *b);
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            int m = 1, im = (r + c) & 1;
            if (mode == 0) { m = (rnd() % 4u) != 0u; im = (int)(rnd() & 1u); }
            if (mode == 2) m = r > 0 && c > 0 && r < h - 1 && c < w - 1;
            if (m) sb(b->mask, r, c);
            if (m && im) sb(b->image, r, c);
        }
    a->bits = b; a->width = (uint8_t)w; a->height = (uint8_t)h;
    a->hot_x = (uint8_t)hx; a->hot_y = (uint8_t)hy;
}

static void seq(const lay *L, const cs_cursor_art *a, int32_t x, int32_t y) {
    cs_cursor_token t;
    setup(L);
    CK(cs_cursor_bind(&st, &sf, pay, gcap, &t) == CS_CURSOR_OK);
    CK(cs_cursor_show(&st, t, a, x, y) == CS_CURSOR_OK);
    expect_vis(L, a, x, y);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);
    CK(qi().phase == CS_CURSOR_HIDDEN);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
}

static void det(const lay *L) {
    static const int32_t P[5][5] = { {16, 16, 0, 0, 1}, {16, 16, 15, 15, 2}, {1, 1, 0, 0, 1}, {16, 16, 15, 15, 1}, {7, 9, 3, 8, 0} };
    cs_cursor_bits b;
    cs_cursor_art a;
    int32_t k, c, r;
    for (k = 0; k < 5; k++) {
        mk(&b, &a, P[k][0], P[k][1], P[k][2], P[k][3], (int)P[k][4]);
        for (c = 0; c < 4; c++) seq(L, &a, (c & 1) ? 511 : 0, (c & 2) ? 341 : 0);
        seq(L, &a, 400, 300);
        seq(L, &a, 255, 171);
    }
    mk(&b, &a, 16, 16, 0, 0, 1);
    for (r = 0; r < 8; r++) { seq(L, &a, 24 + r, 10 + r); seq(L, &a, 488 + r, 100); }
    mk(&b, &a, 16, 16, 0, 0, 2);
    for (r = 0; r < 8; r++) seq(L, &a, 24 + r, 200 + r);
}

static int32_t rpos(int32_t n) {
    uint32_t k = rnd() % 3u;
    if (k == 0u) return (int32_t)(rnd() % 20u);
    if (k == 1u) return n - 1 - (int32_t)(rnd() % 20u);
    return (int32_t)(rnd() % (uint32_t)n);
}

static void motion(const lay *L, int n) {
    cs_cursor_bits b;
    cs_cursor_art a;
    cs_cursor_token t;
    int32_t x, y, w, h;
    int i;
    setup(L);
    CK(cs_cursor_bind(&st, &sf, pay, gcap, &t) == CS_CURSOR_OK);
    for (i = 0; i <= n; i++) {
        w = 1 + (int32_t)(rnd() % 16u); h = 1 + (int32_t)(rnd() % 16u);
        mk(&b, &a, w, h, (int32_t)(rnd() % (uint32_t)w), (int32_t)(rnd() % (uint32_t)h), 0);
        x = rpos(512); y = rpos(342);
        if (i == 0) CK(cs_cursor_show(&st, t, &a, x, y) == CS_CURSOR_OK);
        else CK(cs_cursor_move(&st, t, &a, x, y) == CS_CURSOR_OK);
        expect_vis(L, &a, x, y);
    }
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
}

static void snap(void) { memcpy(&sst, &st, sizeof st); memcpy(expd, store, total); memcpy(spay, pay, sizeof pay); }
static int unch(void) {
    return memcmp(&sst, &st, sizeof st) == 0 && memcmp(expd, store, total) == 0 && memcmp(spay, pay, sizeof pay) == 0;
}
static void poke(size_t o, int both) {
    store[o] = (uint8_t)~store[o];
    if (both) base[o] = store[o];
}

static void life(void) {
    cs_cursor_bits b, bad;
    cs_cursor_art a, a8, ba;
    cs_cursor_token t, t2, o;
    cs_cursor_info in;
    cs_rect r;
    cs_surface sf2;
    cs_rect all = { 0, 0, 512, 342 };
    mk(&b, &a, 16, 16, 0, 0, 1);
    mk(&bad, &a8, 8, 8, 0, 0, 1);

    setup(&M);
    CK(cs_cursor_init(NULL) == CS_CURSOR_ERR_ARGUMENT);
    in = qi();
    CK(in.phase == CS_CURSOR_UNBOUND && in.token.binding == 0u && in.token.revision == 0u && rect_is(in.saved, 0, 0, 0, 0));
    REFUSE(cs_cursor_bind(&st, NULL, NULL, 0, &t), CS_CURSOR_ERR_ARGUMENT);
    REFUSE(cs_cursor_bind(&st, &sf, pay, 47, &t), CS_CURSOR_ERR_CAPACITY);
    sf2 = sf; sf2.stride = 63;
    REFUSE(cs_cursor_bind(&st, &sf2, pay, 48, &t), CS_CURSOR_ERR_SURFACE);
    sf2 = sf; sf2.width = 8193;
    REFUSE(cs_cursor_bind(&st, &sf2, pay, 48, &t), CS_CURSOR_ERR_SURFACE);
    sf2 = sf; sf2.storage_size = sf.stride * (size_t)sf.height - 1u;
    REFUSE(cs_cursor_bind(&st, &sf2, pay, 48, &t), CS_CURSOR_ERR_SURFACE);
    REFUSE(cs_cursor_bind(&st, &sf, store + total - 48, 48, &t), CS_CURSOR_ERR_ALIAS);
    REFUSE(cs_cursor_bind(&st, &sf, pay, 48, (cs_cursor_token *)(store + 8)), CS_CURSOR_ERR_ALIAS);
    CK(qi().phase == CS_CURSOR_UNBOUND);
    BIND();
    CK(t.binding == 1u && t.revision == 1u && qi().phase == CS_CURSOR_HIDDEN);

    REFUSE(cs_cursor_show(&st, t, NULL, 10, 10), CS_CURSOR_ERR_ARGUMENT);
    ba = a8; ba.bits = NULL;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); bad.mask[0] = (uint8_t)(bad.mask[0] & 0x7Fu); bad.image[0] = (uint8_t)(bad.image[0] | 0x80u);
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    REFUSE(cs_cursor_show(&st, t, &ba, 600, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); bad.mask[1] = 0x01;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); bad.image[2 * 9] = 0x80;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); ba.width = 17;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); ba.hot_x = 8;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); ba.hot_y = 8;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    mk(&bad, &ba, 8, 8, 0, 0, 1); ba.height = 0;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ART);
    REFUSE(cs_cursor_show(&st, t, &a, 512, 10), CS_CURSOR_ERR_COORD);
    REFUSE(cs_cursor_show(&st, t, &a, 10, -1), CS_CURSOR_ERR_COORD);
    REFUSE(cs_cursor_show(&st, t, &a, 10, 342), CS_CURSOR_ERR_COORD);
    ba = a; ba.bits = (const cs_cursor_bits *)store;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ALIAS);
    ba.bits = (const cs_cursor_bits *)pay;
    REFUSE(cs_cursor_show(&st, t, &ba, 10, 10), CS_CURSOR_ERR_ALIAS);
    ba = a8; sf.width = 0;
    REFUSE(cs_cursor_show(&st, t, &ba, 600, 10), CS_CURSOR_ERR_SURFACE);
    sf.width = 512;
    t2 = t; t2.revision = 9u;
    REFUSE(cs_cursor_show(&st, t2, &a, 10, 10), CS_CURSOR_ERR_STALE);
    REFUSE(cs_cursor_advance(&st, t2, &o), CS_CURSOR_ERR_STALE);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_STATE);
    REFUSE(cs_cursor_move(&st, t, &a, 10, 10), CS_CURSOR_ERR_STATE);
    REFUSE(cs_cursor_repaired(&st, t, all, &o), CS_CURSOR_ERR_STATE);

    CK(cs_cursor_show(&st, t, &a, 100, 100) == CS_CURSOR_OK);
    expect_vis(&M, &a, 100, 100);
    REFUSE(cs_cursor_show(&st, t, &a, 100, 100), CS_CURSOR_ERR_STATE);
    REFUSE(cs_cursor_advance(&st, t, &o), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_unbind(&st, t), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_advance(&st, t2, &o), CS_CURSOR_ERR_STALE);
    REFUSE(cs_cursor_move(&st, t2, &a, 100, 100), CS_CURSOR_ERR_STALE);
    CK(cs_cursor_move(&st, t, &a, 100, 100) == CS_CURSOR_OK);
    expect_vis(&M, &a, 100, 100);
    CK(cs_cursor_move(&st, t, &a, 104, 102) == CS_CURSOR_OK);
    expect_vis(&M, &a, 104, 102);
    CK(cs_cursor_move(&st, t, &a8, 104, 102) == CS_CURSOR_OK);
    expect_vis(&M, &a8, 104, 102);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_STATE);

    poke(100u * 64u + 20u, 1);
    CK(cs_cursor_advance(&st, t, &o) == CS_CURSOR_OK && o.revision == t.revision + 1u);
    REFUSE(cs_cursor_show(&st, t, &a, 100, 100), CS_CURSOR_ERR_STALE);
    t = o;
    CK(cs_cursor_show(&st, t, &a, 100, 100) == CS_CURSOR_OK);
    expect_vis(&M, &a, 100, 100);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);

    CK(cs_cursor_show(&st, t, &a, 30, 40) == CS_CURSOR_OK);
    poke(300u * 64u + 5u, 1);
    memset(&r, 0xEE, sizeof r);
    CK(cs_cursor_discard(&st, t, &r) == CS_CURSOR_OK);
    CK(rect_is(r, 30, 40, 46, 56) && qi().phase == CS_CURSOR_REPAIR);
    REFUSE(cs_cursor_show(&st, t, &a, 30, 40), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_move(&st, t, &a, 31, 40), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_advance(&st, t, &o), CS_CURSOR_ERR_BUSY);
    REFUSE(cs_cursor_unbind(&st, t), CS_CURSOR_ERR_BUSY);
    { cs_rect c1 = { 30, 40, 45, 56 }, c2 = { 46, 40, 30, 56 };
      REFUSE(cs_cursor_repaired(&st, t, c1, &o), CS_CURSOR_ERR_COVERAGE);
      REFUSE(cs_cursor_repaired(&st, t, c2, &o), CS_CURSOR_ERR_COVERAGE); }
    memcpy(store, base, total);
    { cs_rect c3 = { 20, 30, 60, 70 };
      CK(cs_cursor_repaired(&st, t, c3, &o) == CS_CURSOR_OK); }
    CK(o.binding == t.binding && o.revision == t.revision + 1u);
    in = qi();
    CK(in.phase == CS_CURSOR_HIDDEN && rect_is(in.saved, 0, 0, 0, 0));
    REFUSE(cs_cursor_show(&st, t, &a, 30, 40), CS_CURSOR_ERR_STALE);
    t = o;
    CK(cs_cursor_show(&st, t, &a, 30, 40) == CS_CURSOR_OK);
    expect_vis(&M, &a, 30, 40);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);

    memset(&r, 0xEE, sizeof r);
    CK(cs_cursor_discard(&st, t, &r) == CS_CURSOR_OK && rect_is(r, 0, 0, 0, 0) && qi().phase == CS_CURSOR_REPAIR);
    { cs_rect e = { 0, 0, 0, 0 };
      CK(cs_cursor_repaired(&st, t, e, &o) == CS_CURSOR_OK); }
    t = o;
    CK(cs_cursor_show(&st, t, &a, 5, 5) == CS_CURSOR_OK);
    expect_vis(&M, &a, 5, 5);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);

    st.revision = 0xFFFFFFFFu; t.revision = 0xFFFFFFFFu;
    REFUSE(cs_cursor_advance(&st, t, &o), CS_CURSOR_ERR_EXHAUSTED);
    CK(cs_cursor_show(&st, t, &a, 5, 5) == CS_CURSOR_OK);
    CK(cs_cursor_discard(&st, t, &r) == CS_CURSOR_OK);
    memcpy(store, base, total);
    CK(cs_cursor_repaired(&st, t, all, &o) == CS_CURSOR_OK);
    in = qi();
    CK(o.revision == 0xFFFFFFFFu && in.fault == 1u && in.phase == CS_CURSOR_HIDDEN);
    t = o;
    REFUSE(cs_cursor_show(&st, t, &a, 5, 5), CS_CURSOR_ERR_EXHAUSTED);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
    BIND();
    CK(t.binding == 2u && t.revision == 1u);
    REFUSE(cs_cursor_show(&st, o, &a, 5, 5), CS_CURSOR_ERR_STALE);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
    in = qi();
    CK(in.phase == CS_CURSOR_UNBOUND && in.token.binding == 0u);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_STATE);

    setup(&M);
    st.next_binding = 0xFFFFFFFFu;
    BIND();
    CK(t.binding == 0xFFFFFFFFu && t.revision == 1u && st.next_binding == 0u);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
    REFUSE(cs_cursor_bind(&st, &sf, pay, gcap, &t), CS_CURSOR_ERR_EXHAUSTED);

    setup(&M);
    BIND();
    CK(cs_cursor_show(&st, t, &a, 100, 100) == CS_CURSOR_OK);
    st.reserved = 1;
    REFUSE(cs_cursor_query(&st, &in), CS_CURSOR_ERR_STATE);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_STATE);
    st.reserved = 0;
    st.save_right = (int16_t)(st.save_left + 17);
    REFUSE(cs_cursor_query(&st, &in), CS_CURSOR_ERR_STATE);
    st.save_right = (int16_t)(st.save_left + 16);
    st.fault = 1;
    REFUSE(cs_cursor_query(&st, &in), CS_CURSOR_ERR_STATE);
    st.fault = 0;
    sf.width = 511;
    REFUSE(cs_cursor_move(&st, t, &a, 101, 100), CS_CURSOR_ERR_SURFACE);
    REFUSE(cs_cursor_hide(&st, t), CS_CURSOR_ERR_SURFACE);
    CK(qi().phase == CS_CURSOR_VISIBLE);
    sf.width = 512;
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(store, base, total) == 0);
    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
    { cs_cursor_state zero;
      memset(&zero, 0, sizeof zero);
      REFUSE(cs_cursor_hide(&zero, t), CS_CURSOR_ERR_STATE); }

    setup(&M);
    BIND();
    REFUSE(cs_cursor_query(&st, (cs_cursor_info *)&st), CS_CURSOR_ERR_ALIAS);
    REFUSE(cs_cursor_discard(&st, t, (cs_rect *)&st), CS_CURSOR_ERR_ALIAS);

    CK(cs_cursor_unbind(&st, t) == CS_CURSOR_OK);
    { cs_surface *hs = malloc(sizeof *hs);
      uint8_t *hst = malloc(64u * 342u), *hp = malloc(48u);
      if (!hs || !hst || !hp) { CK(0); free(hs); free(hst); free(hp); return; }
      memset(hst, 0, 64u * 342u);
      CK(cs_surface_init(hs, hst, 64u * 342u, 512, 342, 64, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
      CK(cs_cursor_init(&st) == CS_CURSOR_OK);
      CK(cs_cursor_bind(&st, hs, hp, 48, &t) == CS_CURSOR_OK);
      CK(cs_cursor_show(&st, t, &a, 100, 100) == CS_CURSOR_OK);
      /* Quarantine while allocations remain live; no dangling-pointer values.
       * Actual expired-allocation behavior is not claimed by this hosted case. */
      hs->storage = NULL; hs->width = 0; hs->stride = 0;
      in = qi();
      CK(in.phase == CS_CURSOR_VISIBLE && rect_is(in.saved, 100, 100, 116, 116));
      memset(&r, 0xEE, sizeof r);
      CK(cs_cursor_abandon(&st, &r) == CS_CURSOR_OK && rect_is(r, 100, 100, 116, 116));
      in = qi();
      CK(in.phase == CS_CURSOR_UNBOUND && in.token.binding == 0u);
      REFUSE(cs_cursor_abandon(&st, &r), CS_CURSOR_ERR_STATE);
      free(hp); free(hst); free(hs);
      CK(cs_cursor_bind(&st, &sf, pay, gcap, &t) == CS_CURSOR_OK && t.binding == 2u);
    }
}

/* Codex supplement: padding is not scene content; changing it between calls
 * must survive masked restore. Original external oracle above is unchanged. */
static void padding_prefix(void) {
    const lay L = { CS_SURFACE_MONO1_MSB, 509, 342, 66 };
    cs_cursor_bits b; cs_cursor_art a; cs_cursor_token t; int32_t y; size_t i;
    setup(&L); mk(&b,&a,16,16,0,0,1);
    CK(cs_cursor_bind(&st,&sf,pay,gcap,&t)==CS_CURSOR_OK);
    CK(cs_cursor_show(&st,t,&a,500,10)==CS_CURSOR_OK);
    for(y=10;y<26;y++) {
        size_t q=(size_t)y*L.stride+63u;
        store[q]^=7u; base[q]^=7u; /* Bits509..511 are outside the frame. */
    }
    expect_vis(&L,&a,500,10);
    CK(cs_cursor_hide(&st,t)==CS_CURSOR_OK);
    CK(memcmp(store,base,total)==0);
    CK(cs_cursor_unbind(&st,t)==CS_CURSOR_OK);
    setup(&M); mk(&b,&a,1,1,0,0,1);
    CK(cs_cursor_bind(&st,&sf,pay,gcap,&t)==CS_CURSOR_OK);
    CK(cs_cursor_show(&st,t,&a,511,341)==CS_CURSOR_OK);
    for(i=1;i<gcap;i++) CK(pay[i]==0xA5);
    CK(cs_cursor_hide(&st,t)==CS_CURSOR_OK);
    CK(cs_cursor_unbind(&st,t)==CS_CURSOR_OK);
    { const lay R={CS_SURFACE_RGBA8,512,342,2048};
      setup(&R); CK(cs_cursor_bind(&st,&sf,pay,gcap,&t)==CS_CURSOR_OK);
      CK(cs_cursor_show(&st,t,&a,511,341)==CS_CURSOR_OK);
      for(i=4;i<gcap;i++) CK(pay[i]==0xA5);
      CK(cs_cursor_hide(&st,t)==CS_CURSOR_OK);
      CK(memcmp(store,base,total)==0);
      CK(cs_cursor_unbind(&st,t)==CS_CURSOR_OK); }
}

int main(void) {
    size_t i;
    for (i = 0; i < sizeof LS / sizeof LS[0]; i++) { det(&LS[i]); motion(&LS[i], 40); }
    life();
    padding_prefix();
    printf("cursor-independent: %d failing checks\n", fails);
    return fails != 0;
}
