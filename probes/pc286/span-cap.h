/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_SPAN_CAP_H
#define CLASSICK_PC286_SPAN_CAP_H
#include "span-bind.h"

/* PROPOSED C counterpart of the SPEC-0069 section 5 checked capability, plus
 * the native BORROW bridge to SPEC-0082 bound spans (pc286-span-cap-contract).
 *
 * Context rules (resolving review R2-1/R2-2):
 *  - A registry is initialized only from fresh zero-filled storage; a used
 *    registry refuses re-initialization (STATE). Lifetimes never recycle.
 *  - Each owner lifetime has ONE authoritative active binding environment.
 *    Changing it is only possible through cs_bq_owner_rebind, which advances
 *    the generation BEFORE the change. Borrow takes no environment argument:
 *    a capability can only ever borrow from its owner's current binding, and
 *    any rebind makes it STALE.
 *  - A capability records its owner, registry, lifetime and generation.
 *  - Caller duties: registry, owner, binding environment, map, catalog and
 *    spans stay alive, unmoved and unmodified while any capability that
 *    references them may still be used; their storage is not reused or
 *    re-zeroed meanwhile. The binding's spans change only via rebind.
 *  - ONE change rule (review R3-2): cs_bq_owner_advance only invalidates
 *    current capabilities; it never licenses a change. Every change to binding
 *    metadata or placement (map, catalog, spans, epoch, backing location) is
 *    made by building a fresh, stable binding and calling cs_bq_owner_rebind.
 *    Ordinary data writes through a permitted writable run are not binding
 *    changes. No registry, owner, binding, map or span storage is recycled
 *    while a capability that references it can still be used.
 * All fields are opaque; forging or editing them is outside the contract.
 * Counter checks do not establish physical ownership. */
#define CS_BQ_CHUNK_MAX 32767UL

typedef struct cs_bq_registry {
    uint32_t issued, current, limit;
} cs_bq_registry;

typedef struct cs_bq_owner {
    const cs_bm_map *map;
    const cs_bq_registry *registry;
    const cs_bx_env *binding;          /* the one active binding */
    uint32_t lifetime, generation, generation_limit;
} cs_bq_owner;

typedef struct cs_bq_cap {
    const cs_bq_owner *owner;
    const cs_bq_registry *registry;
    uint32_t lifetime, generation, access, offset, left;
    uint16_t bank;    /* index into map->banks */
    uint16_t released;
} cs_bq_cap;

typedef struct cs_bq_descriptor {
    uint32_t bank, offset, length;
} cs_bq_descriptor;

/* A borrowed native run. write is NULL unless WRITE was requested and held.
 * Valid only until the next call on the same capability, any owner advance,
 * rebind or new lifetime, release, or any callout/reentry. Never retain it. */
typedef struct cs_bq_run {
    const uint8_t *read;
    uint8_t *write;
    uint32_t bank, offset, length;
} cs_bq_run;

typedef enum cs_bq_result {
    CS_BQ_OK = 0, CS_BQ_ARGUMENT = 1, CS_BQ_MAP = 2, CS_BQ_ADDRESS = 3,
    CS_BQ_PERMISSION = 4, CS_BQ_STALE = 5, CS_BQ_CAPABILITY = 6,
    CS_BQ_EXHAUSTED = 7, CS_BQ_BACKING = 8, CS_BQ_BINDING = 9, CS_BQ_STATE = 10
} cs_bq_result;

cs_bq_result cs_bq_registry_init(cs_bq_registry *r, uint32_t limit);
cs_bq_result cs_bq_owner_open(cs_bq_registry *r, const cs_bx_env *binding,
                              uint32_t generation_limit, cs_bq_owner *out);
cs_bq_result cs_bq_owner_advance(cs_bq_owner *o);
cs_bq_result cs_bq_owner_rebind(cs_bq_owner *o, const cs_bx_env *binding);
cs_bq_result cs_bq_open(const cs_bq_owner *o, uint32_t at, uint32_t bytes,
                        uint32_t access, cs_bq_cap *out);
cs_bq_result cs_bq_chunk(cs_bq_cap *k, uint32_t limit, cs_bq_descriptor *out,
                         uint16_t *emitted);
cs_bq_result cs_bq_borrow(cs_bq_cap *k, uint32_t need, uint32_t limit,
                          cs_bq_run *out, uint16_t *emitted);
cs_bq_result cs_bq_rights(const cs_bq_cap *k, uint32_t *access);
cs_bq_result cs_bq_release(cs_bq_cap *k);
#endif
