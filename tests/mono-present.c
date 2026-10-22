/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* TEST-0071: SPEC-0060 bridge. Build normally, and again with -DCS_MONO_FAULTS
 * where only the bridge object is compiled with -Dcs_fb_present=cs_test_fb_present. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pc/mono-present.h"

#define CW 512u
#define CH 342u
#define CSTRIDE 64u
#define CBYTES (CSTRIDE * CH)
#define CTAIL 128u
#define GUARD 64u
#define ROWSENT 0xC3

static unsigned long g_cases, g_checks, g_frames, g_fail;

#define CHECK(c) check_((c), #c, __LINE__)
static int check_(int ok, const char *e, int line) {
    g_checks++;
    if (!ok) { g_fail++; fprintf(stderr, "FAIL line %d: %s\n", line, e); }
    return ok;
}

typedef struct { uint32_t w, h, pitch, fmt; size_t tail; } geom;
typedef struct {
    geom g;
    unsigned char *dev, *exp;
    size_t size;
    uint8_t *canvas;
    size_t csize;
    cs_surface cv;
    cs_fb_target t;
    uint8_t *row;
    size_t rcap, rhigh;
} ctx;

#ifdef CS_MONO_FAULTS
static const ctx *g_ctx;
static const cs_pc_mono_report *g_out;
static uint32_t g_initial, g_calls, g_fail_at;

cs_fb_result cs_test_fb_present(const cs_fb_target *t, const cs_surface *s,
    int32_t left, int32_t top);
cs_fb_result cs_test_fb_present(const cs_fb_target *t, const cs_surface *s,
    int32_t left, int32_t top) {
    uint32_t i = g_calls++, x0, y0, req;
    const ctx *c = g_ctx;
    if (!CHECK(c != NULL && t != NULL && s != NULL && g_out != NULL))
        return CS_FB_ARGUMENT;
    x0 = (c->g.w - CW) / 2u;
    y0 = (c->g.h - CH) / 2u;
    req = g_initial ? c->g.h : CH;
    CHECK(i < req);
    CHECK(t->base == c->t.base && t->size == c->t.size);
    CHECK(t->width == c->t.width && t->height == c->t.height);
    CHECK(t->pitch == c->t.pitch && t->format == c->t.format);
    CHECK(s->format == CS_SURFACE_RGBA8 && s->height == 1);
    CHECK(s->storage == c->row && s->storage_size <= c->rcap);
    CHECK(s->width == (int32_t)(g_initial ? c->g.w : CW));
    CHECK(s->stride >= (size_t)(g_initial ? c->g.w : CW) * 4u);
    CHECK(left == (g_initial ? 0 : (int32_t)x0));
    CHECK(top == (int32_t)(g_initial ? i : y0 + i));
    CHECK(g_out->phase == CS_MONO_VALIDATED && g_out->rows == i);
    CHECK(g_out->required == req && g_out->cause == 0);
    {
        uint32_t bad=0;
        for(size_t a=3u;a<(size_t)s->width*4u;a+=4u)
            bad|=(uint32_t)s->storage[a]^255u;
        CHECK(bad==0u);
    }
    if (i == g_fail_at) return CS_FB_STORAGE;
    return cs_fb_present(t, s, left, top);
}
#endif

static int invoke(const ctx *c, const cs_fb_target *t, const cs_surface *cv,
    uint8_t *row, size_t cap, uint32_t initial, cs_pc_mono_report *out,
    uint32_t fail_at) {
#ifdef CS_MONO_FAULTS
    g_ctx = c; g_out = out; g_initial = initial; g_calls = 0u; g_fail_at = fail_at;
#else
    (void)c; (void)fail_at;
#endif
    return cs_pc_mono_present(t, cv, row, cap, initial, out);
}

static int bit(const uint8_t *cv, uint32_t x, uint32_t y) {
    return (cv[(size_t)y * CSTRIDE + x / 8u] >> (7u - (x % 8u))) & 1;
}

