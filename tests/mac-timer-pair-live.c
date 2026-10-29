/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Host control: build with -DCLASSICK_MAC_TIMER_PAIR_PROBE=1 -DCLASSICK_MAC_TIMER_REFRESH_PROBE=1
 * -DCLASSICK_MAC_TIMER_LIVE_PAIR=1 (activity off); links inbox.c, polled-phase.c, timer-poll.c.
 * Scripted pair callback is NOT a hardware model. */
#include <string.h>
#include <stdio.h>
#include "../platform/mac128k/timer-poll.h"

#if !CLASSICK_MAC_TIMER_PAIR_PROBE || !CLASSICK_MAC_TIMER_LIVE_PAIR || !CLASSICK_MAC_TIMER_REFRESH_PROBE || CLASSICK_MAC_TIMER_ACTIVITY_PROBE
#error live pair test needs pair,live,refresh and no activity
#endif
#define K(c) do { checks++; if (!(c)) { fails++; printf("FAIL:%d:%s\n",__LINE__,#c); } } while (0)

typedef struct { uint8_t sw[2][16], ifi, acr, ll, lh, sc[6]; unsigned pairs; } F;
typedef struct { F f; cs_mac_timer_poll p; cs_window_inbox in; cs_mac_io via; cs_mac_motion_io scc; } T;
static int fails;
static unsigned checks;

static uint8_t vrd(void *c, unsigned r)
{
    F *f = c;
    if (r == 13) return f->ifi++ == 0 ? 0x40 : 0;
    return r == 6 ? f->ll : r == 7 ? f->lh : r == 11 ? f->acr : 0;
}
static void vwr(void *c, unsigned r, uint8_t v)
{
    F *f = c;
    if (r == 4) f->ll = v; else if (r == 5) f->lh = v; else if (r == 11) f->acr = v;
}
static uint8_t srd(void *c, uint32_t ch, uint32_t reg) { return ((F *)c)->sw[ch][reg & 15]; }
static void swr(void *c, uint32_t ch, uint32_t reg, uint8_t v) { ((F *)c)->sw[ch][reg & 15] = v; }
static uint8_t pbr(void *c) { (void)c; return 0; }
static void pairr(void *c, uint8_t out[6]) { F *f = c; f->pairs++; memcpy(out, f->sc, 6); }

static int svc(T *t, int p, int a, int b)
{
    uint8_t s[6] = { (uint8_t)p, (uint8_t)a, (uint8_t)b, (uint8_t)p, (uint8_t)a, (uint8_t)b };
    memcpy(t->f.sc, s, 6);
    t->f.ifi = 0;
    return cs_mac_timer_poll_service(&t->p);
}
static void take(T *t, int kind)
{
    K(cs_inbox_claim(&t->in, t->in.epoch) == kind);
    K(cs_inbox_complete(&t->in, t->in.epoch, 0) == CS_INBOX_OK);
}

int main(void)
{
    static T t;
    unsigned i, bad = 0;

    cs_inbox_begin(&t.in);
    t.via.context = &t.f; t.via.read = vrd; t.via.write = vwr;
    t.scc.cookie = &t.f; t.scc.read = srd; t.scc.write = swr; t.scc.pb = pbr; t.scc.pair = pairr;
    K(cs_mac_timer_poll_prepare(&t.p, &t.scc, &t.via, &t.in, 0, 0x102) == CS_MAC_MOTION_OK);
    K(cs_mac_timer_poll_arm(&t.p) == CS_MAC_MOTION_OK);
    K(svc(&t, 0x08, 0, 0) == CS_MAC_MOTION_BASELINED);
    take(&t, CS_INBOX_BASELINE_ITEM);
    K(svc(&t, 0x08, 8, 0) == CS_MAC_MOTION_OK);
    take(&t, CS_INBOX_SAMPLE_ITEM);

    /* Far beyond 512 services: no halt, no loss, state stays armed. */
    for (i = 0; i < 600; i++) if (svc(&t, 0x08, 8, 0) != CS_MAC_MOTION_EMPTY) bad++;
    K(!bad && t.p.motion.services == 602 && t.f.pairs == 602);
    K(t.p.motion.phase == CS_MAC_MOTION_ARMED && !t.p.motion.losses);

    /* Later sample still delivered. */
    K(svc(&t, 0x08, 0, 0) == CS_MAC_MOTION_OK);
    take(&t, CS_INBOX_SAMPLE_ITEM);
    K(t.p.motion.samples == 2);

    /* Later loss, recovery and second baseline. */
    K(svc(&t, 0x18, 8, 0) == CS_MAC_MOTION_LOST);
    K(cs_inbox_discard(&t.in, t.in.epoch) == CS_INBOX_OK);
    K(cs_inbox_resume(&t.in, t.in.epoch) == CS_INBOX_BASELINE);
    K(svc(&t, 0x08, 0, 0) == CS_MAC_MOTION_BASELINED);
    take(&t, CS_INBOX_BASELINE_ITEM);
    K(t.p.motion.baselines == 2 && t.p.motion.phase == CS_MAC_MOTION_ARMED);
    printf("timer pair live author:%u assertions,%d failures\n", checks, fails);
    return fails;
}
