/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_MEMORY_REGISTERS_H
#define CLASSICK_SYSTEM1_MEMORY_REGISTERS_H
#include "../../core/memory/manager.h"
#include "../../core/services/traps.h"
typedef struct { cs_mm_result status;uint16_t word;cs_mm_reply reply; } cs_mm_register_report;
/* Separate register boundary; not installed in the CPU/gateway, SPEC-0023. */
cs_mm_result cs_mm_register_call(cs_mm_manager *m,uint16_t word,unsigned interrupt_context,
    cs_trap_frame *frame,cs_mm_register_report *report);
#endif
