/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0065 pure normalized window inbox. Isolated operation-level state
 * machine only: no masking, atomics, publication or ISR-safety claim. Caller
 * provides exclusion/ordering. No allocation, globals, callbacks or CRT. */
#include "inbox.h"

/* Canonical metadata check; returns 0 or CS_INBOX_STATE. Never writes. */
static int inbox_valid(const cs_window_inbox *b, uint32_t epoch, int check)
{
    uint32_t phase, count, claimed;
    if (b->valid != CS_INBOX_VALID || b->epoch == 0u) return CS_INBOX_STATE;
    if (check && b->epoch != epoch) return CS_INBOX_STATE;
    phase = b->phase; count = b->count; claimed = b->claimed;
    if (phase < (uint32_t)CS_INBOX_WAIT || phase > (uint32_t)CS_INBOX_ENDED)
        return CS_INBOX_STATE;
    if (b->head >= (uint32_t)CS_INBOX_CAPACITY) return CS_INBOX_STATE;
    if (count > (uint32_t)CS_INBOX_CAPACITY) return CS_INBOX_STATE;
    if (claimed > (uint32_t)CS_INBOX_CLAIM_SAMPLE) return CS_INBOX_STATE;
    switch (phase) {
    case CS_INBOX_WAIT:
    case CS_INBOX_ENDED:
        if (b->head != 0u || count != 0u || claimed != 0u)
            return CS_INBOX_STATE;
        break;
    case CS_INBOX_FAULT:
        if (claimed != 0u) return CS_INBOX_STATE;
        break;
    case CS_INBOX_PRIMING:
        if (claimed == (uint32_t)CS_INBOX_CLAIM_SAMPLE) return CS_INBOX_STATE;
        if (claimed == (uint32_t)CS_INBOX_NONE && count == 0u)
            return CS_INBOX_STATE;
        break;
    case CS_INBOX_READY:
        if (claimed == (uint32_t)CS_INBOX_CLAIM_BASELINE)
            return CS_INBOX_STATE;
        break;
    default:
        break;
    }
    return 0;
}

/* Ordinary unavailable-operation result for LOSS/WAIT/FAULT/ENDED. */
static int inbox_unavailable(uint32_t phase)
{
    switch (phase) {
    case CS_INBOX_LOSS: return CS_INBOX_LOST;
    case CS_INBOX_WAIT: return CS_INBOX_BASELINE;
    case CS_INBOX_FAULT: return CS_INBOX_FAULTED;
    case CS_INBOX_ENDED: return CS_INBOX_STOPPED;
    default: return CS_INBOX_STATE;
    }
}

static uint32_t inbox_sat(uint32_t v)
{
    return v != UINT32_MAX ? v + 1u : v;
}

/* Metadata and last only; unused records/padding are left unspecified. */
static void inbox_reset(cs_window_inbox *b, uint32_t epoch)
{
    b->valid = CS_INBOX_VALID;
    b->epoch = epoch;
    b->phase = (uint32_t)CS_INBOX_WAIT;
    b->head = 0u;
    b->count = 0u;
    b->claimed = (uint32_t)CS_INBOX_NONE;
    b->loss_reason = (uint32_t)CS_INBOX_STARTUP;
    b->losses = 0u;
    b->delivered = 0u;
    b->cause = 0;
    b->last.dx = 0;
    b->last.dy = 0;
    b->last.buttons = 0u;
}

/* Producer-side loss entry; never touches last or the unread prefix. */
static int inbox_enter_loss(cs_window_inbox *b, uint32_t reason)
{
    b->phase = (uint32_t)CS_INBOX_LOSS;
    b->loss_reason = reason;
    b->losses = inbox_sat(b->losses);
    return CS_INBOX_LOST;
}

int cs_inbox_begin(cs_window_inbox *b)
{
    if (!b) return CS_INBOX_ARGUMENT;
    inbox_reset(b, 1u);
    return CS_INBOX_OK;
}

int cs_inbox_restart(cs_window_inbox *b)
{
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, 0u, 0)) return CS_INBOX_STATE;
    if (b->phase != (uint32_t)CS_INBOX_ENDED) return CS_INBOX_STATE;
    if (b->epoch == UINT32_MAX) return CS_INBOX_EXHAUSTED;
    inbox_reset(b, b->epoch + 1u);
    return CS_INBOX_OK;
}

int cs_inbox_baseline(cs_window_inbox *b, uint32_t epoch, uint32_t buttons)
{
    if (!b || buttons > (uint32_t)CS_POINTER_BUTTON_MASK)
        return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->phase != (uint32_t)CS_INBOX_WAIT) return inbox_unavailable(b->phase);
    b->records[0].dx = 0;
    b->records[0].dy = 0;
    b->records[0].buttons = buttons;
    b->count = 1u;
    b->loss_reason = 0u;
    b->phase = (uint32_t)CS_INBOX_PRIMING;
    return CS_INBOX_OK;
}

int cs_inbox_push(cs_window_inbox *b, uint32_t epoch, int32_t dx, int32_t dy,
    uint32_t buttons)
{
    uint32_t slot;
    if (!b || buttons > (uint32_t)CS_POINTER_BUTTON_MASK)
        return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->phase != (uint32_t)CS_INBOX_PRIMING &&
        b->phase != (uint32_t)CS_INBOX_READY)
        return inbox_unavailable(b->phase);
    if (b->count == (uint32_t)CS_INBOX_CAPACITY)
        return inbox_enter_loss(b, (uint32_t)CS_INBOX_OVERFLOW);
    slot = (b->head + b->count) % (uint32_t)CS_INBOX_CAPACITY;
    b->records[slot].dx = dx;
    b->records[slot].dy = dy;
    b->records[slot].buttons = buttons;
    b->count = b->count + 1u;
    return CS_INBOX_OK;
}

