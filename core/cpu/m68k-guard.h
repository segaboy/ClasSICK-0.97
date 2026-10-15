/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_GUARD_H
#define CLASSICK_CPU_M68K_GUARD_H
#include "m68k-bus.h"
typedef enum {
    CS_M68K_GUARD_OPCODE=0,CS_M68K_GUARD_EXTENSION=1,
    CS_M68K_GUARD_READ=2,CS_M68K_GUARD_WRITE=3,
    CS_M68K_GUARD_TARGET=4,CS_M68K_GUARD_INSTRUCTION=5,
    CS_M68K_GUARD_COMMIT=6
} cs_m68k_guard_kind;
typedef struct {
    cs_m68k_guard_kind kind;
    uint32_t address,bytes,access;
    uint16_t word;
    const cs_trap_frame *before,*proposed;
} cs_m68k_guard_event;
/* Trusted, immutable native policy: return nonzero to permit. Synchronous,
 * side-effect-free, non-reentrant; do not retain event/frame pointers. */
typedef int (*cs_m68k_guard_check)(const void *context,const cs_m68k_guard_event *event);
typedef struct {
    cs_m68k_guard_check check;
    const void *context;
    size_t context_bytes;
} cs_m68k_guard;
/* SPEC-0034 X-1 only. Strict24, bounded even A7, S=1/T=0/I=7. All A-lines
 * denied, including legacy gateway fallback. No dispatch/options/EXEC override.
 * Controls, outputs, context and every distinct backing must be disjoint;
 * malformed native envelopes return ARGUMENT without changing the report.
 * Objects/buffers must be live, aligned and synchronized. Context is NULL iff
 * context_bytes==0; the stated extent covers every policy-dependent byte.
 * Whole actual accesses precede reads/store staging; INSTRUCTION precedes decode;
 * COMMIT follows proposed-state validation and precedes RAM/frame publication.
 * Event word is zero before opcode fetch; COMMIT has proposed PC/0 bytes/0 access.
 * Reports may change on ordinary policy/CPU/address refusal; RAM/frame/gateway
 * do not. No session, image lease, return-token or application-entry authority. */
cs_m68k_result cs_m68k_bus_step_guarded(const cs_guest_map *map,cs_trap_gateway *gateway,
    cs_trap_frame *frame,cs_m68k_bus_report *report,const cs_m68k_stack_limits *limits,
    const cs_m68k_guard *guard);
/* SPEC-0035 internal coordinator primitive. A trusted immutable owner policy
 * must validate an exclusive live image lease before every call/access. This
 * function alone grants no lease. READ-backed fetches remain guarded; ordinary
 * step_guarded keeps its map EXEC requirement. Only a real RTS at root_slot may
 * target token without reading it. Token/slot are checked 24-bit even addresses.
 * No guest-selected policy, alternate executor, dispatch or service is admitted. */
cs_m68k_result cs_m68k_bus_step_admitted(const cs_guest_map *map,cs_trap_gateway *gateway,
    cs_trap_frame *frame,cs_m68k_bus_report *report,const cs_m68k_stack_limits *limits,
    const cs_m68k_guard *guard,uint32_t token,uint32_t root_slot);
/* Pure native-envelope check, including actual persistent driver locals; no
 * report/frame/RAM publication and no instruction is fetched or executed. */
cs_m68k_result cs_m68k_bus_guard_validate(const cs_guest_map *map,cs_trap_gateway *gateway,
    cs_trap_frame *frame,cs_m68k_bus_report *report,const cs_m68k_stack_limits *limits,
    const cs_m68k_guard *guard);
#endif
