/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original conditional-DCD-latch fixture for cs_mac_motion_*. Engineering model of the
 * admitted interface facts only; not Snow's part, hardware timing or an ISR recipe. */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/motion.h"
#if CLASSICK_MAC_PENDING_DEFER_PROBE
#error "mac-motion-latch covers the base core only; probe cases stay separate"
#endif

static unsigned g_checks;
#define CHECK(c) do{g_checks++;if(!(c)){printf("FAIL line %d: %s\n",__LINE__,#c);return 1;}}while(0)
#define RUN(x) do{if(x)return 1;}while(0)
enum { PBB = 0x08, PBX = 0x10, PBY = 0x20 };

typedef struct { int live, latched, closed; uint8_t wr1, wr15; unsigned resets, ius; } ch_t;
typedef struct {
    ch_t c[2];
    uint8_t wr9, pb, rr3_or;
    unsigned reads, writes, pbs, last_reg, last_v;
    int last_ch;
    unsigned hook_at;          /* inject on this pb() call number */
    uint8_t hook_xor;
    int hook_ch, hook_val;     /* optional raw DCD change with the pb hook */
    int pre_ch, pre_val;       /* raw DCD change just before that channel's ResetExt */
} fx_t;

static void fx_init(fx_t *f, uint8_t pb)
{
    memset(f, 0, sizeof *f);
    f->pb = pb; f->hook_ch = -1; f->pre_val = -1;
}
/* WR15 controls DCD latching;WR1 gates its external interrupt indication.
 * Engineering subset: disabled sources retain state but expose no pending. */
static int en(const ch_t *c) { return (c->wr15 & 8) != 0; }
static int pend(const ch_t *c) { return c->closed && en(c) && (c->wr1 & 1); }
static void set(fx_t *f, int ch, int v)
{
    ch_t *c = &f->c[ch];
    if (c->live == v) return;
    if (en(c) && !c->closed) { c->closed = 1; c->latched = v; }
    c->live = v;            /* closed: live moves, no event queue */
}
static uint8_t rr0(const fx_t *f, int ch)
{
    const ch_t *c = &f->c[ch];
    return (uint8_t)(((c->closed && en(c)) ? c->latched : c->live) ? 0x08 : 0);
}
static uint8_t rr3(const fx_t *f)
{
    return (uint8_t)((pend(&f->c[0]) ? 8 : 0) | (pend(&f->c[1]) ? 1 : 0) | f->rr3_or);
}
static uint8_t ior(void *k, uint32_t ch, uint32_t reg)
{
    fx_t *f = k;
    f->reads++;
    if (reg == 0) return rr0(f, (int)ch);
    if (reg == 3) return ch == 0 ? rr3(f) : 0;   /* aggregate visible via channel A only */
    if (reg == 15) return f->c[ch].wr15;
    return 0;
}
static void iow(void *k, uint32_t chn, uint32_t reg, uint8_t v)
{
    fx_t *f = k;
    ch_t *c = &f->c[chn];
    int i;
    f->writes++; f->last_ch = (int)chn; f->last_reg = reg; f->last_v = v;
    if (reg == 9) {
        if ((v & 0xC0) == 0xC0) {   /* reset: mock regs/latches only */
            for (i = 0; i < 2; i++) {
                f->c[i].wr1 = f->c[i].wr15 = 0;
                f->c[i].closed = 0; f->c[i].latched = f->c[i].live;
            }
            f->wr9 = 0;
        } else f->wr9 = v;      /* master bit gates availability only */
    } else if (reg == 0 && v == 0x10) {
        if (f->pre_val >= 0 && (int)chn == f->pre_ch) {
            set(f, (int)chn, f->pre_val); f->pre_val = -1;
        }
        if (c->closed) {
            if (c->live != c->latched) c->latched = c->live; /* keep/relatch */
            else c->closed = 0;
        }
        c->resets++;
    } else if (reg == 0 && v == 0x38) c->ius++;   /* tracked separately */
    else if (reg == 1) c->wr1 = v;
    else if (reg == 15) c->wr15 = v;
}
static uint8_t iopb(void *k)
{
    fx_t *f = k;
    f->pbs++;
    if (f->hook_at && f->pbs == f->hook_at) {
        f->hook_at = 0; f->pb ^= f->hook_xor;
        if (f->hook_ch >= 0) set(f, f->hook_ch, f->hook_val);
    }
    return f->pb;
}
static unsigned iotot(const fx_t *f) { return f->reads + f->writes + f->pbs; }
static void both(fx_t *f)
{
    int i;
    for (i = 0; i < 2; i++) { f->c[i].wr1 = 1; f->c[i].wr15 = 8; }
}

