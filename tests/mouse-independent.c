/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "core/input/mouse.h"
#include "platform/pc/ps2-mouse.h"

static unsigned long fails;
#define CHECK(c) do { if (!(c)) { fails++; if (fails < 20) printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static int same(const void *a, const void *b, size_t n) { return memcmp(a, b, n) == 0; }

static void test_button(void)
{
    cs_mouse_button b = {7, 9}, keep;
    uint32_t i;
    CHECK(cs_mouse_button_begin(NULL, 0) == CS_MOUSE_ARGUMENT);
    CHECK(cs_mouse_button_begin(&b, 2) == CS_MOUSE_ARGUMENT);
    CHECK(b.down == 7 && b.presses == 9);
    CHECK(cs_mouse_button_begin(&b, 0xFFFFFFFFu) == CS_MOUSE_ARGUMENT);
    CHECK(b.down == 7 && b.presses == 9);
    CHECK(cs_mouse_button_begin(&b, 0) == CS_MOUSE_OK && b.down == 0 && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_OK && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.down == 1 && b.presses == 1);
    for (i = 0; i < 5; i++)
        CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_OK && b.down == 1 && b.presses == 1);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED && b.down == 0 && b.presses == 1);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_OK && b.presses == 1);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.presses == 2);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED && b.down == 0 && b.presses == 2);
    /* baseline held: no invented press; release is a change without count */
    CHECK(cs_mouse_button_begin(&b, 1) == CS_MOUSE_OK && b.down == 1 && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_OK && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED && b.down == 0 && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.presses == 1);
    /* invalid inputs leave state untouched */
    CHECK(cs_mouse_button_sample(NULL, 1) == CS_MOUSE_ARGUMENT);
    keep = b;
    CHECK(cs_mouse_button_sample(&b, 2) == CS_MOUSE_ARGUMENT && same(&b, &keep, sizeof b));
    b.down = 2; b.presses = 5; keep = b;
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_STATE && same(&b, &keep, sizeof b));
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_STATE && same(&b, &keep, sizeof b));
    b.down = 0xFFFFFFFFu; keep = b;
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_STATE && same(&b, &keep, sizeof b));
    /* saturation */
    b.down = 0; b.presses = 0xFFFFFFFEu;
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.presses == 0xFFFFFFFFu);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED && b.presses == 0xFFFFFFFFu);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.down == 1 && b.presses == 0xFFFFFFFFu);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_OK && b.presses == 0xFFFFFFFFu);
}

static const cs_ps2_mouse_packet SENT = {111, -222, 333};

/* Feeds three bytes; checks intermediate MORE state; returns final result. */
static int run3(cs_ps2_mouse *m, uint32_t h, uint32_t x, uint32_t y, cs_ps2_mouse_packet *o)
{
    *o = SENT;
    CHECK(cs_ps2_mouse_feed(m, h, 0, o) == CS_PS2_MOUSE_MORE);
    CHECK(same(o, &SENT, sizeof SENT) && m->used == 1 && m->header == h && m->x == 0);
    CHECK(cs_ps2_mouse_feed(m, x, 0, o) == CS_PS2_MOUSE_MORE);
    CHECK(same(o, &SENT, sizeof SENT) && m->used == 2 && m->header == h && m->x == x);
    return cs_ps2_mouse_feed(m, y, 0, o);
}

static void expect(uint32_t h, uint32_t x, uint32_t y, int r, int32_t dx, int32_t dy)
{
    cs_ps2_mouse m;
    cs_ps2_mouse_packet o;
    cs_ps2_mouse zero = {0, 0, 0};
    CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE);
    CHECK(run3(&m, h, x, y, &o) == r);
    CHECK(same(&m, &zero, sizeof m));
    if (r == CS_PS2_MOUSE_PACKET)
        CHECK(o.dx == dx && o.dy == dy && o.buttons == (h & 7u));
    else
        CHECK(same(&o, &SENT, sizeof SENT));
}

static void test_exhaustive(void)
{
    uint32_t h, x, y;
    cs_ps2_mouse m;
    cs_ps2_mouse_packet o;
    cs_ps2_mouse zero = {0, 0, 0};
    CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE);
    for (h = 0; h < 256; h++) {
        if (!(h & 8u)) {
            o = SENT;
            CHECK(cs_ps2_mouse_feed(&m, h, 0, &o) == CS_PS2_MOUSE_DROPPED);
            CHECK(same(&o, &SENT, sizeof SENT) && same(&m, &zero, sizeof m));
            continue;
        }
        for (x = 0; x < 256; x++)
            for (y = 0; y < 256; y++) {
                int r = run3(&m, h, x, y, &o);
                int32_t ex = (int32_t)x - ((h & 16u) ? 256 : 0);
                int32_t ey = (int32_t)y - ((h & 32u) ? 256 : 0);
                if (h >= 64u && (h & 0xC0u)) {
                    CHECK(r == CS_PS2_MOUSE_DROPPED && same(&o, &SENT, sizeof SENT));
                } else {
                    CHECK(r == CS_PS2_MOUSE_PACKET);
                    CHECK(o.dx == ex && o.dy == ey && o.buttons == (h & 7u));
                }
                CHECK(same(&m, &zero, sizeof m));
            }
    }
}

