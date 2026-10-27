/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Standalone fixture for CLASSICK_MAC_TIMER_REFRESH_PROBE=1; links inbox.c,
 * polled-phase.c and timer-poll.c built with the probe. The fake SCC keeps an
 * independent DCD latch per channel: while WR15 bit3 is set RR0 bit3 reports
 * the latch, which is loaded from the live level on enable and on WR0 0x10
 * and otherwise held. Engineering model only, not emulator/hardware evidence.
 * No native driver input exists; nothing here enables SCC interrupts. */
#include <string.h>
#include <stdio.h>
#include "../platform/mac128k/timer-poll.h"
#if CLASSICK_MAC_TIMER_REFRESH_PROBE != 1
#error build with CLASSICK_MAC_TIMER_REFRESH_PROBE=1
#endif

/* log: type(1 VIA w,2 VIA r,3 SCC w,4 SCC non-RR0 r,5 RR0 r,6 PB r) */
#define E(t,c,r,v) (((uint32_t)(t)<<24)|((uint32_t)(c)<<16)|((uint32_t)(r)<<8)|(uint32_t)(v))
#define K(c) do { checks++; if (!(c)) { fails++; printf("FAIL:%d:%s\n",__LINE__,#c); } } while (0)
#define LOG_MAX 256

typedef struct {
    uint8_t wr[2][16], live[2], latch[2], pb, ifq[2], ifi, acr, ll, lh, bad15, n15;
    uint32_t mie, flip, rr0n, pbflip, pbreads, n, log[LOG_MAX];
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
    if (r == 13) return f->ifi < 2 ? f->ifq[f->ifi++] : 0;
    if (r == 6) return f->ll;
    if (r == 7) return f->lh;
    return r == 11 ? f->acr : 0;
}

static void vwr(void *c, unsigned r, uint8_t v)
{
    fk *f = c;
    lg(f, E(1, 0, r, v));
    if (r == 4) f->ll = v;
    else if (r == 5) f->lh = v;
    else if (r == 11) f->acr = v;
}

static uint8_t srd(void *c, uint32_t ch, uint32_t reg)
{
    fk *f = c;
    uint8_t v;
    if (reg) {
        lg(f, E(4, ch, reg, 0));
        return f->wr[ch][reg & 15];
    }
    lg(f, E(5, ch, 0, 0));
    v = (f->wr[ch][15] & 8) ? f->latch[ch] : f->live[ch];
    if (f->flip && ++f->rr0n == f->flip)
        f->live[0] ^= 8; /* live change after the first RR0A read */
    return v;
}

static void swr(void *c, uint32_t ch, uint32_t reg, uint8_t v)
{
    fk *f = c;
    lg(f, E(3, ch, reg, v));
    if (reg == 0) {
        if (v == 0x10) f->latch[ch] = f->live[ch];
        return;
    }
    if (reg == 9 && (v & 8)) f->mie++;
    if (reg == 15 && f->n15++ == f->bad15) v = 0; /* selection/readback fault */
    if (reg == 15 && (v & 8)) f->latch[ch] = f->live[ch];
    f->wr[ch][reg & 15] = v;
}

static uint8_t pbr(void *c)
{
    fk *f = c;
    uint8_t v = f->pb;
    lg(f, E(6, 0, 0, 0));
    if (f->pbflip && ++f->pbreads == f->pbflip) f->pb = (uint8_t)(f->pb ^ 0x10u);
    return v;
}

static void init(T *t)
{
    memset(t, 0, sizeof *t);
    cs_inbox_begin(&t->in);
    t->via.context = &t->f; t->via.read = vrd; t->via.write = vwr;
    t->scc.cookie = &t->f; t->scc.read = srd; t->scc.write = swr; t->scc.pb = pbr;
    t->f.bad15 = 0xff;
    t->f.pb = 0x08;
}

static int prep(T *t)
{
    return cs_mac_timer_poll_prepare(&t->p, &t->scc, &t->via, &t->in, 0, 0x102);
}

static void ready(T *t)
{
    init(t);
    K(prep(t) == CS_MAC_MOTION_OK);
    K(cs_mac_timer_poll_arm(&t->p) == CS_MAC_MOTION_OK);
}

