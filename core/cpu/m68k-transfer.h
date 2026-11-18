/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_TRANSFER_H
#define CLASSICK_CPU_M68K_TRANSFER_H
#include "m68k-backend.h"
#define CS_M68K_TRANSFER_BYTES 64u
/* SPEC-0123: additive descriptor; scalar descriptor layout stays unchanged.
 * write_bytes admits 1..64 bytes, validates the whole extent then publishes all
 * bytes or none. Source is stable native staging, borrowed only during call.
 * The full wrapper/context/backing obey SPEC-0120 ownership/exclusion rules. */
typedef struct {
    cs_m68k_bus_backend scalar;
    cs_guest_result (*write_bytes)(const void *context,uint32_t at,
        unsigned bytes,const uint8_t *source);
} cs_m68k_transfer_backend;
cs_m68k_result cs_m68k_bus_step_transfer(const cs_m68k_transfer_backend *backend,
    cs_trap_frame *frame,cs_m68k_bus_report *report,
    const cs_m68k_stack_limits *limits,const cs_m68k_guard *guard);
#endif
