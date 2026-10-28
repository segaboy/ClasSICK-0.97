/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "timer-poll.h"

enum { T1CL = 4, T1CH = 5, T1LL = 6, T1LH = 7, ACR = 11, IFR = 13, IER = 14,
       IFR_T1 = 0x40 };

/* Run-time WR15 after the alias test: probe keeps DCD enable on both channels. */
#if CLASSICK_MAC_TIMER_REFRESH_PROBE
#define RUN_WR15 8u
#else
#define RUN_WR15 0u
#endif

static uint8_t vr(const cs_mac_timer_poll *p, unsigned r)
{
    return p->via.read(p->via.context, r);
}

static void vw(const cs_mac_timer_poll *p, unsigned r, uint8_t v)
{
    p->via.write(p->via.context, r, v);
}

static uint8_t sr(const cs_mac_timer_poll *p, uint32_t ch, uint32_t r)
{
    return p->motion.io.read(p->motion.io.cookie, ch, r);
}

static void sw(const cs_mac_timer_poll *p, uint32_t ch, uint32_t r, uint8_t v)
{
    p->motion.io.write(p->motion.io.cookie, ch, r, v);
}

static void sat(uint32_t *c)
{
    if (*c != UINT32_MAX)
        (*c)++;
}

static void t1_off(const cs_mac_timer_poll *p)
{
    vw(p, IER, 0x40);
    (void)vr(p, T1CL);
}

static void scc_off(const cs_mac_timer_poll *p)
{
    sw(p, CS_MAC_SCC_A, 9, 0x02);
}

static int fail(cs_mac_timer_poll *p, uint32_t stage, int32_t cause)
{
    t1_off(p);
    scc_off(p);
    p->motion.phase = CS_MAC_MOTION_FAILED;
    p->motion.stage = stage;
    p->motion.cause = cause;
    return CS_MAC_MOTION_FAULT;
}

static int inbox_ok(const struct cs_mac_motion *m)
{
    const cs_window_inbox *i = m->inbox;
    return i && i->valid == CS_INBOX_VALID && i->epoch && i->epoch == m->epoch;
}

int cs_mac_timer_poll_prepare(cs_mac_timer_poll *p, const cs_mac_motion_io *scc,
    const cs_mac_io *via, cs_window_inbox *inbox, uint32_t invert,
    uint32_t reload)
{
    static const struct cs_mac_motion zero;
    cs_mac_motion_io s;
    cs_mac_io v;
    uint8_t lo = (uint8_t)reload, hi = (uint8_t)(reload >> 8);

    if (!p || !scc || !via || !inbox || !scc->read || !scc->write || !scc->pb ||
#if CLASSICK_MAC_TIMER_PAIR_PROBE
        !scc->pair ||
#endif
        !via->read || !via->write || invert > 3u || reload < 1u ||
        reload > 65535u || inbox->valid != CS_INBOX_VALID ||
        inbox->phase != CS_INBOX_WAIT || !inbox->epoch)
        return CS_MAC_MOTION_ARGUMENT;
    s = *scc;
    v = *via;
    p->motion = zero;
    p->motion.io = s;
    p->motion.inbox = inbox;
    p->motion.epoch = inbox->epoch;
    p->motion.invert = invert;
    p->motion.phase = CS_MAC_MOTION_PREPARED;
    p->via = v;
    p->reload = reload;
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
    (void)cs_mac_activity_begin(&p->activity);
#endif
    if (cs_mac_polled_begin(&p->decoder, invert) != CS_MAC_POLLED_EMPTY)
        return fail(p, CS_MAC_STAGE_CONFIG, CS_MAC_CAUSE_INBOX_RESULT);

    p->motion.stage = CS_MAC_STAGE_RESET;
    t1_off(p);
    sw(p, CS_MAC_SCC_A, 9, 0xC0);
    sw(p, CS_MAC_SCC_A, 9, 0x02);
    p->motion.stage = CS_MAC_STAGE_CONFIG;
    sw(p, CS_MAC_SCC_A, 1, 0);
    sw(p, CS_MAC_SCC_A, 15, 8);
    sw(p, CS_MAC_SCC_B, 1, 0);
    sw(p, CS_MAC_SCC_B, 15, 0);
    sw(p, CS_MAC_SCC_A, 2, 0);
    p->motion.stage = CS_MAC_STAGE_READBACK_A;
    if (sr(p, CS_MAC_SCC_A, 15) != 8)
        return fail(p, CS_MAC_STAGE_READBACK_A, CS_MAC_CAUSE_READBACK);
    p->motion.stage = CS_MAC_STAGE_READBACK_B;
    if (sr(p, CS_MAC_SCC_B, 15) != 0)
        return fail(p, CS_MAC_STAGE_READBACK_B, CS_MAC_CAUSE_READBACK);
    sw(p, CS_MAC_SCC_A, 15, RUN_WR15);
#if CLASSICK_MAC_TIMER_REFRESH_PROBE
    sw(p, CS_MAC_SCC_B, 15, RUN_WR15);
#endif
    p->motion.stage = CS_MAC_STAGE_READBACK_A;
    if (sr(p, CS_MAC_SCC_A, 15) != RUN_WR15)
        return fail(p, CS_MAC_STAGE_READBACK_A, CS_MAC_CAUSE_READBACK);
    p->motion.stage = CS_MAC_STAGE_READBACK_B;
    if (sr(p, CS_MAC_SCC_B, 15) != RUN_WR15)
        return fail(p, CS_MAC_STAGE_READBACK_B, CS_MAC_CAUSE_READBACK);

    p->motion.stage = CS_MAC_STAGE_CONFIG;
    vw(p, ACR, 0x40);
    vw(p, T1CL, lo);
    vw(p, T1CH, hi);
    if (vr(p, ACR) != 0x40 || vr(p, T1LL) != lo || vr(p, T1LH) != hi)
        return fail(p, CS_MAC_STAGE_CONFIG, CS_MAC_CAUSE_READBACK);
    return CS_MAC_MOTION_OK;
}

