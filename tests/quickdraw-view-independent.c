/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independent SPEC-0041 view tests. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../personality/system1/quickdraw-view.h"

static int fails;
#define CK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

typedef struct { uint8_t pre[16]; cs_qd_rect_port p; uint8_t post[16]; } fx;

static uint32_t rs = 0x2468ACE1u;
static uint32_t nxt(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static const int32_t EDG[] = { -32768, -32767, -129, -65, -1, 0, 1, 7, 8, 9, 63, 64, 65, 127, 128, 129, 32766, 32767 };
#define NEDG (sizeof EDG / sizeof EDG[0])
static int32_t rpick(void) {
    switch (nxt() % 5u) {
    case 0: return (int32_t)(nxt() % 301u) - 100;
    case 1: return (int32_t)(nxt() & 0xFFFFu) - 32768;
    case 2: return EDG[nxt() % NEDG];
    case 3: return (int32_t)(nxt() % 140u) - 5;
    default: return (int32_t)(nxt() % 72u) - 4;
    }
}
static void put16(uint8_t *p, int32_t v) {
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u >> 8); p[1] = (uint8_t)(u & 0xFFu);
}
static void enc(uint8_t r[8], const int32_t q[4]) {
    int i;
    for (i = 0; i < 4; i++) put16(r + 2 * i, q[i]);
}
static int32_t clampi(int32_t v, int32_t lo, int32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void fx_init(fx *f, int mode) {
    size_t i;
    memset(f->pre, 0xA5, sizeof f->pre);
    memset(f->post, 0xA5, sizeof f->post);
    for (i = 0; i < CS_QD_RECT_BYTES; i++)
        f->p.pixels[i] = mode == 0 ? 0u : (mode == 1 ? 0xFFu : (uint8_t)(nxt() >> 11));
    f->p.paints = 0; f->p.erases = 0;
}
static int guards_ok(const fx *f) {
    size_t i;
    for (i = 0; i < 16; i++) if (f->pre[i] != 0xA5u || f->post[i] != 0xA5u) return 0;
    return 1;
}
static int view_eq(const cs_qd_view *a, const cs_qd_view *b) {
    return a->port == b->port && a->x == b->x && a->y == b->y && a->top == b->top &&
           a->left == b->left && a->bottom == b->bottom && a->right == b->right;
}
static int fx_eq(const fx *a, const fx *b) {
    return memcmp(a->pre,b->pre,sizeof a->pre)==0 && memcmp(a->post,b->post,sizeof a->post)==0 &&
        memcmp(a->p.pixels,b->p.pixels,sizeof a->p.pixels)==0 && a->p.paints==b->p.paints && a->p.erases==b->p.erases;
}

/* Per-pixel inverse membership: physical pixel -> logical coordinate in int64. */
static void model(uint8_t *img, const cs_qd_view *v, const int32_t q[4], uint16_t w) {
    int px, py;
    for (py = 0; py < 64; py++) for (px = 0; px < 128; px++) {
        int64_t ly = (int64_t)py - v->y, lx = (int64_t)px - v->x;
        if (py >= v->top && py < v->bottom && px >= v->left && px < v->right &&
            ly >= q[0] && ly < q[2] && lx >= q[1] && lx < q[3]) {
            size_t i = (size_t)py * 16u + (size_t)(px / 8);
            uint8_t m = (uint8_t)(0x80u >> (px % 8));
            if (w == CS_QD_PAINT_RECT) img[i] = (uint8_t)(img[i] | m);
            else img[i] = (uint8_t)(img[i] & (uint8_t)~m);
        }
    }
}

/* Draw through view and verify image, counts, guards, saturation, unchanged view. */
static void draw_check(fx *f, cs_qd_view *v, const int32_t q[4], uint16_t w) {
    uint8_t exp[CS_QD_RECT_BYTES], r[8];
    cs_qd_view vc = *v;
    uint32_t pc = f->p.paints, ec = f->p.erases;
    int ret;
    memcpy(exp, f->p.pixels, sizeof exp);
    model(exp, v, q, w);
    if (w == CS_QD_PAINT_RECT) { if (pc != UINT32_MAX) pc++; }
    else if (ec != UINT32_MAX) ec++;
    enc(r, q);
    ret = cs_qd_view_rect(v, r, w);
    CK(ret == 1);
    CK(memcmp(exp, f->p.pixels, sizeof exp) == 0);
    CK(f->p.paints == pc);
    CK(f->p.erases == ec);
    CK(guards_ok(f));
    CK(view_eq(v, &vc));
}

static void setup(fx *f, cs_qd_view *v, int32_t x, int32_t y, const int32_t c[4]) {
    uint8_t r[8];
    enc(r, c);
    CK(cs_qd_view_init(v, &f->p) == 1);
    CK(cs_qd_view_offset(v, x, y) == 1);
    CK(cs_qd_view_clip(v, r) == 1);
    CK(v->port == &f->p && v->x == x && v->y == y);
    CK(v->top == clampi(c[0], 0, 64) && v->left == clampi(c[1], 0, 128));
    CK(v->bottom == clampi(c[2], 0, 64) && v->right == clampi(c[3], 0, 128));
}

static void suite_oracle(void) {
    int i, a, b;
    /* 256 deterministic random cases, two draws each (paint then erase or reverse) */
    for (i = 0; i < 256; i++) {
        fx f; cs_qd_view v; int32_t c[4], q[4];
        int k;
        fx_init(&f, (int)(nxt() % 3u));
        for (k = 0; k < 4; k++) { c[k] = rpick(); q[k] = rpick(); }
        setup(&f, &v, rpick(), rpick(), c);
        draw_check(&f, &v, q, (i & 1) ? CS_QD_PAINT_RECT : CS_QD_ERASE_RECT);
        for (k = 0; k < 4; k++) q[k] = rpick();
        draw_check(&f, &v, q, (i & 1) ? CS_QD_ERASE_RECT : CS_QD_PAINT_RECT);
    }
    {   /* directed: byte boundaries, offset with default clip */
        static const int32_t col[] = { 0, 1, 7, 8, 9, 15, 16, 17, 127, 128 };
        for (a = 0; a < 10; a++) for (b = 0; b < 10; b++) {
            fx f; cs_qd_view v; int32_t full[4] = { 0, 0, 64, 128 };
            int32_t q[4]; q[0] = 3; q[1] = col[a]; q[2] = 5; q[3] = col[b];
            fx_init(&f, 0);
            setup(&f, &v, 0, 0, full);
            draw_check(&f, &v, q, CS_QD_PAINT_RECT);
            fx_init(&f, 1);
            setup(&f, &v, 3, -2, full);
            draw_check(&f, &v, q, CS_QD_ERASE_RECT);
        }
    }
    {   /* workload-shaped: offset 8, clip 8..56x8..120, extreme paint then erase */
        fx f; cs_qd_view v; int32_t c[4] = { 8, 8, 56, 120 };
        int32_t big[4] = { -32768, -32768, 32767, 32767 }, in[4] = { 8, 8, 40, 104 };
        fx_init(&f, 0);
        setup(&f, &v, 8, 8, c);
        draw_check(&f, &v, big, CS_QD_PAINT_RECT);
        draw_check(&f, &v, in, CS_QD_ERASE_RECT);
        CK(f.p.paints == 1 && f.p.erases == 1);
        CK(f.p.pixels[7 * 16 + 5] == 0 && f.p.pixels[56 * 16 + 5] == 0);
        CK(f.p.pixels[8 * 16 + 0] == 0 && f.p.pixels[8 * 16 + 15] == 0);
        CK(f.p.pixels[10 * 16 + 4] == 0xFFu);
    }
    {   /* saturation: counter stays at max, other counter still counts */
        fx f; cs_qd_view v; int32_t full[4] = { 0, 0, 64, 128 }, q[4] = { 0, 0, 1, 1 };
        fx_init(&f, 0);
        setup(&f, &v, 0, 0, full);
        f.p.paints = UINT32_MAX; f.p.erases = UINT32_MAX - 1u;
        draw_check(&f, &v, q, CS_QD_PAINT_RECT);
        draw_check(&f, &v, q, CS_QD_ERASE_RECT);
        draw_check(&f, &v, q, CS_QD_ERASE_RECT);
        CK(f.p.paints == UINT32_MAX && f.p.erases == UINT32_MAX);
    }
}

static void suite_refusal(void) {
    fx f, g; cs_qd_view v, vc; uint8_t r[8] = { 0, 0, 0, 10, 0, 10, 0, 10 };
    int32_t full[4] = { 0, 0, 64, 128 };
    const uint16_t bad[] = { 0, 0xA8A1u, 0xA8A4u, 0xA8A2u ^ 0x0100u, 0xFFFFu };
    size_t i;
    fx_init(&f, 2);
    g = f;
    CK(cs_qd_view_init(NULL, &f.p) == 0);
    memset(&v, 0x5A, sizeof v); vc = v;
    CK(cs_qd_view_init(&v, NULL) == 0);
    CK(memcmp(&v, &vc, sizeof v) == 0);
    CK(cs_qd_view_offset(NULL, 1, 1) == 0);
    CK(cs_qd_view_clip(NULL, r) == 0);
    CK(cs_qd_view_rect(NULL, r, CS_QD_PAINT_RECT) == 0);
    CK(fx_eq(&f, &g));
    setup(&f, &v, 3, 4, full);
    g = f; vc = v;
    CK(cs_qd_view_clip(&v, NULL) == 0);
    CK(cs_qd_view_rect(&v, NULL, CS_QD_PAINT_RECT) == 0);
    for (i = 0; i < sizeof bad / sizeof bad[0]; i++) CK(cs_qd_view_rect(&v, r, bad[i]) == 0);
    CK(view_eq(&v, &vc) && fx_eq(&f, &g));
    {   /* corrupt views: every field in turn, then check no effect */
        int fld;
        for (fld = 0; fld < 8; fld++) {
            cs_qd_view c = vc, cc;
            switch (fld) {
            case 0: c.port = NULL; break;
            case 1: c.x = 32768; break;
            case 2: c.y = -32769; break;
            case 3: c.top = -1; break;
            case 4: c.left = 129; break;
            case 5: c.bottom = 65; break;
            case 6: c.right = -1; break;
            default: c.top = 65; break;
            }
            cc = c; g = f;
            CK(cs_qd_view_offset(&c, 0, 0) == 0);
            CK(cs_qd_view_clip(&c, r) == 0);
            CK(cs_qd_view_rect(&c, r, CS_QD_PAINT_RECT) == 0);
            CK(cs_qd_view_rect(&c, r, CS_QD_ERASE_RECT) == 0);
            CK(view_eq(&c, &cc) && fx_eq(&f, &g));
            CK(cs_qd_view_init(&c, &f.p) == 1);   /* init may recover corrupt view */
            CK(c.port == &f.p && c.x == 0 && c.y == 0 && c.top == 0 && c.left == 0 &&
               c.bottom == 64 && c.right == 128);
            CK(fx_eq(&f, &g));
        }
    }
    CK(guards_ok(&f));
}

static void suite_state(void) {
    fx f, g, h; cs_qd_view v, vc; uint8_t r[8];
    int32_t q[4] = { -100, -100, 100, 300 };
    int32_t full[4] = { 0, 0, 64, 128 };
    fx_init(&f, 2); fx_init(&h, 2);
    g = f;
    memset(&v, 0x77, sizeof v);
    CK(cs_qd_view_init(&v, &f.p) == 1);
    CK(v.port == &f.p && v.x == 0 && v.y == 0 && v.top == 0 && v.left == 0 && v.bottom == 64 && v.right == 128);
    CK(fx_eq(&f, &g));
    f.p.paints = 7; f.p.erases = 9; g = f;
    CK(cs_qd_view_init(&v, &f.p) == 1);   /* reinit never resets port */
    CK(fx_eq(&f, &g));
    h.p.paints = 3; g = h;
    CK(cs_qd_view_offset(&v, -32768, 32767) == 1 && v.x == -32768 && v.y == 32767);
    CK(cs_qd_view_offset(&v, 32767, -32768) == 1 && v.x == 32767 && v.y == -32768);
    vc = v;
    CK(cs_qd_view_offset(&v, 32768, 0) == 0 && view_eq(&v, &vc));
    CK(cs_qd_view_offset(&v, 0, -32769) == 0 && view_eq(&v, &vc));
    CK(cs_qd_view_offset(&v, INT32_MAX, INT32_MIN) == 0 && view_eq(&v, &vc));
    CK(fx_eq(&h, &g));
    CK(cs_qd_view_init(&v, &h.p) == 1 && v.port == &h.p);   /* rebind borrows new port */
    CK(f.p.paints == 7 && f.p.erases == 9);
    enc(r, q);
    CK(cs_qd_view_clip(&v, r) == 1);
    CK(v.top == 0 && v.left == 0 && v.bottom == 64 && v.right == 128 && v.x == 0);
    {   /* independent clamping and inverted accepted without swap */
        int32_t c[4] = { 40, 90, 10, 20 };
        enc(r, c);
        CK(cs_qd_view_clip(&v, r) == 1);
        CK(v.top == 40 && v.left == 90 && v.bottom == 10 && v.right == 20);
        c[0] = 200; c[1] = -7; c[2] = -300; c[3] = 32767;
        enc(r, c);
        CK(cs_qd_view_clip(&v, r) == 1);
        CK(v.top == 64 && v.left == 0 && v.bottom == 0 && v.right == 128);
        c[0] = -32768; c[1] = 128; c[2] = 64; c[3] = 129;
        enc(r, c);
        CK(cs_qd_view_clip(&v, r) == 1);
        CK(v.top == 0 && v.left == 128 && v.bottom == 64 && v.right == 128);
    }
    CK(fx_eq(&h, &g) && guards_ok(&h));   /* offset/clip never touch pixels/counters */
    {   /* offset and clip independent */
        int32_t c[4] = { 5, 6, 30, 40 };
        enc(r, c);
        CK(cs_qd_view_clip(&v, r) == 1);
        CK(cs_qd_view_offset(&v, 50, -9) == 1);
        CK(v.top == 5 && v.left == 6 && v.bottom == 30 && v.right == 40);
        CK(cs_qd_view_offset(&v, 99999, 0) == 0 && v.x == 50 && v.y == -9);
        CK(cs_qd_view_clip(&v, r) == 1 && v.x == 50 && v.y == -9);
    }
    {   /* inverted/empty clip draws nothing but still counts */
        fx e; cs_qd_view ev; int32_t ic[4] = { 40, 10, 10, 100 }, ec2[4] = { 20, 20, 20, 90 }, big[4] = { -32768, -32768, 32767, 32767 };
        fx_init(&e, 0);
        setup(&e, &ev, 0, 0, ic);
        draw_check(&e, &ev, big, CS_QD_PAINT_RECT);
        CK(e.p.pixels[0] == 0 && e.p.paints == 1);
        setup(&e, &ev, 0, 0, ec2);
        draw_check(&e, &ev, big, CS_QD_PAINT_RECT);
        CK(e.p.paints == 2);
        setup(&e, &ev, 0, 0, full);
        draw_check(&e, &ev, big, CS_QD_PAINT_RECT);
    }
}

static void suite_extremes(void) {
    static const int32_t O[] = { -32768, -1, 0, 1, 32767 };
    static const int32_t E[] = { -32768, -1, 0, 64, 128, 32767 };
    static const int32_t CE[] = { -32768, 0, 64, 128, 32767 };
    size_t xo, yo, a, b, c, d;
    fx f; cs_qd_view v;
    int32_t full[4] = { 0, 0, 64, 128 }, cl[4] = { 7, 9, 57, 113 };
    int n = 0;
    for (xo = 0; xo < 5; xo++) for (yo = 0; yo < 5; yo++)
    for (a = 0; a < 6; a++) for (b = 0; b < 6; b++) for (c = 0; c < 6; c++) for (d = 0; d < 6; d++) {
        int32_t q[4];
        q[0] = E[a]; q[1] = E[b]; q[2] = E[c]; q[3] = E[d];
        fx_init(&f, (n & 1) ? 0 : 1);
        setup(&f, &v, O[xo], O[yo], (n & 2) ? full : cl);
        draw_check(&f, &v, q, (n & 1) ? CS_QD_PAINT_RECT : CS_QD_ERASE_RECT);
        n++;
    }
    for (a = 0; a < 5; a++) for (b = 0; b < 5; b++) for (c = 0; c < 5; c++) for (d = 0; d < 5; d++) {
        int32_t k[4], q[4] = { -32768, -32768, 32767, 32767 };
        k[0] = CE[a]; k[1] = CE[b]; k[2] = CE[c]; k[3] = CE[d];
        fx_init(&f, 0);
        setup(&f, &v, 0, 0, k);
        draw_check(&f, &v, q, CS_QD_PAINT_RECT);
    }
    {   /* empty/inverted logical rect: no pixels, counted once */
        int32_t inv[4] = { 40, 50, 10, 20 }, emp[4] = { 5, 5, 5, 100 }, emp2[4] = { 5, 30, 50, 30 };
        fx_init(&f, 0);
        setup(&f, &v, 0, 0, full);
        draw_check(&f, &v, inv, CS_QD_PAINT_RECT);
        draw_check(&f, &v, emp, CS_QD_PAINT_RECT);
        draw_check(&f, &v, emp2, CS_QD_PAINT_RECT);
        { size_t i; for (i = 0; i < CS_QD_RECT_BYTES; i++) CK(f.p.pixels[i] == 0); }
        CK(f.p.paints == 3 && f.p.erases == 0);
        fx_init(&f, 1);
        setup(&f, &v, -32768, 32767, full);
        draw_check(&f, &v, inv, CS_QD_ERASE_RECT);
        { size_t i; for (i = 0; i < CS_QD_RECT_BYTES; i++) CK(f.p.pixels[i] == 0xFFu); }
    }
}

static void suite_isolation(void) {
    fx a, b, bs; cs_qd_view va, vb, vbs;
    int32_t ca[4] = { 0, 0, 32, 64 }, cb[4] = { 20, 30, 64, 128 };
    int32_t q1[4] = { -5, -5, 100, 100 }, q2[4] = { 2, 3, 40, 70 }, q3[4] = { 0, 0, 64, 128 };
    fx_init(&a, 0); fx_init(&b, 2);
    setup(&a, &va, 5, 6, ca);
    setup(&b, &vb, -4, 12, cb);
    bs = b; vbs = vb;
    draw_check(&a, &va, q1, CS_QD_PAINT_RECT);
    CK(fx_eq(&b, &bs) && view_eq(&vb, &vbs));
    {   fx as = a; cs_qd_view vas = va;
        draw_check(&b, &vb, q2, CS_QD_ERASE_RECT);
        CK(fx_eq(&a, &as) && view_eq(&va, &vas));
        draw_check(&b, &vb, q3, CS_QD_PAINT_RECT);
        CK(fx_eq(&a, &as));
        CK(cs_qd_view_offset(&vb, 100, 100) == 1);
        CK(fx_eq(&a, &as) && view_eq(&va, &vas) && vb.port == &b.p && va.port == &a.p);
        draw_check(&a, &va, q2, CS_QD_ERASE_RECT);
    }
    {   /* two views sharing one port keep separate state */
        fx s; cs_qd_view v1, v2; int32_t c1[4] = { 0, 0, 10, 10 }, c2[4] = { 50, 100, 64, 128 };
        fx_init(&s, 0);
        setup(&s, &v1, 1, 1, c1);
        setup(&s, &v2, -1, -1, c2);
        draw_check(&s, &v1, q3, CS_QD_PAINT_RECT);
        draw_check(&s, &v2, q3, CS_QD_PAINT_RECT);
        CK(v1.x == 1 && v2.x == -1 && v1.bottom == 10 && v2.top == 50);
    }
    CK(guards_ok(&a) && guards_ok(&b));
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "usage: %s oracle|refusal|state|extremes|isolation\n", argv[0]); return 2; }
    if (strcmp(argv[1], "oracle") == 0) suite_oracle();
    else if (strcmp(argv[1], "refusal") == 0) suite_refusal();
    else if (strcmp(argv[1], "state") == 0) suite_state();
    else if (strcmp(argv[1], "extremes") == 0) suite_extremes();
    else if (strcmp(argv[1], "isolation") == 0) suite_isolation();
    else { fprintf(stderr, "unknown suite\n"); return 2; }
    if (fails) { fprintf(stderr, "%d failure(s)\n", fails); return 1; }
    printf("%s ok\n", argv[1]);
    return 0;
}