int cs_inbox_uncertain(cs_window_inbox *b, uint32_t epoch)
{
    uint32_t phase;
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    phase = b->phase;
    if (phase != (uint32_t)CS_INBOX_WAIT &&
        phase != (uint32_t)CS_INBOX_PRIMING &&
        phase != (uint32_t)CS_INBOX_READY)
        return inbox_unavailable(phase);
    return inbox_enter_loss(b, (uint32_t)CS_INBOX_UNCERTAIN);
}

int cs_inbox_claim(cs_window_inbox *b, uint32_t epoch)
{
    uint32_t head, phase;
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    phase = b->phase;
    if (phase != (uint32_t)CS_INBOX_PRIMING &&
        phase != (uint32_t)CS_INBOX_READY)
        return inbox_unavailable(phase);
    if (b->claimed != (uint32_t)CS_INBOX_NONE) return CS_INBOX_STATE;
    if (b->count == 0u) return CS_INBOX_EMPTY;
    head = b->head;
    b->last.dx = b->records[head].dx;
    b->last.dy = b->records[head].dy;
    b->last.buttons = b->records[head].buttons;
    b->head = (head + 1u) % (uint32_t)CS_INBOX_CAPACITY;
    b->count = b->count - 1u;
    if (phase == (uint32_t)CS_INBOX_PRIMING) {
        b->claimed = (uint32_t)CS_INBOX_CLAIM_BASELINE;
        return CS_INBOX_BASELINE_ITEM;
    }
    b->claimed = (uint32_t)CS_INBOX_CLAIM_SAMPLE;
    return CS_INBOX_SAMPLE_ITEM;
}

int cs_inbox_complete(cs_window_inbox *b, uint32_t epoch, int32_t cause)
{
    uint32_t phase, claimed;
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    phase = b->phase;
    claimed = b->claimed;
    if (phase != (uint32_t)CS_INBOX_PRIMING &&
        phase != (uint32_t)CS_INBOX_READY && phase != (uint32_t)CS_INBOX_LOSS)
        return inbox_unavailable(phase);
    if (claimed == (uint32_t)CS_INBOX_NONE)
        return CS_INBOX_STATE;
    b->claimed = (uint32_t)CS_INBOX_NONE;
    if (cause != 0) {
        if (b->cause == 0) b->cause = cause;
        b->phase = (uint32_t)CS_INBOX_FAULT;
        return CS_INBOX_FAULTED;
    }
    if (claimed == (uint32_t)CS_INBOX_CLAIM_SAMPLE)
        b->delivered = inbox_sat(b->delivered);
    else if (phase == (uint32_t)CS_INBOX_PRIMING)
        b->phase = (uint32_t)CS_INBOX_READY;
    return phase == (uint32_t)CS_INBOX_LOSS ? CS_INBOX_LOST : CS_INBOX_OK;
}

int cs_inbox_discard(cs_window_inbox *b, uint32_t epoch)
{
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->phase == (uint32_t)CS_INBOX_PRIMING ||
        b->phase == (uint32_t)CS_INBOX_READY)
        return CS_INBOX_STATE;
    if (b->phase != (uint32_t)CS_INBOX_LOSS) return inbox_unavailable(b->phase);
    if (b->claimed != (uint32_t)CS_INBOX_NONE) return CS_INBOX_STATE;
    b->head = 0u;
    b->count = 0u;
    return CS_INBOX_OK;
}

int cs_inbox_resume(cs_window_inbox *b, uint32_t epoch)
{
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->phase == (uint32_t)CS_INBOX_PRIMING ||
        b->phase == (uint32_t)CS_INBOX_READY)
        return CS_INBOX_STATE;
    if (b->phase != (uint32_t)CS_INBOX_LOSS) return inbox_unavailable(b->phase);
    if (b->claimed != (uint32_t)CS_INBOX_NONE || b->head != 0u ||
        b->count != 0u)
        return CS_INBOX_STATE;
    b->phase = (uint32_t)CS_INBOX_WAIT;
    return CS_INBOX_BASELINE;
}

int cs_inbox_fault(cs_window_inbox *b, uint32_t epoch, int32_t cause)
{
    if (!b || cause == 0) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->cause == 0) b->cause = cause;
    b->claimed = (uint32_t)CS_INBOX_NONE;
    if (b->phase != (uint32_t)CS_INBOX_ENDED)
        b->phase = (uint32_t)CS_INBOX_FAULT;
    return CS_INBOX_FAULTED;
}

int cs_inbox_stop(cs_window_inbox *b, uint32_t epoch)
{
    if (!b) return CS_INBOX_ARGUMENT;
    if (inbox_valid(b, epoch, 1)) return CS_INBOX_STATE;
    if (b->phase == (uint32_t)CS_INBOX_ENDED) return CS_INBOX_STOPPED;
    b->phase = (uint32_t)CS_INBOX_ENDED;
    b->head = 0u;
    b->count = 0u;
    b->claimed = (uint32_t)CS_INBOX_NONE;
    return CS_INBOX_STOPPED;
}
