/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original paired selected-port workload. */
#ifndef CLASSICK_SELECTED_DRAWING_WORKLOAD_H
#define CLASSICK_SELECTED_DRAWING_WORKLOAD_H
#include "../native-drawing/workload.h"
typedef struct {
    cs_qd_rect_port ports[2];
    uint8_t pixels[CS_QD_RECT_BYTES]; /* presentation-only union of both ports */
} cs_selected_draw_workspace;
typedef struct {
    cs_native_draw_result draw;
    uint32_t current,selections;
} cs_selected_draw_result;
/* Truthful live/aligned/disjoint native objects,exclusive synchronous access.
 * NULL refuses without effects. Otherwise reset workspace/result then execute
 * actual native prepared-port selection and view drawing. No allocation,
 * guest execution/traps/loader or retained pointers. Failure keeps phase and
 * completed port effects; composite/success/value97 publish only on success.
 * See SPEC-0047 for exact shapes,phases and display composition. */
int cs_selected_draw(cs_selected_draw_workspace *workspace,
    cs_selected_draw_result *result);
#endif
