/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original fixed-port SPEC-0036 profile. */
#ifndef CLASSICK_SYSTEM1_QUICKDRAW_RECTS_H
#define CLASSICK_SYSTEM1_QUICKDRAW_RECTS_H
#include <stdint.h>
#define CS_QD_RECT_WIDTH 128u
#define CS_QD_RECT_HEIGHT 64u
#define CS_QD_RECT_BYTES 1024u
#define CS_QD_PAINT_RECT UINT16_C(0xA8A2)
#define CS_QD_ERASE_RECT UINT16_C(0xA8A3)
typedef struct {
    uint8_t pixels[CS_QD_RECT_BYTES];
    uint32_t paints,erases;
} cs_qd_rect_port;
/* Native borrowed eight-byte BE signed16 top,left,bottom,right record.
 * Objects/buffers live/aligned/disjoint/synchronized. No guest pointers,
 * callbacks or retention. Unknown word/null returns0 unchanged; success1.
 * Fixed128x64/stride16 black patCopy/white background; not a guest GrafPort.
 * Empty/inverted/clipped calls succeed; diagnostic counts saturate.
 * Original algorithm, no libc/mutable global/allocator. */
int cs_qd_rect(cs_qd_rect_port *port,const uint8_t rect[8],uint16_t word);
#endif
