/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_GUEST_PATCHES_H
#define CLASSICK_SYSTEM1_GUEST_PATCHES_H
#include "memory-traps.h"
#include "../../core/cpu/m68k-call.h"
typedef enum { CS_S1_BUILTIN=1,CS_S1_GUEST_CODE=2 } cs_s1_entry_kind;
typedef struct { cs_s1_entry_kind kind;uint32_t target;cs_trap_word signature; } cs_s1_entry;
typedef struct { cs_s1_traps *traps;cs_m68k_calls calls;uint32_t budget; } cs_s1_patches;
typedef struct { cs_m68k_bus_report cpu;cs_s1_report service;
    cs_m68k_call_report call;unsigned entered;cs_m68k_call_result nested_failure; } cs_s1_patch_report;
/* SPEC-0027: original typed OS entries; output changes only on success. */
cs_trap_result cs_s1_entry_resolve(const cs_s1_traps *runtime,uint16_t word,cs_s1_entry *out);
cs_m68k_call_result cs_s1_patches_init(cs_s1_patches *runtime,cs_s1_traps *traps,
    const cs_m68k_call_config *config,uint32_t budget);
/* Opt-in step. Late guest effects survive failure; complete CPU frame restored.
 * Explicit ordinary context only. Legacy cs_s1_step retains its atomic profile. */
cs_m68k_result cs_s1_patch_step(cs_s1_patches *runtime,unsigned interrupt_context,
    cs_trap_frame *frame,cs_s1_patch_report *report);
#endif
