/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_BANKED_GUEST_H
#define CLASSICK_PC286_BANKED_GUEST_H
#include "../../arch/i286/c16.h"

#define CS_PG_REGIONS 8u
#define CS_PG_BANKS 64u
#define CS_PG_READ 1UL
#define CS_PG_WRITE 2UL
#define CS_PG_EXECUTE 4UL

typedef enum {
    CS_PG_OK = 0, CS_PG_ARGUMENT = 1, CS_PG_STATE = 2, CS_PG_STALE = 3,
    CS_PG_MAP = 4, CS_PG_BACKING = 5, CS_PG_WIDTH = 6,
    CS_PG_ADDRESS = 7, CS_PG_PERMISSION = 8, CS_PG_EXHAUSTED = 9
} cs_pg_result;

typedef struct {
    uint32_t base, bytes, flags;
    uint16_t first_bank, bank_count;
} cs_pg_region;

typedef struct {
    uint16_t id, segment, offset, bytes;
} cs_pg_bank;

/* Caller-owned opaque state after zero initialization. Never copy, erase or
 * edit an active lifetime. Native input/output objects are live and disjoint;
 * owner reserves stable exclusive conventional RAM. SPEC-0117 defines limits. */
typedef struct cs_pc286_guest {
    uint32_t tag, generation, arena_first, arena_bytes;
    const struct cs_pc286_guest *self;
    uint16_t active, cs, ds, ss, region_count, bank_count;
    cs_pg_region regions[CS_PG_REGIONS];
    cs_pg_bank banks[CS_PG_BANKS];
} cs_pc286_guest;

cs_pg_result cs_pc286_guest_bind(cs_pc286_guest *service,
    const cs_pg_region *regions, uint16_t region_count,
    const cs_pg_bank *banks, uint16_t bank_count,
    uint32_t arena_first, uint32_t arena_bytes, uint32_t *generation);
cs_pg_result cs_pc286_guest_retire(cs_pc286_guest *service, uint32_t generation);
/* Numeric-only admission for a stable scalar transaction owner. Neither
 * operation reads backing or publishes a borrowed native pointer. */
cs_pg_result cs_pc286_guest_validate(const cs_pc286_guest *service, uint32_t generation);
cs_pg_result cs_pc286_guest_span(const cs_pc286_guest *service, uint32_t generation,
    uint32_t address, uint32_t bytes, uint32_t access);
cs_pg_result cs_pc286_guest_read(const cs_pc286_guest *service, uint32_t generation,
    uint32_t address, unsigned width, uint32_t access, uint32_t *out);
cs_pg_result cs_pc286_guest_write(const cs_pc286_guest *service, uint32_t generation,
    uint32_t address, unsigned width, uint32_t value);
#endif
