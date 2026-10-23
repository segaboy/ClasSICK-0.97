/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "foreground.h"

/* Original serialized foreground coordinator. No globals, statics data,
 * allocation, loops, CRT helpers, MMIO or atomics. Hooks bracket every
 * inbox access; every helper/owner call happens after leave. */

static uint32_t cs_fg_enter(const cs_window_foreground *fg)
{
    return fg->hooks.enter(fg->hooks.cookie);
}

static void cs_fg_leave(const cs_window_foreground *fg, uint32_t token)
{
    fg->hooks.leave(fg->hooks.cookie, token);
}

static int cs_fg_owner_ready(const cs_window_input *owner)
{
    return owner->valid == CS_WINDOW_INPUT_VALID &&
        owner->phase == (uint32_t)CS_WINDOW_INPUT_READY;
}

/* Normal stages advance only while no failure is retained. */
static void cs_fg_stage(cs_window_foreground *fg, uint32_t stage)
{
    if (fg->cause == 0)
        fg->stage = stage;
}

/* First nonzero cause and its stage freeze until a new restart attempt. */
static void cs_fg_note(cs_window_foreground *fg, uint32_t stage, int32_t cause)
{
    if (fg->cause == 0) {
        fg->stage = stage;
        fg->cause = cause != 0 ? cause : (int32_t)CS_FG_STATE;
    }
}

/* Raw owner cause for CS_CAPTURE_OWNER, otherwise the raw helper result. */
static int32_t cs_fg_raw(const cs_window_capture *capture, int r)
{
    if (r == CS_CAPTURE_OWNER && capture->cause != 0)
        return capture->cause;
    return (int32_t)r;
}

/* Freeze failure, then close admission under exclusion. A still-claimed
 * record is completed with the failure cause first; fault keeps prefix. */
static int cs_fg_fail(cs_window_foreground *fg, uint32_t stage,
    int32_t cause, int claimed)
{
    uint32_t t;
    cs_fg_note(fg, stage, cause);
    t = cs_fg_enter(fg);
    if (claimed)
        (void)cs_inbox_complete(fg->inbox, fg->epoch, fg->cause);
    (void)cs_inbox_fault(fg->inbox, fg->epoch, fg->cause);
    cs_fg_leave(fg, t);
    fg->phase = (uint32_t)CS_FG_FAILED;
    return CS_FG_FAULT;
}

static int cs_fg_check(const cs_window_foreground *fg)
{
    if (!fg)
        return CS_FG_ARGUMENT;
    if (fg->valid != CS_FG_VALID || fg->epoch == 0u ||
        fg->phase < (uint32_t)CS_FG_ACTIVE ||
        fg->phase > (uint32_t)CS_FG_ENDED || !fg->inbox || !fg->capture ||
        !fg->hooks.enter || !fg->hooks.leave)
        return CS_FG_STATE;
    return CS_FG_OK;
}

int cs_foreground_begin(cs_window_foreground *fg, cs_window_inbox *inbox,
    cs_window_capture *capture, cs_window_input *owner,
    const cs_foreground_hooks *hooks)
{
    uint32_t t;
    int r, ri;
    if (!fg || !inbox || !capture || !owner || !hooks || !hooks->enter ||
        !hooks->leave)
        return CS_FG_ARGUMENT;
    if (!cs_fg_owner_ready(owner))
        return CS_FG_STATE;
    fg->valid = CS_FG_VALID;
    fg->epoch = 1u;
    fg->phase = (uint32_t)CS_FG_ACTIVE;
    fg->stage = (uint32_t)CS_FG_BEGIN;
    fg->cause = 0;
    fg->inbox = inbox;
    fg->capture = capture;
    fg->hooks.enter = hooks->enter;
    fg->hooks.leave = hooks->leave;
    fg->hooks.cookie = hooks->cookie;
    r = cs_capture_begin(capture, owner);
    if (r != CS_CAPTURE_OK)
        cs_fg_note(fg, (uint32_t)CS_FG_BEGIN, cs_fg_raw(capture, r));
    t = cs_fg_enter(fg);
    ri = cs_inbox_begin(inbox);
    if (ri != CS_INBOX_OK)
        cs_fg_note(fg, (uint32_t)CS_FG_BEGIN, (int32_t)ri);
    else if (fg->cause != 0)
        (void)cs_inbox_fault(inbox, 1u, fg->cause);
    cs_fg_leave(fg, t);
    if (fg->cause != 0) {
        fg->phase = (uint32_t)CS_FG_FAILED;
        return CS_FG_FAULT;
    }
    return CS_FG_OK;
}

