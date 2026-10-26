/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "store4-model.h"

#define CS_BS_OBJECT_MAX 32767UL

/* Metadata-only walk in map bank order; no data access. */
static cs_bs_result cs_bs_bind(const cs_bm_map *m, const cs_bs_span *s,
                               uint32_t n, uint8_t *first, uint8_t *cnt)
{
    uint32_t i, k, at = 0UL, run;

    for (i = 0; i < m->bank_count; ++i) {
        run = 0UL;
        first[i] = (uint8_t)at;
        cnt[i] = 0u;
        for (k = 0; k < 3u && run < m->banks[i].bytes; ++k) {
            const cs_bs_span *p;

            if (at >= n) {
                return CS_BS_BINDING;
            }
            p = &s[at];
            if (p->bank != m->banks[i].id || p->offset != run ||
                p->bytes == 0UL || p->bytes > CS_BS_OBJECT_MAX ||
                p->data == NULL || p->bytes > m->banks[i].bytes - run) {
                return CS_BS_BINDING;
            }
            run += p->bytes;
            ++at;
            cnt[i] = (uint8_t)(k + 1u);
        }
        if (run != m->banks[i].bytes) {
            return CS_BS_BINDING;
        }
    }
    return at == n ? CS_BS_OK : CS_BS_BINDING;
}

/* Resolves all four destinations; each pointer add stays inside one span. */
static cs_bs_result cs_bs_resolve(const cs_bm_map *m, const cs_bs_span *s,
                                  const uint8_t *first, const uint8_t *cnt,
                                  const cs_bm_plan *plan, uint8_t **dst,
                                  uint8_t *val)
{
    uint32_t p, j, q, bi, off, t = 0UL;

    for (p = 0; p < plan->count && p < 4u; ++p) {
        const cs_bm_piece *pc = &plan->pieces[p];

        for (bi = 0; bi < m->bank_count && m->banks[bi].id != pc->bank; ++bi) {
        }
        if (bi >= m->bank_count) {
            return CS_BS_BINDING;
        }
        for (j = 0; j < pc->length && j < 4u; ++j) {
            uint8_t *found = NULL;

            if (t >= 4UL) {
                return CS_BS_BINDING;
            }
            off = pc->offset + j;
            for (q = 0; q < cnt[bi]; ++q) {
                const cs_bs_span *sp = &s[(uint32_t)first[bi] + q];

                if (off >= sp->offset && off - sp->offset < sp->bytes) {
                    found = sp->data + (uint16_t)(off - sp->offset);
                    break;
                }
            }
            if (found == NULL) {
                return CS_BS_BINDING;
            }
            dst[t] = found;
            val[t] = pc->data[j];
            ++t;
        }
    }
    return t == 4UL ? CS_BS_OK : CS_BS_BINDING;
}

cs_bs_result cs_bs_store4(const cs_bm_map *map, const cs_bc_bank *catalog,
    uint16_t catalog_count, uint32_t ram_first, uint32_t ram_bytes,
    const cs_bs_span *spans, uint16_t span_count,
    uint32_t current_epoch, uint32_t expected_epoch,
    uint32_t at, uint32_t value)
{
    cs_bm_plan plan;
    cs_bc_result cr;
    cs_bm_result pr;
    cs_bs_result br;
    uint8_t first[64], cnt[64], val[4];
    uint8_t *dst[4];
    uint32_t i;

    if (map == NULL || catalog == NULL || spans == NULL || span_count == 0u ||
        span_count > CS_BS_MAX_SPANS || current_epoch == 0UL ||
        expected_epoch == 0UL) {
        return CS_BS_ARGUMENT;
    }
    cr = cs_bc_validate(map, catalog, catalog_count, ram_first, ram_bytes);
    if (cr == CS_BC_ARGUMENT) {
        return CS_BS_ARGUMENT;
    }
    if (cr == CS_BC_MAP) {
        return CS_BS_MAP;
    }
    if (cr != CS_BC_OK) {
        return CS_BS_BACKING;
    }
    if (current_epoch != expected_epoch) {
        return CS_BS_STALE;
    }
    pr = cs_bm_plan4(map, at, value, &plan);
    if (pr == CS_BM_ARGUMENT) {
        return CS_BS_ARGUMENT;
    }
    if (pr == CS_BM_MAP) {
        return CS_BS_MAP;
    }
    if (pr == CS_BM_ADDRESS) {
        return CS_BS_ADDRESS;
    }
    if (pr != CS_BM_OK) {
        return CS_BS_PERMISSION;
    }
    br = cs_bs_bind(map, spans, span_count, first, cnt);
    if (br != CS_BS_OK) {
        return br;
    }
    br = cs_bs_resolve(map, spans, first, cnt, &plan, dst, val);
    if (br != CS_BS_OK) {
        return br;
    }
    for (i = 0; i < 4u; ++i) {
        *dst[i] = val[i];
    }
    return CS_BS_OK;
}

cs_bs_result cs_bs_epoch_next(uint32_t current, uint32_t *out)
{
    if (out == NULL) {
        return CS_BS_ARGUMENT;
    }
    if (current == 0xFFFFFFFFUL) {
        return CS_BS_EXHAUSTED;
    }
    *out = current + 1UL;
    return CS_BS_OK;
}
