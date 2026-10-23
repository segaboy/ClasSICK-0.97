/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_MAC128K_WINDOW_SESSION_H
#define CLASSICK_MAC128K_WINDOW_SESSION_H
#include "../../apps/window-scene/foreground.h"

/* Software-only Mac RAM window-session bootstrap. Composes existing surface,
 * cursor, owner, capture, inbox and foreground APIs over one borrowed
 * 512x342 Mono1MSB stride64 canvas in ordinary stable RAM. No hardware,
 * mapping, ISR, CPU primitive, allocation, libc or globals. Caller supplies a
 * fresh/quiescent live aligned session, stable valid hook pair and keeps the
 * producer off until a later fresh external baseline. Session fields are an
 * owned diagnostic graph, not a hostile-object boundary. */
#define CS_MAC_WINDOW_MAGIC 0x434D5753u /* arbitrary tag */
enum { CS_MAC_WINDOW_WIDTH=512, CS_MAC_WINDOW_HEIGHT=342,
       CS_MAC_WINDOW_STRIDE=64, CS_MAC_WINDOW_BYTES=21888 };
enum { CS_MAC_WINDOW_OK=0, CS_MAC_WINDOW_ARGUMENT=1, CS_MAC_WINDOW_FAULT=2 };
/* Bootstrap outcome only; not a duplicate of the ongoing foreground phase. */
enum { CS_MAC_WINDOW_BUILDING=1, CS_MAC_WINDOW_READY=2,
       CS_MAC_WINDOW_FAILED=3 };
enum { CS_MAC_WINDOW_STAGE_START=1, CS_MAC_WINDOW_STAGE_SURFACE=2,
       CS_MAC_WINDOW_STAGE_CURSOR_INIT=3, CS_MAC_WINDOW_STAGE_CURSOR_BIND=4,
       CS_MAC_WINDOW_STAGE_INPUT=5, CS_MAC_WINDOW_STAGE_FOREGROUND=6,
       CS_MAC_WINDOW_STAGE_READY=7 };
typedef struct {
    uint32_t magic,phase,stage;
    int32_t cause; /* exact raw result of the failing call, else 0 */
    cs_surface surface;
    cs_cursor_state cursor;
    cs_cursor_bits bits;
    cs_cursor_art art;
    uint8_t payload[CS_CURSOR_MONO_PAYLOAD];
    cs_window_input input;
    cs_window_capture capture;
    cs_window_inbox inbox;
    cs_window_foreground foreground;
} cs_mac_window_session;
_Static_assert(sizeof(cs_mac_window_session)<=1536,"Bounded RAM graph; canvas and stack separate");

/* Numeric preflight (null, callbacks, short capacity, span overflow,
 * alignment, pairwise overlap of session/canvas/hooks) precedes any write or
 * hook; refusal returns ARGUMENT with no effects. Hooks are copied by value
 * first. Later failure returns FAULT with FAILED phase, reached stage, raw
 * cause and reached graph/pixels retained; no rollback, retry or recovery. */
int cs_mac_window_begin(cs_mac_window_session *session,uint8_t *canvas,
    size_t capacity,const cs_foreground_hooks *hooks);
#endif
