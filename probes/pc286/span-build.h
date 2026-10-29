/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_SPAN_BUILD_H
#define CLASSICK_PC286_SPAN_BUILD_H
#include "store4-model.h"

/* SPEC-0082 experimental construction under SPEC-0069: build store4 spans from an admitted numeric
 * catalog under an explicit real-address-mode relation (engineering input):
 *     physical_base == segment * 16 + native_offset.
 * Planning is numeric only. Binding performs the single representation step.
 * Tuple0:0 is refused to preserve the non-NULL span domain. Caller owns
 * truthful extents,liveness,disjoint immutable metadata and stability;C16
 * metadata objects must fit one addressable segment window. */
#define CS_BK_CHUNK_MAX 32767UL
#define CS_BK_REAL_TOP 0x100000UL

typedef struct cs_bk_chunk {
    uint32_t bank, offset, bytes;     /* same meaning as cs_bs_span */
    uint16_t segment, native_offset;  /* real-mode parts of the first byte */
} cs_bk_chunk;

typedef enum cs_bk_result {
    CS_BK_OK = 0, CS_BK_ARGUMENT = 1, CS_BK_MAP = 2, CS_BK_BACKING = 3,
    CS_BK_REPRESENTATION = 4, CS_BK_CAPACITY = 5
} cs_bk_result;

/* Chunks in map bank-array order, 1..3 per bank, each <= 32767 bytes and
 * inside one 64 KiB segment window. Refusal leaves out/count unchanged. */
cs_bk_result cs_bk_plan(const cs_bm_map *map, const cs_bc_bank *catalog,
    uint16_t catalog_count, uint32_t ram_first, uint32_t ram_bytes,
    cs_bk_chunk *out, uint16_t capacity, uint16_t *count);

/* Re-checks every chunk, then converts all of them. Host builds map into a
 * caller-owned simulated real-address space of CS_BK_REAL_TOP bytes;
 * Open Watcom 16-bit builds require arena == NULL and far data pointers.
 * Refusal leaves out unchanged. */
cs_bk_result cs_bk_bind(const cs_bk_chunk *chunks, uint16_t count,
    uint8_t *arena, cs_bs_span *out, uint16_t capacity);
#endif
