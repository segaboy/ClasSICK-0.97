/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent adversarial tests for cs_ps2_bus. Scripted controller model; no real ports.
 * PRIMED cases set keyboard phase34/READY and bus phase N directly after bus_begin to isolate
 * the AUX pipeline; the separate lead integration fixture covers complete keyboard plus AUX startup. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../platform/pc/ps2-bus.h"

typedef struct { char k; uint8_t p, v; } Ev;
static Ev lg[8192];
static int n, qh, qt, ibf, fails;
static struct { uint8_t b, aux, err; } q[8192];
static cs_ps2_bus b;
static cs_input_queue Q;
static cs_input_record st[8];
#define CK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s ", __LINE__, #c); fails++; } } while (0)
#define S(v) {'r', 0x64, v}
#define D(v) {'r', 0x60, v}
#define C(v) {'w', 0x64, v}
#define X(v) {'w', 0x60, v}
#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

static uint8_t rd(void *c, uint16_t p) {
    uint8_t v = 0xEE; (void)c; CK(p == 0x60 || p == 0x64);
    if (p == 0x64) {
        v = 0;
        if (ibf > 0) { v = 2; ibf--; }
        if (qh < qt) v |= 1 | (q[qh].aux ? 0x20 : 0) | q[qh].err;
    } else if (p == 0x60 && qh < qt) v = q[qh++].b;
    lg[n++] = (Ev){'r', (uint8_t)p, v};
    return v;
}
static void wr(void *c, uint16_t p, uint8_t v) { (void)c; CK(p == 0x60 || p == 0x64); lg[n++] = (Ev){'w', (uint8_t)p, v}; }
static const cs_ps2_io io = { rd, wr, 0 };
static void qp(uint8_t v, int aux, int err) { q[qt].b = v; q[qt].aux = (uint8_t)aux; q[qt].err = (uint8_t)err; qt++; }
static uint32_t P(uint64_t t) { return cs_ps2_bus_poll(&b, &io, t, &Q); }
static int same(const Ev *e, int c) {
    int i;
    if (n != c) return 0;
    for (i = 0; i < c; i++) if (lg[i].k != e[i].k || lg[i].p != e[i].p || lg[i].v != e[i].v) return 0;
    return 1;
}
static int writes(void) { int i, w = 0; for (i = 0; i < n; i++) w += lg[i].k == 'w'; return w; }
static int badw(void) {
    int i;
    for (i = 0; i < n; i++)
        if (lg[i].k == 'w' && lg[i].p == 0x64 && (lg[i].v == 0xD0 || lg[i].v == 0xD1 || lg[i].v == 0xAA)) return 1;
    return 0;
}
/* PRIMED: keyboard READY, bus at phase ph, deadline base 1000. */
static void prime(uint32_t ph) {
    memset(&b, 0xA5, sizeof b);
    (void)cs_ps2_bus_begin(&b);
    b.keyboard.phase = CS_PS2_PHASE_READY; b.keyboard.result = CS_PS2_READY;
    b.phase = ph; b.started = 1; b.since = b.last_tick = 1000;
    if (ph == CS_PS2_BUS_READY) b.result = CS_PS2_READY;
    n = qh = qt = ibf = 0;
    CK(cs_input_init(&Q, st, 8) == CS_INPUT_OK);
}
static void stick(uint32_t r) {
    int m = n;
    qp(0xFA, 1, 0); qp(0, 0, 0);
    CK(P(1003) == r); CK(n == m && b.result == r);
}

