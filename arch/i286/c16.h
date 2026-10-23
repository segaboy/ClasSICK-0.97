/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_I286_C16_H
#define CLASSICK_I286_C16_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/* Owned C99 constraint, not a reserved-keyword substitute. Unique name required. */
#define CS_C16_ASSERT(name, predicate) typedef char name[(predicate) ? 1 : -1]
CS_C16_ASSERT(cs_c16_byte_is_8, CHAR_BIT == 8);
CS_C16_ASSERT(cs_c16_u32_is_32, sizeof(uint32_t) == 4);
#ifdef CS_C16_REQUIRE_16BIT
CS_C16_ASSERT(cs_c16_int_is_16, sizeof(int) == 2);
CS_C16_ASSERT(cs_c16_size_is_16, sizeof(size_t) == 2);
CS_C16_ASSERT(cs_c16_long_is_32, sizeof(long) == 4);
#endif

/* One native chunk only. Larger logical guest operations need the future
 * segmented backend. Zero is representable. Failure leaves *out unchanged.
 * The sizeof guards make each narrowed maximum meaningful on the host too. */
static inline int cs_c16_chunk_size(uint32_t length, size_t *out)
{
    if (out == NULL ||
        (sizeof(size_t) < sizeof(uint32_t) && length > (uint32_t)SIZE_MAX) ||
        (sizeof(ptrdiff_t) <= sizeof(uint32_t) && length > (uint32_t)PTRDIFF_MAX)) {
        return 0;
    }
    *out = (size_t)length;
    return 1;
}

static inline uint32_t cs_c16_clear_flags(uint32_t flags, uint32_t mask)
{
    return flags & (uint32_t)~mask;
}

/* Numeric guest-offset divisibility only: no C type alignment query, native
 * pointer cast, physical-address ordering, or far-pointer alias proof. */
static inline int cs_c16_offset_multiple(uint32_t offset, uint16_t requirement)
{
    if (requirement != 1u && requirement != 2u && requirement != 4u) {
        return 0;
    }
    return (offset & (uint32_t)(requirement - 1u)) == 0u;
}
#endif
