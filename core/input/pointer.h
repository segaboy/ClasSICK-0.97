/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_CORE_INPUT_POINTER_H
#define CS_CORE_INPUT_POINTER_H
#include <stdint.h>

/* SPEC-0051 native pointer semantic core. State is caller-owned and
 * relocatable; no globals, allocation or retained pointers. The press
 * counter remains an owner concern via the unchanged mouse.h API. */
enum { CS_POINTER_WIDTH = 512, CS_POINTER_HEIGHT = 342 };

typedef struct { int32_t x, y; uint32_t down, suppress, valid; } cs_pointer;
typedef struct { int32_t dx, dy; uint32_t buttons; } cs_pointer_sample;
typedef struct { int32_t x, y; uint32_t flags, buttons; } cs_pointer_event;
enum { CS_POINTER_OK = 0, CS_POINTER_CHANGED = 1,
       CS_POINTER_ARGUMENT = 2, CS_POINTER_STATE = 3 };
enum { CS_POINTER_LEFT = 1u, CS_POINTER_RIGHT = 2u, CS_POINTER_MIDDLE = 4u,
       CS_POINTER_BUTTON_MASK = 7u };
enum { CS_POINTER_MOVED = 1u, CS_POINTER_PRESS = 2u,
       CS_POINTER_RELEASE = 4u, CS_POINTER_DOWN = 8u,
       CS_POINTER_SUPPRESSED = 16u, CS_POINTER_CANCELLED = 32u,
       CS_POINTER_CLAMP_X = 64u, CS_POINTER_CLAMP_Y = 128u };
#define CS_POINTER_VALID 0x50544E31u /* begun marker, not a security token */

/* Fresh/quiescent owner session only; never reads the old *p. */
int cs_pointer_begin(cs_pointer *p, uint32_t buttons);
/* One sample, one transition: motion, then button, then publish. */
int cs_pointer_sample_apply(cs_pointer *p, const cs_pointer_sample *s,
    cs_pointer_event *out);
/* Uncertainty gate: never fabricates a press; always CHANGED. */
int cs_pointer_cancel(cs_pointer *p, cs_pointer_event *out);

#endif
