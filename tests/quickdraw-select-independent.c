/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Independent API-only tests for SPEC-0043 native selection values. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "personality/system1/quickdraw-select.h"

typedef struct { const void *p; size_t n; } Reg;
typedef struct { int ok; uint32_t value; const cs_qd_view *view; } Exp;

static unsigned fails;
static unsigned long checks, rnd_ok, rnd_no;
#define FAILF(...) do { if (fails < 40) { printf(__VA_ARGS__); printf("\n"); } fails++; } while (0)
#define SENTV UINT32_C(0xDEADBEEF)

static cs_qd_selection S;
static cs_qd_select_plan P;
static cs_qd_view V[2];
static cs_qd_rect_port PT[2];
static cs_qd_view SENT;
static union { long double ld; long long ll; void *vp; unsigned char b[2400]; } mb;

static void setup_into(cs_qd_selection *s)
{
    memset(s, 0, sizeof *s);
    s->globals_begin = 0x1000; s->a5 = 0x2000; s->g = 0x1800; s->count = 2;
    s->bindings[0].id = 0x1100; s->bindings[0].view = &V[0];
    s->bindings[1].id = 0x1200; s->bindings[1].view = &V[1];
}

static void setup(void)
{
    size_t i, j;
    memset(V, 0, sizeof V);
    memset(PT, 0, sizeof PT);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 1024; j++) PT[i].pixels[j] = (uint8_t)(j * 7u + i);
        PT[i].paints = (uint32_t)(3u + i);
        PT[i].erases = 4;
        V[i].port = &PT[i];
    }
    V[0].x = -5; V[0].y = 7; V[0].top = 1; V[0].left = 2; V[0].bottom = 60; V[0].right = 100;
    V[1].x = 100; V[1].y = -100; V[1].top = 3; V[1].left = 4; V[1].bottom = 5; V[1].right = 6;
    setup_into(&S);
}

static void run(const char *tag, const cs_qd_selection *s, cs_qd_select_plan *pl,
                uint32_t cur, uint32_t req, int set, int expect, uint32_t ev,
                const cs_qd_view *evw, const Reg *r, size_t nr)
{
    unsigned char *snap;
    size_t tot = 0, i, off = 0;
    int rc;
    for (i = 0; i < nr; i++) tot += r[i].n;
    snap = malloc(tot ? tot : 1u);
    if (!snap) { puts("out of memory"); exit(1); }
    for (i = 0; i < nr; i++) { memcpy(snap + off, r[i].p, r[i].n); off += r[i].n; }
    rc = set ? cs_qd_select_set(s, cur, req, pl) : cs_qd_select_get(s, cur, pl);
    checks++;
    if (rc != expect) {
        FAILF("%s: rc %d want %d (set=%d cur=%lx req=%lx)", tag, rc, expect, set,
              (unsigned long)cur, (unsigned long)req);
    } else if (expect && (pl->value != ev || pl->view != evw)) {
        FAILF("%s: plan value %lx want %lx or wrong view (set=%d cur=%lx req=%lx)", tag,
              (unsigned long)pl->value, (unsigned long)ev, set, (unsigned long)cur,
              (unsigned long)req);
    }
    off = 0;
    for (i = 0; i < nr; i++) {
        if (memcmp(snap + off, r[i].p, r[i].n) != 0)
            FAILF("%s: region %u modified (rc=%d set=%d)", tag, (unsigned)i, rc, set);
        off += r[i].n;
    }
    free(snap);
}

static void check_exp(const char *tag, uint32_t cur, uint32_t req, int set, int expect,
                      uint32_t ev, const cs_qd_view *evw)
{
    Reg r[4];
    r[0].p = &S; r[0].n = sizeof S;
    r[1].p = V; r[1].n = sizeof V;
    r[2].p = PT; r[2].n = sizeof PT;
    r[3].p = &P; r[3].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    run(tag, &S, &P, cur, req, set, expect, ev, evw, r, expect ? 3u : 4u);
}

static const cs_qd_view *lookup(const cs_qd_selection *s, uint32_t id)
{
    uint32_t i;
    for (i = 0; i < s->count && i < CS_QD_SELECT_CAPACITY; i++)
        if (s->bindings[i].id == id) return s->bindings[i].view;
    return NULL;
}

