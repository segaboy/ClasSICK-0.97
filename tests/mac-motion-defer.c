/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Standalone host test; link with platform/mac128k/motion.c and the inbox
 * implementation. Build with -DCLASSICK_MAC_PENDING_DEFER_PROBE=1 for defer tests. */
#include <stdio.h>
#include <string.h>
#include "../platform/mac128k/motion.h"

static int bad;
#define CK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); bad++; } } while (0)

typedef struct { uint8_t rr3, pb[3], a[3], b[3]; int ip, ia, ib, n; char log[48]; } fk;
static struct cs_mac_motion m;
static cs_window_inbox in;
static fk f;

static void add(fk *k, char c) { if (k->n < 47) k->log[k->n++] = c; }
static uint8_t rd(void *c, uint32_t ch, uint32_t reg)
{
    fk *k = c;
    if (reg == 15u) return ch == CS_MAC_SCC_A ? 8 : 0;
    if (reg == 3u) { add(k, '3'); return k->rr3; }
    if (ch == CS_MAC_SCC_A) { add(k, 'A'); return k->a[k->ia++ % 3]; }
    add(k, 'B');
    return k->b[k->ib++ % 3];
}
static void wrt(void *c, uint32_t ch, uint32_t reg, uint8_t v)
{
    (void)ch; (void)reg; (void)v;
    add(c, 'W');
}
static uint8_t pbr(void *c) { fk *k = c; add(k, 'P'); return k->pb[k->ip++ % 3]; }

static const uint8_t U[3] = {8, 8, 8}, Z[3] = {0, 0, 0}, H[3] = {8, 8, 8};
static const uint8_t E01[3] = {0, 8, 8};
#if CLASSICK_MAC_PENDING_DEFER_PROBE
static const uint8_t E00[3] = {0, 0, 8};
static const uint8_t PYM[3] = {8, 0x28, 0x28}, PY[3] = {0x28, 0x28, 0x28};
static const uint8_t PXABA[3] = {8, 0x18, 8}, PX[3] = {0x18, 0x18, 0x18};
static const uint8_t DOWN[3] = {0, 0, 0};
#endif

static int pass(uint8_t rr3, const uint8_t *pb, const uint8_t *a, const uint8_t *b)
{
    f.rr3 = rr3;
    memcpy(f.pb, pb, 3); memcpy(f.a, a, 3); memcpy(f.b, b, 3);
    f.ip = f.ia = f.ib = f.n = 0;
    memset(f.log, 0, sizeof f.log);
    return cs_mac_motion_service(&m);
}

static void setup(int ready)
{
    cs_mac_motion_io io = {&f, rd, wrt, pbr};
    memset(&f, 0, sizeof f);
    memset(&m, 0xCC, sizeof m);
    CK(cs_inbox_begin(&in) == CS_INBOX_OK);
    CK(cs_mac_motion_prepare(&m, &io, &in, 0u) == CS_MAC_MOTION_OK);
    CK(cs_mac_motion_arm(&m) == CS_MAC_MOTION_OK);
    CK(pass(0, U, Z, Z) == CS_MAC_MOTION_BASELINED);
    if (!ready) return;
    CK(cs_inbox_claim(&in, m.epoch) == CS_INBOX_BASELINE_ITEM);
    CK(cs_inbox_complete(&in, m.epoch, 0) == CS_INBOX_OK);
    CK(in.phase == CS_INBOX_READY);
}

static void order_test(void)
{
    setup(1);
    CK(pass(0, U, Z, Z) == CS_MAC_MOTION_EMPTY);
    CK(strcmp(f.log, "3PABPABWPAB") == 0);
    CK(pass(8, U, H, H) == CS_MAC_MOTION_LOST);
    setup(1);
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_OK);
    CK(strcmp(f.log, "3PABPABWWPAB") == 0);
    CK(in.count == 1u);
}

#if CLASSICK_MAC_PENDING_DEFER_PROBE
static void deferred(const uint8_t *pb, const uint8_t *a, const uint8_t *b,
    uint32_t want)
{
    uint32_t s = m.samples, ph = m.have_phase;
    CK(pass(0, pb, a, b) == CS_MAC_MOTION_EMPTY);
    CK(strcmp(f.log, "3PABPABWPAB") == 0);
    CK(in.count == 0u && m.samples == s && m.have_phase == ph);
    CK(m.last_raw_a == 0u && m.last_raw_b == 0u && m.last_buttons == 0u);
    CK(m.losses == 0u && m.baselines == 1u);
    CK(m.defer_pending == want);
    CK(m.defer_snapshot == (want == 8u ? 0x00080800UL : 0x00280008UL));
}

