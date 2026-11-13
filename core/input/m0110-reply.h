/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_CORE_INPUT_M0110_REPLY_H
#define CS_CORE_INPUT_M0110_REPLY_H
#include <stdint.h>

/* SPEC-0099: pure experimental key-transition reply parsing. This accepts
 * only replies to a caller-owned key inquiry, not other keyboard commands. */
typedef struct { uint32_t position, pressed; } cs_m0110_event;
enum { CS_M0110_EVENT = 0, CS_M0110_EMPTY = 1,
       CS_M0110_INVALID = 2, CS_M0110_ARGUMENT = 3 };

/* On EMPTY, INVALID or ARGUMENT, out is unchanged. No I/O or retained state. */
int cs_m0110_reply(uint32_t reply, cs_m0110_event *out);
#endif
