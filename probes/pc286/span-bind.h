/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_SPAN_BIND_H
#define CLASSICK_PC286_SPAN_BIND_H
#include "store4-model.h"

/* PROPOSED shared validation for consumers of SPEC-0082 bound spans (checked
 * access; capability borrow bridge). One binding walk, same rules as store4.
 * Caller obligations of SPEC-0082 and P1-P6 apply unchanged: truthful live
 * extents, physical nonaliasing, exclusive stability from validation through
 * the last native access, disjoint metadata/outputs, and every C16 metadata
 * array fitting one addressable segment window. */
typedef struct cs_bx_env {
    const cs_bm_map *map;
    const cs_bc_bank *catalog;
    const cs_bs_span *spans;
    uint32_t ram_first, ram_bytes, current_epoch, expected_epoch;
    uint16_t catalog_count, span_count;
} cs_bx_env;

/* Model-only results; existing guest/bm/bc/bs/bk enums are unchanged. */
typedef enum cs_bx_result {
    CS_BX_OK = 0, CS_BX_ARGUMENT = 1, CS_BX_MAP = 2, CS_BX_BACKING = 3,
    CS_BX_STALE = 4, CS_BX_WIDTH = 5, CS_BX_ADDRESS = 6,
    CS_BX_PERMISSION = 7, CS_BX_BINDING = 8
} cs_bx_result;

/* cs_sb_env is a checked entry point. Its argument must be NULL or point to a
 * valid, readable environment object whose array pointers and counts are
 * truthful and reachable; it does not validate arbitrary native pointers and
 * is not a security boundary. It refuses in order: NULL env or spans/catalog, span_count 0 or > 192, zero
 * epoch: ARGUMENT; then cs_bc_validate ARGUMENT/MAP/BACKING; then
 * current != expected: STALE. It reads, but cannot validate, the truthful
 * extents of the arrays it is given. */
cs_bx_result cs_sb_env(const cs_bx_env *e);

/* BACKEND-INTERNAL HELPERS. Not safe arbitrary-input entry points; callers
 * (cs_bx_read/write, cs_bq_borrow) establish every precondition first.
 *
 * cs_sb_walk preconditions: m has passed cs_bm_validate (bank_count 1..64);
 * s points to n truthful, live span records (n <= 192) that are stable and
 * disjoint from first/cnt; first and cnt each point to >= 64 writable bytes.
 * Result: BINDING unless spans follow map bank-array order, 1..3 per bank
 * from offset 0, each 1..32767 bytes with non-NULL data, covering each bank
 * exactly with no extras. Fills first/cnt. Metadata only: no data access. */
cs_bx_result cs_sb_walk(const cs_bm_map *m, const cs_bs_span *s, uint16_t n,
                        uint8_t *first, uint8_t *cnt);
/* cs_sb_find preconditions: the immediately preceding cs_sb_walk on the SAME
 * map/spans returned OK and produced these first/cnt, all unchanged since;
 * b < map bank_count; off < that bank's bytes. Returns the native byte of
 * (bank index b, bank offset off) and the bytes left in its span (NULL only
 * if a precondition was violated). One 16-bit in-span add; no other effect. */
uint8_t *cs_sb_find(const cs_bs_span *s, const uint8_t *first, const uint8_t *cnt,
                    uint16_t b, uint32_t off, uint32_t *available);
#endif
