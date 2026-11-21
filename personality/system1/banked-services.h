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
/* SPEC-0129: separate explicit Get+canonical A047 Set profile. Same native
 * envelope and immutable/exclusive call lifetime. Set prepares one directory
 * delta; after the final atomic six-byte RAM write, its directory assignment
 * and frame publication cannot fail. No guest patch dispatch, active manager,
 * A147 or other A-line expansion. The Get-only entry above is unchanged. */
cs_m68k_result cs_s1_banked_trap_step(const cs_m68k_transfer_backend *backend,
    cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard);
/* SPEC-0135. Each CPU owner supplies a zero-initialized, disjoint private
 * state and retains it with the same directory, guest memory and stack. */
#define CS_S1_PATCH_DEPTH 4
typedef struct {
    uint32_t slot,continuation,a0,a1,d1,d2;
    uint16_t word;
} cs_s1_patch_call;
typedef struct {
    unsigned depth;
    cs_s1_patch_call calls[CS_S1_PATCH_DEPTH];
} cs_s1_patch_state;
cs_m68k_result cs_s1_banked_patch_step(const cs_m68k_transfer_backend *backend,
    cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard,cs_s1_patch_state *state);

/* SPEC-0146. The standalone 286 runtime owns the exclusive fixed RAM map,
 * heap and service callback. Admission precedes delegation: a RAM patch
 * takes priority and only the bound original NewPtr token enters native C.
 * The callback preflights all of its writes before publishing the result. */
#define CS_S1_STARTUP_NEW_PTR_TOKEN UINT32_C(0x400004)
typedef cs_m68k_result (*cs_s1_startup_newptr_fn)(const cs_trap_frame *before,
    cs_trap_frame *proposed,cs_m68k_bus_report *report,uint32_t scratch,
    const cs_m68k_guard *guard);
cs_m68k_result cs_s1_banked_startup_step(const cs_m68k_transfer_backend *backend,
    cs_trap_gateway *gateway,cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard,cs_s1_patch_state *state,
    cs_s1_startup_newptr_fn newptr);
#endif
