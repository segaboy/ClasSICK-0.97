/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Engineering fixture, not real SCC. Build: motion.c inbox.c pointer.c, with
 * -DCLASSICK_MAC_PHASE_IDLE_PROBE=1 (or 0 for base expectations). */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/motion.h"

static int fails;
#define CHECK(c) do { if (!(c)) { fails++; printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); } } while (0)

struct fx {
    uint8_t pb[3], a[3], b[3], rr3, regs[2][16];
    int npb, nr, nw;
    uint8_t w[16][3];
};
static uint8_t rd(void *c, uint32_t ch, uint32_t reg)
{
    struct fx *f = c;
    f->nr++;
    if (reg == 3u) return f->rr3;
    if (reg == 0u) return (ch ? f->b : f->a)[f->npb ? f->npb - 1 : 0];
    return f->regs[ch][reg];
}
static void wrf(void *c, uint32_t ch, uint32_t reg, uint8_t v)
{
    struct fx *f = c;
    if (f->nw < 16) { f->w[f->nw][0] = (uint8_t)ch; f->w[f->nw][1] = (uint8_t)reg; f->w[f->nw][2] = v; }
    f->nw++;
    f->regs[ch][reg] = v;
}
static uint8_t pbf(void *c) { struct fx *f = c; f->nr++; return f->pb[f->npb++ % 3]; }

static void script(struct fx *f, const uint8_t pb[3], const uint8_t a[3],
    const uint8_t b[3], uint8_t rr3)
{
    memcpy(f->pb, pb, 3); memcpy(f->a, a, 3); memcpy(f->b, b, 3);
    f->rr3 = rr3; f->npb = f->nr = f->nw = 0;
}

static const uint8_t Z[3] = {0, 0, 0}, REL[3] = {8, 8, 8};

/* ready=0 leaves the inbox PRIMING. */
static void setup(struct cs_mac_motion *m, cs_window_inbox *in, struct fx *f, int ready)
{
    cs_mac_motion_io io;
    memset(f, 0, sizeof *f);
    io.cookie = f; io.read = rd; io.write = wrf; io.pb = pbf;
    CHECK(cs_inbox_begin(in) == CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(m, &io, in, 0u) == CS_MAC_MOTION_OK);
    CHECK(cs_mac_motion_arm(m) == CS_MAC_MOTION_OK);
    script(f, REL, Z, Z, 0);
    CHECK(cs_mac_motion_service(m) == CS_MAC_MOTION_BASELINED);
    if (ready) {
        CHECK(cs_inbox_claim(in, in->epoch) == CS_INBOX_BASELINE_ITEM);
        CHECK(cs_inbox_complete(in, in->epoch, 0) == CS_INBOX_OK);
        CHECK(in->phase == CS_INBOX_READY);
    } else CHECK(in->phase == CS_INBOX_PRIMING);
}

/* kind: 0 plain, 1 forge no phase, 2 forge quiesced. */
static int run(const uint8_t pb[3], const uint8_t a[3], const uint8_t b[3],
    uint8_t rr3, int ready, int kind, struct cs_mac_motion *bef,
    struct cs_mac_motion *aft, struct fx *f, cs_window_inbox *in,
    cs_window_inbox *inb)
{
    struct cs_mac_motion m;
    int r;
    setup(&m, in, f, ready);
    if (kind == 1) m.have_phase = 0u; /* narrow predicate test: forged state */
    if (kind == 2) m.quiesced = 1u;   /* narrow predicate test: forged state */
    script(f, pb, a, b, rr3);
    memcpy(bef, &m, sizeof m); memcpy(inb, in, sizeof *in);
    r = cs_mac_motion_service(&m);
    memcpy(aft, &m, sizeof m);
    return r;
}

static void expect_lost(int r, const struct cs_mac_motion *a, uint32_t reason)
{
    CHECK(r == CS_MAC_MOTION_LOST);
    CHECK(a->losses == 1u);
    CHECK(a->first_loss.reason == reason);
}

