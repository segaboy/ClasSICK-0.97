/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/polled-phase.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

/* Build raw inputs from Gray phases; up=1 means button pressed (PB3 low). */
static int go(cs_mac_polled_phase *s, uint32_t xp, uint32_t yp, int down,
    cs_pointer_sample *o)
{
    uint32_t pb = (down ? 0u : 8u) | ((xp & 2u) ? 0x10u : 0u) |
                  ((yp & 2u) ? 0x20u : 0u);
    return cs_mac_polled_step(s, pb, (xp & 1u) << 3, (yp & 1u) << 3, o);
}

static void base(cs_mac_polled_phase *s, uint32_t inv)
{
    cs_pointer_sample o;
    CHECK(cs_mac_polled_begin(s, inv) == CS_MAC_POLLED_EMPTY);
    CHECK(go(s, 0, 0, 0, &o) == CS_MAC_POLLED_BASELINED);
}

int main(void)
{
    cs_mac_polled_phase s, b;
    cs_pointer_sample o;
    union { cs_mac_polled_phase s; cs_pointer_sample o[4]; } u;
    unsigned char sv[sizeof s], ov[sizeof o];

    /* X literal direction: DCD-changing forward steps give -1, reverse +1. */
    base(&s, 0);
    CHECK(go(&s, 1, 0, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dx == -1 && o.dy == 0);
    CHECK(go(&s, 3, 0, 0, &o) == CS_MAC_POLLED_EMPTY && o.dx == 0); /* PB-only */
    CHECK(go(&s, 2, 0, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dx == -1);
    CHECK(go(&s, 3, 0, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dx == 1);
    CHECK(s.samples == 3 && s.services == 5 && s.baselines == 1);

    /* Y forward +1; PB-only step has no motion. */
    base(&s, 0);
    CHECK(go(&s, 0, 1, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dy == 1 && o.dx == 0);
    CHECK(go(&s, 0, 3, 0, &o) == CS_MAC_POLLED_EMPTY && o.dy == 0);
    CHECK(go(&s, 0, 1, 0, &o) == CS_MAC_POLLED_EMPTY && o.dy == 0);
    CHECK(go(&s, 0, 0, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dy == -1);

    /* Inversion bits are per axis. */
    base(&s, 1);
    CHECK(go(&s, 1, 1, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dx == 1 && o.dy == 1);
    base(&s, 2);
    CHECK(go(&s, 1, 1, 0, &o) == CS_MAC_POLLED_SAMPLE && o.dx == -1 && o.dy == -1);

    /* Button alone, active low. */
    base(&s, 0);
    CHECK(go(&s, 0, 0, 1, &o) == CS_MAC_POLLED_SAMPLE && o.buttons == 1 && !o.dx && !o.dy);
    CHECK(go(&s, 0, 0, 1, &o) == CS_MAC_POLLED_EMPTY && o.buttons == 1);
    CHECK(go(&s, 0, 0, 0, &o) == CS_MAC_POLLED_SAMPLE && o.buttons == 0);

    /* Two-bit jump on X loses whole sample: no Y motion, no button. */
    base(&s, 0);
    o.dx = 9; o.dy = 9; o.buttons = 9;
    CHECK(go(&s, 3, 1, 1, &o) == CS_MAC_POLLED_LOST);
    CHECK(o.dx == 0 && o.dy == 0 && o.buttons == 0);
    CHECK(s.have_phase == 0 && s.losses == 1 && s.buttons == 0 && s.y_phase == 0);
    /* Same for a Y jump with X legal. */
    base(&s, 0);
    CHECK(go(&s, 1, 3, 1, &o) == CS_MAC_POLLED_LOST && !o.dx && !o.dy && !o.buttons);

    /* Rebaseline with held button: no press event is claimed. */
    CHECK(go(&s, 2, 2, 1, &o) == CS_MAC_POLLED_BASELINED);
    CHECK(o.dx == 0 && o.dy == 0 && o.buttons == 1 && s.buttons == 1);
    CHECK(go(&s, 2, 2, 1, &o) == CS_MAC_POLLED_EMPTY);
    CHECK(s.baselines == 2);

    /* Counters saturate independently. */
    base(&s, 0);
    s.services = s.samples = UINT32_MAX;
    CHECK(go(&s, 1, 0, 0, &o) == CS_MAC_POLLED_SAMPLE);
    CHECK(s.services == UINT32_MAX && s.samples == UINT32_MAX && s.baselines == 1);
    s.losses = UINT32_MAX;
    CHECK(go(&s, 0, 0, 0, &o) == CS_MAC_POLLED_SAMPLE);
    CHECK(go(&s, 3, 0, 0, &o) == CS_MAC_POLLED_LOST && s.losses == UINT32_MAX);
    s.baselines = UINT32_MAX;
    CHECK(go(&s, 3, 0, 0, &o) == CS_MAC_POLLED_BASELINED && s.baselines == UINT32_MAX);

    /* begin rejects without change; accepts and ignores old bytes. */
    memset(&s, 0xA5, sizeof s); memcpy(sv, &s, sizeof s);
    CHECK(cs_mac_polled_begin(NULL, 0) == CS_MAC_POLLED_ARGUMENT);
    CHECK(cs_mac_polled_begin(&s, 4) == CS_MAC_POLLED_ARGUMENT);
    CHECK(memcmp(&s, sv, sizeof s) == 0);
    CHECK(cs_mac_polled_begin(&s, 3) == CS_MAC_POLLED_EMPTY);
    CHECK(s.valid == CS_MAC_POLLED_VALID && s.invert == 3 && s.services == 0);

    /* Bad arguments leave state/output unchanged; argument beats bad state. */
    base(&s, 0);
    memcpy(sv, &s, sizeof s);
    memset(&o, 0x5A, sizeof o); memcpy(ov, &o, sizeof o);
    CHECK(cs_mac_polled_step(NULL, 0, 0, 0, &o) == CS_MAC_POLLED_ARGUMENT);
    CHECK(cs_mac_polled_step(&s, 0, 0, 0, NULL) == CS_MAC_POLLED_ARGUMENT);
    CHECK(cs_mac_polled_step(&s, 256, 0, 0, &o) == CS_MAC_POLLED_ARGUMENT);
    CHECK(cs_mac_polled_step(&s, 0, 256, 0, &o) == CS_MAC_POLLED_ARGUMENT);
    CHECK(cs_mac_polled_step(&s, 0, 0, 256, &o) == CS_MAC_POLLED_ARGUMENT);
    CHECK(memcmp(&s, sv, sizeof s) == 0 && memcmp(&o, ov, sizeof o) == 0);
    s.valid = 0; memcpy(&b, &s, sizeof b);
    CHECK(cs_mac_polled_step(&s, 256, 0, 0, &o) == CS_MAC_POLLED_ARGUMENT);
    CHECK(memcmp(&s, &b, sizeof b) == 0);

    /* Each malformed state field is rejected with bytes preserved. */
    {
        uint32_t *f[] = { &s.valid, &s.invert, &s.have_phase, &s.x_phase,
                          &s.y_phase, &s.buttons };
        uint32_t bad[] = { 0, 4, 2, 4, 4, 2 };
        unsigned i;
        for (i = 0; i < 6; i++) {
            base(&s, 0);
            *f[i] = bad[i];
            memcpy(sv, &s, sizeof s); memcpy(ov, &o, sizeof o);
            CHECK(go(&s, 1, 0, 0, &o) == CS_MAC_POLLED_STATE);
            CHECK(memcmp(&s, sv, sizeof s) == 0 && memcmp(&o, ov, sizeof o) == 0);
        }
    }

    /* Overlapping state/output rejected before any write. */
    base(&u.s, 0);
    memcpy(&b, &u.s, sizeof b);
    CHECK(cs_mac_polled_step(&u.s, 0x08, 8, 0, &u.o[0]) == CS_MAC_POLLED_ARGUMENT);
    CHECK(memcmp(&u.s, &b, sizeof b) == 0);
    CHECK(cs_mac_polled_step(&u.s, 0x08, 8, 0, &u.o[2]) == CS_MAC_POLLED_ARGUMENT);
    CHECK(memcmp(&u.s, &b, sizeof b) == 0);

    puts(fails ? "mac-polled-phase: FAILED" : "mac-polled-phase: ok");
    return fails != 0;
}
