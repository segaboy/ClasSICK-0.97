/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_CORE_INPUT_MOUSE_H
#define CS_CORE_INPUT_MOUSE_H
#include <stdint.h>

/* Shared button-edge semantics: a press is a sampled 0->1 edge only.
 * A button already held at begin is not a press; a 1->0 cancellation
 * is a plain release sample and never counts. */
typedef struct { uint32_t down, presses; } cs_mouse_button;
enum { CS_MOUSE_OK = 0, CS_MOUSE_CHANGED = 1, CS_MOUSE_ARGUMENT = 2, CS_MOUSE_STATE = 3 };

int cs_mouse_button_begin(cs_mouse_button *button, uint32_t down);
int cs_mouse_button_sample(cs_mouse_button *button, uint32_t down);

#endif
