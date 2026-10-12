/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_H
#define CLASSICK_CPU_M68K_H
#include "../services/traps.h"
typedef enum { CS_M68K_OK=0,CS_M68K_ARGUMENT=1,CS_M68K_ADDRESS=2,
    CS_M68K_MODE=3,CS_M68K_OPCODE=4,CS_M68K_SERVICE=5 } cs_m68k_result;
typedef struct {
    const uint8_t *code; uint32_t code_base,code_size;
    uint8_t *stack; uint32_t stack_base,stack_size;
} cs_m68k_memory;
typedef struct { cs_m68k_result status; uint32_t pc; uint16_t word;
    uint16_t service_called; cs_trap_report service; } cs_m68k_report;
/* Caller-owned aligned, nonoverlapping live objects/buffers; SPEC-0021. */
cs_m68k_result cs_m68k_aline(cs_trap_gateway *g,uint16_t word,cs_trap_frame *f,cs_m68k_report *r);
cs_m68k_result cs_m68k_step(const cs_m68k_memory *m,cs_trap_gateway *g,cs_trap_frame *f,cs_m68k_report *r);
#endif
