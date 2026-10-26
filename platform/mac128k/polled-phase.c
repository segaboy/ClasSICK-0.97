/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "polled-phase.h"

static void sat(uint32_t *c)
{
    if (*c != UINT32_MAX)
        (*c)++;
}

static uint32_t phase(uint32_t dcd, uint32_t pb, uint32_t mask)
{
    return ((dcd >> 3) & 1u) | ((pb & mask) ? 2u : 0u);
}

/* Gray order 0,1,3,2. Returns 0 on a legal step, else -1 (two-bit jump).
 * *d: 0 unchanged, 1 forward, -1 reverse; *dcd: DCD bit (bit0) changed. */
static int step_axis(uint32_t o, uint32_t n, int *d, int *dcd)
{
    static const uint32_t pos[4] = { 0, 1, 3, 2 };
    uint32_t delta = (pos[n] - pos[o]) & 3u;
    if (delta == 2u)
        return -1;
    *d = delta == 0u ? 0 : (delta == 1u ? 1 : -1);
    *dcd = ((o ^ n) & 1u) != 0u;
    return 0;
}

int cs_mac_polled_begin(cs_mac_polled_phase *s, uint32_t invert)
{
    if (!s || invert > 3u)
        return CS_MAC_POLLED_ARGUMENT;
    s->valid = CS_MAC_POLLED_VALID;
    s->invert = invert;
    s->have_phase = s->x_phase = s->y_phase = s->buttons = 0;
    s->services = s->samples = s->baselines = s->losses = 0;
    return CS_MAC_POLLED_EMPTY;
}

int cs_mac_polled_step(cs_mac_polled_phase *s, uint32_t pb, uint32_t rr0a,
    uint32_t rr0b, cs_pointer_sample *out)
{
    uintptr_t sa, se, oa, oe;
    uint32_t xp, yp, btn;
    int dx = 0, dy = 0, xd = 0, yd = 0, xc = 0, yc = 0;

    if (!s || !out || pb > 255u || rr0a > 255u || rr0b > 255u)
        return CS_MAC_POLLED_ARGUMENT;
    sa = (uintptr_t)s; se = sa + sizeof *s;
    oa = (uintptr_t)out; oe = oa + sizeof *out;
    if (se < sa || oe < oa || (sa < oe && oa < se))
        return CS_MAC_POLLED_ARGUMENT;
    if (s->valid != CS_MAC_POLLED_VALID || s->invert > 3u ||
        s->have_phase > 1u || s->x_phase > 3u || s->y_phase > 3u ||
        s->buttons > 1u)
        return CS_MAC_POLLED_STATE;

    sat(&s->services);
    xp = phase(rr0a, pb, 0x10u);
    yp = phase(rr0b, pb, 0x20u);
    btn = (pb & 0x08u) ? 0u : 1u; /* PB3 active low */

    if (!s->have_phase) {
        s->have_phase = 1; s->x_phase = xp; s->y_phase = yp;
        s->buttons = btn;
        out->dx = out->dy = 0; out->buttons = btn;
        sat(&s->baselines);
        return CS_MAC_POLLED_BASELINED;
    }
    if (step_axis(s->x_phase, xp, &xd, &xc) ||
        step_axis(s->y_phase, yp, &yd, &yc)) {
        s->have_phase = 0;
        sat(&s->losses);
        out->dx = out->dy = 0; out->buttons = 0;
        return CS_MAC_POLLED_LOST;
    }
    if (xc)
        dx = (s->invert & 1u) ? xd : -xd;
    if (yc)
        dy = (s->invert & 2u) ? -yd : yd;
    {
        int changed = dx || dy || btn != s->buttons;
        s->x_phase = xp; s->y_phase = yp; s->buttons = btn;
        out->dx = dx; out->dy = dy; out->buttons = btn;
        if (!changed)
            return CS_MAC_POLLED_EMPTY;
    }
    sat(&s->samples);
    return CS_MAC_POLLED_SAMPLE;
}
