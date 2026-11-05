/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* SPEC-0097 cs_inbox_push_motion tests. Independent executable; links only
 * apps/window-scene/inbox.c. Terminal-equivalence uses a local per-axis clamp
 * model, NOT pointer.c (unverified here). */
#include <stdio.h>
#include <string.h>
#include "../apps/window-scene/inbox.h"

static int fails, checks;
#define CHECK(c) do { ++checks; if (!(c)) { ++fails; \
    printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); } } while (0)

static cs_window_inbox b, snap;

/* READY, head=1, count=0, no claim. */
static void ready(void)
{
    memset(&b, 0, sizeof b);
    CHECK(cs_inbox_begin(&b) == CS_INBOX_OK);
    CHECK(cs_inbox_baseline(&b, 1u, 0u) == CS_INBOX_OK);
    CHECK(cs_inbox_claim(&b, 1u) == CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&b, 1u, 0) == CS_INBOX_OK);
    CHECK(b.phase == CS_INBOX_READY && b.count == 0u);
}
static void push(int32_t dx, int32_t dy, uint32_t bt)
{
    CHECK(cs_inbox_push(&b, 1u, dx, dy, bt) == CS_INBOX_OK);
}
static cs_pointer_sample *tailrec(void)
{
    return &b.records[(b.head + b.count - 1u) % CS_INBOX_CAPACITY];
}
/* Expect MERGED with tail (ex,ey); nothing else changes. */
static void merged(int32_t dx, int32_t dy, uint32_t bt, int32_t ex, int32_t ey)
{
    snap = b;
    CHECK(cs_inbox_push_motion(&b, 1u, dx, dy, bt) == CS_INBOX_MERGED);
    snap.records[(snap.head + snap.count - 1u) % CS_INBOX_CAPACITY].dx = ex;
    snap.records[(snap.head + snap.count - 1u) % CS_INBOX_CAPACITY].dy = ey;
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
}
/* Expect fallback identical to strict push on a copy. */
static void fallback(int32_t dx, int32_t dy, uint32_t bt)
{
    int r1, r2;
    snap = b;
    r1 = cs_inbox_push(&snap, 1u, dx, dy, bt);
    r2 = cs_inbox_push_motion(&b, 1u, dx, dy, bt);
    CHECK(r1 == r2 && r2 != CS_INBOX_MERGED);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
}

static void test_basic(void)
{
    ready(); push(1, 2, 0u); push(3, 4, 0u);
    merged(5, 0, 0u, 8, 4);
    CHECK(b.count == 2u);
    /* strict push stays strict */
    ready(); push(1, 1, 0u); push(1, 1, 0u);
    CHECK(cs_inbox_push(&b, 1u, 1, 1, 0u) == CS_INBOX_OK && b.count == 3u);
    /* count 1 */
    ready(); push(1, 1, 0u); fallback(1, 1, 0u); CHECK(b.count == 2u);
    /* count 0 */
    ready(); fallback(1, 1, 0u);
}

static void test_priming(void)
{
    memset(&b, 0, sizeof b);
    cs_inbox_begin(&b);
    CHECK(cs_inbox_baseline(&b, 1u, 0u) == CS_INBOX_OK);
    fallback(2, 0, 0u); fallback(2, 0, 0u); /* baseline + staged, PRIMING */
    CHECK(b.phase == CS_INBOX_PRIMING && b.count == 3u);
    CHECK(cs_inbox_claim(&b, 1u) == CS_INBOX_BASELINE_ITEM);
    fallback(2, 0, 0u); /* claimed baseline, still PRIMING */
}

static void test_buttons(void)
{
    ready(); push(1, 0, 0u); push(1, 0, 1u); fallback(1, 0, 1u); /* press edge */
    ready(); push(1, 0, 1u); push(1, 0, 0u); fallback(1, 0, 0u); /* release */
    ready(); push(1, 0, 1u); push(1, 0, 1u); fallback(1, 0, 0u); /* incoming */
    ready(); push(1, 0, 3u); push(1, 0, 3u); merged(1, 0, 3u, 2, 0);
    ready(); push(1, 0, 2u); push(1, 0, 2u); fallback(1, 0, 4u);
}

