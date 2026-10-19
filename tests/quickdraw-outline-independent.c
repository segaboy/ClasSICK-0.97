/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independent SPEC-0048 per-pixel inverse-map oracle. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "personality/system1/quickdraw-outline.h"

#define P sizeof(void *)
_Static_assert(sizeof(cs_qd_rect_port) == 1032u, "port size");
_Static_assert(offsetof(cs_qd_rect_port, pixels) == 0u, "pixels off");
_Static_assert(offsetof(cs_qd_rect_port, paints) == 1024u, "paints off");
_Static_assert(offsetof(cs_qd_rect_port, erases) == 1028u, "erases off");
_Static_assert(offsetof(cs_qd_view, port) == 0u, "view port off");
_Static_assert(offsetof(cs_qd_view, x) == P, "view x off");
_Static_assert(offsetof(cs_qd_view, y) == P + 4u, "view y off");
_Static_assert(offsetof(cs_qd_view, top) == P + 8u, "view top off");
_Static_assert(offsetof(cs_qd_view, left) == P + 12u, "view left off");
_Static_assert(offsetof(cs_qd_view, bottom) == P + 16u, "view bottom off");
_Static_assert(offsetof(cs_qd_view, right) == P + 20u, "view right off");
_Static_assert(sizeof(cs_qd_view) == P + 24u, "view size");

typedef struct {
    uint8_t c0[32];
    cs_qd_rect_port port;
    uint8_t c1[32];
    cs_qd_view view;
    uint8_t c2[32];
    uint8_t rect[8];
    uint8_t c3[32];
} Env;

static const char *cur = "init";
static int na, nf, ncase, nok, nref, npaint;
static int g_nv, g_nr, g_pn;
static uint32_t g_paints, g_erases = 0x12345678u, rs = 0x1234567u;

static void chk(int c, int line)
{
    na++;
    if (!c) {
        nf++;
        fprintf(stderr, "FAIL case=%s line=%d\n", cur, line);
    }
}
#define CK(c) chk((c) ? 1 : 0, __LINE__)

