/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original SPEC-0117 implementation. No production flat-view API is altered. */
#include "banked-guest.h"
#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This provider requires the admitted Open Watcom 16-bit target
#endif
CS_C16_ASSERT(cs_pg_near_data, sizeof(uint8_t *) == 2);
CS_C16_ASSERT(cs_pg_far_data, sizeof(uint8_t __far *) == 4);
CS_C16_ASSERT(cs_pg_native_object, sizeof(cs_pc286_guest) < 32767u);
#define CS_PG_TAG 0x50474231UL

static unsigned get_ds(void);
static unsigned get_ss(void);
static unsigned get_cs(void);
#pragma aux get_ds = "mov ax,ds" value [ax] modify exact [ax];
#pragma aux get_ss = "mov ax,ss" value [ax] modify exact [ax];
#pragma aux get_cs = "mov ax,cs" value [ax] modify exact [ax];

static int overlaps(uint32_t a, uint32_t an, uint32_t b, uint32_t bn)
{
    return a < b + bn && b < a + an;
}

static uint32_t physical(const cs_pg_bank *bank)
{
    return ((uint32_t)bank->segment << 4) + (uint32_t)bank->offset;
}

static cs_pg_result geometry(const cs_pg_region *regions, uint16_t nr,
    const cs_pg_bank *banks, uint16_t nb, uint32_t first, uint32_t bytes,
    uint16_t code, uint16_t data)
{
    uint8_t used[CS_PG_BANKS];
    uint16_t i, j, k;
    if (nr == 0u || nr > CS_PG_REGIONS || nb == 0u || nb > CS_PG_BANKS)
        return CS_PG_MAP;
    for (i = 0; i < nb; ++i) {
        used[i] = 0;
        if (banks[i].id == 0u || banks[i].id > CS_PG_BANKS || banks[i].bytes == 0u)
            return CS_PG_MAP;
        for (j = 0; j < i; ++j) if (banks[i].id == banks[j].id) return CS_PG_MAP;
    }
    for (i = 0; i < nr; ++i) {
        const cs_pg_region *r = &regions[i];
        uint32_t sum = 0;
        if (r->bytes == 0UL || ((r->base | r->bytes) & 1UL) != 0UL ||
            r->base >= 0x1000000UL || r->bytes > 0x1000000UL - r->base ||
            (r->flags & 0xfffffff8UL) != 0UL || (r->flags & CS_PG_READ) == 0UL ||
            r->bank_count == 0u || r->first_bank >= nb || r->bank_count > nb - r->first_bank)
            return CS_PG_MAP;
        for (j = 0; j < i; ++j)
            if (overlaps(r->base, r->bytes, regions[j].base, regions[j].bytes)) return CS_PG_MAP;
        for (k = r->first_bank; k < r->first_bank + r->bank_count; ++k) {
            if (used[k] != 0u) return CS_PG_MAP;
            used[k] = 1;
            sum += (uint32_t)banks[k].bytes;
        }
        if (sum != r->bytes) return CS_PG_MAP;
    }
    for (i = 0; i < nb; ++i) if (used[i] == 0u) return CS_PG_MAP;
    if (first < 0x1000UL || first >= 0xa0000UL || bytes == 0UL || bytes > 0xa0000UL - first)
        return CS_PG_BACKING;
    for (i = 0; i < nb; ++i) {
        uint32_t p = physical(&banks[i]);
        uint32_t length = banks[i].bytes;
        if (length > 0x10000UL - (uint32_t)banks[i].offset || p < first ||
            p > first + bytes || length > first + bytes - p ||
            overlaps(p, length, (uint32_t)code << 4, 0x10000UL) ||
            overlaps(p, length, (uint32_t)data << 4, 0x10000UL)) return CS_PG_BACKING;
        for (j = 0; j < i; ++j)
            if (overlaps(p, length, physical(&banks[j]), banks[j].bytes)) return CS_PG_BACKING;
    }
    return CS_PG_OK;
}

static cs_pg_result live(const cs_pc286_guest *s, uint32_t generation)
{
    if (s == NULL) return CS_PG_ARGUMENT;
    if (s->tag != CS_PG_TAG || s->self != s || s->cs != get_cs() ||
        s->ds != get_ds() || s->ss != get_ss() || s->ds != s->ss ||
        s->active > 1u || s->generation == 0UL) return CS_PG_STATE;
    if (s->active == 0u || generation != s->generation) return CS_PG_STALE;
    return geometry(s->regions, s->region_count, s->banks, s->bank_count,
        s->arena_first, s->arena_bytes, s->cs, s->ds);
}

