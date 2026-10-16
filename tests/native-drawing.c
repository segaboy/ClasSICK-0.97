/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original SPEC-0038 native drawing conformance test. */
#include "../apps/native-drawing/workload.h"
#include "../apps/drawing-rom-app/probe.h"
#include "../apps/service-probe/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANARY_LEN 32u
#define STRIDE (CS_QD_RECT_WIDTH / 8u)
#define EXPECT_BLACK 2304u
#define FNV_BASIS UINT32_C(2166136261)
#define FNV_PRIME UINT32_C(16777619)

typedef struct {
    uint8_t before[CANARY_LEN];
    cs_qd_rect_port port;
    uint8_t between[CANARY_LEN];
    cs_native_draw_result result;
    uint8_t after[CANARY_LEN];
} fixture;

static int failures;

static void fail(const char *suite, const char *what)
{
    fprintf(stderr, "FAIL [%s]: %s\n", suite, what);
    failures++;
}

static int inside(unsigned x, unsigned y, unsigned top, unsigned left,
                  unsigned bottom, unsigned right)
{
    return y >= top && y < bottom && x >= left && x < right;
}

/* Expected image from rectangle membership only: painted outer, erased inner. */
static int expect_black(unsigned x, unsigned y)
{
    return inside(x, y, 8u, 8u, 56u, 120u) && !inside(x, y, 16u, 16u, 48u, 112u);
}

static void build_expected(uint8_t out[CS_QD_RECT_BYTES])
{
    unsigned x, y;
    memset(out, 0, CS_QD_RECT_BYTES);
    for (y = 0; y < CS_QD_RECT_HEIGHT; y++)
        for (x = 0; x < CS_QD_RECT_WIDTH; x++)
            if (expect_black(x, y)) {
                unsigned i = y * STRIDE + x / 8u;
                out[i] = (uint8_t)(out[i] | (0x80u >> (x % 8u)));
            }
}

static unsigned pixel_at(const uint8_t *p, unsigned x, unsigned y)
{
    return ((unsigned)p[y * STRIDE + x / 8u] >> (7u - x % 8u)) & 1u;
}

static uint32_t count_black(const uint8_t *p)
{
    uint32_t n = 0;
    unsigned x, y;
    for (y = 0; y < CS_QD_RECT_HEIGHT; y++)
        for (x = 0; x < CS_QD_RECT_WIDTH; x++)
            n += pixel_at(p, x, y);
    return n;
}

static uint32_t fnv1a(const uint8_t *p)
{
    uint32_t h = FNV_BASIS;
    unsigned i;
    for (i = 0; i < CS_QD_RECT_BYTES; i++) {
        h ^= (uint32_t)p[i];
        h *= FNV_PRIME;
    }
    return h;
}

static void set_canaries(fixture *f)
{
    memset(f->before, 0xC3, CANARY_LEN);
    memset(f->between, 0x5A, CANARY_LEN);
    memset(f->after, 0x3C, CANARY_LEN);
}

static int canary_ok(const uint8_t *c, uint8_t v)
{
    unsigned i;
    for (i = 0; i < CANARY_LEN; i++)
        if (c[i] != v) return 0;
    return 1;
}

static void check_image(const char *suite, const uint8_t *pixels)
{
    uint8_t expected[CS_QD_RECT_BYTES];
    unsigned x, y;
    build_expected(expected);
    for (y = 0; y < CS_QD_RECT_HEIGHT; y++)
        for (x = 0; x < CS_QD_RECT_WIDTH; x++)
            if (pixel_at(pixels, x, y) != (unsigned)expect_black(x, y)) {
                fprintf(stderr, "  first pixel mismatch at x=%u y=%u got=%u\n",
                        x, y, pixel_at(pixels, x, y));
                fail(suite, "pixel image differs from rectangle predicate");
                return;
            }
    if (memcmp(pixels, expected, CS_QD_RECT_BYTES) != 0)
        fail(suite, "1024-byte image differs from expected bytes");
    if (count_black(expected) != EXPECT_BLACK)
        fail(suite, "test oracle black count is not 2304");
}

static void check_native(const char *suite, const fixture *f)
{
    uint8_t expected[CS_QD_RECT_BYTES];
    const cs_native_draw_result *r = &f->result;
    build_expected(expected);
    check_image(suite, f->port.pixels);
    if (f->port.paints != 1u) fail(suite, "paint counter is not 1");
    if (f->port.erases != 1u) fail(suite, "erase counter is not 1");
    if (r->success != 1u) fail(suite, "result.success is not 1");
    if (r->phase != 6u) fail(suite, "result.phase is not 6");
    if (r->value != 97u) fail(suite, "result.value is not 97");
    if (r->black_pixels != EXPECT_BLACK) fail(suite, "result.black_pixels is not 2304");
    if (r->black_pixels != count_black(f->port.pixels))
        fail(suite, "result.black_pixels differs from actual port count");
    if (r->pattern != fnv1a(f->port.pixels))
        fail(suite, "result.pattern differs from actual port FNV-1a");
    if (r->pattern != fnv1a(expected))
        fail(suite, "result.pattern differs from expected image FNV-1a");
    if (!canary_ok(f->before, 0xC3)) fail(suite, "leading canary modified");
    if (!canary_ok(f->between, 0x5A)) fail(suite, "port/result canary modified");
    if (!canary_ok(f->after, 0x3C)) fail(suite, "trailing canary modified");
    if (failures)
        fprintf(stderr, "  result: success=%lu phase=%lu value=%lu pattern=0x%08lx "
                "black=%lu paints=%lu erases=%lu\n",
                (unsigned long)r->success, (unsigned long)r->phase,
                (unsigned long)r->value, (unsigned long)r->pattern,
                (unsigned long)r->black_pixels, (unsigned long)f->port.paints,
                (unsigned long)f->port.erases);
}

