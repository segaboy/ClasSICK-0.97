/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_BACKEND_H
#define CLASSICK_CPU_M68K_BACKEND_H
#include "m68k-guard.h"

/* SPEC-0120: trusted synchronous native implementation, not guest callbacks.
 * Context extent covers ALL retained data, including nested backing metadata.
 * No callbacks retain pointers, reenter, mutate controls or publish on failure.
 * span does not access backing; write validates then commits all bytes or none.
 * Ownership and every callback remain stable throughout the complete step. */
typedef struct {
    const void *context;
    size_t context_bytes;
    cs_guest_result (*validate)(const void *context);
    cs_guest_result (*span)(const void *context,uint32_t at,uint32_t bytes,uint32_t access);
    cs_guest_result (*read)(const void *context,uint32_t at,unsigned width,uint32_t access,uint32_t *out);
    cs_guest_result (*write)(const void *context,uint32_t at,unsigned width,uint32_t value);
} cs_m68k_bus_backend;

/* Strict guarded engine, S=1/T=0/I=7, mandatory bounded stack. All A-lines
 * refused before service dispatch. No gateway, options, admitted EXEC override
 * or return token. Existing flat entry points/types remain unchanged.
 * Controls/context/guard data are aligned, live, disjoint native objects; the
 * backend must additionally exclude them and native code from backing RAM.
 * A failed final atomic write preserves RAM and frame; reports may change. */
cs_m68k_result cs_m68k_bus_step_backend(const cs_m68k_bus_backend *backend,
    cs_trap_frame *frame,cs_m68k_bus_report *report,const cs_m68k_stack_limits *limits,
    const cs_m68k_guard *guard);
#endif