/* ---- fixture self-tests (model checks only, no hardware claim) ---- */
static int self(void)
{
    fx_t f;
    fx_init(&f, PBB); both(&f);
    CHECK(rr0(&f, 0) == 0 && rr3(&f) == 0);
    set(&f, 0, 1);                       /* enabled live edge closes/latches */
    CHECK(f.c[0].closed && f.c[0].latched == 1 && rr0(&f, 0) == 8 && rr3(&f) == 8);
    set(&f, 0, 0);                       /* opposite transition: RR0 keeps old value */
    CHECK(f.c[0].live == 0 && f.c[0].closed && rr0(&f, 0) == 8);
    iow(&f, 0, 0, 0x10);                 /* differing raw: relatch, still pending */
    CHECK(f.c[0].closed && f.c[0].latched == 0 && rr0(&f, 0) == 0 && rr3(&f) == 8);
    iow(&f, 0, 0, 0x10);                 /* unchanged raw: clear */
    CHECK(!f.c[0].closed && rr3(&f) == 0 && f.c[0].resets == 2);
    set(&f, 0, 1); set(&f, 0, 0); set(&f, 0, 1);   /* returns to the latched value */
    CHECK(f.c[0].closed && f.c[0].live == f.c[0].latched);
    iow(&f, 0, 0, 0x10);
    CHECK(!f.c[0].closed && rr3(&f) == 0);
    set(&f, 1, 1); set(&f, 0, 0);        /* IUS reset never clears external pending */
    CHECK(rr3(&f) == 9);
    iow(&f, 0, 0, 0x38); iow(&f, 1, 0, 0x38);
    CHECK(rr3(&f) == 9 && f.c[0].ius == 1 && f.c[1].ius == 1
        && f.c[0].resets == 3 && f.c[1].resets == 0);
    f.c[0].wr15 = 0;                     /* channel masks are separate */
    CHECK(rr3(&f) == 1 && ior(&f, 1, 3) == 0 && ior(&f, 0, 3) == 1);
    f.c[0].wr15 = 8; f.c[1].wr1 = 0;
    CHECK(rr3(&f) == 8);
    f.c[1].wr1 = 1;
    iow(&f, 0, 9, 0x02);                 /* master off does not erase latches */
    CHECK(rr3(&f) == 9 && f.c[0].closed && f.c[1].closed);
    iow(&f, 0, 9, 0xC0);                 /* WR9 reset clears mock regs/latches only */
    CHECK(f.wr9 == 0 && f.c[0].wr1 == 0 && f.c[1].wr15 == 0 && rr3(&f) == 0
        && !f.c[0].closed && f.c[0].resets == 3);
    {fx_t g;fx_init(&g,PBB);g.c[0].wr15=8; /* WR1=0: latch still works */
     set(&g,0,1);CHECK(g.c[0].closed && rr0(&g,0)==8 && rr3(&g)==0);
     set(&g,0,0);CHECK(rr0(&g,0)==8 && rr3(&g)==0);
     iow(&g,0,0,0x10);CHECK(g.c[0].closed && rr0(&g,0)==0 && rr3(&g)==0);
     iow(&g,0,0,0x10);CHECK(!g.c[0].closed && rr3(&g)==0);}
    return 0;
}

/* ---- controller harness ---- */
typedef struct { fx_t f; cs_window_inbox in; struct cs_mac_motion m; } T;

static int setup(T *t)
{
    cs_mac_motion_io io;
    memset(t, 0, sizeof *t);
    fx_init(&t->f, PBB);
    io.cookie = &t->f; io.read = ior; io.write = iow; io.pb = iopb;
    CHECK(cs_inbox_begin(&t->in) == CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&t->m, &io, &t->in, 0u) == CS_MAC_MOTION_OK);
    CHECK(t->f.wr9 == 0x02 && t->f.c[0].wr15 == 8 && t->f.c[1].wr15 == 8);
    CHECK(cs_mac_motion_arm(&t->m) == CS_MAC_MOTION_OK && t->f.wr9 == 0x0A);
    CHECK(cs_mac_motion_service(&t->m) == CS_MAC_MOTION_BASELINED);
    CHECK(cs_inbox_claim(&t->in, t->m.epoch) == CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&t->in, t->m.epoch, 0) == CS_INBOX_OK);
    CHECK(t->in.phase == CS_INBOX_READY && t->in.count == 0u);
    t->f.pbs = 0;
    return 0;
}
static int drain(T *t, int dx, int dy)
{
    CHECK(cs_inbox_claim(&t->in, t->m.epoch) == CS_INBOX_SAMPLE_ITEM);
    CHECK(t->in.last.dx == dx && t->in.last.dy == dy && t->in.last.buttons == 0u);
    CHECK(cs_inbox_complete(&t->in, t->m.epoch, 0) == CS_INBOX_OK);
    CHECK(cs_inbox_claim(&t->in, t->m.epoch) == CS_INBOX_EMPTY);
    return 0;
}

