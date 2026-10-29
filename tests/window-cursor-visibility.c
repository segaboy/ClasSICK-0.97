/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../core/graphics/cursor.h"
#include "../apps/window-scene/input.h"

enum { G = 16, W = 512, H = 342, S = 66, N = S * H };
static uint8_t buf[G + N + G], pay[CS_CURSOR_MONO_PAYLOAD];
static int bad;
#define CK(c) do { if (!(c)) { bad++; fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); } } while (0)

/* Frozen from geometry: black plus, and its clipped 1px halo (x0..6 MSB-first). */
static const uint8_t PI[7] = {0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10};
static const uint8_t HM[7] = {0x38, 0x38, 0xFE, 0xFE, 0xFE, 0x38, 0x38};

static void put(uint8_t *s, int x, int y, int v)
{
    uint8_t m = (uint8_t)(0x80u >> (x % 8)), *p = s + y * S + x / 8;
    *p = v ? (uint8_t)(*p | m) : (uint8_t)(*p & ~m);
}

static void setbg(int bg)
{
    int x, y;
    memset(buf, 0xA5, sizeof buf);
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
            put(buf + G, x, y, bg == 2 ? ((x / 8 + y / 8) & 1) : bg);
}

static void overlay(uint8_t *e, int x, int y, const uint8_t *mk, const uint8_t *im)
{
    int r, c, px, py;
    for (r = 0; r < 7; r++)
        for (c = 0; c < 7; c++) {
            px = x - 3 + c; py = y - 3 + r;
            if (px >= 0 && px < W && py >= 0 && py < H && (mk[r] & (0x80 >> c)))
                put(e + G, px, py, (im[r] & (0x80 >> c)) != 0);
        }
}

static void fixture(cs_cursor_state *st, cs_surface *sf, cs_cursor_token *t)
{
    CK(cs_surface_init(sf, buf + G, N, W, H, S, CS_SURFACE_MONO1_MSB) == CS_SURFACE_OK);
    CK(cs_cursor_init(st) == CS_CURSOR_OK);
    CK(cs_cursor_bind(st, sf, pay, sizeof pay, t) == CS_CURSOR_OK);
}

static int diffbits(const uint8_t *a, const uint8_t *b)
{
    int n = 0, i;
    uint8_t v;
    for (i = 0; i < N; i++)
        for (v = (uint8_t)(a[G + i] ^ b[G + i]); v; v >>= 1)
            n += v & 1;
    return n;
}

static int scenario(int bg, int x, int y, const cs_cursor_art *a, const uint8_t *mk, const uint8_t *im)
{
    static uint8_t snap[sizeof buf], exp[sizeof buf];
    cs_cursor_state st; cs_surface sf; cs_cursor_token t; int d;
    setbg(bg);
    memcpy(snap, buf, sizeof buf); memcpy(exp, buf, sizeof buf);
    fixture(&st, &sf, &t);
    CK(cs_cursor_show(&st, t, a, x, y) == CS_CURSOR_OK);
    overlay(exp, x, y, mk, im);
    CK(memcmp(buf, exp, sizeof buf) == 0); /* exact art, no writes outside 7x7, guards intact */
    d = diffbits(buf, snap);
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(buf, snap, sizeof buf) == 0);
    return d;
}

int main(void)
{
    static const int px[5] = {256, 0, 511, 0, 511}, py[5] = {171, 0, 0, 341, 341};
    cs_cursor_bits nb, ob;
    cs_cursor_art na, oa;
    cs_cursor_state st; cs_surface sf; cs_cursor_token t;
    static uint8_t snap[sizeof buf], exp[sizeof buf];
    int i, bg, r;
    memset(&nb, 0xA5, sizeof nb); memset(&ob, 0, sizeof ob);
    cs_window_cursor_bits_init(&nb);
    for (r = 0; r < 7; r++)
        ob.mask[2 * r] = ob.image[2 * r] = PI[r];
    for (i = 0; i < 32; i++) /* unused rows/columns transparent, second bytes zero */
        CK(i < 14 && !(i & 1) ? 1 : (nb.mask[i] == 0 && nb.image[i] == 0));
    na.bits = &nb; oa.bits = &ob;
    na.width = oa.width = na.height = oa.height = 7;
    na.hot_x = oa.hot_x = na.hot_y = oa.hot_y = 3;
    for (bg = 0; bg < 2; bg++)
        for (i = 0; i < 5; i++)
            CK(scenario(bg, px[i], py[i], &na, HM, PI) > 0);
    CK(scenario(2, px[0], py[0], &na, HM, PI) > 0);
    /* Old black-only art draws zero difference on uniform black; new art does not. */
    CK(scenario(1, 256, 171, &oa, PI, PI) == 0);
    CK(scenario(1, 256, 171, &na, HM, PI) == 20); /* 33 mask px minus 13 black plus px */
    CK(scenario(0, 256, 171, &na, HM, PI) == 13);
    setbg(2);
    memcpy(snap, buf, sizeof buf); memcpy(exp, buf, sizeof buf);
    fixture(&st, &sf, &t);
    CK(cs_cursor_show(&st, t, &na, 100, 100) == CS_CURSOR_OK);
    CK(cs_cursor_move(&st, t, &na, 200, 150) == CS_CURSOR_OK);
    overlay(exp, 200, 150, HM, PI);
    CK(memcmp(buf, exp, sizeof buf) == 0); /* old position restored */
    CK(cs_cursor_hide(&st, t) == CS_CURSOR_OK);
    CK(memcmp(buf, snap, sizeof buf) == 0);
    if (bad)
        return 1;
    puts("window-cursor-visibility: ok");
    return 0;
}
