/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "span-cap.h"

static cs_bq_result cs_bq_from_bx(cs_bx_result r)
{
    switch (r) {
    case CS_BX_OK: return CS_BQ_OK;
    case CS_BX_ARGUMENT: return CS_BQ_ARGUMENT;
    case CS_BX_MAP: return CS_BQ_MAP;
    case CS_BX_BACKING: return CS_BQ_BACKING;
    case CS_BX_STALE: return CS_BQ_STALE;
    default: return CS_BQ_BINDING;
    }
}

/* Whole binding check: env (ARGUMENT, MAP, BACKING, STALE) then BINDING. */
static cs_bq_result cs_bq_binding_ok(const cs_bx_env *b, uint8_t *first, uint8_t *cnt)
{
    cs_bx_result r = cs_sb_env(b);

    if (r == CS_BX_OK) {
        r = cs_sb_walk(b->map, b->spans, b->span_count, first, cnt);
    }
    return cs_bq_from_bx(r);
}

cs_bq_result cs_bq_registry_init(cs_bq_registry *r, uint32_t limit)
{
    if (r == NULL) {
        return CS_BQ_ARGUMENT;
    }
    if (r->issued != 0UL || r->current != 0UL) {
        return CS_BQ_STATE;   /* used registry: re-initialization would recycle */
    }
    r->limit = limit;
    return CS_BQ_OK;
}

cs_bq_result cs_bq_owner_open(cs_bq_registry *r, const cs_bx_env *binding,
                              uint32_t generation_limit, cs_bq_owner *out)
{
    uint8_t first[64], cnt[64];
    cs_bq_result st;

    if (r == NULL || out == NULL || binding == NULL) {
        return CS_BQ_ARGUMENT;
    }
    st = cs_bq_binding_ok(binding, first, cnt);
    if (st != CS_BQ_OK) {
        return st;
    }
    if (r->issued >= r->limit) {
        return CS_BQ_EXHAUSTED;
    }
    r->issued += 1UL;
    r->current = r->issued;   /* every older lifetime becomes STALE */
    out->map = binding->map;
    out->registry = r;
    out->binding = binding;
    out->lifetime = r->issued;
    out->generation = 0UL;
    out->generation_limit = generation_limit;
    return CS_BQ_OK;
}

cs_bq_result cs_bq_owner_advance(cs_bq_owner *o)
{
    if (o == NULL) {
        return CS_BQ_ARGUMENT;
    }
    if (o->generation >= o->generation_limit) {
        return CS_BQ_EXHAUSTED;
    }
    o->generation += 1UL;
    return CS_BQ_OK;
}

cs_bq_result cs_bq_owner_rebind(cs_bq_owner *o, const cs_bx_env *binding)
{
    uint8_t first[64], cnt[64];
    cs_bq_result st;

    if (o == NULL || binding == NULL || o->registry == NULL) {
        return CS_BQ_ARGUMENT;
    }
    if (binding->map != o->map) {
        return CS_BQ_MAP;     /* another map needs a new owner lifetime */
    }
    st = cs_bq_binding_ok(binding, first, cnt);
    if (st != CS_BQ_OK) {
        return st;
    }
    if (o->generation >= o->generation_limit) {
        return CS_BQ_EXHAUSTED;
    }
    o->generation += 1UL;     /* advance BEFORE the binding changes */
    o->binding = binding;
    return CS_BQ_OK;
}

cs_bq_result cs_bq_open(const cs_bq_owner *o, uint32_t at, uint32_t bytes,
                        uint32_t access, cs_bq_cap *out)
{
    const cs_bm_map *m;
    cs_bm_result st;
    uint16_t i, b = 0u;
    uint32_t off = 0UL;

    if (o == NULL || out == NULL || o->registry == NULL || o->binding == NULL) {
        return CS_BQ_ARGUMENT;
    }
    m = o->map;
    st = cs_bm_check(m, at, bytes, access); /* ARGUMENT, MAP, ADDRESS, PERMISSION */
    if (st == CS_BM_ARGUMENT) {
        return CS_BQ_ARGUMENT;
    }
    if (st == CS_BM_MAP) {
        return CS_BQ_MAP;
    }
    if (st == CS_BM_ADDRESS) {
        return CS_BQ_ADDRESS;
    }
    if (st != CS_BM_OK) {
        return CS_BQ_PERMISSION;
    }
    if (o->registry->current != o->lifetime) {
        return CS_BQ_STALE;
    }
    for (i = 0u; i < m->region_count; ++i) {
        const cs_bm_region *r = &m->regions[i];

        if (at >= r->base && bytes <= r->bytes && at - r->base <= r->bytes - bytes) {
            off = at - r->base;
            b = r->first_bank;
            while (off >= m->banks[b].bytes) { /* bounded by region coverage */
                off -= m->banks[b].bytes;
                ++b;
            }
            break;
        }
    }
    out->owner = o;
    out->registry = o->registry;
    out->lifetime = o->lifetime;
    out->generation = o->generation;
    out->access = access;
    out->bank = b;
    out->offset = off;
    out->left = bytes;
    out->released = 0u;
    return CS_BQ_OK;
}

