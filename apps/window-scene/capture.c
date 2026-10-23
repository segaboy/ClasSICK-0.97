/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "capture.h"

/* SPEC-0063 sequential ordered capture/drain helper. No globals, statics,
 * allocation, callbacks, MMIO or CRT helpers. Caller provides exclusion. */

#define CS_CAP_MASK ((uint32_t)CS_CAPTURE_CAPACITY - 1u)
_Static_assert((CS_CAPTURE_CAPACITY & (CS_CAPTURE_CAPACITY-1))==0,
    "Capture mask requires a power-of-two capacity");

static int cs_cap_owner_ok(int r)
{
    return r == CS_WINDOW_INPUT_OK || r == CS_WINDOW_INPUT_CHANGED;
}

static uint32_t cs_cap_saturate(uint32_t v)
{
    return v == UINT32_MAX ? v : v + 1u;
}

/* Owner ready check; foreground-only callers. */
static int cs_cap_owner_ready(const cs_window_input *owner)
{
    return owner->valid == CS_WINDOW_INPUT_VALID &&
        owner->phase == (uint32_t)CS_WINDOW_INPUT_READY;
}

/* Direct descriptor/owner integer span overflow and overlap check. */
static int cs_cap_disjoint(const cs_window_capture *capture,
    const cs_window_input *owner)
{
    uintptr_t a = (uintptr_t)capture;
    uintptr_t b = (uintptr_t)owner;
    uintptr_t sa = (uintptr_t)sizeof(*capture);
    uintptr_t sb = (uintptr_t)sizeof(*owner);
    if (a > UINTPTR_MAX - sa || b > UINTPTR_MAX - sb)
        return 0;
    return a + sa <= b || b + sb <= a;
}

/* Record the owner call stage and first raw non-success cause. Once a
 * failure is retained, stage/cause stay frozen until explicit restart. */
static int cs_cap_note(cs_window_capture *capture, uint32_t stage, int r)
{
    int ok = cs_cap_owner_ok(r);
    if (capture->cause == 0) {
        capture->stage = stage;
        if (!ok)
            capture->cause = (int32_t)r;
    }
    return ok;
}

/* Scalar metadata validation; never dereferences owner. */
static int cs_cap_check(const cs_window_capture *capture, uint32_t epoch)
{
    if (!capture)
        return CS_CAPTURE_ARGUMENT;
    if (capture->valid != CS_CAPTURE_VALID || capture->epoch == 0u ||
        capture->phase < (uint32_t)CS_CAPTURE_READY ||
        capture->phase > (uint32_t)CS_CAPTURE_ENDED ||
        capture->head >= (uint32_t)CS_CAPTURE_CAPACITY ||
        capture->count > (uint32_t)CS_CAPTURE_CAPACITY ||
        !capture->owner || capture->epoch != epoch)
        return CS_CAPTURE_STATE;
    return CS_CAPTURE_OK;
}

/* Result for producer/drain calls outside READY. */
static int cs_cap_gated(uint32_t phase)
{
    switch (phase) {
    case CS_CAPTURE_LOSS: return CS_CAPTURE_LOST;
    case CS_CAPTURE_WAIT: return CS_CAPTURE_BASELINE;
    case CS_CAPTURE_FAULT: return CS_CAPTURE_OWNER;
    default: return CS_CAPTURE_STOPPED;
    }
}

/* Shared begin behavior; never reads old *capture fields. */
static int cs_cap_start(cs_window_capture *capture, cs_window_input *owner,
    uint32_t epoch)
{
    int r;
    capture->valid = CS_CAPTURE_VALID;
    capture->epoch = epoch;
    capture->phase = (uint32_t)CS_CAPTURE_WAIT;
    capture->head = 0u;
    capture->count = 0u;
    capture->loss_reason = (uint32_t)CS_CAPTURE_STARTUP;
    capture->stage = (uint32_t)CS_CAPTURE_BEGIN_CANCEL;
    capture->cause = 0;
    capture->delivered = 0u;
    capture->losses = 0u;
    capture->owner = owner;
    capture->last.dx = 0;
    capture->last.dy = 0;
    capture->last.buttons = 0u;
    r = cs_window_input_cancel(owner);
    if (!cs_cap_note(capture, (uint32_t)CS_CAPTURE_BEGIN_CANCEL, r)) {
        capture->phase = (uint32_t)CS_CAPTURE_FAULT;
        return CS_CAPTURE_OWNER;
    }
    return CS_CAPTURE_OK;
}

int cs_capture_begin(cs_window_capture *capture, cs_window_input *owner)
{
    if (!capture || !owner || !cs_cap_disjoint(capture, owner))
        return CS_CAPTURE_ARGUMENT;
    if (!cs_cap_owner_ready(owner))
        return CS_CAPTURE_STATE;
    return cs_cap_start(capture, owner, 1u);
}

int cs_capture_restart(cs_window_capture *capture, cs_window_input *owner)
{
    int r;
    uint32_t epoch;
    if (!capture)
        return CS_CAPTURE_ARGUMENT;
    r = cs_cap_check(capture, capture->epoch);
    if (r != CS_CAPTURE_OK)
        return r;
    if (capture->phase != (uint32_t)CS_CAPTURE_ENDED)
        return CS_CAPTURE_STATE;
    epoch = capture->epoch;
    if (epoch == UINT32_MAX)
        return CS_CAPTURE_EXHAUSTED;
    if (!owner || !cs_cap_disjoint(capture, owner))
        return CS_CAPTURE_ARGUMENT;
    if (!cs_cap_owner_ready(owner))
        return CS_CAPTURE_STATE;
    return cs_cap_start(capture, owner, epoch + 1u);
}

