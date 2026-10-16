/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../core/input/mouse.h"
#include "../platform/pc/ps2-mouse.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); } } while (0)

static void button_tests(void)
{
    cs_mouse_button b = {7, 7};
    CHECK(cs_mouse_button_begin(0, 0) == CS_MOUSE_ARGUMENT);
    CHECK(cs_mouse_button_begin(&b, 2) == CS_MOUSE_ARGUMENT && b.down == 7 && b.presses == 7);
    CHECK(cs_mouse_button_begin(&b, 1) == CS_MOUSE_OK && b.down == 1 && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_OK && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED && b.down == 0 && b.presses == 0);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.presses == 1);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_OK && b.presses == 1);
    CHECK(cs_mouse_button_sample(&b, 2) == CS_MOUSE_ARGUMENT && b.down == 1 && b.presses == 1);
    CHECK(cs_mouse_button_sample(0, 0) == CS_MOUSE_ARGUMENT);
    b.presses = UINT32_MAX;
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_CHANGED);
    CHECK(cs_mouse_button_sample(&b, 1) == CS_MOUSE_CHANGED && b.presses == UINT32_MAX);
    b.down = 2;
    CHECK(cs_mouse_button_sample(&b, 0) == CS_MOUSE_STATE && b.down == 2);
}

static void ps2_tests(void)
{
    cs_ps2_mouse m = {9, 9, 9};
    cs_ps2_mouse_packet p, keep;
    memset(&p, 0x5A, sizeof p);
    keep = p;
    CHECK(cs_ps2_mouse_begin(0) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_begin(&m) == CS_PS2_MOUSE_MORE && m.used == 0 && m.header == 0 && m.x == 0);
    /* Left button, dx +5, dy negative via bit 5. */
    CHECK(cs_ps2_mouse_feed(&m, 0x29, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0x05, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(memcmp(&p, &keep, sizeof p) == 0);
    CHECK(cs_ps2_mouse_feed(&m, 0xFE, 0, &p) == CS_PS2_MOUSE_PACKET
        && p.dx == 5 && p.dy == -2 && p.buttons == 1 && m.used == 0 && m.header == 0 && m.x == 0);
    /* Payload 0xFA/0xAA is data; dx negative via bit 4, middle button. */
    CHECK(cs_ps2_mouse_feed(&m, 0x1C, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0xFA, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0xAA, 0, &p) == CS_PS2_MOUSE_PACKET
        && p.dx == -6 && p.dy == 170 && p.buttons == 4);
    keep = p;
    CHECK(cs_ps2_mouse_feed(&m, 0x00, 0, &p) == CS_PS2_MOUSE_DROPPED && m.used == 0);
    CHECK(cs_ps2_mouse_feed(&m, 0x48, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0x01, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0x02, 0, &p) == CS_PS2_MOUSE_DROPPED && m.used == 0);
    CHECK(cs_ps2_mouse_feed(&m, 0x08, 0, &p) == CS_PS2_MOUSE_MORE);
    CHECK(cs_ps2_mouse_feed(&m, 0x08, 1, &p) == CS_PS2_MOUSE_DROPPED && m.used == 0 && m.header == 0);
    CHECK(cs_ps2_mouse_feed(&m, 256, 0, &p) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_feed(&m, 0x08, 2, &p) == CS_PS2_MOUSE_ARGUMENT);
    CHECK(cs_ps2_mouse_feed(&m, 0x08, 0, 0) == CS_PS2_MOUSE_ARGUMENT && m.used == 0);
    m.used = 1; m.header = 0; m.x = 0;
    CHECK(cs_ps2_mouse_feed(&m, 0x08, 1, &p) == CS_PS2_MOUSE_STATE && m.used == 1);
    CHECK(memcmp(&p, &keep, sizeof p) == 0);
}

int main(void)
{
    button_tests();
    ps2_tests();
    puts(failures == 0 ? "SPEC-0039 mouse button/PS2 decoder: PASS" : "SPEC-0039 mouse: FAIL");
    return failures != 0;
}
