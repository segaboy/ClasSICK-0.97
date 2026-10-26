/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Links inbox.c, polled-phase.c and timer-poll.c only. Fake VIA keeps the
 * T1 counter read (reg 4) and latch reads (6/7) separate and never touches CA1. */
#include <string.h>
#include <stdio.h>
#include "../platform/mac128k/timer-poll.h"

/* log entry: type(1 VIA w,2 VIA r,3 SCC w,4 SCC non-RR0 r), channel, reg, value */
#define E(t,c,r,v) (((uint32_t)(t)<<24)|((uint32_t)(c)<<16)|((uint32_t)(r)<<8)|(uint32_t)(v))
#define K(c) do { checks++; if (!(c)) { fails++; printf("FAIL:%d:%s\n",__LINE__,#c); } } while (0)
#define LOG_MAX 128

typedef struct {
    uint8_t sw[2][16], q[3][8], qi[3], ifq[2], ifi, ier, acr, ll, lh, alias, drop5;
    uint32_t ierr, mie, r7w, pbn, rr0n, n, log[LOG_MAX];
} fk;
typedef struct {
    fk f;
    cs_mac_timer_poll p;
    cs_window_inbox in;
    cs_mac_io via;
    cs_mac_motion_io scc;
} T;

static int fails;
static unsigned checks;

static void lg(fk *f, uint32_t e)
{
    if (f->n < LOG_MAX)
        f->log[f->n] = e;
    f->n++;
}

static uint8_t vrd(void *c, unsigned r)
{
    fk *f = c;
    lg(f, E(2, 0, r, 0));
    if (r == 13)
        return f->ifi < 2 ? f->ifq[f->ifi++] : 0;
    if (r == 6) return f->ll;
    if (r == 7) return f->lh;
    if (r == 11) return f->acr;
    if (r == 14) f->ierr++;
    return 0;
}

static void vwr(void *c, unsigned r, uint8_t v)
{
    fk *f = c;
    lg(f, E(1, 0, r, v));
    if (r == 4) f->ll = v;
    else if (r == 5) { if (!f->drop5) f->lh = v; }
    else if (r == 7) f->r7w++;
    else if (r == 11) f->acr = v;
    else if (r == 14) { if (v & 0x80) f->ier |= v & 0x7f; else f->ier &= ~v; }
}

static uint8_t srd(void *c, uint32_t ch, uint32_t reg)
{
    fk *f = c;
    if (reg == 0) {
        f->rr0n++;
        return f->q[1 + ch][f->qi[1 + ch]++ & 7];
    }
    lg(f, E(4, ch, reg, 0));
    return f->sw[(f->alias && reg == 15) ? 0 : ch][reg & 15];
}

static void swr(void *c, uint32_t ch, uint32_t reg, uint8_t v)
{
    fk *f = c;
    lg(f, E(3, ch, reg, v));
    if (reg == 9 && (v & 8)) f->mie++;
    f->sw[(f->alias && reg == 15) ? 0 : ch][reg & 15] = v;
}

static uint8_t pbr(void *c)
{
    fk *f = c;
    f->pbn++;
    return f->q[0][f->qi[0]++ & 7];
}

static int svc(T *t, int i1, int i2, int p1, int a1, int b1, int p2, int a2, int b2)
{
    fk *f = &t->f;
    f->q[0][0] = (uint8_t)p1; f->q[0][1] = (uint8_t)p2;
    f->q[1][0] = (uint8_t)a1; f->q[1][1] = (uint8_t)a2;
    f->q[2][0] = (uint8_t)b1; f->q[2][1] = (uint8_t)b2;
    memset(f->qi, 0, 3);
    f->ifq[0] = (uint8_t)i1; f->ifq[1] = (uint8_t)i2; f->ifi = 0;
    return cs_mac_timer_poll_service(&t->p);
}

static void init(T *t)
{
    memset(t, 0, sizeof *t);
    cs_inbox_begin(&t->in);
    t->via.context = &t->f; t->via.read = vrd; t->via.write = vwr;
    t->scc.cookie = &t->f; t->scc.read = srd; t->scc.write = swr; t->scc.pb = pbr;
}

static int prep(T *t, uint32_t rl)
{
    return cs_mac_timer_poll_prepare(&t->p, &t->scc, &t->via, &t->in, 0, rl);
}

static T *ready(T *t)
{
    init(t);
    K(prep(t, 0x102) == CS_MAC_MOTION_OK);
    K(cs_mac_timer_poll_arm(&t->p) == CS_MAC_MOTION_OK);
    return t;
}

static int base(T *t)
{
    return svc(t, 0x40, 0, 0x08, 0, 0, 0x08, 0, 0);
}

static int logeq(const fk *f, uint32_t at, const uint32_t *e, uint32_t n)
{
    return f->n == at + n && !memcmp(f->log + at, e, n * sizeof *e);
}