static void t_args(void) {
    cs_ps2_bus z; cs_ps2_io bad = { 0, wr, 0 }; uint64_t lt;
    CK(cs_ps2_bus_begin(0) == CS_PS2_ARGUMENT);
    memset(&z, 0xA5, sizeof z); (void)cs_ps2_bus_begin(&z);
    CK(z.phase == 0 && z.result == CS_PS2_ACTIVE && z.synced == 1 && z.polls == 0 && z.packets == 0);
    CK(z.since == 0 && z.last_tick == 0 && z.started == 0 && z.drained == 0 && z.errors == 0 && z.left.down == 0);
    CK(z.report.buttons == 0 && z.decoder.used == 0 && z.keyboard.result == CS_PS2_ACTIVE);
    prime(5);
    CK(cs_ps2_bus_poll(0, &io, 1001, &Q) == CS_PS2_ARGUMENT);
    CK(cs_ps2_bus_poll(&b, 0, 1001, &Q) == CS_PS2_ARGUMENT);
    CK(cs_ps2_bus_poll(&b, &bad, 1001, &Q) == CS_PS2_ARGUMENT);
    CK(cs_ps2_bus_poll(&b, &io, 1001, 0) == CS_PS2_ARGUMENT);
    CK(n == 0 && b.polls == 0 && b.last_tick == 1000);
    b.phase = 25; CK(P(1001) == CS_PS2_STATE); b.phase = 5;
    b.result = 10; CK(P(1001) == CS_PS2_STATE); b.result = 0;
    b.synced = 2; CK(P(1001) == CS_PS2_STATE); b.synced = 1;
    CK(n == 0 && b.phase == 5);
    CK(P(999) == CS_PS2_STATE && n == 0);
    b.result = CS_PS2_DEVICE; lt = b.last_tick;
    CK(P(5000) == CS_PS2_DEVICE && n == 0 && b.last_tick == lt && b.phase == 5);
    b.result = CS_PS2_MACHINE; CK(P(5000) == CS_PS2_MACHINE && n == 0);
    prime(1); b.started = 0; b.since = b.last_tick = 0;
    CK(P(5000) == CS_PS2_ACTIVE && b.started == 1 && b.since == 5000 && b.last_tick == 5000);
    CK(P(4999) == CS_PS2_STATE);
    {   /* receive contract: no effects on ARGUMENT/STATE */
        cs_ps2 k; memset(&k, 0, sizeof k);
        CK(cs_ps2_receive(0, 1, 0, &Q) == CS_PS2_ARGUMENT);
        CK(cs_ps2_receive(&k, 0x00, 0x11, &Q) == CS_PS2_ARGUMENT && k.last_byte == 0);
        CK(cs_ps2_receive(&k, 0x01, 0x11, &Q) == CS_PS2_STATE && k.last_byte == 0);
        k.phase = CS_PS2_PHASE_READY; k.result = CS_PS2_READY; k.held = 1; k.extended = 1;
        CK(cs_ps2_receive(&k, 0x21, 0x44, &Q) == CS_PS2_READY && k.auxiliary == 1 && k.last_byte == 0x44 && k.extended == 1);
        CK(cs_ps2_receive(&k, 0x81, 0x45, &Q) == CS_PS2_READY && k.errors == 1 && k.held == 0 && k.extended == 0);
    }
}

static void t_happy(void) {
    static const struct { uint8_t k, v, ph; } s[] = {
        {0,0,2},{0,0,3},{0,0,4},{0,0,5},{1,0,6},{0,0,7},{0,0,8},{0,0,9},{2,0xFA,10},{0,0,11},{0,0,12},
        {2,0xFA,13},{2,0xAA,14},{2,0,15},{0,0,16},{0,0,17},{2,0xFA,18},{0,0,19},{1,0x10,20},{0,0,21},
        {0,0,22},{2,0xFA,23},{0,0,24} };
    static const Ev e[] = {
        S(0), C(0xAD), S(0), C(0xA7), S(0), S(0), C(0xA9), S(1), D(0), S(0), C(0xA8),
        S(0), C(0xD4), S(0), X(0xF5), S(0x21), D(0xFA), S(0), C(0xD4), S(0), X(0xFF),
        S(0x21), D(0xFA), S(0x21), D(0xAA), S(0x21), D(0), S(0), C(0xD4), S(0), X(0xF6),
        S(0x21), D(0xFA), S(0), C(0x20), S(1), D(0x10), S(0), C(0xD4), S(0), X(0xF4),
        S(0x21), D(0xFA), S(0), C(0xAE) };
    int i;
    prime(1);
    for (i = 0; i < N(s); i++) {
        uint32_t r;
        if (s[i].k) qp(s[i].v, s[i].k == 2, 0);
        r = P((uint64_t)(2000 + i));
        CK(r == (i == 22 ? CS_PS2_READY : CS_PS2_ACTIVE)); CK(b.phase == s[i].ph);
        CK(b.since == (uint64_t)(2000 + i) && b.polls == 0);
    }
    CK(same(e, N(e))); CK(!badw());
    CK(b.result == CS_PS2_READY && b.configuration == 0x10 && b.last_byte == 0xFA);
    CK(b.retries == 0 && b.resends == 0 && b.errors == 0 && b.auxiliary == 6 && b.drained == 0);
    n = 0; CK(P(3000) == CS_PS2_READY && n == 1 && lg[0].k == 'r' && lg[0].p == 0x64 && Q.count == 0);
}

