/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "arena.h"
#include <stdint.h>

static int valid(const cs_arena *arena)
{
    return arena->capacity <= (size_t)PTRDIFF_MAX && arena->used <= arena->capacity
        && (arena->capacity == 0 || arena->storage != NULL);
}

cs_arena_result cs_arena_init(cs_arena *out, void *storage, size_t capacity)
{
    if (out == NULL) return CS_ARENA_ERR_ARGUMENT;
    if (capacity > (size_t)PTRDIFF_MAX) return CS_ARENA_ERR_OVERFLOW;
    if (storage == NULL && capacity != 0) return CS_ARENA_ERR_ARGUMENT;
    out->storage = storage;
    out->capacity = capacity;
    out->used = 0;
    return CS_ARENA_OK;
}

cs_arena_result cs_arena_alloc(cs_arena *arena, size_t size, size_t alignment,
    cs_arena_span *out)
{
    size_t padding, start;
    if (arena == NULL || out == NULL) return CS_ARENA_ERR_ARGUMENT;
    if (!valid(arena)) return CS_ARENA_ERR_STATE;
    if (size == 0) return CS_ARENA_ERR_ARGUMENT;
    if (alignment == 0 || alignment > CS_ARENA_MAX_ALIGNMENT
            || (alignment & (alignment - 1u)) != 0) return CS_ARENA_ERR_ALIGNMENT;
    padding = (alignment - (arena->used & (alignment - 1u))) & (alignment - 1u);
    if (padding > (size_t)PTRDIFF_MAX - arena->used) return CS_ARENA_ERR_OVERFLOW;
    start = arena->used + padding;
    if (size > (size_t)PTRDIFF_MAX - start) return CS_ARENA_ERR_OVERFLOW;
    if (start > arena->capacity || size > arena->capacity - start)
        return CS_ARENA_ERR_EXHAUSTED;
    out->data = arena->storage + start;
    out->size = size;
    arena->used = start + size;
    return CS_ARENA_OK;
}

cs_arena_result cs_arena_reset(cs_arena *arena)
{
    if (arena == NULL) return CS_ARENA_ERR_ARGUMENT;
    if (!valid(arena)) return CS_ARENA_ERR_STATE;
    arena->used = 0;
    return CS_ARENA_OK;
}
