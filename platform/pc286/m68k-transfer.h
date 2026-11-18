/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_M68K_TRANSFER_H
#define CLASSICK_PC286_M68K_TRANSFER_H
#include "m68k-banked.h"
#include "../../core/cpu/m68k-transfer.h"
/* Same stable lifetime/context and whole-arena exclusion as scalar binding.
 * Refusal preserves the complete destination descriptor. */
cs_pg_result cs_pc286_m68k_transfer_backend(cs_m68k_transfer_backend *out,
    const cs_pc286_m68k_memory *memory);
#endif