static void flood(uint32_t ph, int aux, uint8_t v, uint64_t lim) {
    int i, m;
    prime(ph);
    for (i = 0; i < 40; i++) qp(v, aux, 0);
    for (i = 0; i < 40; i++) { CK(P((uint64_t)(1001 + i)) == CS_PS2_ACTIVE); CK(b.phase == ph && b.since == 1000); }
    CK(writes() == 0);
    CK(aux ? b.auxiliary == 40 : b.last_byte == v);
    CK(P(1000 + lim - 1) == CS_PS2_ACTIVE); m = n;
    CK(P(1000 + lim) == CS_PS2_TIMEOUT && n == m && b.result == CS_PS2_TIMEOUT);
    stick(CS_PS2_TIMEOUT);
}
static void t_timeouts(void) {
    flood(5, 1, 0x00, CS_PS2_SHORT_TICKS); flood(19, 1, 0x10, CS_PS2_SHORT_TICKS);
    flood(9, 0, 0xFA, CS_PS2_SHORT_TICKS); flood(12, 0, 0xFA, CS_PS2_SHORT_TICKS);
    flood(13, 0, 0xAA, CS_PS2_BAT_TICKS); flood(14, 0, 0x00, CS_PS2_SHORT_TICKS);
    flood(17, 0, 0xFA, CS_PS2_SHORT_TICKS); flood(22, 0, 0xFA, CS_PS2_SHORT_TICKS);
    prime(13); CK(P(1000 + CS_PS2_SHORT_TICKS) == CS_PS2_ACTIVE);
    prime(1); CK(P(1000 + CS_PS2_SHORT_TICKS - 1) == CS_PS2_ACTIVE && n == 2);
    prime(1); n = 0; CK(P(1000 + CS_PS2_SHORT_TICKS) == CS_PS2_TIMEOUT && n == 0);
    prime(5); b.polls = CS_PS2_PHASE_POLLS; CK(P(1001) == CS_PS2_TIMEOUT && n == 0);
    prime(24); CK(P(1000 + 99999999) == CS_PS2_READY);
    prime(24); b.result=CS_PS2_ACTIVE;
    CK(P(1001)==CS_PS2_STATE && n==0); /* cannot expose unearned readiness */
    prime(5); b.result=CS_PS2_READY;
    CK(P(1001)==CS_PS2_STATE && n==0);
}

static void t_fail(void) {
    static const struct { uint8_t ph, v, aux; uint32_t exp; } f[] = {
        {9,0xFC,1,CS_PS2_DEVICE},{9,0xFD,1,CS_PS2_DEVICE},{9,0x00,1,CS_PS2_DEVICE},{9,0xFF,1,CS_PS2_DEVICE},
        {9,0x55,1,CS_PS2_PROTOCOL},{12,0x55,1,CS_PS2_PROTOCOL},{17,0xAA,1,CS_PS2_PROTOCOL},
        {22,0xFC,1,CS_PS2_DEVICE},{13,0xAB,1,CS_PS2_DEVICE},{14,0x03,1,CS_PS2_DEVICE},
        {5,0x01,0,CS_PS2_CONTROLLER},{19,0x30,0,CS_PS2_CONTROLLER},{19,0x11,0,CS_PS2_CONTROLLER},
        {19,0x00,0,CS_PS2_CONTROLLER},{19,0x12,0,CS_PS2_CONTROLLER} };
    static const struct { uint8_t ph, v, aux, err; } t[] = {
        {9,0xFA,1,0x80},{5,0x00,0,0x40},{14,0x00,1,0xC0},{13,0xAA,0,0x80},{19,0x10,0,0x80},{22,0xFA,1,0x40} };
    int i;
    for (i = 0; i < N(f); i++) {
        uint32_t r;
        prime(f[i].ph); qp(f[i].v, f[i].aux, 0); r = P(1001);
        CK(r == f[i].exp && b.result == f[i].exp && writes() == 0); stick(r);
    }
    for (i = 0; i < N(t); i++) {
        uint32_t r;
        prime(t[i].ph); qp(t[i].v, t[i].aux, t[i].err); r = P(1001);
        CK(r == CS_PS2_CONTROLLER && b.errors == 1 && writes() == 0); stick(r);
    }
}