static void put_px(unsigned char *b, const geom *g, uint32_t x, uint32_t y, int black) {
    size_t o = ((size_t)y * g->pitch + x) * 4u;
    unsigned char v = (unsigned char)(black ? 0x00 : 0xFF);
    b[o] = v; b[o + 1u] = v; b[o + 2u] = v; b[o + 3u] = 0;
}

/* Independent oracle: apply the first rows publication rows onto c->exp. */
static void build_expected(ctx *c, uint32_t initial, uint32_t rows) {
    uint32_t x0 = (c->g.w - CW) / 2u, y0 = (c->g.h - CH) / 2u, x, y;
    if (initial) {
        for (y = 0; y < rows; y++)
            for (x = 0; x < c->g.w; x++) {
                int black = 0;
                if (y >= y0 && y < y0 + CH && x >= x0 && x < x0 + CW)
                    black = bit(c->canvas, x - x0, y - y0);
                put_px(c->exp, &c->g, x, y, black);
            }
    } else {
        for (y = 0; y < rows; y++)
            for (x = 0; x < CW; x++)
                put_px(c->exp, &c->g, x0 + x, y0 + y, bit(c->canvas, x, y));
    }
}

static void fill_canvas(ctx *c, uint32_t seed) {
    uint32_t s = seed * 2654435761u + 0x9E3779B9u;
    size_t i;
    for (i = 0; i < CBYTES; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        c->canvas[i] = (uint8_t)(s >> 24);
    }
    c->canvas[0] = 0x80u;
    c->canvas[CSTRIDE - 1u] = 0x01u;
    c->canvas[CBYTES - CSTRIDE] = 0xC0u;
    c->canvas[CBYTES - 1u] = 0x01u;
}

static int compare_dev(const ctx *c, const char *what) {
    size_t i, n = 0, first = 0, rb = (size_t)c->g.pitch * 4u;
    int any = 0;
    if (memcmp(c->dev, c->exp, c->size) == 0) return CHECK(1);
    for (i = 0; i < c->size; i++)
        if (c->dev[i] != c->exp[i]) {
            if (!any) { first = i; any = 1; }
            n++;
        }
    fprintf(stderr, "  %s %ux%u pitch=%u fmt=%u: %lu bad bytes; first off=%lu "
        "y=%lu x=%lu b=%lu got=%02X want=%02X\n", what, (unsigned)c->g.w,
        (unsigned)c->g.h, (unsigned)c->g.pitch, (unsigned)c->g.fmt,
        (unsigned long)n, (unsigned long)first, (unsigned long)(first / rb),
        (unsigned long)((first % rb) / 4u), (unsigned long)(first % 4u),
        (unsigned)c->dev[first], (unsigned)c->exp[first]);
    return CHECK(0);
}

static void note_row(ctx *c, uint32_t initial) {
    size_t used = (size_t)(initial ? c->g.w : CW) * 4u, i;
    if (used > c->rhigh) c->rhigh = used;
    for (i = c->rhigh; i < c->rcap + GUARD; i++)
        if (c->row[i] != ROWSENT) break;
    if (!CHECK(i == c->rcap + GUARD))
        fprintf(stderr, "  row byte %lu beyond used prefix %lu changed\n",
            (unsigned long)i, (unsigned long)c->rhigh);
}

static void reset(ctx *c) {
    memset(c->dev, 0x5A, c->size);
    memcpy(c->exp, c->dev, c->size);
    memset(c->row, ROWSENT, c->rcap + GUARD);
    c->rhigh = 0;
}

