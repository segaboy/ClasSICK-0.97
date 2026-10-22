/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_WINDOW_H
#define CLASSICK_UEFI_WINDOW_H
#include "native.h"
#include "../pc/window-session.h"
enum { CS_UWIN_RUNNING=0, CS_UWIN_STOPPED=1, CS_UWIN_ARGUMENT=2,
    CS_UWIN_HANDOFF=3, CS_UWIN_NOT_EXITED=4, CS_UWIN_NOT_READY=5,
    CS_UWIN_MAP=6, CS_UWIN_TIMER=7, CS_UWIN_NO_8042=8,
    CS_UWIN_TARGET=9, CS_UWIN_ARENA=10, CS_UWIN_SESSION=11,
    CS_UWIN_CLOCK=12 };
enum { CS_UWIN_GATE=1, CS_UWIN_ACTIVE=2, CS_UWIN_DONE=3, CS_UWIN_FAULT=4 };
enum { CS_UWIN_GATE_MODULE=1, CS_UWIN_CLOCK_MODULE=2,
    CS_UWIN_SESSION_MODULE=3, CS_UWIN_STOP_MODULE=4 };
enum { CS_UWIN_TRACE_MAGIC=0x31505743u, CS_UWIN_TRACE_VERSION=1,
    CS_UWIN_TRACE_BYTES=160, CS_UWIN_TRACE_OFFSET=2048 };
/* Readonly, synchronous observer. Nonzero requests explicit stop; no mutation,
 * reentry, mappings invalidation or storage reuse, including after return. */
typedef int (*cs_native_window_observer)(void *, const unsigned char *,
    const cs_pc_window_session *);
typedef struct {
    const cs_native_devices *devices;
    const cs_ps2_io *keyboard;
    uint64_t physical, size;
    uintptr_t base; /* native mapping of physical; validate each actual span */
    cs_native_window_observer observer;
    void *context;
} cs_native_window_binding;
/* SPEC-0062: one owned arena allocation, continuous native session and distinct
 * versioned trace. seconds1..3600 is metadata only; NULL observer never stops
 * healthy execution. No firmware service, UART, old drawing or implicit detach. */
uint32_t cs_native_window_loop(const cs_uefi_handoff *, uint32_t ready,
    const cs_native_window_binding *, uint32_t seconds, unsigned char *trace);
#endif