/* 1: frozen direction table (invert 0): raw DCD, PB bits, expected dx/dy. */
static int scen1(void)
{
    static const struct { int ch, raw; uint8_t pb; int dx, dy; } K[8] = {
        {0, 1, PBB | PBX, 1, 0}, {1, 1, PBB, 0, 1},
        {0, 0, PBB | PBX, -1, 0}, {1, 0, PBB | PBY, 0, 1},
        {0, 1, PBB, -1, 0}, {1, 1, PBB | PBY, 0, -1},
        {0, 0, PBB, 1, 0}, {1, 0, PBB, 0, -1} };
    T t;
    unsigned i;
    RUN(setup(&t));
    for (i = 0; i < 8; i++) {
        t.f.pb = K[i].pb;
        set(&t.f, K[i].ch, K[i].raw);
        CHECK(t.f.c[K[i].ch].closed && rr3(&t.f) == (K[i].ch ? 1 : 8));
        CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_OK);
        CHECK(t.in.count == 1u && t.m.samples == i + 1u && !t.f.c[K[i].ch].closed);
        RUN(drain(&t, K[i].dx, K[i].dy));
        CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_EMPTY);
        CHECK(t.in.count == 0u && t.m.samples == i + 1u && t.m.losses == 0u);
    }
    return 0;
}

static int expect_unstable(T *t, uint32_t s0, uint32_t s1, uint32_t s2, uint32_t rr3v)
{
    struct cs_mac_motion_loss *l = &t->m.first_loss;
    CHECK(cs_mac_motion_service(&t->m) == CS_MAC_MOTION_LOST);
    CHECK(l->reason == CS_MAC_LOSS_UNSTABLE && l->rr3 == rr3v);
    CHECK(l->snapshots[0] == s0 && l->snapshots[1] == s1 && l->snapshots[2] == s2);
    CHECK(l->previous == 0x01000000u && l->inbox_phase == CS_INBOX_READY);
    CHECK(t->in.phase == CS_INBOX_LOSS && t->in.loss_reason == CS_INBOX_UNCERTAIN);
    CHECK(t->m.losses == 1u && t->in.losses == 1u && t->m.samples == 0u);
    CHECK(t->in.count == 0u && t->m.have_phase == 0u);
    CHECK(t->m.quiesced == 1u && t->f.wr9 == 0x02);
    return 0;
}

/* 2: strict UNSTABLE; possibly conservative, recorded as-is, not a bug claim. */
static int scen2(void)
{
    T t;
    RUN(setup(&t));                      /* PB-only change on the second pb() call */
    t.f.hook_at = 2; t.f.hook_xor = PBX;
    RUN(expect_unstable(&t, 0x00080000u, 0x00180000u, 0x00180000u, 0));
    CHECK(t.f.c[0].resets == 0 && t.f.c[1].resets == 0 && rr3(&t.f) == 0);
    RUN(setup(&t));                      /* new DCD latch between pre-ack snapshots */
    t.f.hook_at = 2; t.f.hook_ch = 0; t.f.hook_val = 1;
    RUN(expect_unstable(&t, 0x00080000u, 0x00080800u, 0x00080800u, 0));
    /* RR3 was read before the edge: no ResetExt, so the closed latch stays closed. */
    CHECK(t.f.c[0].resets == 0 && t.f.c[0].closed && rr3(&t.f) == 8);
    return 0;
}