static void defer_tests(void)
{
    setup(1);
    deferred(U, E01, Z, 8u);
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_OK);
    CK(in.count == 1u && m.samples == 1u && m.defer_pending == 0u);
    CK(m.last_raw_a == 1u);
    setup(1);
    deferred(U, E00, Z, 8u);
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_OK && in.count == 1u);

    setup(1);
    deferred(PYM, Z, E01, 1u);
    CK(pass(1, PY, Z, H) == CS_MAC_MOTION_OK);
    CK(in.count == 1u && m.last_raw_b == 1u && m.last_raw_a == 0u);
    setup(1);
    deferred(PYM, Z, E00, 1u);
    CK(pass(1, PY, Z, H) == CS_MAC_MOTION_OK && in.count == 1u);

    /* wrong RR3, endpoint, button, other-axis on next service */
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(0, U, H, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING && in.count == 0u);
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(1, U, H, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING);
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(8, U, Z, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING);
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(8, DOWN, H, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING);
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(8, PY, H, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING);

    /* second unstable pass: no second defer */
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(0, U, E01, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_UNSTABLE && m.losses == 1u);
    CK(m.defer_pending == 0u && m.quiesced == 1u && in.phase == CS_INBOX_LOSS);

    /* no defer in PRIMING */
    setup(0);
    CK(in.phase == CS_INBOX_PRIMING);
    CK(pass(0, U, E01, Z) == CS_MAC_MOTION_LOST);
    CK(m.defer_pending == 0u && m.first_loss.reason == CS_MAC_LOSS_UNSTABLE);

    /* two-axis and direction a-b-a first instability */
    setup(1);
    CK(pass(0, U, E01, E01) == CS_MAC_MOTION_LOST);
    CK(m.defer_pending == 0u && m.first_loss.reason == CS_MAC_LOSS_UNSTABLE);
    setup(1);
    CK(pass(0, PXABA, E01, Z) == CS_MAC_MOTION_LOST);
    CK(m.defer_pending == 0u);
    setup(1);
    CK(pass(0, PXABA, Z, Z) == CS_MAC_MOTION_LOST);
    setup(1); /* other axis direction moves */
    CK(pass(0, PYM, E01, Z) == CS_MAC_MOTION_LOST);
    setup(1); /* Constant moving-axis direction is eligible. */
    CK(pass(0, PX, E01, Z) == CS_MAC_MOTION_EMPTY);
    CK(m.defer_pending == 8u && m.defer_snapshot == 0x00180800UL);
    CK(pass(8, PX, H, Z) == CS_MAC_MOTION_OK);
    CK(cs_inbox_claim(&in, 1) == CS_INBOX_SAMPLE_ITEM);
    CK(in.last.dx == 1 && in.last.dy == 0 && in.last.buttons == 0);
    setup(1);
    { static const uint8_t pbb[3] = {8, 0, 0};
      CK(pass(0, pbb, E01, Z) == CS_MAC_MOTION_LOST); }
    setup(1); /* RR3 nonzero: original path */
    CK(pass(8, U, E01, Z) == CS_MAC_MOTION_LOST);
    CK(m.defer_pending == 0u);

    /* hard fault precedence; faulted state clears defer */
    setup(1); deferred(U, E01, Z, 8u);
    CK(pass(0x80, U, H, Z) == CS_MAC_MOTION_FAULT);
    CK(m.defer_pending == 0u && m.cause == CS_MAC_CAUSE_PENDING);
    setup(1); deferred(U, E01, Z, 8u);
    in.epoch++;
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_FAULT);
    CK(m.defer_pending == 0u && m.cause == CS_MAC_CAUSE_INBOX_STATE);

    /* LOSS/WAIT/stop clear; recovery cannot replay */
    setup(1); deferred(U, E01, Z, 8u);
    CK(cs_inbox_uncertain(&in, 1) == CS_INBOX_LOST);
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_LOST && m.defer_pending == 0u);
    setup(1); deferred(U, E01, Z, 8u);
    CK(cs_inbox_uncertain(&in, 1) == CS_INBOX_LOST);
    CK(cs_inbox_discard(&in, 1) == CS_INBOX_OK);
    CK(cs_inbox_resume(&in, 1) == CS_INBOX_BASELINE);
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_BASELINED && m.defer_pending == 0u);
    setup(1); deferred(U, E01, Z, 8u);
    CK(cs_mac_motion_stop(&m) == CS_MAC_MOTION_OK && m.defer_pending == 0u);
    setup(0); /* Fresh prepare over unspecified initial storage clears fields. */
    CK(m.defer_pending == 0u && m.defer_snapshot == 0u);
    setup(1); deferred(U, E01, Z, 8u);
    m.have_phase = 0;
    CK(pass(8, U, H, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_PENDING);
    CK(m.defer_pending == 0u && m.defer_snapshot == 0u);
}
#else
static void default_tests(void)
{
    setup(1);
    CK(pass(0, U, E01, Z) == CS_MAC_MOTION_LOST);
    CK(m.first_loss.reason == CS_MAC_LOSS_UNSTABLE);
}
#endif

int main(void)
{
    order_test();
#if CLASSICK_MAC_PENDING_DEFER_PROBE
    defer_tests();
#else
    default_tests();
#endif
    puts(bad ? "mac-motion-defer: FAIL" : "mac-motion-defer: ok");
    return bad != 0;
}
