/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original SPEC-0043 selection values. */
#include <stdint.h>
#include "quickdraw-select.h"

#define CS_QD_SELECT_MAX24 UINT32_C(0xFFFFFF)
#define CS_QD_SELECT_RES_BELOW UINT32_C(202)
#define CS_QD_SELECT_RES_ABOVE UINT32_C(4)
#define CS_QD_SELECT_SPANS (2u + 2u * CS_QD_SELECT_CAPACITY)

typedef struct {
    uintptr_t begin, end;
} cs_qd_select_span;

static int cs_qd_select_native(const void *p, uintptr_t size, uintptr_t align,
                               cs_qd_select_span *span)
{
    uintptr_t a;
    if (p == 0)
        return 0;
    a = (uintptr_t)p;
    if (align == 0u || a % align != 0u)
        return 0;
    if (a > UINTPTR_MAX - size)
        return 0;
    span->begin = a;
    span->end = a + size;
    return 1;
}

static int cs_qd_select_apart(const cs_qd_select_span *a,
                              const cs_qd_select_span *b)
{
    return a->end <= b->begin || b->end <= a->begin;
}

static int cs_qd_select_apart_all(const cs_qd_select_span *spans,
                                  unsigned n, const cs_qd_select_span *s)
{
    unsigned i;
    for (i = 0u; i < n; ++i)
        if (!cs_qd_select_apart(&spans[i], s))
            return 0;
    return 1;
}

static int cs_qd_select_even24(uint32_t v)
{
    return (v & 1u) == 0u && v <= CS_QD_SELECT_MAX24;
}

static int cs_qd_select_range(int32_t v, int32_t lo, int32_t hi)
{
    return v >= lo && v <= hi;
}

static int cs_qd_select_fields(const cs_qd_view *view)
{
    return cs_qd_select_range(view->x, -32768, 32767) &&
           cs_qd_select_range(view->y, -32768, 32767) &&
           cs_qd_select_range(view->top, 0, (int32_t)CS_QD_RECT_HEIGHT) &&
           cs_qd_select_range(view->bottom, 0, (int32_t)CS_QD_RECT_HEIGHT) &&
           cs_qd_select_range(view->left, 0, (int32_t)CS_QD_RECT_WIDTH) &&
           cs_qd_select_range(view->right, 0, (int32_t)CS_QD_RECT_WIDTH);
}

/* Validates the whole table, plan storage and current. On success *found is
 * the active index of current, or count when current is NIL. Writes nothing
 * except the caller's local *found. */
static int cs_qd_select_validate(const cs_qd_selection *selection,
                                 uint32_t current,
                                 const cs_qd_select_plan *plan,
                                 uint32_t *found)
{
    cs_qd_select_span spans[CS_QD_SELECT_SPANS];
    cs_qd_select_span s;
    const cs_qd_view *views[CS_QD_SELECT_CAPACITY];
    uint32_t ids[CS_QD_SELECT_CAPACITY];
    uint32_t gb, a5, g, count, res_lo, res_hi, i, j, hit, matches;
    unsigned n = 0u;

    if (!cs_qd_select_native(selection, (uintptr_t)sizeof *selection,
                             (uintptr_t)_Alignof(cs_qd_selection), &s))
        return 0;
    spans[n++] = s;
    if (!cs_qd_select_native(plan, (uintptr_t)sizeof *plan,
                             (uintptr_t)_Alignof(cs_qd_select_plan), &s))
        return 0;
    if (!cs_qd_select_apart_all(spans, n, &s))
        return 0;
    spans[n++] = s;

    gb = selection->globals_begin;
    a5 = selection->a5;
    g = selection->g;
    count = selection->count;
    if (count < 1u || count > CS_QD_SELECT_CAPACITY)
        return 0;
    if (!cs_qd_select_even24(gb) || !cs_qd_select_even24(a5) ||
        !cs_qd_select_even24(g) || gb >= a5)
        return 0;
    if (g < CS_QD_SELECT_RES_BELOW)
        return 0;
    res_lo = g - CS_QD_SELECT_RES_BELOW;
    res_hi = g + CS_QD_SELECT_RES_ABOVE; /* g <= 0xFFFFFF: no overflow */
    if (res_lo < gb || res_hi > a5)
        return 0;

    for (i = 0u; i < count; ++i) {
        uint32_t id = selection->bindings[i].id;
        ids[i] = id;
        views[i] = selection->bindings[i].view;
        if (id == 0u || !cs_qd_select_even24(id))
            return 0;
        if (id < gb || id > a5 || a5 - id < CS_QD_SELECT_RECORD_BYTES)
            return 0;
        /* id + 108 <= a5 <= 0xFFFFFF: no overflow below */
        if (!(id + CS_QD_SELECT_RECORD_BYTES <= res_lo || res_hi <= id))
            return 0;
        for (j = 0u; j < i; ++j)
            if (!(id + CS_QD_SELECT_RECORD_BYTES <= ids[j] ||
                  ids[j] + CS_QD_SELECT_RECORD_BYTES <= id))
                return 0;
    }

    /* All view spans checked before any view field is read. */
    for (i = 0u; i < count; ++i) {
        if (!cs_qd_select_native(views[i], (uintptr_t)sizeof *views[i],
                                 (uintptr_t)_Alignof(cs_qd_view), &s))
            return 0;
        if (!cs_qd_select_apart_all(spans, n, &s))
            return 0;
        spans[n++] = s;
    }
    for (i = 0u; i < count; ++i) {
        const cs_qd_rect_port *port = views[i]->port;
        if (!cs_qd_select_native(port, (uintptr_t)sizeof *port,
                                 (uintptr_t)_Alignof(cs_qd_rect_port), &s))
            return 0;
        if (!cs_qd_select_apart_all(spans, n, &s))
            return 0;
        spans[n++] = s;
    }
    for (i = 0u; i < count; ++i)
        if (!cs_qd_select_fields(views[i]))
            return 0;

    hit = count;
    if (current != 0u) {
        matches = 0u;
        for (i = 0u; i < count; ++i)
            if (ids[i] == current) {
                hit = i;
                ++matches;
            }
        if (matches != 1u)
            return 0;
    }
    *found = hit;
    return 1;
}

int cs_qd_select_get(const cs_qd_selection *selection, uint32_t current,
                     cs_qd_select_plan *plan)
{
    uint32_t found;
    if (!cs_qd_select_validate(selection, current, plan, &found))
        return 0;
    if (current == 0u) {
        plan->value = 0u;
        plan->view = 0;
    } else {
        plan->value = current;
        plan->view = selection->bindings[found].view;
    }
    return 1;
}

int cs_qd_select_set(const cs_qd_selection *selection, uint32_t current,
                     uint32_t requested, cs_qd_select_plan *plan)
{
    uint32_t found, i, hit, matches = 0u;
    if (!cs_qd_select_validate(selection, current, plan, &found))
        return 0;
    if (requested == 0u)
        return 0;
    hit = 0u;
    for (i = 0u; i < selection->count; ++i)
        if (selection->bindings[i].id == requested) {
            hit = i;
            ++matches;
        }
    if (matches != 1u)
        return 0;
    plan->value = requested;
    plan->view = selection->bindings[hit].view;
    return 1;
}
