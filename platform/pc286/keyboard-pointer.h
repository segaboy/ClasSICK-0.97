/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_KEYBOARD_POINTER_H
#define CLASSICK_PC286_KEYBOARD_POINTER_H
#include <stdint.h>
#include "../../core/input/pointer.h"

typedef struct { unsigned held; } cs_pc286_keyboard_pointer;
/* BIOS-style key word: scan code high byte, ASCII low byte. Arrow keys move
 * 16 pixels, Space toggles the held button. Unknown keys do not change state
 * or the output sample. State and output must be valid nonoverlapping objects.
 * The caller owns keyboard setup, event delivery, and any retry/recovery. */
int cs_pc286_keyboard_pointer_begin(cs_pc286_keyboard_pointer *state);
int cs_pc286_keyboard_pointer_key(cs_pc286_keyboard_pointer *state,uint16_t key,
                                 cs_pointer_sample *out);
#endif