static void lit(const char *tag, uint32_t cur, uint32_t req, int set, int expect)
{
    uint32_t v = set ? req : cur;
    check_exp(tag, cur, req, set, expect, expect ? v : 0u,
              (expect && v) ? lookup(&S, v) : NULL);
}

/* Independent numeric oracle. */
static int vok(const cs_qd_view *v)
{
    if (!v || !v->port) return 0;
    return v->x >= -32768 && v->x <= 32767 && v->y >= -32768 && v->y <= 32767 &&
           v->top >= 0 && v->top <= 64 && v->bottom >= 0 && v->bottom <= 64 &&
           v->left >= 0 && v->left <= 128 && v->right >= 0 && v->right <= 128;
}

static int tab_ok(const cs_qd_selection *s)
{
    uint64_t gb = s->globals_begin, a5 = s->a5, g = s->g;
    uint32_t i, j;
    if (s->count < 1 || s->count > 2) return 0;
    if (gb > 0xFFFFFFu || a5 > 0xFFFFFFu || g > 0xFFFFFFu) return 0;
    if (((gb | a5 | g) & 1u) != 0 || gb >= a5) return 0;
    if (g < gb + 202u || g + 4u > a5) return 0;
    for (i = 0; i < s->count; i++) {
        uint64_t id = s->bindings[i].id;
        if (id == 0 || (id & 1u) || id > 0xFFFFFFu) return 0;
        if (id < gb || id + 108u > a5) return 0;
        if (id + 108u > g - 202u && id < g + 4u) return 0;
        if (!vok(s->bindings[i].view)) return 0;
        for (j = 0; j < i; j++) {
            uint64_t o = s->bindings[j].id;
            if (id < o + 108u && o < id + 108u) return 0;
            if (s->bindings[j].view == s->bindings[i].view) return 0;
            if (s->bindings[j].view->port == s->bindings[i].view->port) return 0;
        }
    }
    return 1;
}

static Exp oracle(const cs_qd_selection *s, uint32_t cur, uint32_t req, int set)
{
    Exp e;
    const cs_qd_view *cv = NULL;
    e.ok = 0; e.value = 0; e.view = NULL;
    if (!tab_ok(s)) return e;
    if (cur != 0) { cv = lookup(s, cur); if (!cv) return e; }
    if (!set) { e.ok = 1; e.value = cur; e.view = cv; return e; }
    if (req == 0 || !(cv = lookup(s, req))) return e;
    e.ok = 1; e.value = req; e.view = cv;
    return e;
}

static void check_orc(const char *tag, uint32_t cur, uint32_t req, int set)
{
    Exp e = oracle(&S, cur, req, set);
    check_exp(tag, cur, req, set, e.ok, e.value, e.view);
    if (e.ok) rnd_ok++; else rnd_no++;
}

static void refuse_both(const char *tag, const cs_qd_selection *s, cs_qd_select_plan *pl,
                        const Reg *r, size_t nr)
{
    static const uint32_t curs[2] = { 0u, 0x1100u };
    int k, set;
    for (k = 0; k < 2; k++)
        for (set = 0; set < 2; set++)
            run(tag, s, pl, curs[k], 0x1200u, set, 0, 0, NULL, r, nr);
}

static void t_basic(void)
{
    setup();
    lit("get nil", 0, 0, 0, 1);
    lit("get id0", 0x1100, 0, 0, 1);
    lit("get id1", 0x1200, 0, 0, 1);
    lit("get unknown", 0x1300, 0, 0, 0);
    lit("get odd", 0x1101, 0, 0, 0);
    lit("get flagged", 0x80001100u, 0, 0, 0);
    lit("get flagged2", 0x01001100u, 0, 0, 0);
    lit("set 0->1", 0, 0x1100, 1, 1);
    lit("set 0->2", 0, 0x1200, 1, 1);
    lit("set 1->2", 0x1100, 0x1200, 1, 1);
    lit("set 2->1", 0x1200, 0x1100, 1, 1);
    lit("set same", 0x1100, 0x1100, 1, 1);
    lit("set nil refused", 0x1100, 0, 1, 0);
    lit("set nil/nil refused", 0, 0, 1, 0);
    lit("set unknown req", 0x1100, 0x1300, 1, 0);
    lit("set unknown cur", 0x1300, 0x1100, 1, 0);
    lit("set flagged req", 0x1100, 0x80001100u, 1, 0);
    lit("set flagged cur", 0x80001100u, 0x1100, 1, 0);
    lit("set max req", 0, 0xFFFFFFFFu, 1, 0);
}