/* One loss episode: prefix already discarded under exclusion. */
static int cs_fg_loss(cs_window_foreground *fg, int rd)
{
    cs_window_capture *c = fg->capture;
    uint32_t t;
    int r;
    cs_fg_stage(fg, (uint32_t)CS_FG_LOSS);
    if (rd != CS_INBOX_OK)
        return cs_fg_fail(fg, (uint32_t)CS_FG_LOSS, (int32_t)rd, 0);
    if (c->phase == (uint32_t)CS_CAPTURE_READY) {
        r = cs_capture_uncertain(c, fg->epoch);
        if (r != CS_CAPTURE_LOST)
            return cs_fg_fail(fg, (uint32_t)CS_FG_LOSS, cs_fg_raw(c, r), 0);
        r = cs_capture_drain(c, fg->epoch);
        if (r != CS_CAPTURE_RESET)
            return cs_fg_fail(fg, (uint32_t)CS_FG_LOSS, cs_fg_raw(c, r), 0);
    }
    t = cs_fg_enter(fg);
    r = cs_inbox_resume(fg->inbox, fg->epoch);
    cs_fg_leave(fg, t);
    if (r != CS_INBOX_BASELINE)
        return cs_fg_fail(fg, (uint32_t)CS_FG_LOSS, (int32_t)r, 0);
    return CS_FG_RESET;
}

int cs_foreground_step(cs_window_foreground *fg)
{
    cs_window_capture *c;
    cs_window_inbox *ib;
    cs_pointer_sample s;
    uint32_t t, cp;
    int r, kind;
    r = cs_fg_check(fg);
    if (r != CS_FG_OK)
        return r;
    if (fg->phase == (uint32_t)CS_FG_ENDED)
        return CS_FG_STOPPED;
    if (fg->phase == (uint32_t)CS_FG_FAILED)
        return CS_FG_FAULT;
    c = fg->capture;
    ib = fg->inbox;
    cp = c->phase;
    if (c->valid != CS_CAPTURE_VALID || c->epoch != fg->epoch ||
        c->count != 0u || c->head >= (uint32_t)CS_CAPTURE_CAPACITY || !c->owner ||
        (cp != (uint32_t)CS_CAPTURE_READY && cp != (uint32_t)CS_CAPTURE_WAIT))
        return cs_fg_fail(fg, (uint32_t)CS_FG_INTEGRATION,
            (int32_t)CS_FG_STATE, 0);
    s.dx = 0;
    s.dy = 0;
    s.buttons = 0u;
    t = cs_fg_enter(fg);
    if (ib->phase == (uint32_t)CS_INBOX_LOSS) {
        r = cs_inbox_discard(ib, fg->epoch);
        cs_fg_leave(fg, t);
        return cs_fg_loss(fg, r);
    }
    if (((ib->phase == (uint32_t)CS_INBOX_PRIMING ||
          ib->phase == (uint32_t)CS_INBOX_WAIT) && cp != (uint32_t)CS_CAPTURE_WAIT) ||
        (ib->phase == (uint32_t)CS_INBOX_READY && cp != (uint32_t)CS_CAPTURE_READY)) {
        cs_fg_leave(fg, t);
        return cs_fg_fail(fg, (uint32_t)CS_FG_INTEGRATION,
            (int32_t)CS_FG_STATE, 0);
    }
    kind = cs_inbox_claim(ib, fg->epoch);
    if (kind == CS_INBOX_BASELINE_ITEM || kind == CS_INBOX_SAMPLE_ITEM) {
        s.dx = ib->last.dx;
        s.dy = ib->last.dy;
        s.buttons = ib->last.buttons;
    }
    cs_fg_leave(fg, t);
    if (kind == CS_INBOX_EMPTY || kind == CS_INBOX_BASELINE)
        return CS_FG_EMPTY;
    if (kind != CS_INBOX_BASELINE_ITEM && kind != CS_INBOX_SAMPLE_ITEM)
        return cs_fg_fail(fg, (uint32_t)CS_FG_CLAIM, (int32_t)kind, 0);
    if (kind == CS_INBOX_BASELINE_ITEM) {
        cs_fg_stage(fg, (uint32_t)CS_FG_BASELINE);
        if (cp != (uint32_t)CS_CAPTURE_WAIT)
            return cs_fg_fail(fg, (uint32_t)CS_FG_INTEGRATION,
                (int32_t)CS_FG_STATE, 1);
        r = cs_capture_rebaseline(c, fg->epoch, s.buttons);
        if (r != CS_CAPTURE_OK)
            return cs_fg_fail(fg, (uint32_t)CS_FG_BASELINE,
                cs_fg_raw(c, r), 1);
        kind = CS_FG_BASELINED;
    } else {
        if (cp != (uint32_t)CS_CAPTURE_READY)
            return cs_fg_fail(fg, (uint32_t)CS_FG_INTEGRATION,
                (int32_t)CS_FG_STATE, 1);
        cs_fg_stage(fg, (uint32_t)CS_FG_PUSH);
        r = cs_capture_push(c, fg->epoch, s.dx, s.dy, s.buttons);
        if (r != CS_CAPTURE_OK)
            return cs_fg_fail(fg, (uint32_t)CS_FG_PUSH, cs_fg_raw(c, r), 1);
        cs_fg_stage(fg, (uint32_t)CS_FG_DRAIN);
        r = cs_capture_drain(c, fg->epoch);
        if (r != CS_CAPTURE_DELIVERED)
            return cs_fg_fail(fg, (uint32_t)CS_FG_DRAIN, cs_fg_raw(c, r), 1);
        kind = CS_FG_DELIVERED;
    }
    cs_fg_stage(fg, (uint32_t)CS_FG_COMPLETE);
    t = cs_fg_enter(fg);
    r = cs_inbox_complete(ib, fg->epoch, 0);
    cs_fg_leave(fg, t);
    if (r != CS_INBOX_OK && r != CS_INBOX_LOST)
        return cs_fg_fail(fg, (uint32_t)CS_FG_COMPLETE, (int32_t)r, 0);
    return kind;
}

