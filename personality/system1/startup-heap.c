/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "startup-heap.h"

void cs_startup_heap_init(cs_startup_heap *heap)
{
    heap->system_next = CS_STARTUP_SYS_ZONE + CS_MM_CELL_BYTES;
    heap->application_next = CS_STARTUP_APP_ZONE + CS_MM_CELL_BYTES;
}

int cs_startup_new_ptr(cs_startup_heap *next, uint32_t current, uint32_t limit,
    uint16_t word, uint32_t size, cs_mm_reply *reply)
{
    if (next == NULL || reply == NULL || (word & 0xf9ffu) != 0xa11eu ||
        (current != CS_STARTUP_SYS_ZONE && current != CS_STARTUP_APP_ZONE) ||
        ((limit | next->application_next | next->system_next) & 1u) != 0 ||
        limit < CS_STARTUP_APP_ZONE + CS_MM_CELL_BYTES || limit > CS_STARTUP_HEAP_END ||
        next->application_next < CS_STARTUP_APP_ZONE + CS_MM_CELL_BYTES ||
        next->application_next > CS_STARTUP_HEAP_END ||
        next->system_next < CS_STARTUP_SYS_ZONE + CS_MM_CELL_BYTES ||
        next->system_next > CS_STARTUP_APP_ZONE)
        return 0;

    uint32_t *cursor = &next->application_next;
    if ((word & 0x0400u) || current == CS_STARTUP_SYS_ZONE) {
        cursor = &next->system_next;
        limit = CS_STARTUP_APP_ZONE;
    }
    /* Compare before rounding: negative, overflowing, and over-capacity sizes
     * all return memFullErr without wrapping or disturbing a prior allocation. */
    reply->moved = 0;
    reply->value = 0;
    reply->code = CS_MM_FULL;
    if (*cursor >= limit || size > limit - *cursor)
        return 1;
    uint32_t extent = size == 0 ? 2u : (size + 1u) & ~UINT32_C(1);
    reply->value = *cursor;
    reply->code = CS_MM_NO_ERROR;
    *cursor += extent;
    return 1;
}
