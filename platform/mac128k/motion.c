/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "motion.h"

enum { WR9_RESET = 0xC0, WR9_NV = 0x02, WR9_MIE_NV = 0x0A,
       WR0_RESET_EXT = 0x10, WR0_RESET_IUS = 0x38,
       RR0_DCD = 0x08, RR3_EXT_A = 0x08, RR3_EXT_B = 0x01,
       PB_MASK = 0x38, PB_BUTTON = 0x08, PB_X2 = 0x10, PB_Y2 = 0x20 };

/* Keep compact byte samples, but pass them by address: the native compiler's
 * three-byte by-value stack copy produced an odd word access on MC68000. */
typedef struct { uint8_t pb, a, b; } snap;

#if CLASSICK_MAC_PENDING_DEFER_PROBE
#define DEFER_CLEAR(m) ((m)->defer_pending = (m)->defer_snapshot = 0u)
#else
#define DEFER_CLEAR(m) ((void)0)
#endif

static uint32_t sat(uint32_t v) { return v == UINT32_MAX ? v : v + 1u; }

static void wr(struct cs_mac_motion *m, uint32_t ch, uint32_t reg, int v)
{
    m->io.write(m->io.cookie, ch, reg, (uint8_t)v);
}

/* Hard failure: first stage/cause retained, master interrupt disabled. */
static int fail(struct cs_mac_motion *m, uint32_t stage, int32_t cause)
{
    if (m->cause == CS_MAC_CAUSE_NONE) { m->stage = stage; m->cause = cause; }
    wr(m, CS_MAC_SCC_A, 9u, WR9_NV);
    DEFER_CLEAR(m);
    m->phase = CS_MAC_MOTION_FAILED;
    return CS_MAC_MOTION_FAULT;
}

/* Loss quiesce: master interrupt off once so foreground can recover. */
static void quiesce(struct cs_mac_motion *m)
{
    if (m->quiesced) return;
    wr(m, CS_MAC_SCC_A, 9u, WR9_NV);
    m->quiesced = 1u;
}

static void take(const struct cs_mac_motion *m, snap *s)
{
    s->pb = m->io.pb(m->io.cookie);
    s->a = m->io.read(m->io.cookie, CS_MAC_SCC_A, 0u);
    s->b = m->io.read(m->io.cookie, CS_MAC_SCC_B, 0u);
}

static int same(const snap *x, const snap *y)
{
    return ((x->pb ^ y->pb) & PB_MASK) == 0 && ((x->a ^ y->a) & RR0_DCD) == 0
        && ((x->b ^ y->b) & RR0_DCD) == 0;
}

static uint32_t bit(uint8_t v, int mask) { return (v & mask) ? 1u : 0u; }

static uint32_t pack(const snap *s)
{
    return (uint32_t)s->pb << 16 | (uint32_t)s->a << 8 | s->b;
}

#if CLASSICK_MAC_PENDING_DEFER_PROBE
static uint32_t ep(const snap *s)
{
    return (uint32_t)(s->pb & PB_MASK) << 16
        | (uint32_t)(s->a & RR0_DCD) << 8 | (uint32_t)(s->b & RR0_DCD);
}

/* Expected RR3 bit if the first nonstable pass is exactly one DCD edge on one
 * axis with everything else steady; otherwise 0. No IO. */