int cs_foreground_stop(cs_window_foreground *fg)
{
    uint32_t t;
    int r, ri, rs, failed = 0;
    r = cs_fg_check(fg);
    if (r != CS_FG_OK)
        return r;
    if (fg->phase == (uint32_t)CS_FG_ENDED)
        return CS_FG_STOPPED;
    cs_fg_stage(fg, (uint32_t)CS_FG_STOP);
    t = cs_fg_enter(fg);
    ri = cs_inbox_stop(fg->inbox, fg->epoch);
    cs_fg_leave(fg, t);
    fg->phase = (uint32_t)CS_FG_ENDED;
    if (ri != CS_INBOX_OK && ri != CS_INBOX_STOPPED) {
        cs_fg_note(fg, (uint32_t)CS_FG_STOP, (int32_t)ri);
        failed = 1;
    }
    rs = cs_capture_stop(fg->capture, fg->epoch);
    if (rs != CS_CAPTURE_STOPPED) {
        cs_fg_note(fg, (uint32_t)CS_FG_STOP, cs_fg_raw(fg->capture, rs));
        t = cs_fg_enter(fg);
        (void)cs_inbox_fault(fg->inbox, fg->epoch, fg->cause);
        cs_fg_leave(fg, t);
        failed = 1;
    }
    return failed ? CS_FG_FAULT : CS_FG_STOPPED;
}

int cs_foreground_restart(cs_window_foreground *fg, cs_window_input *owner)
{
    cs_window_capture *c;
    cs_window_inbox *ib;
    uint32_t t;
    int r;
    r = cs_fg_check(fg);
    if (r != CS_FG_OK)
        return r;
    if (!owner)
        return CS_FG_ARGUMENT;
    if (fg->phase != (uint32_t)CS_FG_ENDED || !cs_fg_owner_ready(owner))
        return CS_FG_STATE;
    c = fg->capture;
    ib = fg->inbox;
    t = cs_fg_enter(fg);
    if (ib->valid != CS_INBOX_VALID ||
        ib->phase != (uint32_t)CS_INBOX_ENDED || ib->epoch != fg->epoch ||
        c->valid != CS_CAPTURE_VALID ||
        c->phase != (uint32_t)CS_CAPTURE_ENDED || c->epoch != fg->epoch ||
        ib->head != 0u || ib->count != 0u || ib->claimed != (uint32_t)CS_INBOX_NONE ||
        c->head != 0u || c->count != 0u || !c->owner)
        r = CS_FG_STATE;
    else if (fg->epoch == UINT32_MAX)
        r = CS_FG_EXHAUSTED;
    cs_fg_leave(fg, t);
    if (r != CS_FG_OK)
        return r;
    fg->cause = 0;
    fg->stage = (uint32_t)CS_FG_RESTART;
    r = cs_capture_restart(c, owner);
    if (r != CS_CAPTURE_OK) {
        cs_fg_note(fg, (uint32_t)CS_FG_RESTART, cs_fg_raw(c, r));
        return CS_FG_FAULT;
    }
    t = cs_fg_enter(fg);
    r = cs_inbox_restart(ib);
    if (r == CS_INBOX_OK && ib->epoch != c->epoch) {
        (void)cs_inbox_stop(ib, ib->epoch);
        r = CS_INBOX_STATE;
    }
    cs_fg_leave(fg, t);
    if (r != CS_INBOX_OK) {
        cs_fg_note(fg, (uint32_t)CS_FG_RESTART, (int32_t)r);
        return CS_FG_FAULT;
    }
    fg->epoch = c->epoch;
    fg->phase = (uint32_t)CS_FG_ACTIVE;
    return CS_FG_OK;
}