static int setup(ctx *c, const geom *g) {
    memset(c, 0, sizeof *c);
    c->g = *g;
    c->size = (size_t)g->pitch * 4u * g->h + g->tail;
    c->csize = CBYTES + CTAIL;
    c->rcap = (size_t)g->w * 4u + 36u;
    c->dev = malloc(c->size);
    c->exp = malloc(c->size);
    c->canvas = malloc(c->csize);
    c->row = malloc(c->rcap + GUARD);
    if (!CHECK(c->dev && c->exp && c->canvas && c->row)) return 0;
    reset(c);
    memset(c->canvas, 0xEE, c->csize);
    if (!CHECK(cs_surface_init(&c->cv, c->canvas, c->csize, (int32_t)CW,
            (int32_t)CH, CSTRIDE, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK)) return 0;
    return CHECK(cs_fb_init(&c->t, (volatile unsigned char *)c->dev, c->size,
        g->w, g->h, g->pitch, g->fmt) == CS_FB_OK);
}

static void teardown(ctx *c) {
    free(c->dev); free(c->exp); free(c->canvas); free(c->row);
}

static void publish(ctx *c, const cs_fb_target *t, const cs_surface *cv,
    uint32_t initial, uint32_t seed, const char *what) {
    cs_pc_mono_report rep;
    cs_surface cvs;
    cs_fb_target ts;
    uint32_t req = initial ? c->g.h : CH;
    unsigned char *snap;
    int r;
    if (!t) t = &c->t;
    if (!cv) cv = &c->cv;
    fill_canvas(c, seed);
    build_expected(c, initial, req);
    snap = malloc(c->csize);
    if (!CHECK(snap != NULL)) return;
    memcpy(snap, c->canvas, c->csize);
    memcpy(&cvs, cv, sizeof cvs);
    memcpy(&ts, t, sizeof ts);
    memset(&rep, 0xCC, sizeof rep);
    r = invoke(c, t, cv, c->row, c->rcap, initial, &rep, UINT32_MAX);
    g_cases++;
    if (CHECK(r == CS_MONO_OK)) g_frames++;
    else fprintf(stderr, "  %s: result %d cause %ld\n", what, r, (long)rep.cause);
    CHECK(rep.phase == CS_MONO_COMPLETE);
    CHECK(rep.rows == req && rep.required == req && rep.cause == 0);
    CHECK(memcmp(snap, c->canvas, c->csize) == 0);
    CHECK(memcmp(&cvs, cv, sizeof cvs) == 0);
    CHECK(memcmp(&ts, t, sizeof ts) == 0);
#ifdef CS_MONO_FAULTS
    CHECK(g_calls == req);
#endif
    compare_dev(c, what);
    note_row(c, initial);
    free(snap);
}

static void refuse(ctx *c, const cs_fb_target *t, const cs_surface *cv,
    uint8_t *row, size_t cap, uint32_t initial, cs_pc_mono_report *out,
    const unsigned char *xb, size_t xn, int want, const char *what) {
    unsigned char *sd = malloc(c->size), *sc = malloc(c->csize);
    unsigned char *sr = malloc(c->rcap + GUARD), *sx = malloc(xn ? xn : 1u);
    unsigned char so[sizeof(cs_pc_mono_report)], st[sizeof(cs_fb_target)];
    unsigned char sv[sizeof(cs_surface)];
    int r;
    if (CHECK(sd && sc && sr && sx)) {
        memcpy(sd, c->dev, c->size);
        memcpy(sc, c->canvas, c->csize);
        memcpy(sr, c->row, c->rcap + GUARD);
        if (xn) memcpy(sx, xb, xn);
        if (out) memcpy(so, out, sizeof so);
        if (t) memcpy(st, t, sizeof st);
        if (cv) memcpy(sv, cv, sizeof sv);
        r = invoke(c, t, cv, row, cap, initial, out, 0u);
        g_cases++;
        if (!CHECK(r == want)) fprintf(stderr, "  %s: got %d want %d\n", what, r, want);
        if (!CHECK(memcmp(sd, c->dev, c->size) == 0)) fprintf(stderr, "  %s: device\n", what);
        CHECK(memcmp(sc, c->canvas, c->csize) == 0);
        CHECK(memcmp(sr, c->row, c->rcap + GUARD) == 0);
        if (xn) CHECK(memcmp(sx, xb, xn) == 0);
        if (out) CHECK(memcmp(so, out, sizeof so) == 0);
        if (t) CHECK(memcmp(st, t, sizeof st) == 0);
        if (cv) CHECK(memcmp(sv, cv, sizeof sv) == 0);
#ifdef CS_MONO_FAULTS
        CHECK(g_calls == 0u);
#endif
    }
    free(sd); free(sc); free(sr); free(sx);
}

static void refusal_suite(void) {
    static const geom rg = {600u, 400u, 608u, CS_FB_RGB_RESERVED, 8u};
    ctx c;
    cs_pc_mono_report rep, *ro;
    cs_fb_target t;
    cs_surface v;
    unsigned char *blk;
    size_t cap;
    if (!setup(&c, &rg)) { teardown(&c); return; }
    cap = (size_t)rg.w * 4u;
    fill_canvas(&c, 99u);
    memset(&rep, 0xCC, sizeof rep);
    CHECK(sizeof(cs_pc_mono_report) == 16u);
#define R(tt, vv, rw, cp, in, o, want, nm) \
    refuse(&c, tt, vv, rw, cp, in, o, NULL, 0u, want, nm)
    R(NULL, &c.cv, c.row, cap, 1u, &rep, CS_MONO_ARGUMENT, "target NULL");
    R(&c.t, NULL, c.row, cap, 1u, &rep, CS_MONO_ARGUMENT, "canvas NULL");
    R(&c.t, &c.cv, NULL, cap, 1u, &rep, CS_MONO_ARGUMENT, "row NULL");
    R(&c.t, &c.cv, c.row, cap, 1u, NULL, CS_MONO_ARGUMENT, "report NULL");
    R(&c.t, &c.cv, c.row, cap, 2u, &rep, CS_MONO_ARGUMENT, "initial 2");
    R(&c.t, &c.cv, c.row, cap, UINT32_MAX, &rep, CS_MONO_ARGUMENT, "initial max");
    v = c.cv; v.format = CS_SURFACE_RGBA8;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src format");
    v = c.cv; v.width = 511;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src width");
    v = c.cv; v.height = 343;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src height");
    v = c.cv; v.stride = 65u;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src stride");
    v = c.cv; v.storage_size = CBYTES - 1u;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src short");
    v = c.cv; v.storage_size = 0u;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src empty");
    v = c.cv; v.storage_size = SIZE_MAX;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src wrap");
    v = c.cv; v.storage = NULL;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_SOURCE, "src NULL");
    t = c.t; t.width = 511u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt width");
    t = c.t; t.height = 341u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt height");
    t = c.t; t.width = 8193u; t.pitch = 8193u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt max");
    t = c.t; t.format = 2u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt format");
    t = c.t; t.pitch = rg.w - 1u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt pitch");
    t = c.t; t.size = (size_t)(rg.h - 1u) * rg.pitch * 4u + (size_t)rg.w * 4u - 1u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt short");
    t = c.t; t.size = 0u;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt empty");
    t = c.t; t.size = SIZE_MAX;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt wrap");
    t = c.t; t.base = NULL;
    R(&t, &c.cv, c.row, cap, 1u, &rep, CS_MONO_TARGET, "tgt NULL");
    R(&c.t, &c.cv, c.row, cap - 1u, 1u, &rep, CS_MONO_CAPACITY, "cap -1 initial");
    R(&c.t, &c.cv, c.row, CW * 4u, 0u, &rep, CS_MONO_CAPACITY, "cap 2048 update");
    R(&c.t, &c.cv, c.row, SIZE_MAX, 1u, &rep, CS_MONO_CAPACITY, "cap wrap");
    v = c.cv; v.width = 1; t = c.t; t.width = 1u;
    R(&t, &v, c.row, cap, 9u, &rep, CS_MONO_ARGUMENT, "order arg");
    R(&t, &v, c.row, 0u, 1u, &rep, CS_MONO_SOURCE, "order source");
    R(&t, &c.cv, c.row, 0u, 1u, &rep, CS_MONO_TARGET, "order target");
    R(&c.t, &c.cv, c.dev, 0u, 1u, &rep, CS_MONO_CAPACITY, "order capacity");
    R(&c.t, &c.cv, c.dev, cap, 1u, &rep, CS_MONO_ALIAS, "row in device");
    R(&c.t, &c.cv, c.dev + c.size - cap, cap, 1u, &rep, CS_MONO_ALIAS, "row at device end");
    R(&c.t, &c.cv, c.canvas, cap, 1u, &rep, CS_MONO_ALIAS, "row on canvas");
    ro = (cs_pc_mono_report *)(void *)(c.dev + 16u);
    R(&c.t, &c.cv, c.row, cap, 1u, ro, CS_MONO_ALIAS, "report in device");
    ro = (cs_pc_mono_report *)(void *)(c.row + 8u);
    R(&c.t, &c.cv, c.row, cap, 1u, ro, CS_MONO_ALIAS, "report in row");
    ro = (cs_pc_mono_report *)(void *)(c.canvas + 64u);
    R(&c.t, &c.cv, c.row, cap, 1u, ro, CS_MONO_ALIAS, "report in canvas");
    v = c.cv; v.storage = c.dev; v.storage_size = CBYTES + CTAIL;
    R(&c.t, &v, c.row, cap, 1u, &rep, CS_MONO_ALIAS, "canvas in device");
    memcpy(c.dev, &c.t, sizeof c.t);
    R((const cs_fb_target *)(void *)c.dev, &c.cv, c.row, cap, 1u, &rep, CS_MONO_ALIAS, "tgt desc in device");
    memcpy(c.dev, &c.cv, sizeof c.cv);
    R(&c.t, (const cs_surface *)(void *)c.dev, c.row, cap, 1u, &rep, CS_MONO_ALIAS, "canvas desc in device");
    memcpy(c.row, &c.t, sizeof c.t);
    R((const cs_fb_target *)(void *)c.row, &c.cv, c.row, cap, 1u, &rep, CS_MONO_ALIAS, "tgt desc in row");
    memcpy(c.row, &c.cv, sizeof c.cv);
    R(&c.t, (const cs_surface *)(void *)c.row, c.row, cap, 1u, &rep, CS_MONO_ALIAS, "canvas desc in row");
#undef R
    blk = malloc(256u);
    if (CHECK(blk != NULL)) {
        ro = (cs_pc_mono_report *)(void *)(blk + 8u);
        memcpy(blk, &c.t, sizeof c.t);
        refuse(&c, (const cs_fb_target *)(void *)blk, &c.cv, c.row, cap, 1u, ro,
            blk, 256u, CS_MONO_ALIAS, "report on tgt desc");
        memcpy(blk, &c.cv, sizeof c.cv);
        refuse(&c, &c.t, (const cs_surface *)(void *)blk, c.row, cap, 1u, ro,
            blk, 256u, CS_MONO_ALIAS, "report on canvas desc");
    }
    free(blk);
    reset(&c);
    memcpy(c.canvas + CBYTES, &c.cv, sizeof c.cv);
    memcpy(c.canvas + CBYTES + 64u, &c.t, sizeof c.t);
    publish(&c, (const cs_fb_target *)(void *)(c.canvas + CBYTES + 64u),
        (const cs_surface *)(void *)(c.canvas + CBYTES), 1u, 5u, "readonly share");
    teardown(&c);
}

