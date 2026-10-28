/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Build with -DCLASSICK_MAC_TIMER_PAIR_PROBE=1; links inbox.c, polled-phase.c,
 * timer-poll.c. Scripted raw responses are NOT a hardware model. */
#include <string.h>
#include <stdio.h>
#include "../platform/mac128k/timer-poll.h"

#if !CLASSICK_MAC_TIMER_PAIR_PROBE
#error pair test requires CLASSICK_MAC_TIMER_PAIR_PROBE=1
#endif
#define K(c) do { checks++; if (!(c)) { fails++; printf("FAIL:%d:%s\n",__LINE__,#c); } } while (0)

typedef struct {
    uint8_t sw[2][16], ifq[2], ifi, acr, ll, lh, sc[6];
    unsigned n, pairs, rdn, pbn;
} F;
typedef struct {
    F f;
    cs_mac_timer_poll p;
    cs_window_inbox in;
    cs_mac_io via;
    cs_mac_motion_io scc;
} T;

static int fails;
static unsigned checks;

static uint8_t vrd(void *c, unsigned r)
{
    F *f = c;
    f->n++;
    if (r == 13) return f->ifi < 2 ? f->ifq[f->ifi++] : 0;
    if (r == 6) return f->ll;
    if (r == 7) return f->lh;
    if (r == 11) return f->acr;
    return 0;
}
static void vwr(void *c, unsigned r, uint8_t v)
{
    F *f = c;
    f->n++;
    if (r == 4) f->ll = v;
    else if (r == 5) f->lh = v;
    else if (r == 11) f->acr = v;
}
static uint8_t srd(void *c, uint32_t ch, uint32_t reg)
{
    F *f = c;
    f->n++;
    if (reg == 0) f->rdn++;
    return f->sw[ch][reg & 15];
}
static void swr(void *c, uint32_t ch, uint32_t reg, uint8_t v)
{
    F *f = c;
    f->n++;
    f->sw[ch][reg & 15] = v;
}
static uint8_t pbr(void *c)
{
    ((F *)c)->pbn++;
    return 0;
}
static void pairr(void *c, uint8_t out[6])
{
    F *f = c;
    f->pairs++;
    memcpy(out, f->sc, 6);
}

static void init(T *t)
{
    memset(t, 0, sizeof *t);
    cs_inbox_begin(&t->in);
    t->via.context = &t->f; t->via.read = vrd; t->via.write = vwr;
    t->scc.cookie = &t->f; t->scc.read = srd; t->scc.write = swr;
    t->scc.pb = pbr; t->scc.pair = pairr;
}
static void ready(T *t)
{
    init(t);
    K(cs_mac_timer_poll_prepare(&t->p, &t->scc, &t->via, &t->in, 0, 0x102) == CS_MAC_MOTION_OK);
    K(cs_mac_timer_poll_arm(&t->p) == CS_MAC_MOTION_OK);
}
static int svc(T *t, int i2, int p1, int a1, int b1, int p2, int a2, int b2)
{
    F *f = &t->f;
    uint8_t s[6] = { (uint8_t)p1, (uint8_t)a1, (uint8_t)b1, (uint8_t)p2, (uint8_t)a2, (uint8_t)b2 };
    memcpy(f->sc, s, 6);
    f->ifq[0] = 0x40; f->ifq[1] = (uint8_t)i2; f->ifi = 0;
    f->pairs = f->pbn = f->rdn = 0;
    return cs_mac_timer_poll_service(&t->p);
}
static int base(T *t) { return svc(t, 0, 0x08, 0, 0, 0x08, 0, 0); }

int main(void)
{
    static T t, before;
    cs_pointer_sample o;
    unsigned n;

    /* missing callback: refused before any IO or effect */
    init(&t);
    t.scc.pair = 0;
    t.p.motion.phase = 0x77;
    memcpy(&before,&t,sizeof t);
    K(cs_mac_timer_poll_prepare(&t.p, &t.scc, &t.via, &t.in, 0, 0x102) == CS_MAC_MOTION_ARGUMENT);
    K(t.f.n == 0 && t.p.motion.phase == 0x77);
    K(!memcmp(&before,&t,sizeof t));

    /* One scripted pair supplier;native IO order is verified separately. */
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(t.f.pairs == 1 && !t.f.rdn && !t.f.pbn);
    K(t.in.phase == CS_INBOX_PRIMING && t.p.motion.baselines == 1);
    K(cs_inbox_claim(&t.in, t.in.epoch) == CS_INBOX_BASELINE_ITEM);
    K(cs_inbox_complete(&t.in, t.in.epoch, 0) == CS_INBOX_OK);

    /* sample */
    K(svc(&t, 0, 0x08, 8, 0, 0x08, 8, 0) == CS_MAC_MOTION_OK);
    K(t.f.pairs == 1 && !t.f.rdn && !t.f.pbn);
    K(cs_inbox_claim(&t.in, t.in.epoch) == CS_INBOX_SAMPLE_ITEM);
    o = t.in.last;
    K(o.dx == -1 && o.dy == 0 && t.p.motion.samples == 1);
    K(cs_inbox_complete(&t.in, t.in.epoch, 0) == CS_INBOX_OK);

    /* Gray loss */
    K(svc(&t, 0, 0x18, 0, 0, 0x18, 0, 0) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_TIMER_LOSS_GRAY);
    K(t.p.motion.first_loss.snapshots[0] == (0x18u << 16));
    K(t.in.phase == CS_INBOX_LOSS);
    n = t.f.n;
    K(svc(&t, 0, 0, 0, 0, 0, 0, 0) == CS_MAC_MOTION_EMPTY && t.f.pairs == 0 && t.f.n == n + 2u);

    /* unstable, overrun, and precedence of unstable over overrun */
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(svc(&t, 0, 0x08, 0, 0, 0x28, 0, 0) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_LOSS_UNSTABLE);
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(svc(&t, 0x40, 0x08, 0, 0, 0x08, 0, 0) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_TIMER_LOSS_OVERRUN);
    ready(&t);
    K(base(&t) == CS_MAC_MOTION_BASELINED);
    K(svc(&t, 0x40, 0x08, 0, 0, 0x28, 0, 0) == CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.reason == CS_MAC_LOSS_UNSTABLE);
    K(t.f.pairs == 1 && !t.f.rdn && !t.f.pbn);

    /* Distinct captured tuple values pin all six boundary positions. */
    ready(&t); K(base(&t)==CS_MAC_MOTION_BASELINED);
    K(svc(&t,0,0x9F,0x4C,0x44,0x8F,0x44,0x44)==CS_MAC_MOTION_LOST);
    K(t.p.motion.first_loss.snapshots[0]==UINT32_C(0x009F4C44));
    K(t.p.motion.first_loss.snapshots[1]==UINT32_C(0x008F4444));
    printf("timer pair author:%u assertions,%d failures\n", checks, fails);
    return fails;
}