int main(void)
{
    struct cs_mac_motion bef, aft, exp;
    struct fx f;
    cs_window_inbox in, inb;
    int c, i, r, varying = 0, stable = 0;

    for (c = 0; c < 64; c++) {
        uint8_t pb[3];
        int var;
        for (i = 0; i < 3; i++) pb[i] = (uint8_t)(8u | ((((unsigned)c >> (2u * (unsigned)i)) & 3u) << 4));
        var = !(pb[0] == pb[1] && pb[1] == pb[2]);
        r = run(pb, Z, Z, 0, 1, 0, &bef, &aft, &f, &in, &inb);
        varying += var; stable += !var;
        if (!var || CLASSICK_MAC_PHASE_IDLE_PROBE) {
            CHECK(r == CS_MAC_MOTION_EMPTY);
            memcpy(&exp, &bef, sizeof exp); exp.services++;
            CHECK(memcmp(&exp, &aft, sizeof exp) == 0);
            CHECK(memcmp(&inb, &in, sizeof in) == 0);
            CHECK(f.nw == 1 && f.w[0][0] == 0 && f.w[0][1] == 0 && f.w[0][2] == 0x38);
            CHECK(in.count == 0u && aft.samples == 0u && aft.losses == 0u);
            CHECK(aft.first_loss.reason == CS_MAC_LOSS_NONE);
        } else {
            expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
            CHECK(f.nw == 2 && f.w[1][1] == 9u && f.w[1][2] == 0x02);
        }
        CHECK(f.nr == 10); /* RR3, 3 PB, 3 A0 and 3 B0 are 10 reads */
    }
    CHECK(varying == 60 && stable == 4);

    {   /* One DCD discrepancy per sample and channel, PB varying: base LOST. */
        static const uint8_t pbv[3] = {8, 0x18, 0x28};
        for (i = 0; i < 6; i++) {
            uint8_t a[3] = {0, 0, 0}, b[3] = {0, 0, 0};
            (i < 3 ? a : b)[i % 3] = 8;
            r = run(pbv, a, b, 0, 1, 0, &bef, &aft, &f, &in, &inb);
            expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
        }
    }
    for (i = 0; i < 3; i++) { /* button discrepancy */
        uint8_t pb[3] = {8, 8, 8};
        pb[i] = 0;
        r = run(pb, Z, Z, 0, 1, 0, &bef, &aft, &f, &in, &inb);
        expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
    }
    {
        static const uint8_t pbv[3] = {8, 0x18, 0x28};
        r = run(pbv, Z, Z, 8, 1, 0, &bef, &aft, &f, &in, &inb); /* RR3 pending */
        expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
        r = run(pbv, Z, Z, 0, 0, 0, &bef, &aft, &f, &in, &inb); /* PRIMING */
        expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
        r = run(pbv, Z, Z, 0, 1, 1, &bef, &aft, &f, &in, &inb); /* missing phase */
        expect_lost(r, &aft, CS_MAC_LOSS_NO_PHASE);
        r = run(pbv, Z, Z, 0, 1, 2, &bef, &aft, &f, &in, &inb); /* quiesced */
        CHECK(r == CS_MAC_MOTION_LOST);
        CHECK(aft.first_loss.reason == CS_MAC_LOSS_UNSTABLE);
    }

    {   /* First-loss DCD regression: retained-style X-bit change stays LOST. */
        static const uint8_t pbv[3] = {8, 0x18, 0x18};
        static const uint8_t a[3] = {0, 0, 8};
        r = run(pbv, a, Z, 0, 1, 0, &bef, &aft, &f, &in, &inb);
        expect_lost(r, &aft, CS_MAC_LOSS_UNSTABLE);
        CHECK(aft.first_loss.snapshots[0] == (8u << 16));
        CHECK(aft.first_loss.snapshots[2] == ((0x18u << 16) | (8u << 8)));
        CHECK(aft.first_loss.previous == (1u << 24));
    }
    /* Proposal, not executed: -DCLASSICK_MAC_PHASE_IDLE_PROBE=2 must hit #error. */
    printf(fails ? "FAILED %d\n" : "PASS\n", fails);
    return fails != 0;
}