static void test_literals(void)
{
    uint32_t b;
    expect(0x08, 0x00, 0x00, CS_PS2_MOUSE_PACKET, 0, 0);
    expect(0x18, 0x00, 0x00, CS_PS2_MOUSE_PACKET, -256, 0);
    expect(0x08, 0xFF, 0x00, CS_PS2_MOUSE_PACKET, 255, 0);
    expect(0x18, 0xFF, 0x00, CS_PS2_MOUSE_PACKET, -1, 0);
    expect(0x08, 0x00, 0x80, CS_PS2_MOUSE_PACKET, 0, 128);
    expect(0x28, 0x00, 0x80, CS_PS2_MOUSE_PACKET, 0, -128);
    expect(0x38, 0x80, 0x7F, CS_PS2_MOUSE_PACKET, -128, -129);
    expect(0x08, 0x7F, 0x7F, CS_PS2_MOUSE_PACKET, 127, 127);
    /* header authority: high payload bit set but header sign clear, and the reverse */
    expect(0x08, 0x80, 0x80, CS_PS2_MOUSE_PACKET, 128, 128);
    expect(0x38, 0x7F, 0x7F, CS_PS2_MOUSE_PACKET, 127 - 256, 127 - 256);
    for (b = 0; b < 8; b++) {
        expect(0x08 | b, 3, 4, CS_PS2_MOUSE_PACKET, 3, 4);
        expect(0x38 | b, 3, 4, CS_PS2_MOUSE_PACKET, 3 - 256, 4 - 256);
        expect(0x48 | b, 3, 4, CS_PS2_MOUSE_DROPPED, 0, 0);
        expect(0x88 | b, 3, 4, CS_PS2_MOUSE_DROPPED, 0, 0);
        expect(0xC8 | b, 3, 4, CS_PS2_MOUSE_DROPPED, 0, 0);
        expect(0xF8 | b, 0xFF, 0xFF, CS_PS2_MOUSE_DROPPED, 0, 0);
    }
    /* command-looking bytes are data */
    expect(0x09, 0xFA, 0xFE, CS_PS2_MOUSE_PACKET, 250, 254);
    expect(0x0A, 0xAA, 0x08, CS_PS2_MOUSE_PACKET, 170, 8);
    expect(0x1C, 0xFE, 0xFA, CS_PS2_MOUSE_PACKET, -2, 250);
    expect(0x2F, 0x08, 0xAA, CS_PS2_MOUSE_PACKET, 8, 170 - 256);
}

static void test_invalid(void)
{
    cs_ps2_mouse m, keep;
    cs_ps2_mouse_packet o, p;
    static const cs_ps2_mouse bad[] = {
        {3, 8, 0}, {0xFFFFFFFFu, 8, 0}, {1, 256, 0}, {2, 8, 256}, {1, 0, 0}, {2, 0x07, 5}, {1, 0x30, 0}
    };
    size_t i;
    CHECK(cs_ps2_mouse_begin(NULL) == CS_PS2_MOUSE_ARGUMENT);
    m.used = 5; m.header = 6; m.x = 7;
    CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE && m.used == 0 && m.header == 0 && m.x == 0);
    m.used = 2; m.header = 0x19; m.x = 0x44; keep = m; o = p = SENT;
    CHECK(cs_ps2_mouse_feed(NULL, 1, 0, &o) == CS_PS2_MOUSE_ARGUMENT && same(&o, &SENT, sizeof o));
    CHECK(cs_ps2_mouse_feed(&m, 1, 0, NULL) == CS_PS2_MOUSE_ARGUMENT && same(&m, &keep, sizeof m));
    CHECK(cs_ps2_mouse_feed(&m, 256, 0, &o) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_feed(&m, 0xFFFFFFFFu, 0, &o) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_feed(&m, 1, 2, &o) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_feed(&m, 256, 2, &o) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(same(&m, &keep, sizeof m) && same(&o, &p, sizeof o));
    for (i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        m = bad[i]; keep = m; o = SENT;
        CHECK(cs_ps2_mouse_feed(&m, 0x08, 0, &o) == CS_PS2_MOUSE_STATE);
        CHECK(same(&m, &keep, sizeof m) && same(&o, &SENT, sizeof o));
    }
}

static void test_corrupt(void)
{
    cs_ps2_mouse m, zero = {0, 0, 0};
    cs_ps2_mouse_packet o;
    uint32_t st;
    for (st = 0; st < 3; st++) {
        CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE);
        o = SENT;
        if (st > 0) CHECK(cs_ps2_mouse_feed(&m, 0x1B, 0, &o) == CS_PS2_MOUSE_MORE);
        if (st > 1) CHECK(cs_ps2_mouse_feed(&m, 0x33, 0, &o) == CS_PS2_MOUSE_MORE);
        CHECK(cs_ps2_mouse_feed(&m, 0x08, 1, &o) == CS_PS2_MOUSE_DROPPED);
        CHECK(same(&m, &zero, sizeof m) && same(&o, &SENT, sizeof o));
        CHECK(cs_ps2_mouse_feed(&m, 0x00, 1, &o) == CS_PS2_MOUSE_DROPPED);
        CHECK(same(&m, &zero, sizeof m) && same(&o, &SENT, sizeof o));
        /* after reset, a fresh packet decodes without residue */
        CHECK(run3(&m, 0x29, 0x05, 0xF0, &o) == CS_PS2_MOUSE_PACKET);
        CHECK(o.dx == 5 && o.dy == -16 && o.buttons == 1 && same(&m, &zero, sizeof m));
    }
}

int main(void)
{
    test_button();
    test_literals();
    test_invalid();
    test_corrupt();
    test_exhaustive();
    if (fails) {
        printf("mouse-independent: %lu failures\n", fails);
        return 1;
    }
    puts("mouse-independent: ok");
    return 0;
}
