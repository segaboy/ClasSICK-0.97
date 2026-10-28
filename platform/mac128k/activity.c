/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "activity.h"

static void sat(uint32_t *c)
{
    if (*c != UINT32_MAX)
        (*c)++;
}

static void chg(uint32_t *c, uint32_t d, uint32_t mask)
{
    if (d & mask)
        sat(c);
}

int cs_mac_activity_begin(cs_mac_activity *a)
{
    if (!a)
        return 1;
    a->tag = CS_MAC_ACTIVITY_TAG;
    a->version = CS_MAC_ACTIVITY_VERSION;
    a->bytes = (uint32_t)sizeof *a;
    a->pairs = a->pb_x_changes = a->pb_y_changes = 0;
    a->dcd_a_changes = a->dcd_b_changes = a->button_changes = 0;
    a->stable_pairs = a->nonzero_queued = a->buttononly_queued = 0;
    a->losses = a->last_observed = a->previous_accepted = 0;
    a->first_previous = a->first_s0 = a->first_s1 = 0;
    a->first_reason = a->first_service = 0;
    return 0;
}

int cs_mac_activity_pair(cs_mac_activity *a, uint32_t s0, uint32_t s1,
    uint32_t flags, uint32_t reason)
{
    uint32_t lost, d;

    if (!a)
        return 1;
    if (a->tag != CS_MAC_ACTIVITY_TAG || a->version != CS_MAC_ACTIVITY_VERSION ||
        a->bytes != (uint32_t)sizeof *a)
        return 2;
    lost = (flags & CS_MAC_ACT_LOST) != 0;
    if ((s0 | s1) > UINT32_C(0xFFFFFF) || (flags & ~UINT32_C(63)) ||
        ((flags & CS_MAC_ACT_QUEUED) && !(flags & CS_MAC_ACT_ACCEPTED)) ||
        ((flags & CS_MAC_ACT_MOTION) && !(flags & CS_MAC_ACT_QUEUED)) ||
        ((flags & CS_MAC_ACT_ACCEPTED) && !(flags & CS_MAC_ACT_STABLE)) ||
        (lost && (flags & (CS_MAC_ACT_ACCEPTED | CS_MAC_ACT_QUEUED |
                           CS_MAC_ACT_MOTION))) ||
        (lost ? (reason < 1u || reason > 6u) : reason != 0u))
        return 1;

    sat(&a->pairs);
    if (a->last_observed & CS_MAC_ACTIVITY_VALID) {
        d = (a->last_observed ^ s1) & 0xFFFFFFu;
        chg(&a->pb_x_changes, d, 0x100000u);
        chg(&a->pb_y_changes, d, 0x200000u);
        chg(&a->dcd_a_changes, d, 0x000800u);
        chg(&a->dcd_b_changes, d, 0x000008u);
        chg(&a->button_changes, d, 0x080000u);
    }
    if (flags & CS_MAC_ACT_STABLE)
        sat(&a->stable_pairs);
    if (flags & CS_MAC_ACT_QUEUED)
        sat((flags & CS_MAC_ACT_MOTION) ? &a->nonzero_queued
                                        : &a->buttononly_queued);
    if (lost) {
        sat(&a->losses);
        if (!a->first_reason) {
            a->first_previous = (flags & CS_MAC_ACT_HAD_PHASE)
                ? a->previous_accepted : 0u;
            a->first_s0 = s0;
            a->first_s1 = s1;
            a->first_service = a->pairs;
            a->first_reason = reason;
        }
        a->previous_accepted = 0;
    }
    if (flags & CS_MAC_ACT_ACCEPTED)
        a->previous_accepted = s1 | CS_MAC_ACTIVITY_VALID;
    a->last_observed = s1 | CS_MAC_ACTIVITY_VALID;
    return 0;
}
