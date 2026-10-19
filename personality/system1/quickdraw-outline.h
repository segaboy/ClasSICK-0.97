/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_QUICKDRAW_OUTLINE_H
#define CLASSICK_QUICKDRAW_OUTLINE_H

#include "quickdraw-view.h"

/* SPEC-0048 native engineering API, not the historical FrameRect ABI.
 * rect: eight big-endian signed16 bytes (top,left,bottom,right).
 * Width/height: [0,32767]; left/right and top/bottom inward thicknesses.
 * Coverage is the union of strips from the unclipped rectangle, intersected
 * with the physical view clip. Black=1, stride16, MSB-first (0x80 >> (x&7)).
 * Preflight view, rect, then pen ranges before reading rect or writing port.
 * Refusal returns0 without effects. Success returns1, ORs covered pixels,
 * increments paints once (saturating), leaves erases/view/rect unchanged.
 * Zero pen, empty/inverted rectangle or empty clip still count as success.
 * Caller supplies live aligned readable view/rect and readable/writable port,
 * pairwise disjoint for their complete extents and exclusively synchronized.
 * Invalid nonnull native addresses cannot be checked; guest pointers forbidden.
 * No allocation, mutable global, external call, callback, I/O or guest access.
 */
int cs_qd_view_outline(cs_qd_view *view, const uint8_t rect[8],
                       int32_t horizontal_width, int32_t vertical_height);

#endif
