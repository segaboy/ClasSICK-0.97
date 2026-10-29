/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "span-build.h"

/* The ONLY representation step. Open Watcom 16-bit: integer-to-far-pointer
 * conversion with the segment in the high word. This is implementation-
 * defined; the pinned-compiler listing must qualify it before any claim.
 * Near-data models cannot hold a span pointer and refuse to compile. */
#if defined(__WATCOMC__) && defined(_M_I86)
CS_C16_ASSERT(cs_bk_data_pointer_is_far, sizeof(uint8_t *) == 4);
#define CS_BK_TARGET 1
static uint8_t *cs_bk_native(uint8_t *arena, uint16_t seg, uint16_t off)
{
    (void)arena;
    return (uint8_t *)(((uint32_t)seg << 16) | (uint32_t)off);
}
#else
#define CS_BK_TARGET 0
/* Host simulation: one flat array stands in for real-mode linear memory. */
static uint8_t *cs_bk_native(uint8_t *arena, uint16_t seg, uint16_t off)
{
    return arena + (((uint32_t)seg << 4) + (uint32_t)off);
}
#endif

static const cs_bc_bank *cs_bk_entry(const cs_bc_bank *c, uint16_t n, uint32_t id)
{
    uint16_t i;
    for (i = 0u; i < n; ++i) {
        if (c[i].id == id) {
            return &c[i];
        }
    }
    return NULL;
}

/* physical_base - native_offset must be a whole paragraph below 1 MiB. */
static int cs_bk_segment(const cs_bc_bank *b, uint16_t *segment)
{
    uint32_t base;

    if ((b->physical_base == 0UL && b->native_offset == 0UL) ||
        b->native_offset > b->physical_base) {
        return 0;
    }
    base = b->physical_base - b->native_offset;
    if ((base & 0xFUL) != 0UL || base >= CS_BK_REAL_TOP) {
        return 0;
    }
    *segment = (uint16_t)(base >> 4);
    return 1;
}

static uint32_t cs_bk_piece(uint32_t left)
{
    return left > CS_BK_CHUNK_MAX ? CS_BK_CHUNK_MAX : left;
}

cs_bk_result cs_bk_plan(const cs_bm_map *map, const cs_bc_bank *catalog,
    uint16_t catalog_count, uint32_t ram_first, uint32_t ram_bytes,
    cs_bk_chunk *out, uint16_t capacity, uint16_t *count)
{
    cs_bc_result cr;
    uint16_t i, seg, total = 0u, at = 0u;
    uint32_t run;

    if (out == NULL || count == NULL) {
        return CS_BK_ARGUMENT;
    }
    cr = cs_bc_validate(map, catalog, catalog_count, ram_first, ram_bytes);
    if (cr == CS_BC_ARGUMENT) {
        return CS_BK_ARGUMENT;
    }
    if (cr == CS_BC_MAP) {
        return CS_BK_MAP;
    }
    if (cr != CS_BC_OK) {
        return CS_BK_BACKING;
    }
    /* Pass 1: representation and capacity only; no output effect. */
    for (i = 0u; i < map->bank_count; ++i) {
        const cs_bc_bank *b = cs_bk_entry(catalog, catalog_count, map->banks[i].id);

        if (b == NULL) {
            return CS_BK_BACKING; /* unreachable after cs_bc_validate */
        }
        if (!cs_bk_segment(b, &seg)) {
            return CS_BK_REPRESENTATION;
        }
        for (run = 0UL; run < map->banks[i].bytes; run += cs_bk_piece(map->banks[i].bytes - run)) {
            ++total; /* <= 3 per bank, <= 64 banks: never exceeds 192 */
        }
    }
    if (total > capacity) {
        return CS_BK_CAPACITY;
    }
    /* Pass 2: emit. Catalog admission gives native_offset + bytes <= 65536. */
    for (i = 0u; i < map->bank_count; ++i) {
        const cs_bc_bank *b = cs_bk_entry(catalog, catalog_count, map->banks[i].id);

        (void)cs_bk_segment(b, &seg);
        for (run = 0UL; run < map->banks[i].bytes; ++at) {
            uint32_t len = cs_bk_piece(map->banks[i].bytes - run);

            out[at].bank = map->banks[i].id;
            out[at].offset = run;
            out[at].bytes = len;
            out[at].segment = seg;
            out[at].native_offset = (uint16_t)(b->native_offset + run);
            run += len;
        }
    }
    *count = at;
    return CS_BK_OK;
}

cs_bk_result cs_bk_bind(const cs_bk_chunk *chunks, uint16_t count,
    uint8_t *arena, cs_bs_span *out, uint16_t capacity)
{
    uint16_t i;

    if (chunks == NULL || out == NULL || count == 0u ||
        (CS_BK_TARGET ? arena != NULL : arena == NULL)) {
        return CS_BK_ARGUMENT;
    }
    if (count > capacity) {
        return CS_BK_CAPACITY;
    }
    /* Every chunk is checked before the first output is written. */
    for (i = 0u; i < count; ++i) {
        const cs_bk_chunk *c = &chunks[i];
        uint32_t start = ((uint32_t)c->segment << 4) + (uint32_t)c->native_offset;

        if ((c->segment == 0u && c->native_offset == 0u) ||
            c->bytes == 0UL || c->bytes > CS_BK_CHUNK_MAX ||
            c->bytes > 65536UL - (uint32_t)c->native_offset ||
            start >= CS_BK_REAL_TOP || c->bytes > CS_BK_REAL_TOP - start) {
            return CS_BK_REPRESENTATION;
        }
    }
    for (i = 0u; i < count; ++i) {
        out[i].bank = chunks[i].bank;
        out[i].offset = chunks[i].offset;
        out[i].bytes = chunks[i].bytes;
        out[i].data = cs_bk_native(arena, chunks[i].segment, chunks[i].native_offset);
    }
    return CS_BK_OK;
}
