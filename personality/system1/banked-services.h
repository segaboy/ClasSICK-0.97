/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_BANKED_SERVICES_H
#define CLASSICK_SYSTEM1_BANKED_SERVICES_H
#include "../../core/cpu/m68k-transfer.h"

/* SPEC-0124. Explicit GetTrapAddress-only opt-in; all other A-lines refuse.
 * Ordinary instructions delegate after a guarded routing fetch. The native
 * directory, complete backend/guard contexts and every control are stable,
 * aligned, live, disjoint and excluded from guest backing. No active manager,
 * reentry, DMA, hidden references or pointer retention. Current directory is
 * read-only. Final six-byte scratch commit is atomic before frame publication.
 * Existing guarded execution entry points continue to refuse all A-lines. */
cs_m68k_result cs_s1_banked_step(const cs_m68k_transfer_backend *backend,
    const cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard);
#endif