static void geo(const char *tag, uint32_t gb, uint32_t a5, uint32_t g, uint32_t id0,
                uint32_t id1, uint32_t count, int expect)
{
    setup();
    S.globals_begin = gb; S.a5 = a5; S.g = g; S.count = count;
    S.bindings[0].id = id0; S.bindings[1].id = id1;
    lit(tag, 0, 0, 0, expect);
    lit(tag, 0, id0, 1, expect);
    lit(tag, id0, id0, 1, expect);
    lit(tag, id0, 0, 0, expect);
}

static void t_geometry(void)
{
    geo("adjacent", 0x1000, 0x2000, 0x1800, 0x1000, 0x106C, 2, 1);
    geo("adjacent rev", 0x1000, 0x2000, 0x1800, 0x106C, 0x1000, 2, 1);
    geo("overlap2", 0x1000, 0x2000, 0x1800, 0x1000, 0x106A, 2, 0);
    geo("same id", 0x1000, 0x2000, 0x1800, 0x1100, 0x1100, 2, 0);
    geo("touch res start", 0x1000, 0x2000, 0x1800, 0x16CA, 0x1200, 2, 1);
    geo("into res start", 0x1000, 0x2000, 0x1800, 0x16CC, 0x1200, 2, 0);
    geo("after res", 0x1000, 0x2000, 0x1800, 0x1804, 0x1200, 2, 1);
    geo("inside res end", 0x1000, 0x2000, 0x1800, 0x1802, 0x1200, 2, 0);
    geo("top fits", 0x1000, 0x2000, 0x1800, 0x1F94, 0x1200, 2, 1);
    geo("top over", 0x1000, 0x2000, 0x1800, 0x1F96, 0x1200, 2, 0);
    geo("at a5", 0x1000, 0x2000, 0x1800, 0x2000, 0x1200, 2, 0);
    geo("below gb", 0x1000, 0x2000, 0x1800, 0x0FFE, 0x1200, 2, 0);
    geo("id zero", 0x1000, 0x2000, 0x1800, 0, 0x1200, 2, 0);
    geo("id odd", 0x1000, 0x2000, 0x1800, 0x1101, 0x1200, 2, 0);
    geo("id huge", 0x1000, 0x2000, 0x1800, 0x1000000, 0x1200, 2, 0);
    geo("res at gb", 0x1000, 0x2000, 0x10CA, 0x10CE, 0x113A, 2, 1);
    geo("res below gb", 0x1000, 0x2000, 0x10C8, 0x10CE, 0x113A, 2, 0);
    geo("reservation underflow", 0, 0x1000, 200, 0x110, 0x200, 2, 0);
    geo("reservation low exact", 0, 0x1000, 202, 206, 314, 2, 1);
    geo("g zero", 0x1000, 0x2000, 0, 0x1100, 0x1200, 2, 0);
    geo("g huge", 0x1000, 0x2000, 0xFFFFFE, 0x1100, 0x1200, 2, 0);
    geo("a5 = g+4", 0x1000, 0x1204, 0x1200 - 0x0, 0x1000, 0x1092, 2, 1);
    geo("a5 < g+4", 0x1000, 0x1202, 0x1200, 0x1000, 0x1092, 2, 0);
    geo("gb == a5", 0x2000, 0x2000, 0x1800, 0x1100, 0x1200, 2, 0);
    geo("gb > a5", 0x3000, 0x2000, 0x1800, 0x1100, 0x1200, 2, 0);
    geo("count 0", 0x1000, 0x2000, 0x1800, 0x1100, 0x1200, 0, 0);
    geo("count 3", 0x1000, 0x2000, 0x1800, 0x1100, 0x1200, 3, 0);
    geo("count huge", 0x1000, 0x2000, 0x1800, 0x1100, 0x1200, 0xFFFFFFFFu, 0);
    geo("g odd", 0x1000, 0x2000, 0x1801, 0x1100, 0x1200, 2, 0);
    geo("gb odd", 0x1001, 0x2000, 0x1800, 0x1100, 0x1200, 2, 0);
    geo("a5 odd", 0x1000, 0x2001, 0x1800, 0x1100, 0x1200, 2, 0);
    geo("gb huge", 0xFFFFFFFEu, 0x2000, 0x1800, 0x1100, 0x1200, 2, 0);
    geo("top24 ok", 0xFFFE00, 0xFFFFFE, 0xFFFF00, 0xFFFF04, 0x1200, 1, 1);
    geo("a5 above 24", 0xFFFE00, 0x1000000, 0xFFFF00, 0xFFFF04, 0x1200, 1, 0);
    geo("a5 FFFFFF", 0xFFFE00, 0xFFFFFF, 0xFFFF00, 0xFFFF04, 0x1200, 1, 0);
}