static void t_retry(void) {
    static const struct { uint8_t ph, rb, byte; } r[] = { {9,7,0xF5}, {12,10,0xFF}, {17,15,0xF6}, {22,20,0xF4} };
    int i, k;
    for (i = 0; i < 4; i++) {
        Ev e[6] = { S(0x21), D(0xFE), S(0), C(0xD4), S(0), X(r[i].byte) };
        prime(r[i].ph); qp(0xFE, 1, 0);
        CK(P(1500) == CS_PS2_ACTIVE && b.phase == r[i].rb && b.retries == 1 && b.resends == 1);
        CK(b.since == 1500 && b.polls == 0);
        CK(P(1501) == CS_PS2_ACTIVE && P(1502) == CS_PS2_ACTIVE && b.phase == r[i].ph);
        CK(same(e, 6)); CK(b.result == CS_PS2_ACTIVE);
    }
    {   /* 3 sends total: the third FE is terminal; the FA after a retry clears retries */
        Ev e[] = { S(0x21), D(0xFE), S(0), C(0xD4), S(0), X(0xF5), S(0x21), D(0xFE), S(0), C(0xD4), S(0), X(0xF5),
                   S(0x21), D(0xFE) };
        prime(9);
        for (k = 0; k < 2; k++) { qp(0xFE, 1, 0); P((uint64_t)(1100 + 3 * k)); P((uint64_t)(1101 + 3 * k)); P((uint64_t)(1102 + 3 * k)); }
        CK(b.retries == 2 && b.resends == 2 && b.phase == 9);
        qp(0xFE, 1, 0); P(1200);
        CK(b.result > CS_PS2_READY && b.result != CS_PS2_ACTIVE && same(e, N(e)));
        stick(b.result);
        prime(9); qp(0xFE, 1, 0); P(1100); P(1101); P(1102); qp(0xFA, 1, 0);
        CK(P(1103) == CS_PS2_ACTIVE && b.phase == 10 && b.retries == 0 && b.resends == 1);
    }
}

static void t_drain_ibf(void) {
    int i, reads60 = 0;
    prime(3);
    for (i = 0; i < 65; i++) qp(0x11, i & 1, 0);
    for (i = 0; i < 64; i++) { CK(P((uint64_t)(1001 + i)) == CS_PS2_ACTIVE && b.phase == 3 && b.drained == (uint32_t)i + 1); }
    CK(b.last_byte == 0 && b.auxiliary == 0 && b.errors == 0 && writes() == 0);
    CK(P(1100) == CS_PS2_CONTROLLER);
    for (i = 0; i < n; i++) reads60 += lg[i].k == 'r' && lg[i].p == 0x60;
    CK(reads60 == 64 && lg[n - 1].p == 0x64 && (lg[n - 1].v & 1) && qt - qh == 1);
    stick(CS_PS2_CONTROLLER);
    prime(1); ibf = 5;
    {
        Ev e[] = { S(2), S(2), S(2), S(2), S(2), S(0), C(0xAD) };
        for (i = 0; i < 6; i++) CK(P((uint64_t)(1001 + i)) == CS_PS2_ACTIVE && b.phase == (i < 5 ? 1u : 2u));
        CK(same(e, N(e)));
    }
    {   /* pending OBF byte: one data read, no write that turn */
        Ev e[] = { S(1), D(0x77) };
        prime(1); qp(0x77, 0, 0);
        CK(P(1001) == CS_PS2_ACTIVE && b.phase == 1 && b.last_byte == 0x77 && same(e, 2));
        CK(P(1002) == CS_PS2_ACTIVE && b.phase == 2 && writes() == 1);
    }
    prime(8); ibf = 1; CK(P(1001) == CS_PS2_ACTIVE && b.phase == 8 && writes() == 0 && n == 1);
    prime(9); CK(P(1001) == CS_PS2_ACTIVE && n == 1 && b.phase == 9 && b.polls == 1);
}

