/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include <stdio.h>
#include <string.h>
#include "platform/mac128k/timer-poll.h"
#include "apps/window-scene/inbox.h"
#ifndef CLASSICK_MAC_TIMER_PHASE_WITNESS
#define CLASSICK_MAC_TIMER_PHASE_WITNESS 0
#endif
#define W CLASSICK_MAC_TIMER_PHASE_WITNESS

typedef struct { uint8_t pb, a, b; } Tri;
typedef struct { uint8_t r[2][16], pend[6]; } Scc;
typedef struct { uint8_t r[16]; int flag; } Via;
typedef struct {
    Scc scc; Via via; cs_window_inbox ib; cs_mac_timer_poll p;
    cs_mac_motion_io io; cs_mac_io vio;
} Rig;

static int fails;
static unsigned checks;
static void check(const char *n, int ok, uint32_t got, uint32_t want)
{
    checks++;
    if (!ok) { fails++; printf("FAIL %s got=%08X want=%08X\n", n, (unsigned)got, (unsigned)want); }
}
#define EQ(n, g, w) do { uint32_t eq_actual = (uint32_t)(g); uint32_t eq_want = (uint32_t)(w); check(n, eq_actual == eq_want, eq_actual, eq_want); } while (0)

static uint8_t sread(void *c, uint32_t ch, uint32_t reg) { return ((Scc *)c)->r[ch & 1][reg & 15]; }
static void swrite(void *c, uint32_t ch, uint32_t reg, uint8_t v) { ((Scc *)c)->r[ch & 1][reg & 15] = v; }
static uint8_t spb(void *c) { (void)c; return 0; }
static void spair(void *c, uint8_t out[6]) { memcpy(out, ((Scc *)c)->pend, 6); }
static uint8_t vread(void *c, unsigned reg)
{
    Via *v = c;
    if (reg == 13) { uint8_t f = v->flag ? 0x40 : 0; v->flag = 0; return f; }
    return v->r[reg & 15];
}
static void vwrite(void *c, unsigned reg, uint8_t val)
{
    Via *v = c;
    v->r[reg & 15] = val;
    if (reg == 4) v->r[6] = val;
    if (reg == 5) v->r[7] = val;
}

/* Prepare and arm through public APIs only. */
static void rig(Rig *r)
{
    memset(r, 0, sizeof *r);
    r->io.cookie = &r->scc; r->io.read = sread; r->io.write = swrite;
    r->io.pb = spb; r->io.pair = spair;
    r->vio.context = &r->via; r->vio.read = vread; r->vio.write = vwrite;
    EQ("inbox begin", cs_inbox_begin(&r->ib), CS_INBOX_OK);
    EQ("prepare", cs_mac_timer_poll_prepare(&r->p, &r->io, &r->vio, &r->ib, 0, 1024), CS_MAC_MOTION_OK);
    EQ("arm", cs_mac_timer_poll_arm(&r->p), CS_MAC_MOTION_OK);
}
static int svc(Rig *r, Tri a, Tri b)
{
    r->scc.pend[0] = a.pb; r->scc.pend[1] = a.a; r->scc.pend[2] = a.b;
    r->scc.pend[3] = b.pb; r->scc.pend[4] = b.a; r->scc.pend[5] = b.b;
    r->via.flag = 1;
    return cs_mac_timer_poll_service(&r->p);
}
static void baseline(Rig *r, Tri t)
{
    EQ("baseline service", svc(r, t, t), CS_MAC_MOTION_BASELINED);
    EQ("claim baseline", cs_inbox_claim(&r->ib, r->ib.epoch), CS_INBOX_BASELINE_ITEM);
    EQ("complete baseline", cs_inbox_complete(&r->ib, r->ib.epoch, 0), CS_INBOX_OK);
}