static int32_t *fld(cs_qd_view *v, int f)
{
    switch (f) {
    case 0: return &v->x;
    case 1: return &v->y;
    case 2: return &v->top;
    case 3: return &v->left;
    case 4: return &v->bottom;
    default: return &v->right;
    }
}

static void t_views(void)
{
    static const int32_t lim[6][2] = { {-32768, 32767}, {-32768, 32767}, {0, 64},
                                       {0, 128}, {0, 64}, {0, 128} };
    int f, w, k;
    for (f = 0; f < 6; f++) {
        int32_t lo = lim[f][0], hi = lim[f][1];
        int32_t vals[10];
        vals[0] = INT32_MIN; vals[1] = lo - 1; vals[2] = lo; vals[3] = lo + 1;
        vals[4] = hi - 1; vals[5] = hi; vals[6] = hi + 1; vals[7] = INT32_MAX;
        vals[8] = 0; vals[9] = -1;
        for (w = 0; w < 2; w++)
            for (k = 0; k < 10; k++) {
                int expect = vals[k] >= lo && vals[k] <= hi;
                setup();
                *fld(&V[w], f) = vals[k];
                lit("view get nil", 0, 0, 0, expect);
                lit("view get id0", 0x1100, 0, 0, expect);
                lit("view get id1", 0x1200, 0, 0, expect);
                lit("view set", 0x1100, 0x1200, 1, expect);
            }
    }
    setup();
    V[0].top = 64; V[0].bottom = 0; V[0].left = 128; V[0].right = 0;
    lit("inverted clip", 0x1100, 0x1200, 1, 1);
    V[1].top = 5; V[1].bottom = 5; V[1].left = 9; V[1].right = 9;
    lit("empty clip", 0x1200, 0x1100, 1, 1);
}

static void t_unused_and_nulls(void)
{
    setup();
    S.count = 1; S.bindings[1].view = NULL; S.bindings[1].id = 7;
    lit("unused null ok", 0x1100, 0x1100, 1, 1);
    lit("unused not registered", 0x1200, 0, 0, 0);
    lit("unused req", 0x1100, 0x1200, 1, 0);
    setup();
    S.count = 1; V[1].top = 999; S.bindings[1].id = 0x1100;
    lit("unused bad view ok", 0, 0, 0, 1);
    setup();
    S.bindings[0].view = NULL;
    lit("null view0", 0, 0, 0, 0);
    setup();
    S.bindings[1].view = NULL;
    lit("null view1", 0x1100, 0x1100, 1, 0);
    setup();
    V[1].port = NULL;
    lit("null port1", 0, 0, 0, 0);
    setup();
    V[0].port = NULL;
    lit("null port0", 0x1100, 0x1200, 1, 0);
    setup();
    {
        Reg r[4];
        r[0].p = &S; r[0].n = sizeof S; r[1].p = V; r[1].n = sizeof V;
        r[2].p = PT; r[2].n = sizeof PT; r[3].p = &P; r[3].n = sizeof P;
        P.value = SENTV; P.view = &SENT;
        refuse_both("null selection", NULL, &P, r, 4);
        refuse_both("null plan", &S, NULL, r, 3);
    }
}