/* READY-only loss entry; retains unread records, no owner access. */
static int cs_cap_lose(cs_window_capture *capture, uint32_t reason)
{
    capture->phase = (uint32_t)CS_CAPTURE_LOSS;
    capture->loss_reason = reason;
    capture->losses = cs_cap_saturate(capture->losses);
    return CS_CAPTURE_LOST;
}

int cs_capture_push(cs_window_capture *capture, uint32_t epoch,
    int32_t dx, int32_t dy, uint32_t buttons)
{
    int r;
    cs_pointer_sample *slot;
    if ((buttons & ~(uint32_t)CS_POINTER_BUTTON_MASK) != 0u)
        return CS_CAPTURE_ARGUMENT;
    r = cs_cap_check(capture, epoch);
    if (r != CS_CAPTURE_OK)
        return r;
    if (capture->phase != (uint32_t)CS_CAPTURE_READY)
        return cs_cap_gated(capture->phase);
    if (capture->count == (uint32_t)CS_CAPTURE_CAPACITY)
        return cs_cap_lose(capture, (uint32_t)CS_CAPTURE_OVERFLOW);
    slot = &capture->records[(capture->head + capture->count) & CS_CAP_MASK];
    slot->dx = dx;
    slot->dy = dy;
    slot->buttons = buttons;
    capture->count++;
    return CS_CAPTURE_OK;
}

int cs_capture_uncertain(cs_window_capture *capture, uint32_t epoch)
{
    int r = cs_cap_check(capture, epoch);
    if (r != CS_CAPTURE_OK)
        return r;
    if (capture->phase != (uint32_t)CS_CAPTURE_READY)
        return cs_cap_gated(capture->phase);
    return cs_cap_lose(capture, (uint32_t)CS_CAPTURE_UNCERTAIN);
}

int cs_capture_drain(cs_window_capture *capture, uint32_t epoch)
{
    int r = cs_cap_check(capture, epoch);
    const cs_pointer_sample *next;
    if (r != CS_CAPTURE_OK)
        return r;
    if (capture->phase == (uint32_t)CS_CAPTURE_LOSS) {
        capture->head = 0u;
        capture->count = 0u;
        r = cs_window_input_cancel(capture->owner);
        if (!cs_cap_note(capture, (uint32_t)CS_CAPTURE_LOSS_CANCEL, r)) {
            capture->phase = (uint32_t)CS_CAPTURE_FAULT;
            return CS_CAPTURE_OWNER;
        }
        capture->phase = (uint32_t)CS_CAPTURE_WAIT;
        return CS_CAPTURE_RESET;
    }
    if (capture->phase != (uint32_t)CS_CAPTURE_READY)
        return cs_cap_gated(capture->phase);
    if (capture->count == 0u)
        return CS_CAPTURE_EMPTY;
    next = &capture->records[capture->head];
    capture->last.dx = next->dx;
    capture->last.dy = next->dy;
    capture->last.buttons = next->buttons;
    capture->head = (capture->head + 1u) & CS_CAP_MASK;
    capture->count--;
    r = cs_window_input_sample(capture->owner, &capture->last);
    if (!cs_cap_note(capture, (uint32_t)CS_CAPTURE_SAMPLE, r)) {
        capture->phase = (uint32_t)CS_CAPTURE_FAULT;
        return CS_CAPTURE_OWNER;
    }
    capture->delivered = cs_cap_saturate(capture->delivered);
    return CS_CAPTURE_DELIVERED;
}

int cs_capture_rebaseline(cs_window_capture *capture, uint32_t epoch,
    uint32_t buttons)
{
    int r;
    if ((buttons & ~(uint32_t)CS_POINTER_BUTTON_MASK) != 0u)
        return CS_CAPTURE_ARGUMENT;
    r = cs_cap_check(capture, epoch);
    if (r != CS_CAPTURE_OK)
        return r;
    if (capture->phase == (uint32_t)CS_CAPTURE_READY)
        return CS_CAPTURE_STATE;
    if (capture->phase != (uint32_t)CS_CAPTURE_WAIT)
        return cs_cap_gated(capture->phase);
    capture->head = 0u;
    capture->count = 0u;
    capture->last.dx = 0;
    capture->last.dy = 0;
    capture->last.buttons = buttons;
    r = cs_window_input_sample(capture->owner, &capture->last);
    if (!cs_cap_note(capture, (uint32_t)CS_CAPTURE_REBASELINE, r)) {
        capture->phase = (uint32_t)CS_CAPTURE_FAULT;
        return CS_CAPTURE_OWNER;
    }
    capture->phase = (uint32_t)CS_CAPTURE_READY;
    return CS_CAPTURE_OK;
}

int cs_capture_stop(cs_window_capture *capture, uint32_t epoch)
{
    int r = cs_cap_check(capture, epoch);
    uint32_t was;
    if (r != CS_CAPTURE_OK)
        return r;
    was = capture->phase;
    if (was == (uint32_t)CS_CAPTURE_ENDED)
        return CS_CAPTURE_STOPPED;
    capture->phase = (uint32_t)CS_CAPTURE_ENDED;
    capture->head = 0u;
    capture->count = 0u;
    if (!cs_cap_owner_ready(capture->owner))
        return CS_CAPTURE_STOPPED;
    r = cs_window_input_cancel(capture->owner);
    if (!cs_cap_note(capture, (uint32_t)CS_CAPTURE_STOP_CANCEL, r))
        return CS_CAPTURE_OWNER;
    return CS_CAPTURE_STOPPED;
}