int cs_mac_timer_poll_arm(cs_mac_timer_poll *p)
{
    if (!p)
        return CS_MAC_MOTION_ARGUMENT;
    if (p->motion.phase != CS_MAC_MOTION_PREPARED || !inbox_ok(&p->motion) ||
        p->motion.inbox->phase != CS_INBOX_WAIT)
        return CS_MAC_MOTION_STATE;
    (void)vr(p, T1CL);
    vw(p, IER, 0xC0);
    p->motion.phase = CS_MAC_MOTION_ARMED;
    p->motion.stage = CS_MAC_STAGE_ARM;
    return CS_MAC_MOTION_OK;
}

static int whole_loss(cs_mac_timer_poll *p, uint32_t reason, uint32_t s0,
    uint32_t s1)
{
    struct cs_mac_motion *m = &p->motion;
    cs_window_inbox *in = m->inbox;

#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
    uint32_t flags = CS_MAC_ACT_LOST;
    if (((s0 ^ s1) & UINT32_C(0x00380808)) == 0u) flags |= CS_MAC_ACT_STABLE;
    if (m->have_phase) flags |= CS_MAC_ACT_HAD_PHASE;
    (void)cs_mac_activity_pair(&p->activity, s0, s1, flags, reason);
#endif

    if (!m->first_loss.reason) {
        struct cs_mac_motion_loss *l = &m->first_loss;
        l->rr3 = 0;
        l->snapshots[0] = s0;
        l->snapshots[1] = s1;
        l->snapshots[2] = 0;
        l->previous = m->have_phase << 24 | m->last_raw_a << 16 |
            m->last_raw_b << 8 | m->last_buttons;
        l->inbox_phase = in->phase;
        l->reason = reason;
    }
    p->decoder.have_phase = m->have_phase = 0;
    sat(&m->losses);
    if (in->phase != CS_INBOX_LOSS) {
        (void)cs_inbox_uncertain(in, m->epoch);
        if (in->phase != CS_INBOX_LOSS)
            return fail(p, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    }
    return CS_MAC_MOTION_LOST;
}

int cs_mac_timer_poll_service(cs_mac_timer_poll *p)
{
    struct cs_mac_motion *m;
    cs_window_inbox *in;
    cs_pointer_sample o;
    uint32_t pb1, a1, b1, pb2, a2, b2, s0, s1, ph;
    int rc, ret, t1;

    if (!p)
        return CS_MAC_MOTION_ARGUMENT;
    m = &p->motion;
    if (m->phase != CS_MAC_MOTION_ARMED || !inbox_ok(m))
        return CS_MAC_MOTION_STATE;
    in = m->inbox;
    if (!(vr(p, IFR) & IFR_T1))
        return CS_MAC_MOTION_EMPTY;
    (void)vr(p, T1CL);
    sat(&m->services);
    ph = in->phase;
    if (ph == CS_INBOX_LOSS) {
        p->decoder.have_phase = m->have_phase = 0;
        return CS_MAC_MOTION_EMPTY;
    }
    if (ph != CS_INBOX_WAIT && ph != CS_INBOX_PRIMING && ph != CS_INBOX_READY)
        return fail(p, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_STATE);
    if (ph == CS_INBOX_WAIT)
        p->decoder.have_phase = m->have_phase = 0;
#if CLASSICK_MAC_TIMER_REFRESH_PROBE
    sw(p, CS_MAC_SCC_A, 0, 0x10);
    sw(p, CS_MAC_SCC_B, 0, 0x10);
#endif

#if CLASSICK_MAC_TIMER_PAIR_PROBE
    {
        uint8_t g[6]; /* Trusted callback writes all six bytes. */
        m->io.pair(m->io.cookie, g);
        pb1 = g[0]; a1 = g[1]; b1 = g[2];
        pb2 = g[3]; a2 = g[4]; b2 = g[5];
    }
#else
    pb1 = m->io.pb(m->io.cookie);
    a1 = sr(p, CS_MAC_SCC_A, 0);
    b1 = sr(p, CS_MAC_SCC_B, 0);
    pb2 = m->io.pb(m->io.cookie);
    a2 = sr(p, CS_MAC_SCC_A, 0);
    b2 = sr(p, CS_MAC_SCC_B, 0);
#endif
    t1 = (vr(p, IFR) & IFR_T1) != 0;
    s0 = pb1 << 16 | a1 << 8 | b1;
    s1 = pb2 << 16 | a2 << 8 | b2;
    if (((pb1 ^ pb2) & 0x38u) || ((a1 ^ a2) & 8u) || ((b1 ^ b2) & 8u))
        return whole_loss(p, CS_MAC_LOSS_UNSTABLE, s0, s1);
    if (t1)
        return whole_loss(p, CS_MAC_TIMER_LOSS_OVERRUN, s0, s1);

    rc = cs_mac_polled_step(&p->decoder, pb2, a2, b2, &o);
    if (rc == CS_MAC_POLLED_LOST)
        return whole_loss(p, CS_MAC_TIMER_LOSS_GRAY, s0, s1);
    if (rc == CS_MAC_POLLED_BASELINED) {
        if (ph != CS_INBOX_WAIT ||
            cs_inbox_baseline(in, m->epoch, o.buttons) != CS_INBOX_OK)
            return fail(p, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
        sat(&m->baselines);
        ret = CS_MAC_MOTION_BASELINED;
    } else if (rc == CS_MAC_POLLED_SAMPLE) {
        int r = cs_inbox_push(in, m->epoch, o.dx, o.dy, o.buttons);
        if (r == CS_INBOX_LOST)
            return whole_loss(p, CS_MAC_LOSS_OVERFLOW, s0, s1);
        if (r != CS_INBOX_OK)
            return fail(p, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
        sat(&m->samples);
        if (in->count > m->peak_queued)
            m->peak_queued = in->count;
        ret = CS_MAC_MOTION_OK;
    } else if (rc == CS_MAC_POLLED_EMPTY) {
        ret = CS_MAC_MOTION_EMPTY;
    } else {
        return fail(p, CS_MAC_STAGE_INBOX, CS_MAC_CAUSE_INBOX_RESULT);
    }
    if (in->count > m->peak_queued) m->peak_queued = in->count;
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
    {
        uint32_t flags = CS_MAC_ACT_STABLE | CS_MAC_ACT_ACCEPTED;
        if (m->have_phase) flags |= CS_MAC_ACT_HAD_PHASE;
        if (rc == CS_MAC_POLLED_SAMPLE) {
            flags |= CS_MAC_ACT_QUEUED;
            if (o.dx || o.dy) flags |= CS_MAC_ACT_MOTION;
        }
        (void)cs_mac_activity_pair(&p->activity, s0, s1, flags, 0u);
    }
#endif
    m->have_phase = p->decoder.have_phase;
    m->last_raw_a = (a2 >> 3) & 1u;
    m->last_raw_b = (b2 >> 3) & 1u;
    m->last_buttons = o.buttons;
    return ret;
}

int cs_mac_timer_poll_stop(cs_mac_timer_poll *p)
{
    uint32_t ph, ch;

    if (!p)
        return CS_MAC_MOTION_ARGUMENT;
    ph = p->motion.phase;
    if (ph == CS_MAC_MOTION_STOPPED)
        return CS_MAC_MOTION_OK;
    if (ph != CS_MAC_MOTION_PREPARED && ph != CS_MAC_MOTION_ARMED &&
        ph != CS_MAC_MOTION_FAILED)
        return CS_MAC_MOTION_STATE;
    t1_off(p);
    scc_off(p);
    for (ch = CS_MAC_SCC_A; ch <= CS_MAC_SCC_B; ch++) {
        sw(p, ch, 1, 0);
        sw(p, ch, 15, 0);
    }
    p->motion.phase = CS_MAC_MOTION_STOPPED;
    return CS_MAC_MOTION_OK;
}