/* Frozen SPEC-0074 engineering assignments; never query a decoder for inputs. */
static Tri phase_triple(unsigned x, unsigned y, unsigned button)
{
    Tri t;
    t.pb = (uint8_t)((button ? 0u : 8u) | ((x & 2u) ? 16u : 0u) | ((y & 2u) ? 32u : 0u));
    t.a = (uint8_t)((x & 1u) << 3); t.b = (uint8_t)((y & 1u) << 3);
    return t;
}
static const uint32_t lit[16] = {
    0x81000000, 0x85000100, 0x89000000, 0x8D000100,
    0x91010000, 0x95010100, 0x99010000, 0x9D010100,
    0xA1000000, 0xA5000100, 0xA9000000, 0xAD000100,
    0xB1010000, 0xB5010100, 0xB9010000, 0xBD010100 };

static uint32_t expect(uint32_t l, uint32_t btn)
{
    return (W ? l : (l & 0x01010101u)) | btn;
}

static void gray_case(unsigned btn, unsigned x, unsigned y)
{
    Rig *r = &(Rig){0}; Tri t = phase_triple(x, y, btn), j = phase_triple(x ^ 3u, y, btn); char n[48];
    struct cs_mac_motion_loss keep;
    snprintf(n, sizeof n, "b%u x%u y%u", btn, x, y);
    rig(r);
    baseline(r, t);
    EQ("no loss reason", r->p.motion.first_loss.reason, 0);
    EQ("no loss word0", r->p.motion.first_loss.previous, 0);
    EQ("gray service", svc(r, j, j), CS_MAC_MOTION_LOST);
    EQ(n, r->p.motion.first_loss.previous, expect(lit[x * 4 + y], btn));
    EQ("gray reason", r->p.motion.first_loss.reason, CS_MAC_TIMER_LOSS_GRAY);
    if (W) EQ("mask", r->p.motion.first_loss.previous & ~0xBD010101u, 0);
    keep = r->p.motion.first_loss;
    EQ("discard", cs_inbox_discard(&r->ib, r->ib.epoch), CS_INBOX_OK);
    EQ("resume", cs_inbox_resume(&r->ib, r->ib.epoch), CS_INBOX_BASELINE);
    baseline(r, phase_triple(0u, 0u, 0u));
    EQ("second loss", svc(r, phase_triple(0u, 0u, 0u), phase_triple(1u, 0u, 0u)), CS_MAC_MOTION_LOST);
    EQ("second loss counted", r->p.motion.losses, 2u);
    check("first loss immutable", memcmp(&keep, &r->p.motion.first_loss, sizeof keep) == 0, 0, 0);
}

int main(void)
{
    Rig *r = &(Rig){0}; Tri t, n2, u = { 0, 0, 0 }, v = { 0x38, 0, 0 };
    for (unsigned b = 0; b < 2u; b++)
        for (unsigned x = 0; x < 4u; x++)
            for (unsigned y = 0; y < 4u; y++) gray_case(b, x, y);
    rig(r);
    svc(r, u, v);
    EQ("unknown reason", r->p.motion.first_loss.reason, CS_MAC_LOSS_UNSTABLE);
    EQ("unknown previous", r->p.motion.first_loss.previous, W ? 0x80000000u : 0);
    {
        t = phase_triple(1u, 1u, 0u);
        rig(r);
        baseline(r, t);
        for (int i = 0; i < CS_INBOX_CAPACITY; i++)
            EQ("fill", cs_inbox_push(&r->ib, r->ib.epoch, 0, 0, 0), CS_INBOX_OK);
        n2 = phase_triple(0u, 1u, 0u);
        {
            EQ("overflow service", svc(r, n2, n2), CS_MAC_MOTION_LOST);
            EQ("overflow reason", r->p.motion.first_loss.reason, CS_MAC_LOSS_OVERFLOW);
            EQ("overflow predecessor", r->p.motion.first_loss.previous, expect(lit[5], 0));
        }
    }
    printf("witness=%d checks=%u %s\n", W, checks, fails ? "FAILED" : "ok");
    return fails ? 1 : 0;
}
