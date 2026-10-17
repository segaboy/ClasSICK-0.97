/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0043 selection values. */
#ifndef CLASSICK_SYSTEM1_QUICKDRAW_SELECT_H
#define CLASSICK_SYSTEM1_QUICKDRAW_SELECT_H
#include <stdint.h>
#include "quickdraw-view.h"
#define CS_QD_SELECT_CAPACITY 2u
#define CS_QD_SELECT_RECORD_BYTES UINT32_C(108)
typedef struct {
    uint32_t id;
    const cs_qd_view *view;
} cs_qd_select_binding;
typedef struct {
    uint32_t globals_begin, a5, g, count;
    cs_qd_select_binding bindings[CS_QD_SELECT_CAPACITY];
} cs_qd_selection;
typedef struct {
    uint32_t value;
    const cs_qd_view *view;
} cs_qd_select_plan;
/* Borrowed synchronous values only; no retained state or guest access.
 * get: plan=current (including NIL); set: plan=requested (NIL refused).
 * Both validate the complete table and the supplied current first.
 * Success1 writes only plan fields; refusal0 leaves every byte unchanged.
 * Native arguments must describe live storage; see SPEC-0043 for alignment,
 * overlap checks, supported bad inputs and caller lifetime obligations. */
int cs_qd_select_get(const cs_qd_selection *selection, uint32_t current,
                     cs_qd_select_plan *plan);
int cs_qd_select_set(const cs_qd_selection *selection, uint32_t current,
                     uint32_t requested, cs_qd_select_plan *plan);
#endif
