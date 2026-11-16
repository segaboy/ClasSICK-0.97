/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "motion.h"

enum { WR9_RESET = 0xC0, WR9_NV = 0x02, WR9_MIE_NV = 0x0A,
       WR0_RESET_EXT = CS_MAC_SCC_RESET_EXT, WR0_RESET_IUS = CS_MAC_SCC_RESET_IUS,
       RR0_DCD = 0x08, RR3_EXT_A = CS_MAC_SCC_EXT_A, RR3_EXT_B = CS_MAC_SCC_EXT_B,
       PB_MASK = 0x38, PB_BUTTON = 0x08, PB_X2 = 0x10, PB_Y2 = 0x20 };

/* Aligned packed observations: PB in bits 16..23, A in 8..15, B in 0..7.
 * No endian-dependent byte alias or odd word access is required. */
typedef struct { uint32_t value; } snap;
#define SNAP_PB(s) ((uint8_t)((s)->value >> 16))
#define SNAP_A(s) ((uint8_t)((s)->value >> 8))
#define SNAP_B(s) ((uint8_t)(s)->value)

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

#if CLASSICK_MAC_REARM_READY_PROBE
static void rearm_ready(struct cs_mac_motion *m)
{
    if (!m->quiesced) return;
    wr(m, CS_MAC_SCC_A, 9u, WR9_MIE_NV);
    m->quiesced = 0u;
}
#endif

#if !CLASSICK_MAC_FAST_COLLECT
static void take(const struct cs_mac_motion *m, snap *s)
{
    uint32_t p = m->io.pb(m->io.cookie);
    uint32_t a = m->io.read(m->io.cookie, CS_MAC_SCC_A, 0u);
    uint32_t b = m->io.read(m->io.cookie, CS_MAC_SCC_B, 0u);
    s->value = p << 16 | a << 8 | b;
}
#endif

static int same(const snap *x, const snap *y)
{
    return ((x->value ^ y->value) & UINT32_C(0x00380808)) == 0u;
}

static uint32_t bit(uint8_t v, int mask) { return (v & mask) ? 1u : 0u; }

static uint32_t pack(const snap *s)
{
    return s->value;
}

#if CLASSICK_MAC_MOTION_TIMELINE
/* Record the actual sampled triples, not inferred transitions. The record is
 * committed by the serialized native caller only after service returns. */
static void timeline_record(struct cs_mac_motion *m, uint8_t rr3,
    const snap *s1, const snap *s2, const snap *s3, uint32_t entry,
    int stable, uint32_t ra, uint32_t rb, uint32_t buttons)
{
    struct cs_mac_timeline *t = &m->timeline;
    struct cs_mac_timeline_entry *e;
    if (!rr3 && stable && (!m->have_phase ||
        (ra == m->last_raw_a && rb == m->last_raw_b && buttons == m->last_buttons)))
        return;
    if (t->count >= CS_MAC_TIMELINE_CAPACITY) {
        t->overflow = sat(t->overflow);
        return;
    }
    e = &t->entries[t->count];
    e->service = m->services;
    e->rr3 = rr3;
    e->snapshots[0] = pack(s1); e->snapshots[1] = pack(s2); e->snapshots[2] = pack(s3);
    e->previous = m->have_phase << 24 | m->last_raw_a << 16
        | m->last_raw_b << 8 | m->last_buttons;
    e->inbox_phase = entry;
    e->result = UINT32_MAX;
    ++t->count;
    t->pending = t->count;
}

void cs_mac_motion_timeline_commit(struct cs_mac_motion *m, int result)
{
    struct cs_mac_timeline *t;
    if (!m) return;
    t = &m->timeline;
    if (t->magic != CS_MAC_TIMELINE_MAGIC || t->pending == 0u ||
        t->pending > t->count || t->pending > CS_MAC_TIMELINE_CAPACITY) return;
    t->entries[t->pending - 1u].result = (uint32_t)result;
    t->pending = 0u;
}
#endif

#if CLASSICK_MAC_PENDING_DEFER_PROBE
static uint32_t ep(const snap *s)
{
    return (uint32_t)(SNAP_PB(s) & PB_MASK) << 16
        | (uint32_t)(SNAP_A(s) & RR0_DCD) << 8 | (uint32_t)(SNAP_B(s) & RR0_DCD);
}