static int take(T *t, cs_pointer_sample *o)
{
    int r = cs_inbox_claim(&t->in, t->in.epoch);
    *o = t->in.last;
    K(cs_inbox_complete(&t->in, t->in.epoch, 0) == CS_INBOX_OK);
    return r;
}

static void clean(const T *t)
{
    K(!t->f.mie && !t->f.ierr && !t->f.r7w);
}

/* Returns first-loss reason of an otherwise stable second pass, or 0. */
static uint32_t lossreason(T *t, int pb2, int a2, int ifr2)
{
    ready(t);
    K(base(t) == CS_MAC_MOTION_BASELINED);
    if (svc(t, 0x40, ifr2, 0x08, 0, 0, pb2, a2, 0) != CS_MAC_MOTION_LOST)
        return 0;
    return t->p.motion.first_loss.reason;
}

int main(void)
{
    static T t;
    static const uint32_t ep[20] = {
        E(1,0,14,0x40), E(2,0,4,0), E(3,0,9,0xC0), E(3,0,9,2), E(3,0,1,0),
        E(3,0,15,8), E(3,1,1,0), E(3,1,15,0), E(3,0,2,0), E(4,0,15,0),
        E(4,1,15,0), E(3,0,15,0), E(4,0,15,0), E(4,1,15,0), E(1,0,11,0x40),
        E(1,0,4,2), E(1,0,5,1), E(2,0,11,0), E(2,0,6,0), E(2,0,7,0) };
    static const uint32_t ea[2] = { E(2,0,4,0), E(1,0,14,0xC0) };
    static const uint32_t es[7] = { E(1,0,14,0x40), E(2,0,4,0), E(3,0,9,2),
        E(3,0,1,0), E(3,0,15,0), E(3,1,1,0), E(3,1,15,0) };
    static const uint32_t off[3] = { E(1,0,14,0x40), E(2,0,4,0), E(3,0,9,2) };
    cs_pointer_sample o;
    uint32_t n0, pbn0, i;
    int rc;

    /* refusals: no effect, no IO */
    init(&t);
    t.p.motion.phase = 0x77;
    K(cs_mac_timer_poll_prepare(0, &t.scc, &t.via, &t.in, 0, 1) == CS_MAC_MOTION_ARGUMENT);
    K(cs_mac_timer_poll_prepare(&t.p, &t.scc, &t.via, &t.in, 4, 1) == CS_MAC_MOTION_ARGUMENT);
    K(prep(&t, 0) == CS_MAC_MOTION_ARGUMENT);
    K(prep(&t, 65536) == CS_MAC_MOTION_ARGUMENT);
    t.scc.pb = 0;
    K(prep(&t, 1) == CS_MAC_MOTION_ARGUMENT);
    t.scc.pb = pbr;
    t.in.valid = 0;
    K(prep(&t, 1) == CS_MAC_MOTION_ARGUMENT);
    K(t.f.n == 0 && t.p.motion.phase == 0x77);

    /* exact prepare setup/readback, then arm */
    init(&t);
    K(prep(&t, 0x102) == CS_MAC_MOTION_OK);
    K(logeq(&t.f, 0, ep, 20));
    K(t.p.motion.phase == CS_MAC_MOTION_PREPARED && t.p.motion.epoch == 1);
    n0 = t.f.n;
    K(cs_mac_timer_poll_service(&t.p) == CS_MAC_MOTION_STATE && t.f.n == n0);
    K(cs_mac_timer_poll_arm(&t.p) == CS_MAC_MOTION_OK);
    K(logeq(&t.f, 20, ea, 2));
    K(t.p.motion.phase == CS_MAC_MOTION_ARMED && t.p.motion.stage == CS_MAC_STAGE_ARM);
    n0 = t.f.n;
    K(cs_mac_timer_poll_arm(&t.p) == CS_MAC_MOTION_STATE && t.f.n == n0);
    clean(&t);

    /* empty IRQ: only IFR is read */
    n0 = t.f.n;
    K(svc(&t, 0, 0, 0, 0, 0, 0, 0, 0) == CS_MAC_MOTION_EMPTY);
    K(t.f.n == n0 + 1 && !t.f.pbn && !t.f.rr0n && !t.p.motion.services);

    /* baseline, Gray sample, PB-only, button */
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(t.in.phase == CS_INBOX_PRIMING && t.p.motion.baselines == 1 && t.p.motion.services == 1);
    K(take(&t, &o) == CS_INBOX_BASELINE_ITEM && o.buttons == 0);
    K(svc(&t, 0x40, 0, 0x08, 8, 0, 0x08, 8, 0) == CS_MAC_MOTION_OK);
    K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM && o.dx == -1 && o.dy == 0);
    K(svc(&t, 0x40, 0, 0x18, 8, 0, 0x18, 8, 0) == CS_MAC_MOTION_EMPTY);
    K(t.in.count == 0 && t.p.motion.samples == 1);
    K(svc(&t, 0x40, 0, 0x10, 8, 0, 0x10, 8, 0) == CS_MAC_MOTION_OK);
    K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM && o.buttons == 1 && o.dx == 0);
    clean(&t);

    /* Gray loss, sticky LOSS, recovery with a held button */
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(svc(&t, 0x40, 0, 0x18, 8, 0, 0x18, 8, 0) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_TIMER_LOSS_GRAY);
    K(t.p.motion.first_loss.snapshots[0] == (0x18u << 16 | 0x08u << 8));
    K(t.p.motion.first_loss.snapshots[2] == 0 && t.p.motion.first_loss.rr3 == 0);
    K(t.p.motion.first_loss.previous == (1u << 24));
    K(t.in.phase == CS_INBOX_LOSS && !t.p.motion.have_phase && t.p.motion.losses == 1);
    pbn0 = t.f.pbn;
    K(svc(&t, 0x40, 0, 0, 0, 0, 0, 0, 0) == CS_MAC_MOTION_EMPTY);
    K(t.f.pbn == pbn0 && t.p.motion.services == 3);
    K(cs_inbox_discard(&t.in, t.in.epoch) == CS_INBOX_OK);
    K(cs_inbox_resume(&t.in, t.in.epoch) == CS_INBOX_BASELINE);
    K(svc(&t, 0x40, 0, 0x00, 0, 0, 0x00, 0, 0) == CS_MAC_MOTION_BASELINED);
    K(take(&t, &o) == CS_INBOX_BASELINE_ITEM && o.buttons == 1);
    K(t.p.motion.first_loss.reason == CS_MAC_TIMER_LOSS_GRAY);

    /* unstable pairs and T1 recurrence */
    K(lossreason(&t, 0x28, 0, 0) == CS_MAC_LOSS_UNSTABLE);
    K(lossreason(&t, 0x08, 8, 0) == CS_MAC_LOSS_UNSTABLE);
    K(lossreason(&t, 0x08, 0, 0x40) == CS_MAC_TIMER_LOSS_OVERRUN);
    K(t.in.phase == CS_INBOX_LOSS && t.in.count <= 1);

    /* queue overflow: no coalescing, one uncertain only */
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    rc = CS_MAC_MOTION_OK;
    for (i = 0; i < 20 && rc == CS_MAC_MOTION_OK; i++) {
        int a = (i & 1) ? 0 : 8;
        rc = svc(&t, 0x40, 0, 0x08, a, 0, 0x08, a, 0);
    }
    K(rc == CS_MAC_MOTION_LOST && t.p.motion.first_loss.reason == CS_MAC_LOSS_OVERFLOW);
    K(t.p.motion.peak_queued >= 15 && t.in.phase == CS_INBOX_LOSS && t.in.losses == 1);

    /* stop */
    ready(&t);
    n0 = t.f.n;
    K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK);
    K(logeq(&t.f, n0, es, 7) && t.p.motion.phase == CS_MAC_MOTION_STOPPED);
    n0 = t.f.n;
    K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK && t.f.n == n0);
    K(svc(&t, 0x40, 0, 0, 0, 0, 0, 0, 0) == CS_MAC_MOTION_STATE && t.f.n == n0);
    init(&t);
    K(prep(&t, 5) == CS_MAC_MOTION_OK);
    K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK);
    clean(&t);

    /* readback alias guard failure */
    init(&t);
    t.f.alias = 1;
    K(prep(&t, 0x102) == CS_MAC_MOTION_FAULT);
    K(t.p.motion.phase == CS_MAC_MOTION_FAILED && t.p.motion.stage == CS_MAC_STAGE_READBACK_A);
    K(t.p.motion.cause == CS_MAC_CAUSE_READBACK);
    K(logeq(&t.f, t.f.n - 3, off, 3));
    K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK);

    /* timer latch readback failure */
    init(&t);
    t.f.drop5 = 1;
    K(prep(&t, 0x102) == CS_MAC_MOTION_FAULT);
    K(t.p.motion.stage == CS_MAC_STAGE_CONFIG && t.p.motion.cause == CS_MAC_CAUSE_READBACK);
    K(t.p.motion.phase == CS_MAC_MOTION_FAILED && logeq(&t.f, t.f.n - 3, off, 3));
    K(!t.f.mie);

    /* unexpected inbox phase is a hard fault with T1/SCC off */
    ready(&t);
    K(cs_inbox_fault(&t.in, t.in.epoch, 5) == CS_INBOX_FAULTED);
    K(svc(&t, 0x40, 0, 0x08, 0, 0, 0x08, 0, 0) == CS_MAC_MOTION_FAULT);
    K(t.p.motion.phase == CS_MAC_MOTION_FAILED && t.p.motion.stage == CS_MAC_STAGE_INBOX);
    K(t.p.motion.cause == CS_MAC_CAUSE_INBOX_STATE && !t.f.pbn);
    K(logeq(&t.f, t.f.n - 3, off, 3));
    clean(&t);

    printf("timer poll author:%u assertions,%d failures\n",checks,fails);
    return fails;
}
