/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CPU_M68K_SESSION_H
#define CLASSICK_CPU_M68K_SESSION_H
#include "m68k-call.h"
typedef enum {
    CS_M68K_SESSION_OK=0,CS_M68K_SESSION_RETURNED=1,CS_M68K_SESSION_EXITED=2,
    CS_M68K_SESSION_ARGUMENT=3,CS_M68K_SESSION_CONFIG=4,CS_M68K_SESSION_CONTEXT=5,
    CS_M68K_SESSION_ADDRESS=6,CS_M68K_SESSION_DEPTH=7,CS_M68K_SESSION_BUDGET=8,
    CS_M68K_SESSION_CPU=9,CS_M68K_SESSION_RETURN=10
} cs_m68k_session_result;
typedef struct {
    uint32_t entry,a5,budget;
    cs_m68k_call_context context;
} cs_m68k_session_request;
typedef struct {
    cs_m68k_session_result status;cs_m68k_result cpu;cs_guest_result memory;
    uint32_t pc,steps,exit_code;unsigned entered;
    cs_trap_frame guest;
} cs_m68k_session_report;
typedef struct {
    cs_m68k_calls calls;
    cs_m68k_call_step driver;void *driver_context;
    cs_m68k_session_report *current;
    uint32_t tag,active,stepping,exit_pending,exit_code;
} cs_m68k_session;
/* SPEC-0028: caller-owned/disjoint truthful storage, no active reinitialization.
 * Depth/budget/transient state is owned by this API. Inactive option/config
 * changes are revalidated; map/options/config stay immutable during execution. */
cs_m68k_session_result cs_m68k_session_init(cs_m68k_session *session,
    const cs_guest_map *map,cs_trap_gateway *gateway,
    const cs_m68k_bus_options *options,const cs_m68k_call_config *config);
/* Complete caller frame restored; report.guest is the root stop snapshot.
 * Completed guest/service effects survive termination; no heap teardown. */
cs_m68k_session_result cs_m68k_session_run(cs_m68k_session *session,
    const cs_m68k_session_request *request,cs_trap_frame *frame,
    cs_m68k_session_report *report);
cs_m68k_session_result cs_m68k_session_run_using(cs_m68k_session *session,
    const cs_m68k_session_request *request,cs_trap_frame *frame,
    cs_m68k_session_report *report,cs_m68k_call_step step,void *context);
/* Trusted synchronous root-step service only. No actual Toolbox trap binding. */
cs_m68k_session_result cs_m68k_session_request_exit(cs_m68k_session *session,uint32_t code);
/* Ordinary returning callbacks share the active session's executor/budgets.
 * Never call the embedded returning-call runtime directly. */
cs_m68k_call_result cs_m68k_session_call(cs_m68k_session *session,
    const cs_m68k_call_request *request,cs_trap_frame *frame,cs_m68k_call_report *report);
#endif
