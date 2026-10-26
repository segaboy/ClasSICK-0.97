/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_PLATFORM_PC_PS2_MOUSE_H
#define CS_PLATFORM_PC_PS2_MOUSE_H
#include <stdint.h>

/* Diagnostic-only: 1 appends the raw header/X/Y bytes of each decoded PACKET
 * to cs_ps2_mouse_packet. Must match in every translation unit. */
#ifndef CLASSICK_PC_RAW_PACKET_PROBE
#define CLASSICK_PC_RAW_PACKET_PROBE 0
#endif
#if (CLASSICK_PC_RAW_PACKET_PROBE+0)!=0 && (CLASSICK_PC_RAW_PACKET_PROBE+0)!=1
#error CLASSICK_PC_RAW_PACKET_PROBE must be 0 or 1
#endif
typedef char cs_pc_raw_packet_probe_numeric[
    (CLASSICK_PC_RAW_PACKET_PROBE==0 || CLASSICK_PC_RAW_PACKET_PROBE==1)?1:-1];

/* Pure three-byte PS/2 report decoder. No port I/O, ACK/BAT handling or
 * keyboard coupling: a future coordinator separates startup responses from
 * report bytes and reports corruption. Strict profile: header bit 3 set. */
typedef struct {
    int32_t dx, dy; uint32_t buttons;
#if CLASSICK_PC_RAW_PACKET_PROBE
    uint32_t raw_header, raw_x, raw_y;
#endif
} cs_ps2_mouse_packet;
typedef struct { uint32_t used, header, x; } cs_ps2_mouse;
enum {
    CS_PS2_MOUSE_MORE = 0, CS_PS2_MOUSE_PACKET = 1, CS_PS2_MOUSE_DROPPED = 2,
    CS_PS2_MOUSE_ARGUMENT = 3, CS_PS2_MOUSE_STATE = 4
};

int cs_ps2_mouse_begin(cs_ps2_mouse *mouse);
int cs_ps2_mouse_feed(cs_ps2_mouse *mouse, uint32_t byte, uint32_t corrupt,
    cs_ps2_mouse_packet *packet);

#endif