static void test_signs(void)
{
    ready(); push(9, 9, 0u); push(2, 0, 0u); merged(0, -3, 0u, 2, -3);
    ready(); push(9, 9, 0u); push(-2, -2, 0u); merged(-1, 0, 0u, -3, -2);
    ready(); push(9, 9, 0u); push(0, 5, 0u); merged(3, 0, 0u, 3, 5);
    ready(); push(9, 9, 0u); push(2, 0, 0u); fallback(-1, 0, 0u); /* x rev */
    ready(); push(9, 9, 0u); push(2, -1, 0u); fallback(1, 1, 0u); /* y rev */
    ready(); push(9, 9, 0u); push(2, 2, 0u); fallback(0, 0, 0u); /* zero in */
    ready(); push(9, 9, 0u); push(0, 0, 0u); fallback(1, 1, 0u); /* zero tail */
    ready(); push(9, 9, 0u); push(0, 0, 0u); fallback(0, 0, 0u);
}

static void test_overflow(void)
{
    ready(); push(1, 1, 0u); push(INT32_MAX - 1, 0, 0u);
    merged(1, 0, 0u, INT32_MAX, 0);
    ready(); push(1, 1, 0u); push(INT32_MAX - 1, 0, 0u); fallback(2, 0, 0u);
    ready(); push(1, 1, 0u); push(0, INT32_MIN + 1, 0u);
    merged(0, -1, 0u, 0, INT32_MIN);
    ready(); push(1, 1, 0u); push(0, INT32_MIN + 1, 0u); fallback(0, -2, 0u);
    ready(); push(1, 1, 0u); push(INT32_MAX, 1, 0u); fallback(1, 1, 0u);
    ready(); push(1, 1, 0u); push(INT32_MIN, -1, 0u); fallback(-1, -1, 0u);
}

static void test_wrap_full_claim(void)
{
    int i;
    ready();
    for (i = 0; i < 14; ++i) {
        push(1, 0, 0u);
        CHECK(cs_inbox_claim(&b, 1u) == CS_INBOX_SAMPLE_ITEM);
        CHECK(cs_inbox_complete(&b, 1u, 0) == CS_INBOX_OK);
    }
    CHECK(b.head == 15u);
    push(1, 1, 0u); push(2, 2, 0u);           /* slots 15, 0 */
    merged(1, 1, 0u, 3, 3);
    CHECK(b.records[0].dx == 3 && b.records[15].dx == 1);
    /* full queue */
    ready();
    for (i = 0; i < CS_INBOX_CAPACITY; ++i) push(1, 0, 0u);
    merged(1, 0, 0u, 2, 0);
    CHECK(b.count == 16u && b.phase == CS_INBOX_READY);
    fallback(-1, 0, 0u);                       /* refusal => existing loss */
    CHECK(b.phase == CS_INBOX_LOSS && b.loss_reason == CS_INBOX_OVERFLOW);
    CHECK(b.losses == 1u && b.count == 16u && tailrec()->dx == 2);
    fallback(1, 0, 0u);                        /* LOSS: LOST, no merge */
    /* claimed sentinel: claim oldest, overwrite last, merge tail */
    ready(); push(1, 0, 0u); push(1, 0, 0u); push(1, 0, 0u);
    CHECK(cs_inbox_claim(&b, 1u) == CS_INBOX_SAMPLE_ITEM);
    b.last.dx = 0x5EEDBEEF; b.last.dy = -7; b.last.buttons = 6u;
    merged(4, 0, 0u, 5, 0);
    CHECK(b.last.dx == 0x5EEDBEEF && b.last.dy == -7 && b.last.buttons == 6u);
    CHECK(b.claimed == CS_INBOX_CLAIM_SAMPLE);
    /* claimed with count 1 left: only one unread, no merge */
    ready(); push(1, 0, 0u); push(1, 0, 0u);
    CHECK(cs_inbox_claim(&b, 1u) == CS_INBOX_SAMPLE_ITEM);
    fallback(1, 0, 0u);
}

