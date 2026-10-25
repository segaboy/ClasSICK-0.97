/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "bank-plan.h"

#define CS_BM_SPACE 0x1000000UL
#define CS_BM_READ 1UL
#define CS_BM_WRITE 2UL

CS_C16_ASSERT(cs_bm_plan_fits_80, sizeof(cs_bm_plan) <= 80);

cs_bm_result cs_bm_validate(const cs_bm_map *m)
{
    uint8_t seen[64];
    uint32_t i, j, k, sum, end;

    if (m == NULL) {
        return CS_BM_ARGUMENT;
    }
    if (m->regions == NULL || m->banks == NULL || m->region_count == 0u ||
        m->region_count > 8u || m->bank_count == 0u || m->bank_count > 64u) {
        return CS_BM_MAP;
    }
    for (i = 0; i < m->bank_count; ++i) {
        seen[i] = 0u;
        if (m->banks[i].id < 1UL || m->banks[i].id > 64UL ||
            m->banks[i].bytes < 1UL || m->banks[i].bytes > 65535UL) {
            return CS_BM_MAP;
        }
        for (j = 0; j < i; ++j) {
            if (m->banks[j].id == m->banks[i].id) {
                return CS_BM_MAP;
            }
        }
    }
    for (i = 0; i < m->region_count; ++i) {
        const cs_bm_region *r = &m->regions[i];

        if (r->bytes == 0UL || ((r->base | r->bytes) & 1UL) != 0UL ||
            r->base >= CS_BM_SPACE || r->bytes > CS_BM_SPACE - r->base ||
            (r->flags & 0xFFFFFFF8UL) != 0UL || (r->flags & CS_BM_READ) == 0UL) {
            return CS_BM_MAP;
        }
        for (j = 0; j < i; ++j) {
            const cs_bm_region *p = &m->regions[j];

            if (!(r->base + r->bytes <= p->base || p->base + p->bytes <= r->base)) {
                return CS_BM_MAP;
            }
        }
        end = (uint32_t)r->first_bank + (uint32_t)r->bank_count;
        if (r->bank_count == 0u || end > m->bank_count) {
            return CS_BM_MAP;
        }
        sum = 0UL;
        for (k = r->first_bank; k < end; ++k) {
            if (seen[k] != 0u) {
                return CS_BM_MAP;
            }
            seen[k] = 1u;
            sum += m->banks[k].bytes;
        }
        if (sum != r->bytes) {
            return CS_BM_MAP;
        }
    }
    for (i = 0; i < m->bank_count; ++i) {
        if (seen[i] == 0u) {
            return CS_BM_MAP;
        }
    }
    return CS_BM_OK;
}

cs_bm_result cs_bm_check(const cs_bm_map *m, uint32_t at, uint32_t bytes,
                         uint32_t access)
{
    cs_bm_result st;
    uint32_t i;

    if (access == 0UL || (access & 0xFFFFFFF8UL) != 0UL) {
        return CS_BM_ARGUMENT;
    }
    st = cs_bm_validate(m);
    if (st != CS_BM_OK) {
        return st;
    }
    if (bytes == 0UL || at >= CS_BM_SPACE || bytes > CS_BM_SPACE - at) {
        return CS_BM_ADDRESS;
    }
    for (i = 0; i < m->region_count; ++i) {
        const cs_bm_region *r = &m->regions[i];

        if (at >= r->base && bytes <= r->bytes && at - r->base <= r->bytes - bytes) {
            return (r->flags & access) == access ? CS_BM_OK : CS_BM_PERMISSION;
        }
    }
    return CS_BM_ADDRESS;
}

cs_bm_result cs_bm_plan4(const cs_bm_map *m, uint32_t at, uint32_t value,
                         cs_bm_plan *out)
{
    cs_bm_plan p;
    uint8_t b[4];
    const cs_bm_region *r = NULL;
    cs_bm_result st;
    uint32_t i, k, pos, off, n, room, left;

    if (out == NULL) {
        return CS_BM_ARGUMENT;
    }
    st = cs_bm_validate(m);
    if (st != CS_BM_OK) {
        return st;
    }
    if ((at & 1UL) != 0UL || at > CS_BM_SPACE - 4UL) {
        return CS_BM_ADDRESS;
    }
    for (i = 0; i < m->region_count; ++i) {
        const cs_bm_region *c = &m->regions[i];

        if (at >= c->base && c->bytes >= 4UL && at - c->base <= c->bytes - 4UL) {
            r = c;
            break;
        }
    }
    if (r == NULL) {
        return CS_BM_ADDRESS;
    }
    if ((r->flags & CS_BM_WRITE) == 0UL) {
        return CS_BM_PERMISSION;
    }

    p.count = 0UL;
    for (i = 0; i < 4u; ++i) {
        p.pieces[i].bank = 0UL;
        p.pieces[i].offset = 0UL;
        p.pieces[i].length = 0UL;
        for (k = 0; k < 4u; ++k) {
            p.pieces[i].data[k] = 0u;
        }
    }
    b[0] = (uint8_t)((value >> 24) & 0xFFu);
    b[1] = (uint8_t)((value >> 16) & 0xFFu);
    b[2] = (uint8_t)((value >> 8) & 0xFFu);
    b[3] = (uint8_t)(value & 0xFFu);

    off = at - r->base;
    i = r->first_bank;
    for (k = 0; k < r->bank_count && off >= m->banks[i].bytes; ++k) {
        off -= m->banks[i].bytes;
        ++i;
    }
    pos = 0UL;
    for (k = 0; k < 4u && pos < 4UL; ++k) {
        room = m->banks[i].bytes - off;
        left = 4UL - pos;
        n = room < left ? room : left;
        p.pieces[k].bank = m->banks[i].id;
        p.pieces[k].offset = off;
        p.pieces[k].length = n;
        for (left = 0; left < n; ++left) {
            p.pieces[k].data[left] = b[pos + left];
        }
        p.count = k + 1UL;
        pos += n;
        ++i;
        off = 0UL;
    }

    out->count = p.count;
    for (i = 0; i < 4u; ++i) {
        out->pieces[i].bank = p.pieces[i].bank;
        out->pieces[i].offset = p.pieces[i].offset;
        out->pieces[i].length = p.pieces[i].length;
        for (k = 0; k < 4u; ++k) {
            out->pieces[i].data[k] = p.pieces[i].data[k];
        }
    }
    return CS_BM_OK;
}
