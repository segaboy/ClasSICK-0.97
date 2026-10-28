/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/activity.h"

static int n, bad;
#define CK(c) do { n++; if (!(c)) { bad++; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)
#define V 0x80000000u
#define SA (CS_MAC_ACT_STABLE | CS_MAC_ACT_ACCEPTED)

static void refuse(cs_mac_activity *a, uint32_t s0, uint32_t s1, uint32_t f,
    uint32_t r, int rc)
{
    cs_mac_activity c = *a;
    CK(cs_mac_activity_pair(a, s0, s1, f, r) == rc);
    CK(memcmp(&c, a, sizeof c) == 0);
}

int main(void)
{
    cs_mac_activity a, b, z;
    uint32_t i;

    CK(sizeof a == 80u);
    CK(offsetof(cs_mac_activity, pairs) == 12u);
    CK(offsetof(cs_mac_activity, stable_pairs) == 36u);
    CK(offsetof(cs_mac_activity, last_observed) == 52u);
    CK(offsetof(cs_mac_activity, first_previous) == 60u);
    CK(offsetof(cs_mac_activity, first_service) == 76u);
    CK(cs_mac_activity_begin(NULL) == 1);
    memset(&a, 0xA5, sizeof a);
    CK(cs_mac_activity_begin(&a) == 0);
    memset(&z, 0, sizeof z);
    z.tag = 0x4D574131u; z.version = 1u; z.bytes = 80u;
    CK(memcmp(&a, &z, sizeof a) == 0);

    CK(cs_mac_activity_pair(NULL, 0, 0, 0, 0) == 1);
    b = a; b.tag ^= 1u;
    refuse(&b, 0, 0, 0, 0, 2);
    b = a; b.version = 2u;
    refuse(&b, 0, 0, 0, 0, 2);
    b = a; b.bytes = 79u;
    refuse(&b, 0, 0, 0, 0, 2);
    refuse(&a, 0x1000000u, 0, 0, 0, 1);
    refuse(&a, 0, 0x1000000u, 0, 0, 1);
    refuse(&a, 0, 0, 64u, 0, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_QUEUED | CS_MAC_ACT_STABLE, 0, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_MOTION | SA, 0, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_ACCEPTED, 0, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_LOST | CS_MAC_ACT_ACCEPTED | CS_MAC_ACT_STABLE, 1, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_LOST, 0, 1);
    refuse(&a, 0, 0, CS_MAC_ACT_LOST, 7, 1);
    refuse(&a, 0, 0, 0, 1, 1);

    /* first call: no previous observation */
    CK(cs_mac_activity_pair(&a, 0x111111u, 0, SA, 0) == 0);
    CK(a.pairs == 1 && a.stable_pairs == 1 && a.pb_x_changes == 0);
    CK(a.last_observed == V && a.previous_accepted == V);
    /* PB4 only, button-only queued */
    CK(cs_mac_activity_pair(&a, 0, 0x100000u, SA | CS_MAC_ACT_QUEUED, 0) == 0);
    CK(a.pb_x_changes == 1 && a.pb_y_changes == 0 && a.dcd_a_changes == 0);
    CK(a.buttononly_queued == 1 && a.nonzero_queued == 0);
    CK(a.previous_accepted == (V | 0x100000u));
    /* PB4 clears, PB5 and DCD A set, nonzero motion */
    CK(cs_mac_activity_pair(&a, 0, 0x200800u, SA | 4u | 8u, 0) == 0);
    CK(a.pb_x_changes == 2 && a.pb_y_changes == 1 && a.dcd_a_changes == 1);
    CK(a.dcd_b_changes == 0 && a.nonzero_queued == 1);
    /* DCD B only; stable but not accepted: observed differs from accepted */
    CK(cs_mac_activity_pair(&a, 0, 0x200808u, CS_MAC_ACT_STABLE, 0) == 0);
    CK(a.dcd_b_changes == 1 && a.dcd_a_changes == 1 && a.stable_pairs == 4);
    CK(a.last_observed == (V | 0x200808u) && a.previous_accepted == (V | 0x200800u));
    /* PB3 button only, unstable */
    CK(cs_mac_activity_pair(&a, 0, 0x280808u, 0, 0) == 0);
    CK(a.button_changes == 1 && a.stable_pairs == 4 && a.pb_y_changes == 1);
    /* Gray loss with HAD_PHASE */
    CK(cs_mac_activity_pair(&a, 0x111111u, 0x222222u,
        CS_MAC_ACT_LOST | CS_MAC_ACT_HAD_PHASE | CS_MAC_ACT_STABLE, 6) == 0);
    CK(a.losses == 1 && a.first_previous == (V | 0x200800u));
    CK(a.first_s0 == 0x111111u && a.first_s1 == 0x222222u);
    CK(a.first_reason == 6 && a.first_service == 6 && a.previous_accepted == 0);
    CK(a.button_changes == 2 && a.dcd_a_changes == 2 && a.dcd_b_changes == 2);
    CK(a.last_observed == (V | 0x222222u) && a.stable_pairs == 5);
    /* second loss: first record immutable */
    CK(cs_mac_activity_pair(&a, 0x333333u, 0x444444u, CS_MAC_ACT_LOST, 5) == 0);
    CK(a.losses == 2 && a.pairs == 7 && a.first_reason == 6);
    CK(a.first_previous == (V | 0x200800u) && a.first_s1 == 0x222222u);
    CK(a.first_service == 6 && a.previous_accepted == 0);

    /* HAD_PHASE false drops previous */
    cs_mac_activity_begin(&b);
    CK(cs_mac_activity_pair(&b, 0, 0x123456u, SA, 0) == 0);
    CK(cs_mac_activity_pair(&b, 1, 2, CS_MAC_ACT_LOST, 1) == 0);
    CK(b.first_previous == 0 && b.first_service == 2 && b.first_reason == 1);
    CK(b.first_s0 == 1 && b.first_s1 == 2);
    /* unstable loss keeps complete PB */
    cs_mac_activity_begin(&b);
    CK(cs_mac_activity_pair(&b, 0, 0x3F0A0Bu, SA, 0) == 0);
    CK(cs_mac_activity_pair(&b, 0x3F0A0Bu, 0x3F0A8Bu,
        CS_MAC_ACT_LOST | CS_MAC_ACT_HAD_PHASE, 1) == 0);
    CK(b.first_previous == (V | 0x3F0A0Bu) && b.first_reason == 1);

    /* saturation */
    cs_mac_activity_begin(&b);
    b.pairs = b.stable_pairs = b.nonzero_queued = b.buttononly_queued = UINT32_MAX;
    b.pb_x_changes = b.pb_y_changes = b.dcd_a_changes = UINT32_MAX;
    b.dcd_b_changes = b.button_changes = b.losses = UINT32_MAX;
    b.last_observed = V;
    CK(cs_mac_activity_pair(&b, 0, 0x3F0808u, SA | 4u | 8u, 0) == 0);
    CK(b.pairs == UINT32_MAX && b.stable_pairs == UINT32_MAX);
    CK(b.nonzero_queued == UINT32_MAX && b.pb_x_changes == UINT32_MAX);
    CK(b.button_changes == UINT32_MAX && b.dcd_b_changes == UINT32_MAX);
    CK(cs_mac_activity_pair(&b, 0, 0, CS_MAC_ACT_LOST, 2) == 0);
    CK(b.losses == UINT32_MAX && b.first_reason == 2);
    CK(cs_mac_activity_pair(&b, 0, 0, SA | 4u, 0) == 0);
    CK(b.buttononly_queued == UINT32_MAX);
    for (i = 0; i < 2; i++)
        CK(cs_mac_activity_pair(&b, 0, 0, SA | 4u | 8u, 0) == 0);

    printf("assertions=%d failures=%d\n", n, bad);
    return bad ? 1 : 0;
}