static void test_malformed(void)
{
    ready(); push(1, 0, 0u); push(1, 0, 0u);
    snap = b;
    CHECK(cs_inbox_push_motion(0, 1u, 1, 0, 0u) == CS_INBOX_ARGUMENT);
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 8u) == CS_INBOX_ARGUMENT);
    CHECK(cs_inbox_push_motion(&b, 2u, 1, 0, 0u) == CS_INBOX_STATE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
    b.valid ^= 1u; snap = b;
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 0u) == CS_INBOX_STATE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
    b.valid ^= 1u; b.head = 16u; snap = b;
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 0u) == CS_INBOX_STATE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
    b.head = 1u; b.count = 17u; snap = b;
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 0u) == CS_INBOX_STATE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
    b.count = 2u; b.claimed = CS_INBOX_CLAIM_BASELINE; snap = b;
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 0u) == CS_INBOX_STATE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
    memset(&b, 0, sizeof b); cs_inbox_begin(&b); snap = b;   /* WAIT */
    CHECK(cs_inbox_push_motion(&b, 1u, 1, 0, 0u) == CS_INBOX_BASELINE);
    CHECK(memcmp(&b, &snap, sizeof b) == 0);
}

/* Local model: per-axis clamp in int64; press position recorded on edge. */
typedef struct { int64_t x, y; uint32_t bt; int presses; int64_t px[32], py[32]; } model;
static int64_t clampv(int64_t v, int64_t hi) { return v < 0 ? 0 : v > hi ? hi : v; }
static void apply(model *m, const cs_pointer_sample *s)
{
    m->x = clampv(m->x + s->dx, CS_POINTER_WIDTH - 1);
    m->y = clampv(m->y + s->dy, CS_POINTER_HEIGHT - 1);
    if (s->buttons & ~m->bt) { m->px[m->presses] = m->x; m->py[m->presses++] = m->y; }
    m->bt = s->buttons;
}
static uint32_t seed = 0x0097C0DEu;
static int32_t rnd(int32_t n) { seed = seed * 1103515245u + 12345u; return (int32_t)((seed >> 8) % (uint32_t)n); }

static void test_oracle(void)
{
    int t, i, n, k, mergedn, total = 0;
    cs_pointer_sample raw[15];
    model a, q;
    for (t = 0; t < 500; ++t) {
        ready(); mergedn = 0;
        n = 1 + rnd(15);
        for (i = 0; i < n; ++i) {
            raw[i].dx = rnd(5) == 0 ? 0 : rnd(1201) - 600;
            raw[i].dy = rnd(5) == 0 ? 0 : (rnd(4) ? 1 : -1) * rnd(400);
            if (t & 1) { raw[i].dx = raw[i].dx < 0 ? -raw[i].dx : raw[i].dx; }
            raw[i].buttons = rnd(6) == 0 ? (uint32_t)rnd(8) : (i ? raw[i - 1].buttons : 0u);
            k = cs_inbox_push_motion(&b, 1u, raw[i].dx, raw[i].dy, raw[i].buttons);
            CHECK(k == CS_INBOX_OK || k == CS_INBOX_MERGED);
            mergedn += k == CS_INBOX_MERGED;
        }
        CHECK(b.count == (uint32_t)(n - mergedn));
        memset(&a, 0, sizeof a); memset(&q, 0, sizeof q);
        a.x = q.x = 256; a.y = q.y = 171;
        for (i = 0; i < n; ++i) apply(&a, &raw[i]);
        while (cs_inbox_claim(&b, 1u) == CS_INBOX_SAMPLE_ITEM) {
            apply(&q, &b.last);
            CHECK(cs_inbox_complete(&b, 1u, 0) == CS_INBOX_OK);
        }
        CHECK(a.x == q.x && a.y == q.y && a.bt == q.bt && a.presses == q.presses);
        for (i = 0; i < a.presses; ++i) CHECK(a.px[i] == q.px[i] && a.py[i] == q.py[i]);
        total += mergedn;
    }
    CHECK(total > 0);
}

int main(void)
{
    test_basic(); test_priming(); test_buttons(); test_signs();
    test_overflow(); test_wrap_full_claim(); test_malformed(); test_oracle();
    printf("window-inbox-motion: %d/%d checks passed\n", checks - fails, checks);
    return fails != 0;
}
