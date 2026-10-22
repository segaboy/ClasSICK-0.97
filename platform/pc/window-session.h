/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_WINDOW_SESSION_H
#define CLASSICK_PC_WINDOW_SESSION_H
#include "window-input.h"
#include "ps2-bus.h"
#include "mono-present.h"

enum { CS_PWS_OK=0, CS_PWS_ARGUMENT=1, CS_PWS_STATE=2, CS_PWS_STARTUP=3,
    CS_PWS_CLOCK=4, CS_PWS_CONTROLLER=5, CS_PWS_QUEUE=6, CS_PWS_OWNER=7,
    CS_PWS_PRESENT=8 };
enum { CS_PWS_RUNNING=1, CS_PWS_FAULTED=2, CS_PWS_STOPPED=3 };
enum { CS_PWS_CURSOR=1, CS_PWS_BEGIN=2, CS_PWS_CANCEL=3,
    CS_PWS_PUBLISH=4, CS_PWS_BUS=5, CS_PWS_TIME=6,
    CS_PWS_POLL=7, CS_PWS_DRAIN=8, CS_PWS_DISPATCH=9 };
#define CS_PWS_MAGIC 0x50575331u
enum { CS_PWS_CANVAS=21888, CS_PWS_PAYLOAD=48,
    CS_PWS_QUEUE_CAPACITY=8, CS_PWS_STALL_LIMIT=1000000 };

typedef struct {
    cs_fb_target target;
    cs_ps2_io io;
    uint8_t *canvas, *payload, *row;
    size_t canvas_capacity, payload_capacity, row_capacity;
} cs_pc_window_config;

/* Caller-owned native graph. Never copy/move/reinitialize while borrowed.
 * See SPEC-0061 for numeric preflight and exclusive lifetime requirements. */
typedef struct {
    cs_surface canvas;
    cs_fb_target target;
    cs_ps2_io io;
    cs_cursor_state cursor;
    cs_cursor_bits bits;
    cs_cursor_art art;
    cs_window_input owner;
    cs_ps2_bus bus;
    cs_input_queue queue;
    cs_input_record records[CS_PWS_QUEUE_CAPACITY];
    cs_ps2_bus_delivery delivery;
    cs_pc_mono_report presentation;
    uint8_t *row;
    size_t row_capacity;
    uint64_t last_tick;
    uint32_t magic, phase, result, stage;
    int32_t cause;
    uint32_t clock_seen, stalled, consumed, drained;
    uint32_t ram_frames, published_frames, bus_result;
} cs_pc_window_session;

/* Start only on fresh/quiescent storage. Preflight refusal has no effects.
 * Success publishes one post-cancel frame, then initializes the controller. */
uint32_t cs_pc_window_session_start(cs_pc_window_session *, const cs_pc_window_config *);
/* Extended monotonic PM ticks; clock_ok must be 1. No clock reads in this leaf.
 * A turn clears delivery before clock checks, polls once, drains <=8, dispatches. */
uint32_t cs_pc_window_session_poll(cs_pc_window_session *, uint64_t now, uint32_t clock_ok);
/* Quiesce only: no cancel/publish/hardware shutdown or implicit cursor detach. */
uint32_t cs_pc_window_session_stop(cs_pc_window_session *);
#endif