static uint32_t defer_bit(const struct cs_mac_motion *m, const snap *s1,
    const snap *s2, const snap *s3)
{
    uint32_t a1 = bit(s1->a, RR0_DCD), b1 = bit(s1->b, RR0_DCD);
    uint32_t ch_a = bit(s3->a, RR0_DCD) != m->last_raw_a;
    uint32_t ch_b = bit(s3->b, RR0_DCD) != m->last_raw_b;
    int other = ch_a ? PB_Y2 : PB_X2, mover = ch_a ? PB_X2 : PB_Y2;
    uint32_t btn = (s1->pb & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;
    if (a1 != m->last_raw_a || b1 != m->last_raw_b || ch_a == ch_b) return 0u;
    if (((s1->pb ^ s2->pb) | (s2->pb ^ s3->pb)) & PB_BUTTON) return 0u;
    if (btn != m->last_buttons) return 0u;
    if (ch_a ? bit(s2->b, RR0_DCD) != b1 : bit(s2->a, RR0_DCD) != a1)
        return 0u;
    if (((s1->pb ^ s2->pb) | (s2->pb ^ s3->pb)) & other) return 0u;
    if (((s1->pb ^ s2->pb) & mover) && ((s2->pb ^ s3->pb) & mover))
        return 0u;
    return ch_a ? RR3_EXT_A : RR3_EXT_B;
}
#endif

#if CLASSICK_MAC_PHASE_IDLE_PROBE
/* Pure: every endpoint DCD and button equals the retained state. PB4/5 free. */
static int idle_only(const struct cs_mac_motion *m, const snap *s1,
    const snap *s2, const snap *s3)
{
    const snap *p[3];
    int i;
    p[0] = s1; p[1] = s2; p[2] = s3;
    for (i = 0; i < 3; i++) {
        uint32_t btn = (p[i]->pb & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;
        if (bit(p[i]->a, RR0_DCD) != m->last_raw_a
            || bit(p[i]->b, RR0_DCD) != m->last_raw_b
            || btn != m->last_buttons)
            return 0;
    }
    return 1;
}
#endif

/* Observation only: no IO. Keeps the first record; reason is written last. */
static void note(struct cs_mac_motion *m, uint32_t reason, uint8_t rr3,
    const snap *s1, const snap *s2, const snap *s3, uint32_t entry)
{
    struct cs_mac_motion_loss *l = &m->first_loss;
    if (l->reason != CS_MAC_LOSS_NONE) return;
    l->rr3 = rr3;
    l->snapshots[0] = pack(s1);
    l->snapshots[1] = pack(s2);
    l->snapshots[2] = pack(s3);
    l->previous = m->have_phase << 24 | m->last_raw_a << 16
        | m->last_raw_b << 8 | m->last_buttons;
    l->inbox_phase = entry;
    l->reason = reason;
}

int cs_mac_motion_prepare(struct cs_mac_motion *m, const cs_mac_motion_io *io,
    cs_window_inbox *inbox, uint32_t invert)
{
    static const struct cs_mac_motion zero;
    if (!m || !io || !io->read || !io->write || !io->pb || !inbox
        || invert > 3u)
        return CS_MAC_MOTION_ARGUMENT;
    if (inbox->valid != CS_INBOX_VALID || inbox->phase != CS_INBOX_WAIT
        || inbox->epoch == 0u)
        return CS_MAC_MOTION_STATE;
    *m = zero;
    m->io = *io;
    m->inbox = inbox;
    m->epoch = inbox->epoch;
    m->invert = invert;
    m->phase = CS_MAC_MOTION_PREPARED;

    m->stage = CS_MAC_STAGE_RESET;
    wr(m, CS_MAC_SCC_A, 9u, WR9_RESET);
    wr(m, CS_MAC_SCC_A, 9u, WR9_NV);

    m->stage = CS_MAC_STAGE_CONFIG;
    wr(m, CS_MAC_SCC_A, 1u, 0x01);
    wr(m, CS_MAC_SCC_A, 15u, 0x08);
    wr(m, CS_MAC_SCC_B, 1u, 0x01);
    wr(m, CS_MAC_SCC_B, 15u, 0x00);
    wr(m, CS_MAC_SCC_A, 2u, 0x00);

    /* Probe distinct controls through the DCD masks this path actually uses.
     * Both writes precede both reads; an alias sees B's final zero in A.
     * MIE stays off. No baud-rate register or unsupported-register fallback. */
    m->stage = CS_MAC_STAGE_READBACK_A;
    if (m->io.read(m->io.cookie, CS_MAC_SCC_A, 15u) != RR0_DCD)
        return fail(m, CS_MAC_STAGE_READBACK_A, CS_MAC_CAUSE_READBACK);
    m->stage = CS_MAC_STAGE_READBACK_B;
    if (m->io.read(m->io.cookie, CS_MAC_SCC_B, 15u) != 0u)
        return fail(m, CS_MAC_STAGE_READBACK_B, CS_MAC_CAUSE_READBACK);
    wr(m, CS_MAC_SCC_B, 15u, RR0_DCD);
    return CS_MAC_MOTION_OK;
}

int cs_mac_motion_arm(struct cs_mac_motion *m)
{
    const cs_window_inbox *in;
    if (!m) return CS_MAC_MOTION_ARGUMENT;
    in = m->inbox;
    if (m->phase != CS_MAC_MOTION_PREPARED || !in
        || in->valid != CS_INBOX_VALID || in->epoch != m->epoch
        || in->phase != CS_INBOX_WAIT)
        return CS_MAC_MOTION_STATE;
    m->phase = CS_MAC_MOTION_ARMED;
    m->stage = CS_MAC_STAGE_ARM;
    wr(m, CS_MAC_SCC_A, 9u, WR9_MIE_NV);
    return CS_MAC_MOTION_OK;
}

static int uncertain(struct cs_mac_motion *m, uint32_t reason, uint8_t rr3,
    const snap *s1, const snap *s2, const snap *s3, uint32_t entry)
{
    int r = cs_inbox_uncertain(m->inbox, m->epoch);
    if (r == CS_INBOX_OK || r == CS_INBOX_LOST)
        note(m, reason, rr3, s1, s2, s3, entry);
    m->have_phase = 0u;
    m->losses = sat(m->losses);
    if (r != CS_INBOX_OK && r != CS_INBOX_LOST)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    quiesce(m);
    return CS_MAC_MOTION_LOST;
}

int cs_mac_motion_service(struct cs_mac_motion *m)
{
    cs_window_inbox *in;
    uint8_t rr3;
    uint32_t pend_a, pend_b, ra, rb, buttons, ch_a, ch_b;
    int32_t dx = 0, dy = 0;
    snap s1, s2, s3;
    int stable, r;
    uint32_t entry;

    if (!m) return CS_MAC_MOTION_ARGUMENT;
    if (m->phase != CS_MAC_MOTION_ARMED) return CS_MAC_MOTION_STATE;
    m->services = sat(m->services);
    in = m->inbox;

    entry = in->phase;
    rr3 = m->io.read(m->io.cookie, CS_MAC_SCC_A, 3u);
    if (rr3 & ~(RR3_EXT_A | RR3_EXT_B) & 0xFF)
        return fail(m, CS_MAC_STAGE_PENDING, CS_MAC_CAUSE_PENDING);
    pend_a = bit(rr3, RR3_EXT_A);
    pend_b = bit(rr3, RR3_EXT_B);
    take(m, &s1);
    take(m, &s2);
    if (pend_a) wr(m, CS_MAC_SCC_A, 0u, WR0_RESET_EXT);
    if (pend_b) wr(m, CS_MAC_SCC_B, 0u, WR0_RESET_EXT);
    wr(m, CS_MAC_SCC_A, 0u, WR0_RESET_IUS); /* whole chip owned */
    take(m, &s3);
    stable = same(&s1, &s2) && same(&s2, &s3);

    if (in->valid != CS_INBOX_VALID || in->epoch != m->epoch)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_STATE);
    ra = bit(s3.a, RR0_DCD);
    rb = bit(s3.b, RR0_DCD);
    buttons = (s3.pb & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;

    switch (in->phase) {
    case CS_INBOX_LOSS:
        DEFER_CLEAR(m);
        m->have_phase = 0u;
        quiesce(m);
        return CS_MAC_MOTION_LOST;
    case CS_INBOX_WAIT:
        DEFER_CLEAR(m);
        if (!stable) { m->have_phase = 0u; return CS_MAC_MOTION_EMPTY; }
        if (cs_inbox_baseline(in, m->epoch, buttons) != CS_INBOX_OK)
            return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
        m->last_raw_a = ra;
        m->last_raw_b = rb;
        m->last_buttons = buttons;
        m->have_phase = 1u;
        m->baselines = sat(m->baselines);
        if (m->quiesced) { /* stable baseline queued: restore master last */
            wr(m, CS_MAC_SCC_A, 9u, WR9_MIE_NV);
            m->quiesced = 0u;
        }
        return CS_MAC_MOTION_BASELINED;
    case CS_INBOX_READY:
    case CS_INBOX_PRIMING:
        break;
    default:
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_STATE);
    }

#if CLASSICK_MAC_PENDING_DEFER_PROBE
    {
        uint32_t dp = m->defer_pending, de = m->defer_snapshot;
        DEFER_CLEAR(m);
        if (dp) {
            if (!m->have_phase || in->phase != CS_INBOX_READY || !stable
                || rr3 != dp || ep(&s3) != de)
                return uncertain(m, stable ? CS_MAC_LOSS_PENDING
                    : CS_MAC_LOSS_UNSTABLE, rr3, &s1, &s2, &s3, entry);
        } else if (in->phase == CS_INBOX_READY && m->have_phase
            && !m->quiesced && !stable && rr3 == 0u) {
            dp = defer_bit(m, &s1, &s2, &s3);
            if (dp) {
                m->defer_pending = dp;
                m->defer_snapshot = ep(&s3);
                return CS_MAC_MOTION_EMPTY;
            }
        }
    }
#endif
#if CLASSICK_MAC_PHASE_IDLE_PROBE
    if (!stable && in->phase == CS_INBOX_READY && m->have_phase
        && !m->quiesced && rr3 == 0u && idle_only(m, &s1, &s2, &s3))
        return CS_MAC_MOTION_EMPTY;
#endif
    if (!m->have_phase || !stable) {
        return uncertain(m, !m->have_phase ? CS_MAC_LOSS_NO_PHASE : CS_MAC_LOSS_UNSTABLE,
            rr3, &s1, &s2, &s3, entry);
    }
    ch_a = ra != m->last_raw_a;
    ch_b = rb != m->last_raw_b;
    if (ch_a != pend_a || ch_b != pend_b) {
        return uncertain(m, CS_MAC_LOSS_PENDING, rr3, &s1, &s2, &s3, entry);
    }
    if (ch_a)
        dx = ((ra ^ (m->invert & 1u)) == bit(s3.pb, PB_X2)) ? 1 : -1;
    if (ch_b)
        dy = ((rb ^ ((m->invert >> 1) & 1u)) != bit(s3.pb, PB_Y2)) ? 1 : -1;
    if (!ch_a && !ch_b && buttons == m->last_buttons)
        return CS_MAC_MOTION_EMPTY;

    r = cs_inbox_push(in, m->epoch, dx, dy, buttons);
    m->samples = sat(m->samples);
    if (r == CS_INBOX_LOST) {
        note(m, CS_MAC_LOSS_OVERFLOW, rr3, &s1, &s2, &s3, entry);
        m->have_phase = 0u;
        quiesce(m);
        m->losses = sat(m->losses);
        return CS_MAC_MOTION_LOST;
    }
    if (r != CS_INBOX_OK)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    m->last_raw_a = ra;
    m->last_raw_b = rb;
    m->last_buttons = buttons;
    if (in->count > m->peak_queued) m->peak_queued = in->count;
    return CS_MAC_MOTION_OK;
}

int cs_mac_motion_stop(struct cs_mac_motion *m)
{
    if (!m) return CS_MAC_MOTION_ARGUMENT;
    if (m->phase == CS_MAC_MOTION_STOPPED) return CS_MAC_MOTION_OK;
    if (m->phase != CS_MAC_MOTION_PREPARED && m->phase != CS_MAC_MOTION_ARMED
        && m->phase != CS_MAC_MOTION_FAILED)
        return CS_MAC_MOTION_STATE;
    wr(m, CS_MAC_SCC_A, 9u, WR9_NV);
    DEFER_CLEAR(m);
    wr(m, CS_MAC_SCC_A, 1u, 0x00);
    wr(m, CS_MAC_SCC_A, 15u, 0x00);
    wr(m, CS_MAC_SCC_B, 1u, 0x00);
    wr(m, CS_MAC_SCC_B, 15u, 0x00);
    m->phase = CS_MAC_MOTION_STOPPED;
    return CS_MAC_MOTION_OK;
}