static void t_aliases(void)
{
    static union { cs_qd_selection s; cs_qd_select_plan p[8]; } ua;
    static union { cs_qd_view v; cs_qd_select_plan p; } ub;
    static union { cs_qd_selection s; cs_qd_rect_port p; } uc;
    static union { cs_qd_select_plan p; cs_qd_rect_port port; } ud;
    static union { cs_qd_selection s; cs_qd_view v; } ue;
    static union { cs_qd_view v; cs_qd_rect_port p; } uf;
    static union { cs_qd_rect_port p; cs_qd_view v[48]; } ug;
    size_t k, nonoverlap = 0;
    Reg r[5];
    char tag[48];

    for (k = 0; k < 8; k++) {
        int ov = k * sizeof ua.p[0] < sizeof ua.s;
        int set;
        setup();
        memset(&ua, 0, sizeof ua);
        setup_into(&ua.s);
        r[1].p = V; r[1].n = sizeof V; r[2].p = PT; r[2].n = sizeof PT;
        sprintf(tag, "plan in selection k=%u", (unsigned)k);
        if (ov) {
            r[0].p = &ua; r[0].n = sizeof ua;
            refuse_both(tag, &ua.s, &ua.p[k], r, 3);
        } else {
            nonoverlap++;
            r[0].p = &ua.s; r[0].n = sizeof ua.s;
            for (set = 0; set < 2; set++) {
                ua.p[k].value = SENTV; ua.p[k].view = &SENT;
                run(tag, &ua.s, &ua.p[k], 0x1100, 0x1200, set, 1, set ? 0x1200u : 0x1100u,
                    set ? &V[1] : &V[0], r, 3);
            }
        }
    }
    if (nonoverlap == 0) FAILF("alias test never exercised a disjoint plan");

    setup();
    memset(&ub, 0, sizeof ub);
    ub.v.port = &PT[0]; ub.v.top = 1; ub.v.bottom = 2; ub.v.left = 3; ub.v.right = 4;
    S.bindings[0].view = &ub.v;
    r[0].p = &ub; r[0].n = sizeof ub; r[1].p = &S; r[1].n = sizeof S;
    r[2].p = &V[1]; r[2].n = sizeof V[1]; r[3].p = PT; r[3].n = sizeof PT;
    refuse_both("plan==view", &S, &ub.p, r, 4);

    setup();
    memset(&uc, 0, sizeof uc);
    setup_into(&uc.s);
    V[0].port = &uc.p;
    r[0].p = &uc; r[0].n = sizeof uc; r[1].p = V; r[1].n = sizeof V;
    r[2].p = &PT[1]; r[2].n = sizeof PT[1]; r[3].p = &P; r[3].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    refuse_both("port==selection", &uc.s, &P, r, 4);

    setup();
    memset(&ud, 0, sizeof ud);
    V[0].port = &ud.port;
    r[0].p = &ud; r[0].n = sizeof ud; r[1].p = &S; r[1].n = sizeof S;
    r[2].p = V; r[2].n = sizeof V; r[3].p = &PT[1]; r[3].n = sizeof PT[1];
    refuse_both("port==plan", &S, &ud.p, r, 4);

    setup();
    memset(&ue, 0, sizeof ue);
    setup_into(&ue.s);
    ue.s.bindings[0].view = &ue.v;
    r[0].p = &ue; r[0].n = sizeof ue; r[1].p = V; r[1].n = sizeof V;
    r[2].p = PT; r[2].n = sizeof PT; r[3].p = &P; r[3].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    refuse_both("view==selection", &ue.s, &P, r, 4);

    setup();
    S.bindings[1].view = &V[0];
    lit("view shared", 0x1100, 0x1200, 1, 0);
    lit("view shared get", 0, 0, 0, 0);
    setup();
    V[1].port = &PT[0];
    lit("port shared", 0x1100, 0x1200, 1, 0);
    lit("port shared get", 0, 0, 0, 0);

    setup();
    memset(&uf, 0, sizeof uf);
    uf.v.port = &PT[1];
    S.bindings[1].view = &uf.v;
    V[0].port = &uf.p;
    r[0].p = &uf; r[0].n = sizeof uf; r[1].p = &S; r[1].n = sizeof S;
    r[2].p = V; r[2].n = sizeof V; r[3].p = PT; r[3].n = sizeof PT;
    r[4].p = &P; r[4].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    refuse_both("view in port exact", &S, &P, r, 5);

    setup();
    memset(&ug, 0, sizeof ug);
    ug.v[1].port = &PT[1];
    S.bindings[1].view = &ug.v[1];
    V[0].port = &ug.p;
    r[0].p = &ug; r[0].n = sizeof ug;
    refuse_both("view inside port", &S, &P, r, 5);
}

