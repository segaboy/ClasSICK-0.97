/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_MEMORY_TRAPS_H
#define CLASSICK_SYSTEM1_MEMORY_TRAPS_H
#include "memory-registers.h"
#include "../../core/cpu/m68k-bus.h"

enum { CS_S1_MEMORY_TRAPS=11 };
typedef struct {
    uint32_t magic,token_base;cs_trap_gateway *gateway;cs_mm_manager *memory;
    cs_m68k_data_policy data_policy;
} cs_s1_traps;
typedef enum { CS_S1_NONE=0,CS_S1_SIGNATURE=1,CS_S1_SCRATCH=2,CS_S1_MEMORY=3 } cs_s1_failure;
typedef struct {
    cs_trap_report trap;cs_mm_register_report memory;unsigned memory_called;
    cs_s1_failure failure;
} cs_s1_report;
/* SPEC-0025: caller-owned disjoint live objects/map; no guest callbacks. */
cs_trap_result cs_s1_traps_init(cs_s1_traps *out,cs_trap_gateway *gateway,
    cs_mm_manager *memory,uint32_t token_base,cs_m68k_data_policy data_policy);
cs_trap_result cs_s1_traps_check(const cs_s1_traps *runtime);
cs_trap_result cs_s1_trap_call(cs_s1_traps *s,uint16_t word,unsigned interrupt_context,
    cs_trap_frame *frame,cs_s1_report *report);
cs_trap_result cs_s1_trap_direct(cs_s1_traps *s,uint32_t target,uint16_t word,
    unsigned interrupt_context,cs_trap_frame *frame,cs_s1_report *report);
cs_m68k_result cs_s1_step(cs_s1_traps *s,unsigned interrupt_context,
    cs_trap_frame *frame,cs_m68k_bus_report *cpu,cs_s1_report *service);
#endif