cs_pg_result cs_pc286_guest_bind(cs_pc286_guest *s,
    const cs_pg_region *regions, uint16_t nr, const cs_pg_bank *banks, uint16_t nb,
    uint32_t first, uint32_t bytes, uint32_t *generation)
{
    cs_pg_result result;
    uint16_t i, code = (uint16_t)get_cs(), data = (uint16_t)get_ds(), stack = (uint16_t)get_ss();
    uint32_t previous;
    if (s == NULL || regions == NULL || banks == NULL || generation == NULL ||
        nr == 0u || nr > CS_PG_REGIONS || nb == 0u || nb > CS_PG_BANKS) return CS_PG_ARGUMENT;
    if (data != stack) return CS_PG_STATE;
    if (s->tag == 0UL) {
        if (s->generation != 0UL || s->active != 0u || s->self != NULL) return CS_PG_STATE;
        previous = 0;
    } else {
        if (s->tag != CS_PG_TAG || s->self != s || s->active != 0u || s->generation == 0UL ||
            s->cs != code || s->ds != data || s->ss != stack) return CS_PG_STATE;
        previous = s->generation;
    }
    result = geometry(regions, nr, banks, nb, first, bytes, code, data);
    if (result != CS_PG_OK) return result;
    if (previous == 0xffffffffUL) return CS_PG_EXHAUSTED;
    for (i = 0; i < nr; ++i) s->regions[i] = regions[i];
    for (i = 0; i < nb; ++i) s->banks[i] = banks[i];
    s->arena_first = first; s->arena_bytes = bytes;
    s->cs = code; s->ds = data; s->ss = stack; s->region_count = nr; s->bank_count = nb;
    s->generation = previous + 1UL; s->active = 1; s->self = s; s->tag = CS_PG_TAG;
    *generation = s->generation;
    return CS_PG_OK;
}

cs_pg_result cs_pc286_guest_retire(cs_pc286_guest *s, uint32_t generation)
{
    cs_pg_result result = live(s, generation);
    if (result != CS_PG_OK) return result;
    s->active = 0;
    return CS_PG_OK;
}

static cs_pg_result resolve(const cs_pc286_guest *s, uint32_t generation,
    uint32_t at, unsigned width, uint32_t access, volatile uint8_t __far **destinations)
{
    cs_pg_result result = live(s, generation);
    const cs_pg_region *region = NULL;
    uint16_t i;
    unsigned n;
    if (result != CS_PG_OK) return result;
    if (width != 1u && width != 2u && width != 4u) return CS_PG_WIDTH;
    if (width != 1u && (at & 1UL) != 0UL) return CS_PG_ADDRESS;
    if (at >= 0x1000000UL || (uint32_t)width > 0x1000000UL - at) return CS_PG_ADDRESS;
    for (i = 0; i < s->region_count; ++i) {
        const cs_pg_region *candidate = &s->regions[i];
        if (at >= candidate->base && width <= candidate->bytes &&
            at - candidate->base <= candidate->bytes - width) { region = candidate; break; }
    }
    if (region == NULL) return CS_PG_ADDRESS;
    if ((region->flags & access) != access) return CS_PG_PERMISSION;
    for (n = 0; n < width; ++n) {
        uint32_t offset = at + (uint32_t)n - region->base;
        uint16_t bank = region->first_bank;
        uint32_t encoded;
        while (offset >= s->banks[bank].bytes) { offset -= s->banks[bank].bytes; ++bank; }
        encoded = ((uint32_t)s->banks[bank].segment << 16) |
            ((uint32_t)s->banks[bank].offset + offset);
        destinations[n] = (volatile uint8_t __far *)encoded;
    }
    return CS_PG_OK;
}

cs_pg_result cs_pc286_guest_read(const cs_pc286_guest *s, uint32_t generation,
    uint32_t at, unsigned width, uint32_t access, uint32_t *out)
{
    volatile uint8_t __far *source[4];
    uint32_t value = 0;
    unsigned i;
    cs_pg_result result;
    if (out == NULL || (access & CS_PG_READ) == 0UL || (access & 0xfffffff8UL) != 0UL)
        return CS_PG_ARGUMENT;
    result = resolve(s, generation, at, width, access, source);
    if (result != CS_PG_OK) return result;
    for (i = 0; i < width; ++i) value = (value << 8) | (uint32_t)*source[i];
    *out = value;
    return CS_PG_OK;
}

cs_pg_result cs_pc286_guest_write(const cs_pc286_guest *s, uint32_t generation,
    uint32_t at, unsigned width, uint32_t value)
{
    volatile uint8_t __far *destinations[4];
    uint8_t staged[4];
    unsigned i;
    cs_pg_result result = resolve(s, generation, at, width, CS_PG_WRITE, destinations);
    if (result != CS_PG_OK) return result;
    staged[0] = (uint8_t)(value >> 24); staged[1] = (uint8_t)(value >> 16);
    staged[2] = (uint8_t)(value >> 8); staged[3] = (uint8_t)value;
    for (i = 0; i < width; ++i) *destinations[i] = staged[4u - width + i];
    return CS_PG_OK;
}