/* Shared head of chunk/borrow: ARGUMENT, CAPABILITY, then STALE when the
 * owner's registry, lifetime or generation no longer match. */
static cs_bq_result cs_bq_live(const cs_bq_cap *k, uint32_t limit)
{
    if (k == NULL || limit == 0UL || limit > CS_BQ_CHUNK_MAX || k->owner == NULL ||
        k->registry == NULL) {
        return CS_BQ_ARGUMENT;
    }
    if (k->released != 0u) {
        return CS_BQ_CAPABILITY;
    }
    if (k->owner->registry != k->registry || k->registry->current != k->lifetime ||
        k->owner->lifetime != k->lifetime || k->owner->generation != k->generation) {
        return CS_BQ_STALE;
    }
    return CS_BQ_OK;
}

static void cs_bq_advance(cs_bq_cap *k, uint32_t n)
{
    const cs_bm_map *m = k->owner->map;

    k->left -= n;
    k->offset += n;
    if (k->offset == m->banks[k->bank].bytes && k->left != 0UL) {
        ++k->bank;
        k->offset = 0UL;
    }
}

cs_bq_result cs_bq_chunk(cs_bq_cap *k, uint32_t limit, cs_bq_descriptor *out,
                         uint16_t *emitted)
{
    const cs_bm_map *m;
    uint32_t n, room;
    cs_bq_result st;

    if (out == NULL || emitted == NULL) {
        return CS_BQ_ARGUMENT;
    }
    st = cs_bq_live(k, limit);
    if (st != CS_BQ_OK) {
        return st;
    }
    if (k->left == 0UL) {
        *emitted = 0u;
        return CS_BQ_OK;
    }
    m = k->owner->map;
    room = m->banks[k->bank].bytes - k->offset;
    n = k->left < limit ? k->left : limit;
    n = n < room ? n : room;
    out->bank = m->banks[k->bank].id;
    out->offset = k->offset;
    out->length = n;
    *emitted = 1u;
    cs_bq_advance(k, n);
    return CS_BQ_OK;
}

cs_bq_result cs_bq_borrow(cs_bq_cap *k, uint32_t need, uint32_t limit,
                          cs_bq_run *out, uint16_t *emitted)
{
    const cs_bx_env *b;
    const cs_bm_map *m;
    uint8_t first[64], cnt[64];
    uint8_t *p;
    uint32_t n, room, avail = 0UL;
    cs_bq_result st;

    if (out == NULL || emitted == NULL || (need != 1UL && need != 3UL)) {
        return CS_BQ_ARGUMENT;
    }
    st = cs_bq_live(k, limit);
    if (st != CS_BQ_OK) {
        return st;
    }
    if ((need & ~k->access) != 0UL) {
        return CS_BQ_PERMISSION;
    }
    b = k->owner->binding;
    m = k->owner->map;
    if (b == NULL || b->map != m) {
        return CS_BQ_STALE;   /* owner invariant broken: no usable binding */
    }
    st = cs_bq_binding_ok(b, first, cnt);  /* revalidated on every borrow */
    if (st != CS_BQ_OK) {
        return st;
    }
    if (k->left == 0UL) {
        *emitted = 0u;
        return CS_BQ_OK;
    }
    p = cs_sb_find(b->spans, first, cnt, k->bank, k->offset, &avail);
    if (p == NULL) {
        return CS_BQ_BINDING; /* unreachable after a complete walk */
    }
    room = m->banks[k->bank].bytes - k->offset;
    n = k->left < limit ? k->left : limit;
    n = n < room ? n : room;
    n = n < avail ? n : avail;
    out->read = p;
    out->write = (need & 2UL) != 0UL ? p : NULL;
    out->bank = m->banks[k->bank].id;
    out->offset = k->offset;
    out->length = n;
    *emitted = 1u;
    cs_bq_advance(k, n);
    return CS_BQ_OK;
}

cs_bq_result cs_bq_rights(const cs_bq_cap *k, uint32_t *access)
{
    if (k == NULL || access == NULL) {
        return CS_BQ_ARGUMENT;
    }
    *access = k->access;
    return CS_BQ_OK;
}

cs_bq_result cs_bq_release(cs_bq_cap *k)
{
    if (k == NULL) {
        return CS_BQ_ARGUMENT;
    }
    k->released = 1u;
    return CS_BQ_OK;
}
