/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0038 native workload. */
#ifndef CLASSICK_NATIVE_DRAWING_WORKLOAD_H
#define CLASSICK_NATIVE_DRAWING_WORKLOAD_H
#include "../../personality/system1/quickdraw-rects.h"
typedef struct {
    uint32_t success,phase,value,pattern,black_pixels;
} cs_native_draw_result;
/* Borrowed truthful aligned live disjoint objects; no retention or allocation.
 * Null refuses unchanged; valid call resets objects then executes real native
 * PaintRect/EraseRect. Failure retains completed effects/phase, success/value0.
 * No guest instruction execution, trap, image loader or callback. */
int cs_native_draw(cs_qd_rect_port *port,cs_native_draw_result *result);
#endif
