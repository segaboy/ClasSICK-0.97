/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_ACTIVITY_H
#define CS_MAC_ACTIVITY_H
#include <stdint.h>

/* Passive caller-owned MWA1v1 diagnostic record: 20 uint32_t, 80 bytes.
 * No MMIO, globals, allocation or callbacks. Raw = PB<<16|RR0A<<8|RR0B
 * (24 bits). Packed endpoint fields use bit31 as VALID. last_observed is the latest
 * observed second triple even if rejected; previous_accepted is the last
 * decoder-admitted triple and is cleared on loss. Counts make no claim of
 * physical edges, missed phases, latency or wiring correctness. */
#define CS_MAC_ACTIVITY_TAG 0x4D574131u
#define CS_MAC_ACTIVITY_VERSION 1u
#define CS_MAC_ACTIVITY_VALID 0x80000000u

enum { CS_MAC_ACT_STABLE = 1, CS_MAC_ACT_ACCEPTED = 2, CS_MAC_ACT_QUEUED = 4,
       CS_MAC_ACT_MOTION = 8, CS_MAC_ACT_LOST = 16, CS_MAC_ACT_HAD_PHASE = 32 };

typedef struct {
    uint32_t tag, version, bytes, pairs, pb_x_changes, pb_y_changes,
        dcd_a_changes, dcd_b_changes, button_changes, stable_pairs,
        nonzero_queued, buttononly_queued, losses, last_observed,
        previous_accepted, first_previous, first_s0, first_s1, first_reason,
        first_service;
} cs_mac_activity;
typedef char cs_mac_activity_is_80[(sizeof(cs_mac_activity) == 80) ? 1 : -1];

/* 0 success, 1 argument. Never reads old bytes. */
int cs_mac_activity_begin(cs_mac_activity *a);
/* 0 success, 1 bad argument, 2 bad state. Refusal has no side effects.
 * HAD_PHASE is the decoder state before the step. reason 1..6 iff LOST. */
int cs_mac_activity_pair(cs_mac_activity *a, uint32_t s0, uint32_t s1,
    uint32_t flags, uint32_t reason);
#endif
