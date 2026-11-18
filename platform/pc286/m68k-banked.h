/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_M68K_BANKED_H
#define CLASSICK_PC286_M68K_BANKED_H
#include "banked-guest.h"
#include "../../core/cpu/m68k-backend.h"

/* The whole retained backing/lifetime state occupies one near native object.
 * Zero initially, then bind guest with &generation as the generation output.
 * No copied active contexts, edits, reentry, retirement or rebind during step. */
typedef struct {
    cs_pc286_guest guest;
    uint32_t generation;
} cs_pc286_m68k_memory;

/* Builds a trusted callback descriptor; objects are live/aligned/disjoint.
 * The admitted arena (including gaps) must exclude entire CS and DS/SS 64K
 * envelopes. Existing SPEC0117 bank binding alone is intentionally weaker. */
cs_pg_result cs_pc286_m68k_backend(cs_m68k_bus_backend *out,
    const cs_pc286_m68k_memory *memory);
#endif