static void t_ready(void) {
    int i; uint32_t pk;
    static const uint8_t pkt[] = { 0x09, 0x02, 0x03 };
    prime(24);
    CK(P(1001) == CS_PS2_READY && n == 1 && writes() == 0);
    for (i = 0; i < 3; i++) {
        qp(pkt[i], 1, 0); CK(P((uint64_t)(1002 + i)) == CS_PS2_READY);
        CK(b.packets == (i == 2 ? 1u : 0u) && b.left.down == (i == 2 ? 1u : 0u));
    }
    CK(b.report.dx == 2 && b.report.dy == 3 && b.report.buttons == 1 && b.left.presses == 1);
    CK(b.auxiliary == 3 && Q.count == 0 && b.last_byte == 0x03 && writes() == 0);
    qp(0x21, 1, 0x80); CK(P(1010) == CS_PS2_READY);   /* corrupt status */
    CK(b.errors == 1 && b.synced == 0 && b.left.down == 0 && b.left.presses == 1 && b.auxiliary == 4);
    qp(0x09, 1, 0); qp(0, 1, 0); qp(0, 1, 0);
    for (i = 0; i < 3; i++) P((uint64_t)(1011 + i));
    CK(b.synced == 1 && b.left.down == 1 && b.left.presses == 1);   /* rebaseline, no press credit */
    qp(0x08, 1, 0); qp(0, 1, 0); qp(0, 1, 0);
    for (i = 0; i < 3; i++) P((uint64_t)(1020 + i));
    CK(b.left.down == 0 && b.left.presses == 1);
    qp(0x09, 1, 0); qp(0, 1, 0); qp(0, 1, 0);
    for (i = 0; i < 3; i++) P((uint64_t)(1030 + i));
    CK(b.left.down == 1 && b.left.presses == 2 && b.packets == 4 && b.errors == 1);
    prime(24); qp(0x00, 1, 0); CK(P(1001) == CS_PS2_READY && b.errors == 1 && b.synced == 0 && b.packets == 0);
    prime(24); qp(0x0A, 1, 0); qp(0xFE, 1, 0); qp(0xAA, 1, 0);   /* valid header; command-looking payload is never filtered */
    for (i = 0; i < 3; i++) CK(P((uint64_t)(1001 + i)) == CS_PS2_READY);
    CK(b.packets == 1 && b.resends == 0 && b.errors == 0 && b.report.buttons == 2 && b.left.down == 0 && b.phase == 24);
    prime(24); qp(0xE0, 0, 0); qp(0x09, 1, 0); qp(0x05, 1, 0); qp(0x07, 1, 0);   /* assumes E0 prefix sets keyboard.extended */
    CK(P(1001) == CS_PS2_READY && b.keyboard.extended == 1 && b.last_byte == 0xE0);
    pk = b.packets;
    for (i = 0; i < 3; i++) CK(P((uint64_t)(1002 + i)) == CS_PS2_READY);
    CK(b.keyboard.extended == 1 && b.packets == pk + 1 && b.auxiliary == 3 && Q.count == 0);
    prime(24); qp(0xE0, 0, 0x80); CK(P(1001) == CS_PS2_READY && b.keyboard.errors == 1 && b.keyboard.extended == 0);
}

static void t_sat(void) {
    prime(24); b.auxiliary = UINT32_MAX; qp(0x09, 1, 0); CK(P(1001) == CS_PS2_READY && b.auxiliary == UINT32_MAX);
    prime(24); b.errors = UINT32_MAX; qp(0x09, 1, 0x80); P(1001); CK(b.errors == UINT32_MAX);
    prime(24); b.errors = UINT32_MAX; qp(0x00, 1, 0); P(1001); CK(b.errors == UINT32_MAX && b.synced == 0);
    prime(9); b.resends = UINT32_MAX; qp(0xFE, 1, 0); P(1001); CK(b.resends == UINT32_MAX && b.phase == 7);
    prime(3); b.drained = UINT32_MAX; qp(0x11, 0, 0); CK(P(1001) == CS_PS2_ACTIVE && b.drained == UINT32_MAX);
    prime(5); b.auxiliary = UINT32_MAX; qp(0, 1, 0); P(1001); CK(b.auxiliary == UINT32_MAX && b.phase == 5);
    prime(24); b.packets = UINT32_MAX; qp(0x09, 1, 0); qp(0, 1, 0); qp(0, 1, 0);
    P(1001); P(1002); CK(P(1003) == CS_PS2_READY && b.packets == UINT32_MAX);
}

int main(void) {
    t_args(); t_happy(); t_timeouts(); t_fail(); t_retry(); t_drain_ibf(); t_ready(); t_sat();
    puts(fails ? "FAIL" : "PASS");
    return fails != 0;
}