#ifdef CS_MONO_FAULTS
static void fault_case(ctx *c, uint32_t initial, uint32_t k) {
    cs_pc_mono_report rep;
    uint32_t req = initial ? c->g.h : CH;
    int r;
    reset(c);
    fill_canvas(c, k * 31u + initial + 7u);
    build_expected(c, initial, k);
    memset(&rep, 0xCC, sizeof rep);
    r = invoke(c, &c->t, &c->cv, c->row, c->rcap, initial, &rep, k);
    g_cases++;
    if (!CHECK(r == CS_MONO_PRESENT))
        fprintf(stderr, "  fault k=%u initial=%u: got %d\n", (unsigned)k, (unsigned)initial, r);
    CHECK(rep.phase == CS_MONO_FAILED && rep.rows == k && rep.required == req);
    CHECK(rep.cause == (int32_t)CS_FB_STORAGE);
    CHECK(g_calls == k + 1u);
    compare_dev(c, "fault partial");
    note_row(c, initial);
}

static void fault_suite(void) {
    static const geom gs[] = {{513u, 343u, 517u, 0u, 12u}, {640u, 480u, 648u, 0u, 0u}};
    size_t gi;
    uint32_t fmt, initial, j;
    for (gi = 0; gi < sizeof gs / sizeof gs[0]; gi++)
        for (fmt = 0; fmt < 2u; fmt++) {
            geom g = gs[gi];
            ctx c;
            g.fmt = fmt;
            if (setup(&c, &g))
                for (initial = 0; initial < 2u; initial++) {
                    uint32_t req = initial ? g.h : CH;
                    uint32_t ks[4];
                    ks[0] = 0u; ks[1] = 1u; ks[2] = req / 2u; ks[3] = req - 1u;
                    for (j = 0; j < 4u; j++) fault_case(&c, initial, ks[j]);
                    reset(&c);
                    publish(&c, NULL, NULL, initial, 11u + initial, "fault-build full");
                }
            teardown(&c);
        }
}
#else
static void normal_suite(void) {
    static const geom gs[] = {
        {512u, 342u, 512u, 0u, 0u}, {513u, 343u, 517u, 0u, 12u},
        {514u, 344u, 514u, 0u, 4u}, {640u, 480u, 648u, 0u, 0u},
        {1023u, 769u, 1030u, 0u, 8u}, {8192u, 346u, 8195u, 0u, 16u}};
    size_t gi;
    uint32_t fmt;
    for (gi = 0; gi < sizeof gs / sizeof gs[0]; gi++)
        for (fmt = 0; fmt < 2u; fmt++) {
            geom g = gs[gi];
            ctx c;
            g.fmt = fmt;
            if (setup(&c, &g)) {
                publish(&c, NULL, NULL, 0u, 1u, "update on fresh target");
                publish(&c, NULL, NULL, 1u, 2u, "initial");
                publish(&c, NULL, NULL, 0u, 3u, "update after initial");
                publish(&c, NULL, NULL, 1u, 4u, "initial reused workspace");
            }
            teardown(&c);
        }
}
#endif

