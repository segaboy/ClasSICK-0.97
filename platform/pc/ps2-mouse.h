/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_PLATFORM_PC_PS2_MOUSE_H
#define CS_PLATFORM_PC_PS2_MOUSE_H
#include <stdint.h>

/* Pure three-byte PS/2 report decoder. No port I/O, ACK/BAT handling or
 * keyboard coupling: a future coordinator separates startup responses from
 * report bytes and reports corruption. Strict profile: header bit 3 set. */
typedef struct { int32_t dx, dy; uint32_t buttons; } cs_ps2_mouse_packet;
typedef struct { uint32_t used, header, x; } cs_ps2_mouse;
enum {
    CS_PS2_MOUSE_MORE = 0, CS_PS2_MOUSE_PACKET = 1, CS_PS2_MOUSE_DROPPED = 2,
    CS_PS2_MOUSE_ARGUMENT = 3, CS_PS2_MOUSE_STATE = 4
};

int cs_ps2_mouse_begin(cs_ps2_mouse *mouse);
int cs_ps2_mouse_feed(cs_ps2_mouse *mouse, uint32_t byte, uint32_t corrupt,
    cs_ps2_mouse_packet *packet);

#endif