static void run_native(const char *suite, fixture *f)
{
    if (cs_native_draw(&f->port, &f->result) != 1)
        fail(suite, "cs_native_draw did not return 1");
    check_native(suite, f);
}

static int suite_native(void)
{
    static fixture f;
    memset(&f, 0, sizeof f);
    set_canaries(&f);
    run_native("native", &f);
    return failures;
}

static void poison(fixture *f, unsigned seed)
{
    unsigned i;
    unsigned char *b = (unsigned char *)f;
    for (i = 0; i < sizeof *f; i++)
        b[i] = (unsigned char)((i * 37u + seed) & 0xFFu);
}

static int suite_refusal(void)
{
    static fixture f, snap;
    unsigned c;
    for (c = 0; c < 3u; c++) {
        int rc;
        poison(&f, 11u + c);
        memcpy(&snap, &f, sizeof f);
        if (c == 0u) rc = cs_native_draw(NULL, &f.result);
        else if (c == 1u) rc = cs_native_draw(&f.port, NULL);
        else rc = cs_native_draw(NULL, NULL);
        if (rc != 0) fail("refusal", "null argument call did not return 0");
        if (memcmp(&f, &snap, sizeof f) != 0)
            fail("refusal", "null argument call modified an object byte");
        if (failures) {
            fprintf(stderr, "  case %u (0=null port, 1=null result, 2=both)\n", c);
            return failures;
        }
    }
    return failures;
}

static int suite_repeat(void)
{
    static fixture f;
    static uint8_t first_pixels[CS_QD_RECT_BYTES];
    cs_native_draw_result first;
    memset(&f.port, 0xFF, sizeof f.port);
    memset(&f.result, 0xEE, sizeof f.result);
    set_canaries(&f);
    run_native("repeat/first", &f);
    memcpy(first_pixels, f.port.pixels, CS_QD_RECT_BYTES);
    first = f.result;

    poison(&f, 0x6Bu);
    f.port.paints = UINT32_MAX;
    f.port.erases = 0x12345678u;
    f.result.success = 1u;
    f.result.phase = 6u;
    f.result.value = 97u;
    f.result.pattern = first.pattern ^ 1u;
    f.result.black_pixels = 0u;
    set_canaries(&f);
    run_native("repeat/second", &f);
    if (memcmp(first_pixels, f.port.pixels, CS_QD_RECT_BYTES) != 0)
        fail("repeat", "second image differs from first");
    if (memcmp(&first, &f.result, sizeof first) != 0)
        fail("repeat", "second result differs from first");
    return failures;
}

static int suite_legacy(void)
{
    static fixture f;
    cs_drawing_app_workspace *w;
    cs_trap_gateway *gw;
    cs_service_probe_result sp;
    cs_loaded_app_result lr;
    memset(&f, 0, sizeof f);
    set_canaries(&f);
    run_native("legacy/native", &f);

    w = calloc(1, sizeof *w);
    gw = calloc(1, sizeof *gw);
    if (!w || !gw) {
        free(w);
        free(gw);
        fail("legacy", "allocation failed");
        return failures;
    }
    memset(&sp, 0, sizeof sp);
    memset(&lr, 0, sizeof lr);
    if (!cs_service_probe(gw, &sp)) fail("legacy", "cs_service_probe failed");
    else if (cs_drawing_app_probe(w, gw, &lr) != 1)
        fail("legacy", "cs_drawing_app_probe did not return 1");
    if (!failures) {
        check_image("legacy/image", w->port.pixels);
        if (memcmp(w->port.pixels, f.port.pixels, CS_QD_RECT_BYTES) != 0)
            fail("legacy", "legacy image differs from native image");
        if (lr.success != 1u) fail("legacy", "legacy success is not 1");
        if (lr.value != f.result.value) fail("legacy", "legacy value differs from native");
        if (fnv1a(w->port.pixels) != f.result.pattern)
            fail("legacy", "legacy image hash differs from native pattern");
        if (count_black(w->port.pixels) != f.result.black_pixels)
            fail("legacy", "legacy black count differs from native");
        if (w->port.paints != f.port.paints || w->port.erases != f.port.erases)
            fail("legacy", "legacy paint/erase counters differ from native");
        if (failures)
            fprintf(stderr, "  legacy: success=%lu phase=%lu value=%lu pattern=0x%08lx "
                    "paints=%lu erases=%lu\n", (unsigned long)lr.success,
                    (unsigned long)lr.phase, (unsigned long)lr.value,
                    (unsigned long)lr.pattern, (unsigned long)w->port.paints,
                    (unsigned long)w->port.erases);
    }
    free(w);
    free(gw);
    return failures;
}

int main(int argc, char **argv)
{
    int rc;
    if (argc != 2) {
        fprintf(stderr, "usage: %s native|refusal|repeat|legacy\n", argc ? argv[0] : "test");
        return 2;
    }
    if (strcmp(argv[1], "native") == 0) rc = suite_native();
    else if (strcmp(argv[1], "refusal") == 0) rc = suite_refusal();
    else if (strcmp(argv[1], "repeat") == 0) rc = suite_repeat();
    else if (strcmp(argv[1], "legacy") == 0) rc = suite_legacy();
    else {
        fprintf(stderr, "unknown suite '%s'\n", argv[1]);
        return 2;
    }
    if (rc) return 1;
    printf("PASS %s\n", argv[1]);
    return 0;
}