int main(void) {
#ifdef CS_MONO_FAULTS
    const char *mode = "fault";
    fault_suite();
#else
    const char *mode = "normal";
    normal_suite();
#endif
    /* Numeric fixed-object wrap must refuse before dereference/snapshot. */
    {
        const geom g={512u,342u,512u,0u,0u};ctx c;cs_pc_mono_report out,saved;
        const uintptr_t high=UINTPTR_MAX-3u;
        if(setup(&c,&g)) {
            memset(&out,0xA5,sizeof out);saved=out;
            CHECK(cs_pc_mono_present((const cs_fb_target *)high,&c.cv,c.row,c.rcap,1u,&out)==CS_MONO_ARGUMENT);
            CHECK(memcmp(&out,&saved,sizeof out)==0);
            CHECK(cs_pc_mono_present(&c.t,(const cs_surface *)high,c.row,c.rcap,1u,&out)==CS_MONO_ARGUMENT);
            CHECK(memcmp(&out,&saved,sizeof out)==0);
            CHECK(cs_pc_mono_present(&c.t,&c.cv,c.row,c.rcap,1u,(cs_pc_mono_report *)high)==CS_MONO_ARGUMENT);
            CHECK(memcmp(c.dev,c.exp,c.size)==0);g_cases+=3u;
        }
        teardown(&c);
    }
    refusal_suite();
    printf("TEST-0071 %s: cases=%lu checks=%lu frames=%lu failures=%lu\n",
        mode, g_cases, g_checks, g_frames, g_fail);
    return g_fail ? 1 : 0;
}
