/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Proposed test; build decoder only with -DCLASSICK_PC_RAW_PACKET_PROBE=1.
 * Fixtures are synthetic, not captured hardware packets. */
#include <stdio.h>
#include "../platform/pc/ps2-mouse.h"

#if CLASSICK_PC_RAW_PACKET_PROBE != 1
#error build with CLASSICK_PC_RAW_PACKET_PROBE=1
#endif

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

static int feed3(cs_ps2_mouse *m, cs_ps2_mouse_packet *p, uint32_t h, uint32_t x, uint32_t y)
{
    CHECK(cs_ps2_mouse_feed(m, h, 0, p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(m, x, 0, p) == CS_PS2_MOUSE_MORE);
    return cs_ps2_mouse_feed(m, y, 0, p);
}

static void expect(uint32_t h, uint32_t x, uint32_t y, int32_t dx, int32_t dy, uint32_t b)
{
    cs_ps2_mouse m; cs_ps2_mouse_packet p = {0};
    CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE);
    CHECK(feed3(&m, &p, h, x, y) == CS_PS2_MOUSE_PACKET);
    CHECK(p.dx == dx && p.dy == dy && p.buttons == b);
    CHECK(p.raw_header == h && p.raw_x == x && p.raw_y == y);
    CHECK(m.used == 0 && m.header == 0 && m.x == 0);
}

static int same(const cs_ps2_mouse_packet *a, const cs_ps2_mouse_packet *b)
{
    return a->dx == b->dx && a->dy == b->dy && a->buttons == b->buttons
        && a->raw_header == b->raw_header && a->raw_x == b->raw_x
        && a->raw_y == b->raw_y;
}

int main(void)
{
    cs_ps2_mouse m; cs_ps2_mouse_packet p, snap;
    /* Synthetic original-report fixture: not a native capture. */
    expect(0x2A, 0xFF, 0x01, 255, -255, 2);
    expect(0x09, 0x05, 0x03, 5, 3, 1);
    expect(0x1C, 0xFB, 0x04, -5, 4, 4);
    expect(0x38, 0xF0, 0xE0, -16, -32, 0);
    expect(0x0F, 0x7F, 0x7F, 127, 127, 7);
    expect(0x38, 0x00, 0x00, -256, -256, 0);

    /* Zero values overwrite previous raw and decoded fields. */
    cs_ps2_mouse_begin(&m);
    CHECK(feed3(&m, &p, 0x2B, 0xFF, 0x01) == CS_PS2_MOUSE_PACKET);
    CHECK(feed3(&m, &p, 0x08, 0x00, 0x00) == CS_PS2_MOUSE_PACKET);
    CHECK(p.dx == 0 && p.dy == 0 && p.buttons == 0);
    CHECK(p.raw_header == 8 && p.raw_x == 0 && p.raw_y == 0);

    /* MORE, DROPPED, ARGUMENT and STATE leave the output untouched. */
    cs_ps2_mouse_begin(&m);
    feed3(&m, &p, 0x2A, 0xFF, 0x01);
    snap = p;
    CHECK(cs_ps2_mouse_feed(&m, 0x2A, 0, &p) == CS_PS2_MOUSE_MORE && same(&p, &snap));
    CHECK(cs_ps2_mouse_feed(&m, 0x77, 0, &p) == CS_PS2_MOUSE_MORE && same(&p, &snap));
    CHECK(cs_ps2_mouse_feed(&m, 0x66, 1, &p) == CS_PS2_MOUSE_DROPPED && same(&p, &snap));
    CHECK(m.used == 0 && m.header == 0 && m.x == 0);
    CHECK(cs_ps2_mouse_feed(&m, 0x07, 0, &p) == CS_PS2_MOUSE_DROPPED && same(&p, &snap));
    CHECK(feed3(&m, &p, 0x48, 0x01, 0x01) == CS_PS2_MOUSE_DROPPED && same(&p, &snap));
    CHECK(cs_ps2_mouse_feed(&m, 256, 0, &p) == CS_PS2_MOUSE_ARGUMENT && same(&p, &snap));
    CHECK(cs_ps2_mouse_feed(&m, 1, 2, &p) == CS_PS2_MOUSE_ARGUMENT && same(&p, &snap));
    CHECK(cs_ps2_mouse_feed(&m, 1, 0, 0) == CS_PS2_MOUSE_ARGUMENT);
    m.used = 3;
    CHECK(cs_ps2_mouse_feed(&m, 1, 0, &p) == CS_PS2_MOUSE_STATE && same(&p, &snap));
    m.used = 1; m.header = 0x07;
    CHECK(cs_ps2_mouse_feed(&m, 1, 0, &p) == CS_PS2_MOUSE_STATE && same(&p, &snap));

    /* Corruption resets mid-packet; the next packet recovers. */
    cs_ps2_mouse_begin(&m);
    CHECK(cs_ps2_mouse_feed(&m, 0x09, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0x01, 1, &p) == CS_PS2_MOUSE_DROPPED);
    CHECK(feed3(&m, &p, 0x1A, 0xFE, 0x02) == CS_PS2_MOUSE_PACKET);
    CHECK(p.dx == -2 && p.dy == 2 && p.buttons == 2);
    CHECK(p.raw_header == 0x1A && p.raw_x == 0xFE && p.raw_y == 2);

    if (failures) return 1;
    puts("pc-raw-packet: ok");
    return 0;
}
