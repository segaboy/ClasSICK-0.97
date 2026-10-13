/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_TOOLBOX_H
#define CLASSICK_SYSTEM1_TOOLBOX_H
#include "../../core/cpu/m68k-session.h"
#define CS_TB_RECORDS 16u
#define CS_TB_ARGUMENTS 8u
#define CS_TB_OUTPUT_BYTES 16u
typedef enum { CS_TB_NONE=0,CS_TB_WORD=1,CS_TB_LONG=2,CS_TB_BOOL=3,
    CS_TB_CHAR=4,CS_TB_SPAN=5,CS_TB_STRING=6 } cs_tb_type;
typedef enum { CS_TB_CONVENTION_NONE=0,CS_TB_STACK=1,CS_TB_REGISTER=2 } cs_tb_convention;
typedef enum { CS_TB_COMPLETION_NONE=0,CS_TB_RETURNS=1,CS_TB_CONTINUES=2,CS_TB_EXITS=3 } cs_tb_completion;
typedef enum { CS_TB_OK=0,CS_TB_ARGUMENT=1,CS_TB_CONFIG=2,CS_TB_CONTEXT=3,
    CS_TB_UNKNOWN=4,CS_TB_FLAGS=5,CS_TB_CONVENTION=6,CS_TB_CONTINUATION=7,
    CS_TB_PATCH=8,CS_TB_SIGNATURE=9,CS_TB_ADDRESS=10,CS_TB_ALIAS=11,
    CS_TB_RETAINED=12,CS_TB_HANDLER=13,CS_TB_REPLY=14,CS_TB_DIRECT=15 } cs_tb_result;
typedef struct { cs_tb_type type;uint32_t bytes,access,alignment,kept; } cs_tb_argument;
typedef struct { uint32_t value[CS_TB_ARGUMENTS],length[CS_TB_ARGUMENTS];
    const uint8_t *span[CS_TB_ARGUMENTS];uint32_t a5;unsigned count; } cs_tb_input;
typedef struct { uint32_t result,bytes;uint8_t output[CS_TB_OUTPUT_BYTES]; } cs_tb_reply;
/* Pure bounded trusted OS computation; no external mutation/reentry/retention.
 * Span views are borrowed only during this invocation. No guest function pointer. */
typedef unsigned (*cs_tb_handler)(void *context,const cs_tb_input *input,cs_tb_reply *reply);
typedef struct { uint16_t word;cs_tb_convention convention;cs_tb_completion completion;
    uint32_t identity;unsigned count;cs_tb_type result;const cs_tb_argument *arguments;
    cs_tb_handler handler;void *context; } cs_tb_record;
typedef struct { cs_tb_result status;cs_tb_convention convention;cs_tb_completion completion;
    uint32_t normalized_sp,parameters,outputs;unsigned invoked; } cs_tb_report;
typedef struct { const cs_guest_map *map;cs_trap_gateway *gateway;cs_m68k_session *session;
    const cs_tb_record *records;size_t count;cs_m68k_bus_options options;
    unsigned interrupt_context;cs_tb_report last;uint32_t tag; } cs_tb_runtime;
/* SPEC-0029: all native objects/buffers live/disjoint/truthful; immutable while
 * active. Init validates all candidates before changing any directory slot. */
cs_tb_result cs_tb_init(cs_tb_runtime *runtime,cs_m68k_session *session,
    const cs_tb_record *records,size_t count,const cs_m68k_bus_options *options);
/* Trusted session run_using driver; strict trap arguments, staged writes.
 * Detailed Toolbox diagnostic is runtime.last; bus output is populated except
 * when a mandatory native pointer is absent, which leaves outputs untouched.
 * Exit is root-only; guest stack patches, continuations/direct tokens refused. */
cs_m68k_result cs_tb_step(void *context,cs_trap_frame *frame,
    cs_m68k_bus_report *report,const cs_m68k_stack_limits *limits);
#endif
