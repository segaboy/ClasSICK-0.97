/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CORE_MEMORY_ARENA_H
#define CLASSICK_CORE_MEMORY_ARENA_H

#include <stddef.h>

#define CS_ARENA_MAX_ALIGNMENT ((size_t)_Alignof(max_align_t))

typedef enum {
    CS_ARENA_OK = 0,
    CS_ARENA_ERR_ARGUMENT = 1,
    CS_ARENA_ERR_STATE = 2,
    CS_ARENA_ERR_ALIGNMENT = 3,
    CS_ARENA_ERR_OVERFLOW = 4,
    CS_ARENA_ERR_EXHAUSTED = 5
} cs_arena_result;

/* Native state, never a guest record. Do not copy a live arena to create aliases. */
typedef struct {
    unsigned char *storage;
    size_t capacity;
    size_t used;
} cs_arena;

typedef struct {
    unsigned char *data;
    size_t size;
} cs_arena_span;

/* Caller guarantees max_align_t-aligned, live, disjoint, truthful storage. */
cs_arena_result cs_arena_init(cs_arena *out, void *storage, size_t capacity);

/* Positive size, power-of-two alignment <= MAX_ALIGNMENT. No byte initialization.
 * Errors preserve arena/output/backing bytes. See SPEC-0003 for all preconditions. */
cs_arena_result cs_arena_alloc(cs_arena *arena, size_t size, size_t alignment,
    cs_arena_span *out);

/* Retire all spans before reset. Bytes survive; the next allocation may reuse them. */
cs_arena_result cs_arena_reset(cs_arena *arena);

#endif