/* Partial native overlaps are rejected before any aliased object read. */
static void t_partial(void)
{
    static union {
        cs_qd_rect_port base;
        struct { uint32_t prefix; cs_qd_rect_port port; } shifted;
    } ports;
    static union {
        cs_qd_view base;
        struct { uint32_t prefix; cs_qd_view view; } shifted;
    } views;
    Reg r[5];
    setup();
    memset(&ports, 0, sizeof ports);
    V[0].port = &ports.base; V[1].port = &ports.shifted.port;
    r[0].p = &S; r[0].n = sizeof S; r[1].p = V; r[1].n = sizeof V;
    r[2].p = &ports; r[2].n = sizeof ports; r[3].p = &P; r[3].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    refuse_both("partial ports", &S, &P, r, 4);
    setup();
    memset(&views, 0, sizeof views);
    S.bindings[0].view = &views.base; S.bindings[1].view = &views.shifted.view;
    r[2].p = PT; r[2].n = sizeof PT; r[4].p = &views; r[4].n = sizeof views;
    refuse_both("partial views", &S, &P, r, 5);
}

static void t_misaligned(void)
{
    Reg r[6];
    setup();
    memset(&mb, 0, sizeof mb);
    r[0].p = &mb; r[0].n = sizeof mb; r[1].p = &S; r[1].n = sizeof S;
    r[2].p = V; r[2].n = sizeof V; r[3].p = PT; r[3].n = sizeof PT;
    r[4].p = &P; r[4].n = sizeof P;
    P.value = SENTV; P.view = &SENT;
    if (_Alignof(cs_qd_selection) > 1u) {
        memcpy(mb.b + 1, &S, sizeof S);
        r[0].p = &mb;
        refuse_both("misaligned selection", (const cs_qd_selection *)(const void *)(mb.b + 1),
                    &P, r, 5);
    }
    memset(&mb, 0, sizeof mb);
    if (_Alignof(cs_qd_select_plan) > 1u)
        refuse_both("misaligned plan", &S, (cs_qd_select_plan *)(void *)(mb.b + 1), r, 4);
    if (_Alignof(cs_qd_view) > 1u) {
        memcpy(mb.b + 1, &V[0], sizeof V[0]);
        S.bindings[0].view = (const cs_qd_view *)(const void *)(mb.b + 1);
        refuse_both("misaligned view", &S, &P, r, 5);
        S.bindings[0].view = &V[0];
    }
    memset(&mb, 0, sizeof mb);
    if (_Alignof(cs_qd_rect_port) > 1u) {
        V[0].port = (cs_qd_rect_port *)(void *)(mb.b + 1);
        refuse_both("misaligned port", &S, &P, r, 5);
    }
}

static uint32_t rs = UINT32_C(0x12345678);
static uint32_t rnd(void)
{
    rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5;
    return rs;
}

static const uint32_t POOL[] = {
    0, 1, 2, 0xFC, 0xFFE, 0x1000, 0x1002, 0x10CA, 0x10CE, 0x1100, 0x1134, 0x11FE, 0x1200,
    0x1204, 0x16CA, 0x1736, 0x17FE, 0x1800, 0x1804, 0x1900, 0x1F94, 0x1FFE, 0x2000, 0x2002,
    0xFFFFFE, 0xFFFFFF, 0x1000000, 0x1000002, 0x7FFFFFFF, 0x80000000u, 0xFFFFFFFEu, 0xFFFFFFFFu
};
#define NPOOL (sizeof POOL / sizeof POOL[0])

static uint32_t pickp(void)
{
    if (rnd() % 4u == 0) return rnd() & 0xFFFFFEu;
    return POOL[rnd() % NPOOL];
}

