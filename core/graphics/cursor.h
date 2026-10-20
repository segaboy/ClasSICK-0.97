/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_GRAPHICS_CURSOR_H
#define CLASSICK_CORE_GRAPHICS_CURSOR_H

#include "surface.h"

/* SPEC-0052 single-cursor save/restore over one borrowed full-frame
 * cs_surface in ordinary RAM. No allocation, globals or callbacks. */
typedef enum {
    CS_CURSOR_OK = 0, CS_CURSOR_ERR_ARGUMENT = 1, CS_CURSOR_ERR_STATE = 2,
    CS_CURSOR_ERR_STALE = 3, CS_CURSOR_ERR_SURFACE = 4, CS_CURSOR_ERR_CAPACITY = 5,
    CS_CURSOR_ERR_ALIAS = 6, CS_CURSOR_ERR_ART = 7, CS_CURSOR_ERR_COORD = 8,
    CS_CURSOR_ERR_EXHAUSTED = 9, CS_CURSOR_ERR_COVERAGE = 10, CS_CURSOR_ERR_BUSY = 11
} cs_cursor_result;
enum { CS_CURSOR_MONO_PAYLOAD = 48, CS_CURSOR_RGBA_PAYLOAD = 1024,
       CS_CURSOR_MAGIC = 0x43535243u /* arbitrary tag */ };
typedef enum { CS_CURSOR_UNBOUND = 1, CS_CURSOR_HIDDEN = 2,
    CS_CURSOR_VISIBLE = 3, CS_CURSOR_REPAIR = 4 } cs_cursor_phase;
typedef struct { uint8_t mask[32]; uint8_t image[32]; } cs_cursor_bits;
typedef struct { const cs_cursor_bits *bits;
    uint8_t width, height, hot_x, hot_y; } cs_cursor_art;
typedef struct { uint32_t binding; uint32_t revision; } cs_cursor_token;
typedef struct {
    const cs_surface *surface; uint8_t *payload;
    uint32_t magic, binding, next_binding, revision, payload_capacity;
    int16_t origin_x, origin_y, frame_w, frame_h;
    int16_t save_left, save_top, save_right, save_bottom;
    uint8_t phase, format, fault, reserved;
} cs_cursor_state;
typedef struct { cs_cursor_token token; cs_rect saved;
    uint8_t phase, fault, reserved[2]; } cs_cursor_info;

cs_cursor_result cs_cursor_init(cs_cursor_state *st);
cs_cursor_result cs_cursor_bind(cs_cursor_state *st, const cs_surface *surface,
    uint8_t *payload, size_t capacity, cs_cursor_token *out);
cs_cursor_result cs_cursor_show(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y);
cs_cursor_result cs_cursor_move(cs_cursor_state *st, cs_cursor_token tok,
    const cs_cursor_art *art, int32_t x, int32_t y);
cs_cursor_result cs_cursor_hide(cs_cursor_state *st, cs_cursor_token tok);
cs_cursor_result cs_cursor_advance(cs_cursor_state *st, cs_cursor_token tok,
    cs_cursor_token *out);
cs_cursor_result cs_cursor_discard(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect *out_old);
cs_cursor_result cs_cursor_repaired(cs_cursor_state *st, cs_cursor_token tok,
    cs_rect covered, cs_cursor_token *out);
cs_cursor_result cs_cursor_unbind(cs_cursor_state *st, cs_cursor_token tok);
cs_cursor_result cs_cursor_abandon(cs_cursor_state *st, cs_rect *out_old);
cs_cursor_result cs_cursor_query(const cs_cursor_state *st, cs_cursor_info *out);

#endif