static int svc(T *t, int i1, int i2)
{
    t->f.ifq[0] = (uint8_t)i1; t->f.ifq[1] = (uint8_t)i2; t->f.ifi = 0;
    return cs_mac_timer_poll_service(&t->p);
}

static int logeq(const fk *f, uint32_t at, const uint32_t *e, uint32_t n)
{
    return f->n == at + n && !memcmp(f->log + at, e, n * sizeof *e);
}

static uint32_t cnt(const fk *f, uint32_t from, uint32_t ty, uint32_t reg)
{
    uint32_t i, k = 0;
    for (i = from; i < f->n && i < LOG_MAX; i++)
        k += (f->log[i] >> 24) == ty && ((f->log[i] >> 8) & 0xffu) == reg;
    return k;
}

static int take(T *t, cs_pointer_sample *o)
{
    int r = cs_inbox_claim(&t->in, t->in.epoch);
    *o = t->in.last;
    K(cs_inbox_complete(&t->in, t->in.epoch, 0) == CS_INBOX_OK);
    return r;
}

int main(void)
{
    static T t;
    static const uint32_t ep[21] = {
        E(1,0,14,0x40), E(2,0,4,0), E(3,0,9,0xC0), E(3,0,9,2), E(3,0,1,0),
        E(3,0,15,8), E(3,1,1,0), E(3,1,15,0), E(3,0,2,0), E(4,0,15,0),
        E(4,1,15,0), E(3,0,15,8), E(3,1,15,8), E(4,0,15,0), E(4,1,15,0),
        E(1,0,11,0x40), E(1,0,4,2), E(1,0,5,1), E(2,0,11,0), E(2,0,6,0),
        E(2,0,7,0) };
    static const uint32_t eo[11] = { E(2,0,13,0), E(2,0,4,0), E(3,0,0,0x10),
        E(3,1,0,0x10), E(6,0,0,0), E(5,0,0,0), E(5,1,0,0), E(6,0,0,0),
        E(5,0,0,0), E(5,1,0,0), E(2,0,13,0) };
    static const uint32_t el[2] = { E(2,0,13,0), E(2,0,4,0) };
    static const int seq[4][4] = { /* pb, live DCD A, rc, dx */
        { 0x08, 8, CS_MAC_MOTION_OK, -1 }, { 0x18, 8, CS_MAC_MOTION_EMPTY, 0 },
        { 0x18, 0, CS_MAC_MOTION_OK, -1 }, { 0x08, 0, CS_MAC_MOTION_EMPTY, 0 } };
    cs_pointer_sample o;
    uint32_t n0, i;
    int rc;

    /* refusals: no effect, no IO, no refresh */
    init(&t);
    t.in.valid = 0;
    K(prep(&t) == CS_MAC_MOTION_ARGUMENT && t.f.n == 0);
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_STATE && t.f.n == 0);

    /* alias test unchanged, then DCD enable 8 on both with readback 8 */
    init(&t);
    K(prep(&t) == CS_MAC_MOTION_OK && logeq(&t.f, 0, ep, 21));
    K(t.f.wr[0][15] == 8 && t.f.wr[1][15] == 8 && !t.f.wr[0][1] && !t.f.wr[1][1]);
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_STATE && t.f.n == 21);
    K(cs_mac_timer_poll_arm(&t.p) == CS_MAC_MOTION_OK);

    /* empty T1: IFR only */
    n0 = t.f.n;
    K(svc(&t, 0, 0) == CS_MAC_MOTION_EMPTY && t.f.n == n0 + 1);

    /* reset A then B once each, after T1 clear, before the first triple */
    n0 = t.f.n;
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_BASELINED && logeq(&t.f, n0, eo, 11));
    K(take(&t, &o) == CS_INBOX_BASELINE_ITEM);

    /* reset-to-live; a live change mid-service is held by the latch */
    t.f.live[0] = 8; t.f.rr0n = 0; t.f.flip = 1;
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_OK);
    K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM && o.dx == -1 && o.dy == 0);
    K(t.f.live[0] == 0 && t.f.latch[0] == 8);
    t.f.flip = 0;
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_OK);
    K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM && o.dx == 1);
    t.f.live[1] = 8; /* independent B latch */
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_OK);
    K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM && o.dy == 1 && o.dx == 0);
    K(t.f.latch[0] == 0 && t.f.latch[1] == 8);

    /* repeated endpoint samples: one refresh pair each, no motion */
    for (i = 0; i < 3; i++) {
        n0 = t.f.n;
        K(svc(&t, 0x40, 0) == CS_MAC_MOTION_EMPTY && cnt(&t.f, n0, 3, 0) == 2);
    }
    K(t.p.motion.samples == 3 && t.p.motion.losses == 0);

    /* overrun loss, then LOSS skip has no SCC IO and no refresh */
    K(svc(&t, 0x40, 0x40) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_TIMER_LOSS_OVERRUN);
    K(t.in.phase == CS_INBOX_LOSS);
    n0 = t.f.n;
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_EMPTY && logeq(&t.f, n0, el, 2));
    K(!t.f.mie);
    K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK);
    K(!t.f.wr[0][15] && !t.f.wr[1][15] && t.f.wr[0][9] == 2);

    /* second-group readback failures on A and on B */
    for (i = 2; i < 4; i++) {
        init(&t);
        t.f.bad15 = (uint8_t)i;
        K(prep(&t) == CS_MAC_MOTION_FAULT && t.p.motion.cause == CS_MAC_CAUSE_READBACK);
        K(t.p.motion.stage == (i == 2 ? CS_MAC_STAGE_READBACK_A : CS_MAC_STAGE_READBACK_B));
        K(t.p.motion.phase == CS_MAC_MOTION_FAILED && t.f.wr[0][9] == 2 && !t.f.mie);
        K(cs_mac_timer_poll_stop(&t.p) == CS_MAC_MOTION_OK);
    }

    /* inbox-state refusal: fault, no refresh, no sampling */
    ready(&t);
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_BASELINED);
    K(cs_inbox_fault(&t.in, t.in.epoch, 5) == CS_INBOX_FAULTED);
    n0 = t.f.n;
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_FAULT && t.p.motion.cause == CS_MAC_CAUSE_INBOX_STATE);
    K(cnt(&t.f, n0, 3, 0) == 0 && cnt(&t.f, n0, 6, 0) == 0 && !t.f.mie);

    /* literal accepted motion: controller + inbox, X Gray 0,1,3,2,0 */
    ready(&t);
    K(svc(&t, 0x40, 0) == CS_MAC_MOTION_BASELINED);
    K(take(&t, &o) == CS_INBOX_BASELINE_ITEM && o.buttons == 0);
    for (i = 0; i < 4; i++) {
        t.f.pb = (uint8_t)seq[i][0];
        t.f.live[0] = (uint8_t)seq[i][1];
        rc = svc(&t, 0x40, 0);
        K(rc == seq[i][2]);
        if (rc == CS_MAC_MOTION_OK) {
            K(take(&t, &o) == CS_INBOX_SAMPLE_ITEM);
            K(o.dx == seq[i][3] && o.dy == 0 && o.buttons == 0);
        }
    }
    K(t.p.motion.samples == 2 && t.p.motion.losses == 0 && !t.f.mie);

    /* Refresh does not weaken incoherence/overrun precedence or Gray loss. */
    ready(&t); K(svc(&t,0x40,0)==CS_MAC_MOTION_BASELINED); (void)take(&t,&o);
    t.f.pbflip=1;
    K(svc(&t,0x40,0x40)==CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    ready(&t); K(svc(&t,0x40,0)==CS_MAC_MOTION_BASELINED); (void)take(&t,&o);
    t.f.pb=0x18;t.f.live[0]=8;
    K(svc(&t,0x40,0)==CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason==CS_MAC_TIMER_LOSS_GRAY);
    ready(&t); ++t.in.epoch;n0=t.f.n;
    K(svc(&t,0x40,0)==CS_MAC_MOTION_STATE && t.f.n==n0);

    printf("timer refresh probe:%u assertions,%d failures\n", checks, fails);
    return fails;
}