static void t_random(void)
{
    static const int32_t VP[] = { INT32_MIN, -32769, -32768, -1, 0, 1, 63, 64, 65, 127, 128,
                                  129, 32767, 32768, INT32_MAX };
    static const uint32_t CP[] = { 0, 1, 2, 3, 0xFFFFFFFFu };
    unsigned it;
    rnd_ok = 0; rnd_no = 0;
    for (it = 0; it < 30000u; it++) {
        uint32_t cur, req;
        setup();
        if (rnd() % 3u == 0) S.globals_begin = pickp();
        if (rnd() % 3u == 0) S.a5 = pickp();
        if (rnd() % 3u == 0) S.g = pickp();
        if (rnd() % 4u == 0) S.count = CP[rnd() % (sizeof CP / sizeof CP[0])];
        if (rnd() % 3u == 0) S.bindings[0].id = pickp();
        if (rnd() % 3u == 0) S.bindings[1].id = pickp();
        if (rnd() % 3u == 0)
            *fld(&V[rnd() % 2u], (int)(rnd() % 6u)) = VP[rnd() % (sizeof VP / sizeof VP[0])];
        if (S.count == 1 && rnd() % 2u) S.bindings[1].view = NULL;
        switch (rnd() % 5u) {
        case 0: cur = 0; break;
        case 1: cur = S.bindings[0].id; break;
        case 2: cur = S.bindings[1].id; break;
        case 3: cur = pickp(); break;
        default: cur = rnd(); break;
        }
        switch (rnd() % 5u) {
        case 0: req = 0; break;
        case 1: req = S.bindings[0].id; break;
        case 2: req = S.bindings[1].id; break;
        case 3: req = pickp(); break;
        default: req = rnd(); break;
        }
        check_orc("random get", cur, req, 0);
        check_orc("random set", cur, req, 1);
    }
    if (rnd_ok == 0 || rnd_no == 0) FAILF("random test lacked valid or invalid outcomes");
}

static void t_exhaustive(void)
{
    static const uint32_t IDC[] = { 0, 1, 2, 0xFFE, 0x1000, 0x1002, 0x106C, 0x106E, 0x1100,
        0x16CA, 0x16CC, 0x1734, 0x1736, 0x1800, 0x1802, 0x1804, 0x1806, 0x1F92, 0x1F94,
        0x1F96, 0x2000, 0x1000000 };
    static const uint32_t GP[] = { 0, 0xFFE, 0x1000, 0x1001, 0x1002, 0x10CA, 0x10CC, 0x17FE,
        0x1800, 0x1802, 0x1804, 0x1FFE, 0x2000, 0xFFFFFE, 0x1000000, 0xFFFFFFFEu };
    size_t a, b, c, d, e, f, n = sizeof IDC / sizeof IDC[0], m = sizeof GP / sizeof GP[0];
    uint32_t cnt;
    for (a = 0; a < n; a++)
        for (b = 0; b < n; b++)
            for (cnt = 1; cnt <= 2; cnt++) {
                uint32_t sel[4];
                setup();
                S.count = cnt; S.bindings[0].id = IDC[a]; S.bindings[1].id = IDC[b];
                sel[0] = 0; sel[1] = IDC[a]; sel[2] = IDC[b]; sel[3] = 0x1300;
                for (c = 0; c < 4; c++)
                    for (d = 0; d < 4; d++) {
                        check_orc("exh get", sel[c], sel[d], 0);
                        check_orc("exh set", sel[c], sel[d], 1);
                    }
            }
    for (c = 0; c < m; c++)
        for (d = 0; d < m; d++)
            for (e = 0; e < m; e++)
                for (f = 0; f < 2; f++) {
                    setup();
                    S.globals_begin = GP[c]; S.a5 = GP[d]; S.g = GP[e];
                    if (f) { S.bindings[0].id = 0x1000; S.bindings[1].id = 0x106C; }
                    check_orc("geo get", S.bindings[0].id, 0, 0);
                    check_orc("geo set", S.bindings[0].id, S.bindings[1].id, 1);
                }
}

int main(void)
{
    t_basic();
    t_geometry();
    t_views();
    t_unused_and_nulls();
    t_aliases();
    t_partial();
    t_misaligned();
    t_exhaustive();
    t_random();
    printf("quickdraw-select independent: %lu checks, %u failures\n", checks, fails);
    return fails ? 1 : 0;
}
