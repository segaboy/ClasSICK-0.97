/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_CALL_H
#define CLASSICK_CPU_M68K_CALL_H
#include "m68k-bus.h"
#define CS_M68K_CALL_DEPTH 4u
#define CS_M68K_CALL_BUDGET 1000000u
typedef enum { CS_M68K_CALL_OK=0,CS_M68K_CALL_ARGUMENT=1,CS_M68K_CALL_CONFIG=2,
    CS_M68K_CALL_SIGNATURE=3,CS_M68K_CALL_ADDRESS=4,CS_M68K_CALL_DEPTH_LIMIT=5,
    CS_M68K_CALL_BUDGET_LIMIT=6,CS_M68K_CALL_CPU=7,CS_M68K_CALL_RETURN=8 } cs_m68k_call_result;
typedef enum { CS_M68K_PROC0=1 } cs_m68k_call_signature;
typedef enum { CS_M68K_CALL_ORDINARY=0,CS_M68K_CALL_INTERRUPT=1 } cs_m68k_call_context;
typedef struct { uint32_t stack_base,stack_size,sentinel_base,max_depth; } cs_m68k_call_config;
typedef struct { uint32_t entry,a5,budget;cs_m68k_call_signature signature;
    cs_m68k_call_context context; } cs_m68k_call_request;
typedef struct { cs_m68k_call_result status;cs_m68k_result cpu;cs_guest_result memory;
    uint32_t pc,steps,changed_preserved; } cs_m68k_call_report;
typedef struct {
    const cs_guest_map *map;cs_trap_gateway *gateway;cs_m68k_bus_options options;
    cs_m68k_call_config config;uint32_t tag,depth;
    uint32_t remaining[CS_M68K_CALL_DEPTH],used[CS_M68K_CALL_DEPTH],sp[CS_M68K_CALL_DEPTH];
} cs_m68k_calls;
/* SPEC-0026: caller-owned, truthful disjoint live objects; no concurrent use.
 * Active map/options/config are immutable; synchronous reentry is supported. */
cs_m68k_call_result cs_m68k_call_init(cs_m68k_calls *runtime,const cs_guest_map *map,
    cs_trap_gateway *gateway,const cs_m68k_bus_options *options,const cs_m68k_call_config *config);
/* Pre-entry refusal is atomic. After entry, guest side effects survive failure;
 * caller frame is restored on failure, PC/A5/A7 on success. */
cs_m68k_call_result cs_m68k_call(cs_m68k_calls *runtime,const cs_m68k_call_request *request,
    cs_trap_frame *frame,cs_m68k_call_report *report);
#endif
