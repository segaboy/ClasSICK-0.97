/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOW_CAPTURE_H
#define CLASSICK_WINDOW_CAPTURE_H
#include "input.h"

/* SPEC-0063: sequential original helper, NOT an interrupt-safe queue.
 * Caller supplies exclusion, live/aligned/disjoint capture and owner graph.
 * Producer push/uncertain never dereference or call the owner.
 * All other functions are foreground-only. No MMIO or allocation. */
enum { CS_CAPTURE_CAPACITY=16 };
enum { CS_CAPTURE_OK=0, CS_CAPTURE_EMPTY=1, CS_CAPTURE_DELIVERED=2,
    CS_CAPTURE_LOST=3, CS_CAPTURE_BASELINE=4, CS_CAPTURE_STOPPED=5,
    CS_CAPTURE_ARGUMENT=6, CS_CAPTURE_STATE=7, CS_CAPTURE_OWNER=8,
    CS_CAPTURE_EXHAUSTED=9, CS_CAPTURE_RESET=10 };
enum { CS_CAPTURE_READY=1, CS_CAPTURE_LOSS=2, CS_CAPTURE_WAIT=3,
    CS_CAPTURE_FAULT=4, CS_CAPTURE_ENDED=5 };
enum { CS_CAPTURE_OVERFLOW=1, CS_CAPTURE_UNCERTAIN=2, CS_CAPTURE_STARTUP=3 };
enum { CS_CAPTURE_BEGIN_CANCEL=1, CS_CAPTURE_SAMPLE=2,
    CS_CAPTURE_LOSS_CANCEL=3, CS_CAPTURE_REBASELINE=4,
    CS_CAPTURE_STOP_CANCEL=5 };
#define CS_CAPTURE_VALID 0x43415031u
typedef struct {
    uint32_t valid,epoch,phase,head,count,loss_reason,stage;
    int32_t cause; /* first non-success owner result, retained until restart */
    uint32_t delivered,losses; /* saturating diagnostics, never freshness */
    cs_window_input *owner;
    cs_pointer_sample records[CS_CAPTURE_CAPACITY],last;
} cs_window_capture;
_Static_assert(sizeof(cs_window_capture)<=256,"Bounded capture descriptor");

/* Once per fresh quiescent object lifetime;never reads old *capture. Cancels
 * ready owner once,starts epoch1 in WAIT;failure retains initialized FAULT. */
int cs_capture_begin(cs_window_capture*,cs_window_input*);
/* ENDED only;increment epoch without wrap,clear old diagnostics,then same begin
 * behavior. Caller has stopped all old producers. New owner is already READY. */
int cs_capture_restart(cs_window_capture*,cs_window_input*);
int cs_capture_push(cs_window_capture*,uint32_t epoch,int32_t dx,int32_t dy,uint32_t buttons);
int cs_capture_uncertain(cs_window_capture*,uint32_t epoch);
/* At most one owner call. LOSS drops prefix,cancels once,WAIT/RESET on success.
 * READY removes one committed record before the call;never retries a failure. */
int cs_capture_drain(cs_window_capture*,uint32_t epoch);
/* WAIT only. Submit zero motion/current buttons through cancelled owner;
 * held left remains suppressed,observed release can clear that gate. */
int cs_capture_rebaseline(cs_window_capture*,uint32_t epoch,uint32_t buttons);
/* End admission/drop pending first;cancel once if owner is READY. Remains
 * ENDED even if cancel fails. Repeated stop makes no owner call. */
int cs_capture_stop(cs_window_capture*,uint32_t epoch);
#endif