/* Expected RR3 bit if the first nonstable pass is exactly one DCD edge on one
 * axis with everything else steady; otherwise 0. No IO. */
static uint32_t defer_bit(const struct cs_mac_motion *m, const snap *s1,
    const snap *s2, const snap *s3)
{
    uint32_t a1 = bit(SNAP_A(s1), RR0_DCD), b1 = bit(SNAP_B(s1), RR0_DCD);
    uint32_t ch_a = bit(SNAP_A(s3), RR0_DCD) != m->last_raw_a;
    uint32_t ch_b = bit(SNAP_B(s3), RR0_DCD) != m->last_raw_b;
    int other = ch_a ? PB_Y2 : PB_X2, mover = ch_a ? PB_X2 : PB_Y2;
    uint32_t btn = (SNAP_PB(s1) & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;
    if (a1 != m->last_raw_a || b1 != m->last_raw_b || ch_a == ch_b) return 0u;
    if (((SNAP_PB(s1) ^ SNAP_PB(s2)) | (SNAP_PB(s2) ^ SNAP_PB(s3))) & PB_BUTTON) return 0u;
    if (btn != m->last_buttons) return 0u;
    if (ch_a ? bit(SNAP_B(s2), RR0_DCD) != b1 : bit(SNAP_A(s2), RR0_DCD) != a1)
        return 0u;
    if (((SNAP_PB(s1) ^ SNAP_PB(s2)) | (SNAP_PB(s2) ^ SNAP_PB(s3))) & other) return 0u;
    if (((SNAP_PB(s1) ^ SNAP_PB(s2)) & mover) && ((SNAP_PB(s2) ^ SNAP_PB(s3)) & mover))
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
        uint32_t btn = (SNAP_PB(p[i]) & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;
        if (bit(SNAP_A(p[i]), RR0_DCD) != m->last_raw_a
            || bit(SNAP_B(p[i]), RR0_DCD) != m->last_raw_b
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
#if CLASSICK_MAC_LOSS_COUNTS
    if (reason >= CS_MAC_LOSS_NO_PHASE && reason <= CS_MAC_LOSS_OVERFLOW)
        m->loss_counts[reason - 1u] = sat(m->loss_counts[reason - 1u]);
#endif
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

#if CLASSICK_MAC_SHORT_COLLECT
/* Pure: a READY, phased, unquiesced pass whose three button samples all equal
 * the retained button. Bounds/claim checked so no inconsistent inbox is kept. */
static int drop_ok(const struct cs_mac_motion *m, const cs_window_inbox *in,
    const snap *s1, const snap *s2, const snap *s3)
{
    unsigned held = m->last_buttons ? 0u : PB_BUTTON;
    return in->phase == CS_INBOX_READY && m->have_phase && !m->quiesced
        && in->head < CS_INBOX_CAPACITY && in->count <= CS_INBOX_CAPACITY
        && (in->claimed == CS_INBOX_NONE || in->claimed == CS_INBOX_CLAIM_SAMPLE)
        && (SNAP_PB(s1) & PB_BUTTON) == held && (SNAP_PB(s2) & PB_BUTTON) == held
        && (SNAP_PB(s3) & PB_BUTTON) == held;
}

/* Omit uncertain displacement: publish nothing, rebaseline DCD to the
 * post-ack sample, keep buttons/queue/epoch. A still-closed latch yields one
 * later pending/no-change pass, dropped the same way. Not lossless capture. */
static int drop(struct cs_mac_motion *m, uint32_t ra, uint32_t rb)
{
    m->last_raw_a = ra;
    m->last_raw_b = rb;
    m->motion_drops = sat(m->motion_drops);
    return CS_MAC_MOTION_EMPTY;
}
#endif

int cs_mac_motion_prepare(struct cs_mac_motion *m, const cs_mac_motion_io *io,
    cs_window_inbox *inbox, uint32_t invert)
{
    static const struct cs_mac_motion zero;
    if (!m || !io || !io->read || !io->write || !io->pb || !inbox
#if CLASSICK_MAC_FAST_COLLECT
        || !io->collect
#endif
        || invert > 3u)
        return CS_MAC_MOTION_ARGUMENT;
    if (inbox->valid != CS_INBOX_VALID || inbox->phase != CS_INBOX_WAIT
        || inbox->epoch == 0u)
        return CS_MAC_MOTION_STATE;
    *m = zero;
#if CLASSICK_MAC_MOTION_TIMELINE
    m->timeline.magic = CS_MAC_TIMELINE_MAGIC;
#endif
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

#if CLASSICK_MAC_FAST_COLLECT
static int motion_decode(struct cs_mac_motion *m, const uint32_t samples[4], int late_pending);

/* Keep the ordinary READY pass separate from exceptional/loss handling.
 * The same snapshots are decoded by the full path if any fast precondition
 * fails. No second hardware collection and no guessed transition. */
int cs_mac_motion_service(struct cs_mac_motion *m)
{
    uint32_t raw[4];
    if (!m) return CS_MAC_MOTION_ARGUMENT;
    if (m->phase != CS_MAC_MOTION_ARMED) return CS_MAC_MOTION_STATE;
#if CLASSICK_MAC_REARM_READY_PROBE
    const cs_window_inbox *in = m->inbox;
    if (m->quiesced && in->valid == CS_INBOX_VALID && in->epoch == m->epoch &&
        in->phase == CS_INBOX_PRIMING && in->head < CS_INBOX_CAPACITY &&
        in->count <= CS_INBOX_CAPACITY &&
        (in->claimed == CS_INBOX_CLAIM_BASELINE ||
         (in->claimed == CS_INBOX_NONE && in->count != 0u))) {
        m->services = sat(m->services);
        return CS_MAC_MOTION_EMPTY;
    }
#endif
    m->io.collect(m->io.cookie, raw);
    return cs_mac_motion_samples(m, raw, -1);
}

int cs_mac_motion_samples(struct cs_mac_motion *m, const uint32_t raw[4], int late_pending)
{
    uint32_t ra, rb, buttons, ca, cb, pending;
    int32_t dx = 0, dy = 0;
    cs_window_inbox *in;
    int r;
    if (!m || !raw || late_pending < -1 || late_pending > 255)
        return CS_MAC_MOTION_ARGUMENT;
    if (m->phase != CS_MAC_MOTION_ARMED) return CS_MAC_MOTION_STATE;
    m->services = sat(m->services);
    in = m->inbox;
    pending = raw[0];
    if (pending & ~UINT32_C(9))
        return fail(m, CS_MAC_STAGE_PENDING, CS_MAC_CAUSE_PENDING);
    if (in->valid != CS_INBOX_VALID || in->epoch != m->epoch ||
        in->phase != CS_INBOX_READY || !m->have_phase || m->quiesced ||
        (((raw[1] ^ raw[2]) | (raw[2] ^ raw[3])) & UINT32_C(0x00380808)))
        return motion_decode(m, raw, late_pending);
    ra = (raw[3] >> 11) & 1u;
    rb = (raw[3] >> 3) & 1u;
    buttons = (raw[3] & UINT32_C(0x00080000)) ? 0u : CS_POINTER_LEFT;
    ca = ra != m->last_raw_a; cb = rb != m->last_raw_b;
    if (ca != ((pending >> 3) & 1u) || cb != (pending & 1u))
        return motion_decode(m, raw, late_pending);
    if (!ca && !cb && buttons == m->last_buttons) return CS_MAC_MOTION_EMPTY;
    if (ca) dx = ((ra ^ (m->invert & 1u)) == ((raw[3] >> 20) & 1u)) ? 1 : -1;
    if (cb) dy = ((rb ^ ((m->invert >> 1) & 1u)) != ((raw[3] >> 21) & 1u)) ? 1 : -1;
#if CLASSICK_MAC_SHORT_COLLECT
    r = cs_inbox_push_motion(in, m->epoch, dx, dy, buttons);
    if (r == CS_INBOX_MERGED) m->motion_merges = sat(m->motion_merges);
#else
    r = cs_inbox_push(in, m->epoch, dx, dy, buttons);
#endif
    m->samples = sat(m->samples);
    if (r == CS_INBOX_LOST) {
        snap a = {raw[1]}, b = {raw[2]}, c = {raw[3]};
        note(m, CS_MAC_LOSS_OVERFLOW, (uint8_t)pending, &a, &b, &c, CS_INBOX_READY);
        m->have_phase = 0u; quiesce(m); m->losses = sat(m->losses);
        return CS_MAC_MOTION_LOST;
    }
    if (r != CS_INBOX_OK && r != CS_INBOX_MERGED)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    m->last_raw_a = ra; m->last_raw_b = rb; m->last_buttons = buttons;
    if (in->count > m->peak_queued) m->peak_queued = in->count;
    return CS_MAC_MOTION_OK;
}
static int motion_decode(struct cs_mac_motion *m, const uint32_t samples[4], int late_pending)
#else
int cs_mac_motion_service(struct cs_mac_motion *m)
#endif
{
    cs_window_inbox *in;
    uint8_t rr3;
    uint32_t pend_a, pend_b, ra, rb, buttons, ch_a, ch_b;
    int32_t dx = 0, dy = 0;
    snap s1, s2, s3;
    int stable, r;
    uint32_t entry;
#if CLASSICK_MAC_FAST_COLLECT
    (void)late_pending; /* Some non-SHORT profiles do not need a late read. */
#endif

    if (!m) return CS_MAC_MOTION_ARGUMENT;
    if (m->phase != CS_MAC_MOTION_ARMED) return CS_MAC_MOTION_STATE;
#if CLASSICK_MAC_MOTION_TIMELINE
    /* An omitted commit stays visibly unknown; never apply a later result. */
    m->timeline.pending = 0u;
#endif
#if !CLASSICK_MAC_FAST_COLLECT
    m->services = sat(m->services);
#endif
    in = m->inbox;

#if CLASSICK_MAC_REARM_READY_PROBE
    /* Do not acknowledge or decode while a valid baseline is in flight. */
    if (m->quiesced && in->valid == CS_INBOX_VALID && in->epoch == m->epoch &&
        in->phase == CS_INBOX_PRIMING && in->head < CS_INBOX_CAPACITY &&
        in->count <= CS_INBOX_CAPACITY &&
        (in->claimed == CS_INBOX_CLAIM_BASELINE ||
         (in->claimed == CS_INBOX_NONE && in->count != 0u)))
        return CS_MAC_MOTION_EMPTY;
#endif

    entry = in->phase;
#if CLASSICK_MAC_FAST_COLLECT
    {
        rr3 = (uint8_t)samples[0];
        if (rr3 & ~(RR3_EXT_A | RR3_EXT_B) & 0xFF)
            return fail(m, CS_MAC_STAGE_PENDING, CS_MAC_CAUSE_PENDING);
        s1.value = samples[1]; s2.value = samples[2]; s3.value = samples[3];
    }
    pend_a = bit(rr3, RR3_EXT_A);
    pend_b = bit(rr3, RR3_EXT_B);
#else
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
#endif
    stable = same(&s1, &s2) && same(&s2, &s3);

    if (in->valid != CS_INBOX_VALID || in->epoch != m->epoch)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_STATE);
    ra = bit(SNAP_A(&s3), RR0_DCD);
    rb = bit(SNAP_B(&s3), RR0_DCD);
    buttons = (SNAP_PB(&s3) & PB_BUTTON) ? 0u : (uint32_t)CS_POINTER_LEFT;
#if CLASSICK_MAC_SHORT_COLLECT
    if (!stable && in->phase == CS_INBOX_READY && m->have_phase && !m->quiesced) {
        /* A quiet axis does not consume its direction phase. Retain every
         * DCD, button and moving-axis phase check, including post-ack reads. */
        unsigned mask = PB_BUTTON;
        if (pend_a || ra != m->last_raw_a) mask |= PB_X2;
        if (pend_b || rb != m->last_raw_b) mask |= PB_Y2;
        stable = ((((SNAP_PB(&s1) ^ SNAP_PB(&s2)) | (SNAP_PB(&s2) ^ SNAP_PB(&s3))) & mask) == 0u)
            && ((((SNAP_A(&s1) ^ SNAP_A(&s2)) | (SNAP_A(&s2) ^ SNAP_A(&s3))) & RR0_DCD) == 0)
            && ((((SNAP_B(&s1) ^ SNAP_B(&s2)) | (SNAP_B(&s2) ^ SNAP_B(&s3))) & RR0_DCD) == 0);
    }
#endif
#if CLASSICK_MAC_MOTION_TIMELINE
    timeline_record(m, rr3, &s1, &s2, &s3, entry, stable, ra, rb, buttons);
#endif

    switch (in->phase) {
    case CS_INBOX_LOSS:
        DEFER_CLEAR(m);
        m->have_phase = 0u;
        quiesce(m);
        return CS_MAC_MOTION_LOST;
    case CS_INBOX_WAIT:
        DEFER_CLEAR(m);
        if (!stable) { m->have_phase = 0u; return CS_MAC_MOTION_EMPTY; }
#if CLASSICK_MAC_WAIT_LATCH_PROBE
        {
            /* A latch may have closed after entry RR3 but before the samples.
             * Leave it pending for a later pass rather than baselining it. */
            uint8_t late =
#if CLASSICK_MAC_FAST_COLLECT
                late_pending >= 0 ? (uint8_t)late_pending :
#endif
                m->io.read(m->io.cookie, CS_MAC_SCC_A, 3u);
            if (late & ~(RR3_EXT_A | RR3_EXT_B) & 0xFF)
                return fail(m, CS_MAC_STAGE_PENDING, CS_MAC_CAUSE_PENDING);
            if (late) { m->have_phase = 0u; return CS_MAC_MOTION_EMPTY; }
        }
#endif
        if (cs_inbox_baseline(in, m->epoch, buttons) != CS_INBOX_OK)
            return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
        m->last_raw_a = ra;
        m->last_raw_b = rb;
        m->last_buttons = buttons;
        m->have_phase = 1u;
        m->baselines = sat(m->baselines);
#if CLASSICK_MAC_REARM_READY_PROBE
        /* Keep master off until a later READY pass has checked fresh input. */
        quiesce(m);
#else
        if (m->quiesced) { /* stable baseline queued: restore master last */
            wr(m, CS_MAC_SCC_A, 9u, WR9_MIE_NV);
            m->quiesced = 0u;
        }
#endif
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
#if CLASSICK_MAC_SHORT_COLLECT
    /* Button-changing or otherwise ineligible passes keep the full cancel. */
    if (!stable && drop_ok(m, in, &s1, &s2, &s3))
        return drop(m, ra, rb);
#endif
    if (!m->have_phase || !stable) {
        return uncertain(m, !m->have_phase ? CS_MAC_LOSS_NO_PHASE : CS_MAC_LOSS_UNSTABLE,
            rr3, &s1, &s2, &s3, entry);
    }
    ch_a = ra != m->last_raw_a;
    ch_b = rb != m->last_raw_b;
#if CLASSICK_MAC_PENDING_IDLE_PROBE
    /* No movement or button decision: hidden even transitions remain unknown. */
    if (in->phase == CS_INBOX_READY && !m->quiesced && rr3 &&
        in->head < CS_INBOX_CAPACITY && in->count <= CS_INBOX_CAPACITY &&
        (in->claimed == CS_INBOX_NONE || in->claimed == CS_INBOX_CLAIM_SAMPLE) &&
        !ch_a && !ch_b && buttons == m->last_buttons)
        return CS_MAC_MOTION_EMPTY;
#endif
    if (ch_a != pend_a || ch_b != pend_b) {
#if CLASSICK_MAC_SHORT_COLLECT
        /* RR3 precedes the samples: a clean edge can arrive between them.
         * With no entry pending bits we acknowledged no channel. A matching
         * late latch can therefore be consumed by the next finite pass.
         * Do not publish, update the baseline, or guess a missing edge here. */
        if (rr3 == 0u && drop_ok(m, in, &s1, &s2, &s3) && (ch_a || ch_b)) {
            uint8_t late =
#if CLASSICK_MAC_FAST_COLLECT
                late_pending >= 0 ? (uint8_t)late_pending :
#endif
                m->io.read(m->io.cookie, CS_MAC_SCC_A, 3u);
            uint32_t expected = (ch_a ? RR3_EXT_A : 0u) | (ch_b ? RR3_EXT_B : 0u);
            if (late & ~(RR3_EXT_A | RR3_EXT_B) & 0xFF)
                return fail(m, CS_MAC_STAGE_PENDING, CS_MAC_CAUSE_PENDING);
            if ((uint32_t)late == expected) return CS_MAC_MOTION_EMPTY;
        }
        /* The clean late-edge retry above takes precedence over dropping. */
        if (drop_ok(m, in, &s1, &s2, &s3)) return drop(m, ra, rb);
#endif
#if CLASSICK_MAC_LOSS_COUNTS
        r = uncertain(m, CS_MAC_LOSS_PENDING, rr3, &s1, &s2, &s3, entry);
        /* A single rejection can disagree on both channels. */
        if (r == CS_MAC_MOTION_LOST) {
            uint32_t context = entry == CS_INBOX_PRIMING ? 0u :
                buttons != m->last_buttons ? 1u : (ch_a || ch_b) ? 2u : 3u;
            m->pending_context[context] = sat(m->pending_context[context]);
            if (ch_a != pend_a) {
                uint32_t i = pend_a ? 0u : 1u;
                m->pending_mismatch[i] = sat(m->pending_mismatch[i]);
            }
            if (ch_b != pend_b) {
                uint32_t i = pend_b ? 2u : 3u;
                m->pending_mismatch[i] = sat(m->pending_mismatch[i]);
            }
        }
        return r;
#else
        return uncertain(m, CS_MAC_LOSS_PENDING, rr3, &s1, &s2, &s3, entry);
#endif
    }
    if (ch_a)
        dx = ((ra ^ (m->invert & 1u)) == bit(SNAP_PB(&s3), PB_X2)) ? 1 : -1;
    if (ch_b)
        dy = ((rb ^ ((m->invert >> 1) & 1u)) != bit(SNAP_PB(&s3), PB_Y2)) ? 1 : -1;
    if (!ch_a && !ch_b && buttons == m->last_buttons) {
#if CLASSICK_MAC_REARM_READY_PROBE
        if (in->phase == CS_INBOX_READY) rearm_ready(m);
#endif
        return CS_MAC_MOTION_EMPTY;
    }

#if CLASSICK_MAC_SHORT_COLLECT
    r = cs_inbox_push_motion(in, m->epoch, dx, dy, buttons);
#else
    r = cs_inbox_push(in, m->epoch, dx, dy, buttons);
#endif
#if CLASSICK_MAC_SHORT_COLLECT
    if (r == CS_INBOX_MERGED) m->motion_merges = sat(m->motion_merges);
#endif
    m->samples = sat(m->samples);
    if (r == CS_INBOX_LOST) {
        note(m, CS_MAC_LOSS_OVERFLOW, rr3, &s1, &s2, &s3, entry);
        m->have_phase = 0u;
        quiesce(m);
        m->losses = sat(m->losses);
        return CS_MAC_MOTION_LOST;
    }
    if (r != CS_INBOX_OK && r != CS_INBOX_MERGED)
        return fail(m, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    m->last_raw_a = ra;
    m->last_raw_b = rb;
    m->last_buttons = buttons;
    if (in->count > m->peak_queued) m->peak_queued = in->count;
#if CLASSICK_MAC_REARM_READY_PROBE
    if (in->phase == CS_INBOX_READY) rearm_ready(m);
#endif
    return CS_MAC_MOTION_OK;
}

#if CLASSICK_MAC_FAST_COLLECT
int cs_mac_motion_capture_lost(struct cs_mac_motion *m)
{
    snap empty = {0};
    if (!m) return CS_MAC_MOTION_ARGUMENT;
    if (m->phase != CS_MAC_MOTION_ARMED) return CS_MAC_MOTION_STATE;
    return uncertain(m, CS_MAC_LOSS_CAPTURE, 0, &empty, &empty, &empty, m->inbox->phase);
}
#endif

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
