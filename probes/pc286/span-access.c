/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "span-access.h"

#define CS_BX_READ 1UL
#define CS_BX_WRITE 2UL

/* WIDTH, alignment ADDRESS, then cs_bm_check window (map already valid). */
static cs_bx_result cs_bx_operand(const cs_bm_map *m, uint32_t at, unsigned width,
                                  uint32_t access)
{
    cs_bm_result st;

    if (width != 1u && width != 2u && width != 4u) {
        return CS_BX_WIDTH;
    }
    if (width != 1u && (at & 1UL) != 0UL) {
        return CS_BX_ADDRESS;
    }
    st = cs_bm_check(m, at, (uint32_t)width, access);
    if (st == CS_BM_OK) {
        return CS_BX_OK;
    }
    if (st == CS_BM_ARGUMENT) {
        return CS_BX_ARGUMENT;
    }
    if (st == CS_BM_PERMISSION) {
        return CS_BX_PERMISSION;
    }
    return st == CS_BM_MAP ? CS_BX_MAP : CS_BX_ADDRESS;
}

/* Every byte of [at, at + width) to a native byte before any data access. */
static cs_bx_result cs_bx_resolve(const cs_bx_env *e, uint32_t at, unsigned width,
                                  uint8_t **dst)
{
    const cs_bm_map *m = e->map;
    const cs_bm_region *r = NULL;
    uint8_t first[64], cnt[64];
    uint16_t i;
    unsigned j;
    cs_bx_result st = cs_sb_walk(m, e->spans, e->span_count, first, cnt);

    if (st != CS_BX_OK) {
        return st;
    }
    for (i = 0u; i < m->region_count && r == NULL; ++i) {
        const cs_bm_region *c = &m->regions[i];

        if (at >= c->base && (uint32_t)width <= c->bytes &&
            at - c->base <= c->bytes - (uint32_t)width) {
            r = c;
        }
    }
    if (r == NULL) {
        return CS_BX_ADDRESS; /* unreachable after cs_bm_check */
    }
    for (j = 0u; j < width; ++j) {
        uint32_t off = at + (uint32_t)j - r->base, avail = 0UL;
        uint16_t b = r->first_bank;

        while (off >= m->banks[b].bytes) { /* bounded by region coverage */
            off -= m->banks[b].bytes;
            ++b;
        }
        dst[j] = cs_sb_find(e->spans, first, cnt, b, off, &avail);
        if (dst[j] == NULL) {
            return CS_BX_BINDING;
        }
    }
    return CS_BX_OK;
}

cs_bx_result cs_bx_read(const cs_bx_env *env, uint32_t at, unsigned width,
                        uint32_t access, uint32_t *out)
{
    uint8_t *src[4];
    uint32_t value = 0UL;
    unsigned j;
    cs_bx_result st;

    if (out == NULL || (access & CS_BX_READ) == 0UL) {
        return CS_BX_ARGUMENT;
    }
    st = cs_sb_env(env);
    if (st == CS_BX_OK) {
        st = cs_bx_operand(env->map, at, width, access);
    }
    if (st == CS_BX_OK) {
        st = cs_bx_resolve(env, at, width, src);
    }
    if (st != CS_BX_OK) {
        return st;
    }
    for (j = 0u; j < width; ++j) {
        value = (value << 8) | (uint32_t)*src[j];
    }
    *out = value;
    return CS_BX_OK;
}

cs_bx_result cs_bx_write(const cs_bx_env *env, uint32_t at, unsigned width,
                         uint32_t value)
{
    uint8_t *dst[4], stage[4];
    unsigned j;
    cs_bx_result st = cs_sb_env(env);

    if (st == CS_BX_OK) {
        st = cs_bx_operand(env->map, at, width, CS_BX_WRITE);
    }
    if (st == CS_BX_OK) {
        st = cs_bx_resolve(env, at, width, dst);
    }
    if (st != CS_BX_OK) {
        return st;
    }
    /* Constant shifts only; width w uses the low w bytes, big-endian. */
    stage[0] = (uint8_t)((value >> 24) & 0xFFUL);
    stage[1] = (uint8_t)((value >> 16) & 0xFFUL);
    stage[2] = (uint8_t)((value >> 8) & 0xFFUL);
    stage[3] = (uint8_t)(value & 0xFFUL);
    for (j = 0u; j < width; ++j) {
        *dst[j] = stage[4u - width + j];
    }
    return CS_BX_OK;
}
