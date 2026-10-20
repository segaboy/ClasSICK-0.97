/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stddef.h>
#include "pointer.h"

#define PTR_MAX_X ((int32_t)(CS_POINTER_WIDTH - 1))
#define PTR_MAX_Y ((int32_t)(CS_POINTER_HEIGHT - 1))

/* Flat uintptr_t range; refuses a range whose end would wrap. */
static int ptr_span(const void *p, size_t n, uintptr_t *b, uintptr_t *e)
{
    uintptr_t base = (uintptr_t)p;

    if (n > UINTPTR_MAX - base)
        return 0;
    *b = base;
    *e = base + (uintptr_t)n;
    return 1;
}

static int ptr_disjoint(uintptr_t b1, uintptr_t e1, uintptr_t b2, uintptr_t e2)
{
    return e1 <= b2 || e2 <= b1;
}

static int ptr_state_ok(const cs_pointer *p)
{
    return p->valid == CS_POINTER_VALID
        && p->x >= 0 && p->x <= PTR_MAX_X
        && p->y >= 0 && p->y <= PTR_MAX_Y
        && p->down <= 1u && p->suppress <= 1u
        && !(p->down == 1u && p->suppress == 1u);
}

/* Overflow-free clamp; pos is already admitted in [0,max]. */
static int32_t ptr_axis(int32_t pos, int32_t d, int32_t max, int *clamped)
{
    if (d > 0) {
        if (d > max - pos) {
            *clamped = 1;
            return max;
        }
        return pos + d;
    }
    if (d < 0) {
        if (d < -pos) {
            *clamped = 1;
            return 0;
        }
        return pos + d;
    }
    return pos;
}

int cs_pointer_begin(cs_pointer *p, uint32_t buttons)
{
    uintptr_t pb, pe;

    if (p == NULL)
        return CS_POINTER_ARGUMENT;
    if (!ptr_span(p, sizeof *p, &pb, &pe))
        return CS_POINTER_ARGUMENT;
    if ((buttons & ~(uint32_t)CS_POINTER_BUTTON_MASK) != 0u)
        return CS_POINTER_ARGUMENT;
    p->x = (int32_t)(CS_POINTER_WIDTH / 2);
    p->y = (int32_t)(CS_POINTER_HEIGHT / 2);
    p->down = 0u;
    p->suppress = (buttons & (uint32_t)CS_POINTER_LEFT) != 0u ? 1u : 0u;
    p->valid = CS_POINTER_VALID;
    return CS_POINTER_OK;
}

int cs_pointer_sample_apply(cs_pointer *p, const cs_pointer_sample *s,
    cs_pointer_event *out)
{
    uintptr_t pb, pe, sb, se, ob, oe;
    int32_t x, y;
    uint32_t buttons, left, down, suppress, flags = 0u;
    int cx = 0, cy = 0, changed;

    if (p == NULL || s == NULL || out == NULL)
        return CS_POINTER_ARGUMENT;
    if (!ptr_span(p, sizeof *p, &pb, &pe) || !ptr_span(s, sizeof *s, &sb, &se)
        || !ptr_span(out, sizeof *out, &ob, &oe))
        return CS_POINTER_ARGUMENT;
    if (!ptr_disjoint(pb, pe, sb, se) || !ptr_disjoint(pb, pe, ob, oe)
        || !ptr_disjoint(sb, se, ob, oe))
        return CS_POINTER_ARGUMENT;
    buttons = s->buttons;
    if ((buttons & ~(uint32_t)CS_POINTER_BUTTON_MASK) != 0u)
        return CS_POINTER_ARGUMENT;
    if (!ptr_state_ok(p))
        return CS_POINTER_STATE;

    x = ptr_axis(p->x, s->dx, PTR_MAX_X, &cx);
    y = ptr_axis(p->y, s->dy, PTR_MAX_Y, &cy);
    left = buttons & (uint32_t)CS_POINTER_LEFT;
    down = p->down;
    suppress = p->suppress;
    if (suppress == 1u) {
        if (left == 0u)
            suppress = 0u;
    } else if (down == 0u && left != 0u) {
        down = 1u;
        flags |= (uint32_t)CS_POINTER_PRESS;
    } else if (down == 1u && left == 0u) {
        down = 0u;
        flags |= (uint32_t)CS_POINTER_RELEASE;
    }
    if (x != p->x || y != p->y)
        flags |= (uint32_t)CS_POINTER_MOVED;
    if (cx)
        flags |= (uint32_t)CS_POINTER_CLAMP_X;
    if (cy)
        flags |= (uint32_t)CS_POINTER_CLAMP_Y;
    changed = (flags & (uint32_t)(CS_POINTER_MOVED | CS_POINTER_PRESS
        | CS_POINTER_RELEASE)) != 0u || suppress != p->suppress;
    if (down == 1u)
        flags |= (uint32_t)CS_POINTER_DOWN;
    if (suppress == 1u)
        flags |= (uint32_t)CS_POINTER_SUPPRESSED;

    p->x = x;
    p->y = y;
    p->down = down;
    p->suppress = suppress;
    out->x = x;
    out->y = y;
    out->flags = flags;
    out->buttons = buttons;
    return changed ? CS_POINTER_CHANGED : CS_POINTER_OK;
}

int cs_pointer_cancel(cs_pointer *p, cs_pointer_event *out)
{
    uintptr_t pb, pe, ob, oe;
    uint32_t flags = (uint32_t)(CS_POINTER_CANCELLED | CS_POINTER_SUPPRESSED);

    if (p == NULL || out == NULL)
        return CS_POINTER_ARGUMENT;
    if (!ptr_span(p, sizeof *p, &pb, &pe)
        || !ptr_span(out, sizeof *out, &ob, &oe)
        || !ptr_disjoint(pb, pe, ob, oe))
        return CS_POINTER_ARGUMENT;
    if (!ptr_state_ok(p))
        return CS_POINTER_STATE;
    if (p->down == 1u)
        flags |= (uint32_t)CS_POINTER_RELEASE;
    p->down = 0u;
    p->suppress = 1u;
    out->x = p->x;
    out->y = p->y;
    out->flags = flags;
    out->buttons = 0u;
    return CS_POINTER_CHANGED;
}
