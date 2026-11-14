/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_KEYBOARD_SET2_H
#define CLASSICK_PC286_KEYBOARD_SET2_H
#include <stdint.h>
#include "../../core/input/pointer.h"
typedef struct { unsigned held,release,extended,pause; } cs_pc286_set2_pointer;
/* Feed one already-delivered set-2 byte. Returns 1 for a sample, 0 for a
 * prefix, repeat or ignored byte, -1 for invalid arguments/state. Output is
 * unchanged unless a sample is returned. State and output must not overlap. */
int cs_pc286_set2_begin(cs_pc286_set2_pointer *state);
int cs_pc286_set2_feed(cs_pc286_set2_pointer *state,uint8_t byte,cs_pointer_sample *out);
/* Alternate translated set-1 stream with the same state/output contract.
 * Select one stream format for the whole initialized session. */
int cs_pc286_set1_feed(cs_pc286_set2_pointer *state,uint8_t byte,cs_pointer_sample *out);
#endif
