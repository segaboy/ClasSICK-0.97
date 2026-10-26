/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_POLLED_PHASE_H
#define CS_MAC_POLLED_PHASE_H
#include <stdint.h>
#include "../../core/input/pointer.h"

/* Pure polled-phase model. No MMIO, IRQ, inbox or SCC programming. Raw PB and
 * RR0A/RR0B are 8-bit values from a future validated live-level sampler.
 * State is caller-owned (<=64 bytes) and serialized; the call checks only
 * flat state/output non-overlap, not arbitrary pointer validity.
 * Endpoint phases cannot reveal missed full or three-step cycles; no physical
 * edge completeness or timing guarantee is claimed. */
#define CS_MAC_POLLED_VALID 0x504f4c31u

enum { CS_MAC_POLLED_SAMPLE = 0, CS_MAC_POLLED_EMPTY = 1,
       CS_MAC_POLLED_BASELINED = 2, CS_MAC_POLLED_LOST = 3,
       CS_MAC_POLLED_ARGUMENT = 4, CS_MAC_POLLED_STATE = 5 };

typedef struct {
    uint32_t valid, invert, have_phase, x_phase, y_phase, buttons;
    uint32_t services, samples, baselines, losses; /* saturating */
} cs_mac_polled_phase;
typedef char cs_mac_polled_state_is_bounded[(sizeof(cs_mac_polled_phase)<=64)?1:-1];

/* Fresh initialization; never reads old bytes. invert bit0=X, bit1=Y.
 * Success is EMPTY. No baseline sample exists until the first step. */
int cs_mac_polled_begin(cs_mac_polled_phase *s, uint32_t invert);
int cs_mac_polled_step(cs_mac_polled_phase *s, uint32_t pb, uint32_t rr0a,
    uint32_t rr0b, cs_pointer_sample *out);

#endif