static uint32_t rnd(void)
{
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static int64_t sx(const uint8_t *p, int i)
{
    int64_t v = (int64_t)p[i] * 256 + (int64_t)p[i + 1];
    return v >= 32768 ? v - 65536 : v;
}

static void enc(uint8_t *p, int i, int v)
{
    unsigned u = (unsigned)((uint32_t)v & UINT32_C(0xFFFF));
    p[i] = (uint8_t)(u >> 8);
    p[i + 1] = (uint8_t)(u & 0xFFu);
}

static int vvalid(const cs_qd_view *v)
{
    return v && v->port && v->x >= -32768 && v->x <= 32767 && v->y >= -32768 &&
           v->y <= 32767 && v->top >= 0 && v->top <= 64 && v->bottom >= 0 &&
           v->bottom <= 64 && v->left >= 0 && v->left <= 128 && v->right >= 0 &&
           v->right <= 128;
}

/* Inverse map the physical pixel to logical space; measure to the original edges. */
static int cov(const cs_qd_view *v, const uint8_t *rp, int32_t w, int32_t h, int r, int c)
{
    int64_t t = sx(rp, 0), l = sx(rp, 2), b = sx(rp, 4), rr = sx(rp, 6);
    int64_t lr = (int64_t)r - v->y, lc = (int64_t)c - v->x;
    if (w <= 0 || h <= 0) return 0;
    if (r < v->top || r >= v->bottom || c < v->left || c >= v->right) return 0;
    if (lr < t || lr >= b || lc < l || lc >= rr) return 0;
    return lr - t < h || b - 1 - lr < h || lc - l < w || rr - 1 - lc < w;
}

static int pc(const Env *e)
{
    int n = 0, i, k;
    for (i = 0; i < 1024; i++)
        for (k = 0; k < 8; k++)
            if (e->port.pixels[i] & (0x80u >> k)) n++;
    return n;
}

static void mk(Env *e, int mode, int x, int y, int ct, int cl, int cb, int cr,
               int t, int l, int b, int r)
{
    int i;
    memset(e, 0xA5, sizeof *e);
    for (i = 0; i < 1024; i++) {
        uint8_t v = 0;
        if (mode == 1) v = 0xFF;
        else if (mode == 2) v = ((i / 16) & 1) ? 0x55 : 0xAA;
        else if (mode == 3) v = (uint8_t)(rnd() >> 7);
        e->port.pixels[i] = v;
    }
    e->port.paints = g_paints;
    e->port.erases = g_erases;
    g_paints = 0;
    g_erases = 0x12345678u;
    e->view.port = &e->port;
    e->view.x = x; e->view.y = y;
    e->view.top = ct; e->view.left = cl; e->view.bottom = cb; e->view.right = cr;
    enc(e->rect, 0, t); enc(e->rect, 2, l); enc(e->rect, 4, b); enc(e->rect, 6, r);
}

static int go(Env *e, int32_t w, int32_t h)
{
    cs_qd_view *vp;
    const uint8_t *rp;
    Env x;
    int ok, r, c, got;
    if (g_pn) e->view.port = NULL;
    vp = g_nv ? NULL : &e->view;
    rp = g_nr ? NULL : e->rect;
    g_nv = g_nr = g_pn = 0;
    memcpy(&x, e, sizeof x);
    ok = vvalid(vp) && rp && w >= 0 && w <= 32767 && h >= 0 && h <= 32767;
    if (ok) {
        int any = 0;
        for (r = 0; r < 64; r++)
            for (c = 0; c < 128; c++)
                if (cov(vp, rp, w, h, r, c)) {
                    x.port.pixels[r * 16 + (c >> 3)] |= (uint8_t)(0x80u >> (c & 7));
                    any = 1;
                }
        if (x.port.paints != UINT32_MAX) x.port.paints++;
        npaint += any;
    }
    ncase++;
    if (ok) nok++; else nref++;
    got = cs_qd_view_outline(vp, rp, w, h);
    CK(got == ok);
    if (memcmp(e, &x, sizeof x) != 0) {
        size_t i;
        for (i = 0; i < sizeof x; i++)
            if (((uint8_t *)e)[i] != ((uint8_t *)&x)[i]) break;
        fprintf(stderr, "DIFF case=%s first byte offset=%zu ok=%d\n", cur, i, ok);
        CK(0);
    } else CK(1);
    return pc(e);
}

static int sc(const char *nm, int mode, int x, int y, int ct, int cl, int cb, int cr,
              int t, int l, int b, int r, int32_t w, int32_t h)
{
    Env e;
    cur = nm;
    mk(&e, mode, x, y, ct, cl, cb, cr, t, l, b, r);
    return go(&e, w, h);
}

static int ed(int near, int mx)
{
    if (near) return (int)(rnd() % (unsigned)(mx + 17)) - 8;
    switch (rnd() % 8u) {
    case 0: return -32768;
    case 1: return 32767;
    case 2: return (int)(rnd() % 200u) - 40;
    case 3: return (int)(rnd() % 140u);
    case 4: return (int)(rnd() % 65535u) - 32767;
    case 5: return 0;
    case 6: return 64;
    default: return 128 - (int)(rnd() % 3u);
    }
}

static int ofs(int near)
{
    unsigned s = rnd() % 40u;
    if (near) return (int)(rnd() % 21u) - 10;
    if (s == 0) return -32768;
    if (s == 1) return 32767;
    if (s == 2) return 32768;
    if (s == 3) return -32769;
    return (int)(rnd() % 256u) - 100;
}

static int clp(int mx)
{
    unsigned s = rnd() % 40u;
    if (s == 0) return -1;
    if (s == 1) return mx + 1;
    if (s < 5) return 0;
    if (s < 8) return mx;
    return (int)(rnd() % (unsigned)(mx + 1));
}

static int32_t pen(void)
{
    unsigned s = rnd() % 40u;
    if (s == 0) return 0;
    if (s == 1) return 32767;
    if (s == 2) return -1;
    if (s == 3) return 32768;
    if (s == 4) return INT32_MIN;
    if (s == 5) return INT32_MAX;
    return (int32_t)(rnd() % 10u);
}

int main(void)
{
    static const int cl_[] = {0, 7, 8, 15, 16, 120, 127};
    static const int cr_[] = {1, 8, 9, 16, 17, 121, 128};
    static const int bad[][4] = {{32768, 0, 0, 0}, {-32769, 0, 0, 0}, {0, 32768, 0, 0},
                                 {0, -32769, 0, 0}, {INT32_MIN, 0, 0, 0}, {INT32_MAX, 0, 0, 0}};
    static const int32_t bp[] = {-1, INT32_MIN, 32768, INT32_MAX};
    static const int ex[] = {-32768, 32767};
    Env e, a, b, snap;
    int i, j, k, n;
    uint32_t p0;

    cur = "m1 null view"; g_nv = 1; sc(cur, 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    cur = "m2 null port"; g_pn = 1; sc(cur, 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    for (i = 0; i < 6; i++) {
        sc("m3 corrupt offset", 0, bad[i][0], bad[i][1], 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    }
    sc("m3 y=32768", 0, 0, 32768, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m3 y=-32769", 0, 0, -32769, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m3 x=INT32_MIN", 0, INT32_MIN, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m3 y=INT32_MAX", 0, 0, INT32_MAX, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 top=-1", 0, 0, 0, -1, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 top=65", 0, 0, 0, 65, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 left=-1", 0, 0, 0, 0, -1, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 left=129", 0, 0, 0, 0, 129, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 bottom=-1", 0, 0, 0, 0, 0, -1, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 bottom=65", 0, 0, 0, 0, 0, 65, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 right=-1", 0, 0, 0, 0, 0, 64, -1, 8, 8, 40, 100, 2, 2);
    sc("m4 right=129", 0, 0, 0, 0, 0, 64, 129, 8, 8, 40, 100, 2, 2);
    sc("m4 valid x/y bounds", 0, -32768, 32767, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m4 valid x/y bounds2", 0, 32767, -32768, 64, 128, 0, 0, 8, 8, 40, 100, 2, 2);
    g_nr = 1; sc("m5 null rect", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    for (i = 0; i < 4; i++) {
        sc("m6 pen w", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, bp[i], 2);
        sc("m6 pen h", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, bp[i]);
        sc("m6 pen both", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, bp[i], bp[i]);
    }
    g_nr = 1; sc("m8 corrupt view+null rect", 0, 32768, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    g_nv = 1; g_nr = 1; sc("m8 null view+null rect", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 2);
    sc("m9 w=0", 3, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 0, 2);
    sc("m9 h=0", 3, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 2, 0);
    CK(sc("m9 both pen 0", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 100, 0, 0) == 0);
    CK(sc("m10 inverted", 0, 0, 0, 0, 0, 64, 128, 40, 100, 8, 8, 2, 2) == 0);
    sc("m10 inverted rows only", 0, 0, 0, 0, 0, 64, 128, 40, 8, 8, 100, 2, 2);
    sc("m10 inverted cols only", 0, 0, 0, 0, 0, 64, 128, 8, 100, 40, 8, 2, 2);
    CK(sc("m11 zero height", 0, 0, 0, 0, 0, 64, 128, 8, 8, 8, 100, 2, 2) == 0);
    CK(sc("m11 zero width", 0, 0, 0, 0, 0, 64, 128, 8, 8, 40, 8, 2, 2) == 0);
    CK(sc("m12 full border 380", 0, 0, 0, 0, 0, 64, 128, 0, 0, 64, 128, 1, 1) == 380);
    CK(sc("m12 10x10 border 36", 0, 0, 0, 0, 0, 64, 128, 10, 10, 20, 20, 1, 1) == 36);
    sc("m13 asym", 0, 0, 0, 0, 0, 64, 128, 5, 9, 40, 100, 3, 2);
    sc("m13 asym offset", 0, 7, -3, 0, 0, 64, 128, 5, 9, 40, 100, 3, 2);
    CK(sc("m14 2w>=width", 0, 0, 0, 0, 0, 64, 128, 10, 10, 20, 20, 5, 5) == 100);
    CK(sc("m14 2h>=height", 0, 0, 0, 0, 0, 64, 128, 10, 10, 20, 60, 1, 5) == 500);
    CK(sc("m15 huge pen", 0, 0, 0, 0, 0, 64, 128, 3, 3, 13, 13, 32767, 32767) == 100);
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            for (k = 0; k < 2; k++) {
                int32_t pw = k ? 32767 : 1;
                sc("m16 extreme rect", 0, 0, 0, 0, 0, 64, 128, -32768, -32768, 32767, 32767, pw, pw);
                sc("m17 extreme offsets", 0, ex[i], ex[j], 0, 0, 64, 128, -32768, -32768, 32767, 32767, pw, pw);
                sc("m17 extreme off sw", 2, ex[i], ex[j], 0, 0, 64, 128, 32767, 32767, -32768, -32768, pw, pw);
                sc("m17 extreme off tiny", 1, ex[i], ex[j], 0, 0, 64, 128, -32768 - ex[i], -32768 - ex[j], -32767 - ex[i], -32767 - ex[j], pw, pw);
            }
    CK(sc("m18 left clipped", 0, 0, 0, 0, 20, 64, 128, 10, 10, 30, 60, 1, 1) == 98);
    sc("m18 left clip offscreen rect", 0, -5, 3, 0, 0, 64, 128, 0, 5, 30, 60, 1, 1);
    sc("m19 middle clip", 0, 0, 0, 20, 30, 40, 90, 10, 10, 60, 120, 4, 3);
    sc("m19 middle clip fill", 0, 0, 0, 20, 30, 40, 90, 10, 10, 60, 120, 99, 99);
    sc("m20 inverted clip", 3, 0, 0, 40, 100, 10, 20, 0, 0, 64, 128, 3, 3);
    sc("m20 empty clip", 3, 0, 0, 20, 20, 20, 90, 0, 0, 64, 128, 3, 3);
    sc("m20 zero-col clip", 3, 0, 0, 0, 128, 64, 128, 0, 0, 64, 128, 3, 3);
    sc("m21 offset vs clip", 0, 20, 10, 12, 25, 50, 70, 0, 0, 30, 50, 2, 2);
    sc("m21 neg offset vs clip", 0, -20, -10, 12, 25, 50, 70, 30, 40, 80, 120, 2, 2);
    for (i = 0; i < 7; i++)
        for (j = 0; j < 7; j++)
            if (cl_[i] < cr_[j]) {
                sc("m22 byte edge w1", 0, 0, 0, 0, 0, 64, 128, 5, cl_[i], 20, cr_[j], 1, 1);
                sc("m22 byte edge w3", 0, 0, 0, 0, 0, 64, 128, 5, cl_[i], 20, cr_[j], 3, 2);
                sc("m22 byte edge clip", 0, 0, 0, 0, cl_[i], 64, cr_[j], 0, 0, 64, 128, 2, 2);
            }
    for (i = 1; i <= 3; i++) {
        sc("m23 prefill", i, 0, 0, 0, 0, 64, 128, 6, 6, 50, 120, 3, 2);
        sc("m23 prefill clipped", i, 0, 0, 10, 10, 30, 70, 6, 6, 50, 120, 3, 2);
    }
    CK(sc("m23 black stays", 1, 0, 0, 0, 0, 64, 128, 6, 6, 50, 120, 3, 2) == 8192);
    cur = "m24 idempotent twice";
    mk(&e, 2, 0, 0, 0, 0, 64, 128, 6, 6, 50, 120);
    go(&e, 3, 2); memcpy(&snap, &e, sizeof snap); go(&e, 3, 2);
    CK(memcmp(e.port.pixels, snap.port.pixels, 1024) == 0);
    CK(e.port.paints == 2u);
    cur = "m25 paints saturate"; g_paints = UINT32_MAX - 1u;
    mk(&e, 0, 0, 0, 0, 0, 64, 128, 6, 6, 50, 120);
    go(&e, 3, 2); CK(e.port.paints == UINT32_MAX); go(&e, 3, 2); CK(e.port.paints == UINT32_MAX);
    cur = "m26 paints max empty"; g_paints = UINT32_MAX;
    mk(&e, 0, 0, 0, 0, 0, 64, 128, 50, 6, 6, 120);
    CK(go(&e, 3, 2) == 0); CK(e.port.paints == UINT32_MAX);
    cur = "m27 erases";
    p0 = 0;
    for (i = 0; i < 3; i++) {
        g_erases = i == 0 ? 0u : i == 1 ? UINT32_MAX : 0x80000000u; p0 = g_erases;
        mk(&e, 3, 0, 0, 0, 0, 64, 128, 6, 6, 50, 120);
        go(&e, 3, 2); CK(e.port.erases == p0);
    }
    cur = "m31 two ports";
    mk(&a, 2, 3, 4, 0, 0, 64, 128, 6, 6, 50, 120);
    mk(&b, 1, -2, 5, 5, 5, 60, 100, 0, 0, 64, 128);
    for (i = 0; i < 6; i++) {
        memcpy(&snap, (i & 1) ? &a : &b, sizeof snap);
        go((i & 1) ? &b : &a, 1 + i % 3, 1 + i % 2);
        CK(memcmp(&snap, (i & 1) ? &a : &b, sizeof snap) == 0);
    }
    CK(a.port.paints == 3u && b.port.paints == 3u);
    cur = "m34 random";
    for (n = 0; n < 640; n++) {
        int near = (int)(rnd() & 1u);
        int x = ofs(near), y = ofs(near);
        int ct = clp(64), cl = clp(128), cb = clp(64), cr = clp(128);
        int t = ed(near, 64), l = ed(near, 128), bt = ed(near, 64), r = ed(near, 128);
        int32_t w = pen(), h = pen();
        int mode = (int)(rnd() % 4u);
        uint32_t pv = (rnd() % 8u == 0) ? UINT32_MAX - rnd() % 3u : rnd();
        g_paints = pv; g_erases = rnd();
        mk(&e, mode, x, y, ct, cl, cb, cr, t, l, bt, r);
        g_nv = rnd() % 25u == 0; g_nr = rnd() % 25u == 1; g_pn = rnd() % 31u == 0;
        go(&e, w, h);
    }
    CK(ncase >= 512);
    CK(nok >= 128 && nref >= 64 && npaint >= 40);
    printf("cases=%d valid=%d refused=%d painting=%d assertions=%d failures=%d\n",
           ncase, nok, nref, npaint, na, nf);
    if (nf) {
        fprintf(stderr, "quickdraw-outline-independent: FAILED\n");
        return 1;
    }
    printf("quickdraw-outline-independent: PASS\n");
    return 0;
}
