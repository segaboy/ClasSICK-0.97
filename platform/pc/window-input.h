/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_WINDOW_INPUT_H
#define CLASSICK_PC_WINDOW_INPUT_H
#include "ps2-mouse.h"
#include "../../apps/window-scene/input.h"

/* SPEC-0057: one already-decoded standard report, delivered exactly once.
 * Engineering profile: raw +X -> right, raw +Y -> up; logical +Y -> down.
 * Results are CS_WINDOW_INPUT_*. No I/O, decoder, allocation or retained borrow.
 * Owner/packet and the owner's borrowed graph are live and disjoint. The
 * caller owns freshness; cumulative/saturating counters are not event tokens.
 * On known report loss call cs_window_input_cancel, not this function. */
int cs_pc_window_packet(cs_window_input *owner,
    const cs_ps2_mouse_packet *packet);
#endif
