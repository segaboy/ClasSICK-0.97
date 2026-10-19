/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_OUTLINE_DRAWING_WORKLOAD_H
#define CLASSICK_OUTLINE_DRAWING_WORKLOAD_H
#include "../selected-drawing/workload.h"
/* SPEC-0049: reuse the owned two-port workspace/result value layout.
 * Truthful aligned/live/disjoint native objects,exclusive synchronous access.
 * NULL returns0 without effects; otherwise resets then performs real native
 * selection and SPEC-0048 outlining. Failure retains completed phase/effects,
 * composite/success/value97 publish only after actual invariants pass.
 * No allocation,guest execution/traps/callbacks or retained native pointers.
 */
int cs_outline_draw(cs_selected_draw_workspace *workspace,
                    cs_selected_draw_result *result);
#endif