/* 3: relatch on ResetExt makes the third snapshot differ. */
static int scen3(void)
{
    T t;
    struct cs_mac_motion_loss keep;
    RUN(setup(&t));
    set(&t.f, 0, 1);                     /* latched A event already present */
    t.f.pre_ch = 0; t.f.pre_val = 0;     /* raw A changes just before its ResetExt */
    RUN(expect_unstable(&t, 0x00080800u, 0x00080800u, 0x00080000u, 8));
    CHECK(t.f.c[0].resets == 1 && t.f.c[0].ius == 2 && t.f.pre_val == -1);
    CHECK(t.f.c[0].closed && t.f.c[0].latched == 0 && rr3(&t.f) == 8);
    keep = t.m.first_loss;
    /* Service of the sticky LOSS creates no event; per production flow it also
     * acknowledges the retained pending (equal raw -> cleared). */
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_LOST);
    CHECK(t.m.losses == 1u && t.in.losses == 1u && t.m.samples == 0u && t.in.count == 0u);
    CHECK(t.in.phase == CS_INBOX_LOSS && !memcmp(&keep, &t.m.first_loss, sizeof keep));
    CHECK(!t.f.c[0].closed && t.m.quiesced == 1u);
    set(&t.f, 0, 1);                     /* engineering step: fresh pending for recovery */
    CHECK(t.f.c[0].closed);
    /* Foreground cancellation is simulated as already done; no renderer/owner call. */
    CHECK(cs_inbox_discard(&t.in, t.m.epoch) == CS_INBOX_OK);
    CHECK(cs_inbox_resume(&t.in, t.m.epoch) == CS_INBOX_BASELINE);
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_BASELINED);
    CHECK(!t.f.c[0].closed && t.m.quiesced == 0u && t.m.baselines == 2u);
    CHECK(t.f.last_reg == 9u && t.f.last_v == 0x0Au && t.f.last_ch == 0 && t.f.wr9 == 0x0A);
    CHECK(!memcmp(&keep, &t.m.first_loss, sizeof keep));
    CHECK(cs_inbox_claim(&t.in, t.m.epoch) == CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&t.in, t.m.epoch, 0) == CS_INBOX_OK);
    set(&t.f, 0, 0);                     /* next stable valid edge: raw 0, PB4 0 -> +1 */
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_OK && t.m.samples == 1u);
    RUN(drain(&t, 1, 0));
    CHECK(!memcmp(&keep, &t.m.first_loss, sizeof keep) && t.m.losses == 1u);
    return 0;
}

/* 4: IUS-only, stop, and hard-fault precedence. */
static int scen4(void)
{
    T t;
    unsigned n;
    RUN(setup(&t));
    set(&t.f, 0, 1);
    iow(&t.f, 0, 0, 0x38); iow(&t.f, 1, 0, 0x38);   /* ResetIUS only */
    CHECK(rr3(&t.f) == 8 && t.f.c[0].closed && t.f.c[0].resets == 0);
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_OK && t.in.count == 1u);
    CHECK(t.f.c[0].resets == 1 && !t.f.c[0].closed);
    RUN(drain(&t, -1, 0)); /* raw rising,X2 low: left;frozen direction rule */

    RUN(setup(&t));
    CHECK(cs_mac_motion_stop(&t.m) == CS_MAC_MOTION_OK && t.m.phase == CS_MAC_MOTION_STOPPED);
    CHECK(t.f.wr9 == 0x02 && t.f.c[0].wr1 == 0 && t.f.c[0].wr15 == 0
        && t.f.c[1].wr1 == 0 && t.f.c[1].wr15 == 0);
    n = iotot(&t.f);
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_STATE && iotot(&t.f) == n);
    CHECK(cs_mac_motion_stop(&t.m) == CS_MAC_MOTION_OK && iotot(&t.f) == n);
    CHECK(t.in.phase == CS_INBOX_READY);   /* stop does not stop the inbox */

    RUN(setup(&t));                       /* unsupported RR3 bit: hard fault, not loss */
    set(&t.f, 0, 1);
    t.f.rr3_or = 0x20;
    CHECK(cs_mac_motion_service(&t.m) == CS_MAC_MOTION_FAULT);
    CHECK(t.m.cause == CS_MAC_CAUSE_PENDING && t.m.stage == CS_MAC_STAGE_PENDING);
    CHECK(t.m.phase == CS_MAC_MOTION_FAILED && t.f.wr9 == 0x02);
    CHECK(t.f.c[0].resets == 0 && t.f.c[1].resets == 0 && t.f.c[0].closed);
    CHECK(t.in.phase == CS_INBOX_READY && t.in.losses == 0u && t.m.samples == 0u);
    return 0;
}

int main(void)
{
    if (self() || scen1() || scen2() || scen3() || scen4()) {
        printf("mac-motion-latch: FAILED after %u checks\n", g_checks);
        return 1;
    }
    printf("mac-motion-latch: %u checks passed\n", g_checks);
    return 0;
}
