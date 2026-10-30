/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "span-bind.h"

/* Bound spans require far data pointers on the pinned 16-bit target. */
#if defined(__WATCOMC__) && defined(_M_I86)
CS_C16_ASSERT(cs_sb_data_pointer_is_far, sizeof(uint8_t *) == 4);
#endif

#define CS_SB_OBJECT_MAX 32767UL

cs_bx_result cs_sb_env(const cs_bx_env *e)
{
    cs_bc_result cr;

    if (e == NULL || e->catalog == NULL || e->spans == NULL ||
        e->span_count == 0u || e->span_count > CS_BS_MAX_SPANS ||
        e->current_epoch == 0UL || e->expected_epoch == 0UL) {
        return CS_BX_ARGUMENT;
    }
    cr = cs_bc_validate(e->map, e->catalog, e->catalog_count, e->ram_first, e->ram_bytes);
    if (cr == CS_BC_ARGUMENT) {
        return CS_BX_ARGUMENT;
    }
    if (cr == CS_BC_MAP) {
        return CS_BX_MAP;
    }
    if (cr != CS_BC_OK) {
        return CS_BX_BACKING;
    }
    return e->current_epoch == e->expected_epoch ? CS_BX_OK : CS_BX_STALE;
}

cs_bx_result cs_sb_walk(const cs_bm_map *m, const cs_bs_span *s, uint16_t n,
                        uint8_t *first, uint8_t *cnt)
{
    uint16_t i, k, at = 0u;

    for (i = 0u; i < m->bank_count; ++i) {
        uint32_t run = 0UL;

        first[i] = (uint8_t)at;
        cnt[i] = 0u;
        for (k = 0u; k < 3u && run < m->banks[i].bytes; ++k, ++at) {
            const cs_bs_span *p;

            if (at >= n) {
                return CS_BX_BINDING;
            }
            p = &s[at];
            if (p->bank != m->banks[i].id || p->offset != run || p->data == NULL ||
                p->bytes == 0UL || p->bytes > CS_SB_OBJECT_MAX ||
                p->bytes > m->banks[i].bytes - run) {
                return CS_BX_BINDING;
            }
            run += p->bytes;
            cnt[i] = (uint8_t)(k + 1u);
        }
        if (run != m->banks[i].bytes) {
            return CS_BX_BINDING;
        }
    }
    return at == n ? CS_BX_OK : CS_BX_BINDING;
}

uint8_t *cs_sb_find(const cs_bs_span *s, const uint8_t *first, const uint8_t *cnt,
                    uint16_t b, uint32_t off, uint32_t *available)
{
    uint16_t q;

    for (q = 0u; q < cnt[b]; ++q) {
        const cs_bs_span *sp = &s[first[b] + q];

        if (off >= sp->offset && off - sp->offset < sp->bytes) {
            *available = sp->bytes - (off - sp->offset);
            return sp->data + (uint16_t)(off - sp->offset);
        }
    }
    return NULL;
}
