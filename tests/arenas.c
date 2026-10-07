/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); return 0; \
} } while (0)

static int same_arena(cs_arena a, cs_arena b)
{
    return a.storage == b.storage && a.capacity == b.capacity && a.used == b.used;
}

static int layout(void)
{
    _Alignas(max_align_t) unsigned char bytes[32];
    unsigned char expected[32];
    cs_arena arena;
    cs_arena_span span, old;
    memset(bytes, 0xA5, sizeof(bytes)); memset(expected, 0xA5, sizeof(expected));
    CHECK(CS_ARENA_MAX_ALIGNMENT >= 4);
    CHECK(cs_arena_init(&arena, bytes, 13) == CS_ARENA_OK);
    CHECK(cs_arena_alloc(&arena, 3, 1, &span) == CS_ARENA_OK);
    CHECK(span.data == bytes && span.size == 3 && arena.used == 3);
    span.data[0] = 0x11; span.data[1] = 0x12; span.data[2] = 0x13;
    expected[0] = 0x11; expected[1] = 0x12; expected[2] = 0x13;
    CHECK(cs_arena_alloc(&arena, 2, 4, &span) == CS_ARENA_OK);
    CHECK(span.data == bytes + 4 && span.size == 2 && arena.used == 6);
    span.data[0] = 0x21; span.data[1] = 0x22;
    expected[4] = 0x21; expected[5] = 0x22;
    CHECK(cs_arena_alloc(&arena, 5, 4, &span) == CS_ARENA_OK);
    CHECK(span.data == bytes + 8 && span.size == 5 && arena.used == 13);
    memset(span.data, 0x33, span.size); memset(expected + 8, 0x33, 5);
    CHECK(memcmp(bytes, expected, sizeof(bytes)) == 0);
    old = span;
    CHECK(cs_arena_alloc(&arena, 1, 1, &span) == CS_ARENA_ERR_EXHAUSTED);
    CHECK(span.data == old.data && span.size == old.size && arena.used == 13);
    CHECK(cs_arena_reset(&arena) == CS_ARENA_OK && arena.used == 0);
    CHECK(memcmp(bytes, expected, sizeof(bytes)) == 0);
    /* Old spans are now retired; fresh reservation deliberately reuses bytes. */
    CHECK(cs_arena_alloc(&arena, 13, 1, &span) == CS_ARENA_OK);
    CHECK(span.data == bytes && span.size == 13 && arena.used == 13);
    CHECK(memcmp(bytes, expected, sizeof(bytes)) == 0);
    CHECK(cs_arena_init(&arena, NULL, 0) == CS_ARENA_OK);
    CHECK(cs_arena_alloc(&arena, 1, 1, &span) == CS_ARENA_ERR_EXHAUSTED);
    CHECK(cs_arena_reset(&arena) == CS_ARENA_OK);
    CHECK(cs_arena_init(&arena, bytes, 0) == CS_ARENA_OK);
    CHECK(cs_arena_alloc(&arena, 1, 1, &span) == CS_ARENA_ERR_EXHAUSTED);
    return 1;
}

static int reject(cs_arena *arena, size_t size, size_t alignment,
    cs_arena_result result)
{
    cs_arena before = *arena;
    cs_arena_span span = {arena->storage, 987};
    CHECK(cs_arena_alloc(arena, size, alignment, &span) == result);
    CHECK(same_arena(*arena, before));
    CHECK(span.data == before.storage && span.size == 987);
    return 1;
}

static int errors(void)
{
    _Alignas(max_align_t) unsigned char bytes[64];
    unsigned char expected[64];
    cs_arena arena = {bytes, 64, 3}, before = arena;
    cs_arena_span span = {bytes, 99};
    memset(bytes, 0x6B, sizeof(bytes)); memset(expected, 0x6B, sizeof(expected));
    CHECK(cs_arena_init(NULL, NULL, SIZE_MAX) == CS_ARENA_ERR_ARGUMENT);
    CHECK(cs_arena_init(&arena, NULL, SIZE_MAX) == CS_ARENA_ERR_OVERFLOW);
    CHECK(same_arena(arena, before));
    CHECK(cs_arena_init(&arena, NULL, 1) == CS_ARENA_ERR_ARGUMENT);
    CHECK(same_arena(arena, before));
    CHECK(cs_arena_init(&arena, bytes, (size_t)PTRDIFF_MAX + 1u) == CS_ARENA_ERR_OVERFLOW);
    CHECK(same_arena(arena, before));
    CHECK(cs_arena_alloc(NULL, 0, 0, &span) == CS_ARENA_ERR_ARGUMENT);
    CHECK(span.data == bytes && span.size == 99);
    CHECK(cs_arena_alloc(&arena, 1, 1, NULL) == CS_ARENA_ERR_ARGUMENT);
    CHECK(same_arena(arena, before));
    CHECK(cs_arena_reset(NULL) == CS_ARENA_ERR_ARGUMENT);
    CHECK(reject(&arena, 0, 0, CS_ARENA_ERR_ARGUMENT));
    CHECK(reject(&arena, 1, 0, CS_ARENA_ERR_ALIGNMENT));
    CHECK(reject(&arena, 1, 3, CS_ARENA_ERR_ALIGNMENT));
    CHECK(reject(&arena, SIZE_MAX, SIZE_MAX, CS_ARENA_ERR_ALIGNMENT));
    CHECK(reject(&arena, 1, CS_ARENA_MAX_ALIGNMENT * 2u, CS_ARENA_ERR_ALIGNMENT));
    CHECK(reject(&arena, SIZE_MAX, 1, CS_ARENA_ERR_OVERFLOW));
    /* Small real backing and valid used=3 suffice to test cumulative overflow. */
    CHECK(reject(&arena, (size_t)PTRDIFF_MAX, CS_ARENA_MAX_ALIGNMENT,
        CS_ARENA_ERR_OVERFLOW));
    CHECK(reject(&arena, 62, 1, CS_ARENA_ERR_EXHAUSTED));
    CHECK(cs_arena_alloc(&arena, 1, 1, &span) == CS_ARENA_OK);
    CHECK(span.data == bytes + 3 && arena.used == 4);
    /* Arithmetic-valid maximal request is ordinary exhaustion at used=0. */
    CHECK(cs_arena_reset(&arena) == CS_ARENA_OK);
    CHECK(reject(&arena, (size_t)PTRDIFF_MAX, 1, CS_ARENA_ERR_EXHAUSTED));
    {
        const cs_arena invalid[] = {
            {bytes, 64, 65}, {NULL, 1, 0}, {bytes, SIZE_MAX, 0}, {NULL, 0, 1}
        };
        for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
            arena = invalid[i]; before = arena;
            CHECK(reject(&arena, 0, 0, CS_ARENA_ERR_STATE));
            CHECK(cs_arena_alloc(&arena, 1, 1, NULL) == CS_ARENA_ERR_ARGUMENT);
            CHECK(cs_arena_reset(&arena) == CS_ARENA_ERR_STATE);
            CHECK(same_arena(arena, before));
        }
    }
    CHECK(memcmp(bytes, expected, sizeof(bytes)) == 0);
    return 1;
}

static int matrix(void)
{
    _Alignas(max_align_t) unsigned char backing[CS_ARENA_MAX_ALIGNMENT + 96];
    unsigned char expected[sizeof(backing)];
    unsigned char *base = backing + CS_ARENA_MAX_ALIGNMENT;
    size_t cases = 0;
    for (size_t capacity = 0; capacity <= 65; ++capacity) {
        for (size_t used = 0; used <= capacity; ++used) {
            for (size_t alignment = 1; alignment <= CS_ARENA_MAX_ALIGNMENT;
                    alignment *= 2u) {
                for (size_t size = 0; size <= 67; ++size) {
                    cs_arena arena, before;
                    cs_arena_span span = {base + 80, 123}, prefix;
                    cs_arena_result wanted;
                    size_t start = used;
                    memset(backing, 0xA5, sizeof(backing));
                    memset(expected, 0xA5, sizeof(expected));
                    CHECK(cs_arena_init(&arena, base, capacity) == CS_ARENA_OK);
                    if (used != 0)
                        CHECK(cs_arena_alloc(&arena, used, 1, &prefix) == CS_ARENA_OK);
                    before = arena;
                    /* Independent oracle: scan offsets, no bit-rounding helper. */
                    while (start % alignment != 0) ++start;
                    wanted = size == 0 ? CS_ARENA_ERR_ARGUMENT
                        : start + size > capacity ? CS_ARENA_ERR_EXHAUSTED : CS_ARENA_OK;
                    CHECK(cs_arena_alloc(&arena, size, alignment, &span) == wanted);
                    CHECK(memcmp(backing, expected, sizeof(backing)) == 0);
                    CHECK(arena.storage == base && arena.capacity == capacity);
                    if (wanted == CS_ARENA_OK) {
                        CHECK(span.data == base + start && span.size == size);
                        CHECK(arena.used == start + size);
                        CHECK((uintptr_t)span.data % alignment == 0);
                        memset(span.data, 0x37, span.size);
                        for (size_t index = start; index < start + size; ++index)
                            expected[CS_ARENA_MAX_ALIGNMENT + index] = 0x37;
                    } else {
                        CHECK(same_arena(arena, before));
                        CHECK(span.data == base + 80 && span.size == 123);
                    }
                    CHECK(memcmp(backing, expected, sizeof(backing)) == 0);
                    ++cases;
                }
            }
        }
    }
    printf("SPEC-0003 independent arena matrix: %zu whole-buffer cases PASS\n", cases);
    return 1;
}

static int ownership(void)
{
    _Alignas(max_align_t) unsigned char left[32], right[32];
    cs_arena a, b;
    cs_arena_span x, y, next;
    unsigned char expected_left[32], expected_right[32];
    memset(left, 0x81, sizeof(left)); memset(right, 0x92, sizeof(right));
    memset(expected_left, 0x81, sizeof(left)); memset(expected_right, 0x92, sizeof(right));
    CHECK(cs_arena_init(&a, left, sizeof(left)) == CS_ARENA_OK);
    CHECK(cs_arena_init(&b, right, sizeof(right)) == CS_ARENA_OK);
    CHECK(cs_arena_alloc(&a, 5, 1, &x) == CS_ARENA_OK);
    CHECK(cs_arena_alloc(&b, 7, 1, &y) == CS_ARENA_OK);
    memset(x.data, 0x23, x.size); memset(expected_left, 0x23, 5);
    memset(y.data, 0x45, y.size); memset(expected_right, 0x45, 7);
    CHECK(cs_arena_alloc(&a, 3, 4, &next) == CS_ARENA_OK);
    CHECK(next.data == left + 8 && a.used == 11 && b.used == 7);
    memset(next.data, 0x67, next.size); memset(expected_left + 8, 0x67, 3);
    CHECK(memcmp(left, expected_left, sizeof(left)) == 0);
    CHECK(memcmp(right, expected_right, sizeof(right)) == 0);
    CHECK(cs_arena_reset(&a) == CS_ARENA_OK && a.used == 0 && b.used == 7);
    CHECK(memcmp(left, expected_left, sizeof(left)) == 0);
    CHECK(memcmp(right, expected_right, sizeof(right)) == 0);
    CHECK(cs_arena_alloc(&b, 1, 1, &next) == CS_ARENA_OK && next.data == right + 7);
    /* Typed native storage is exercised only over allocated, undeclared backing. */
    {
        typedef struct { uint32_t tag; size_t count; void *link; } record;
        void *pool = malloc(128);
        cs_arena native;
        record *value;
        int passed;
        if (pool == NULL) return 0;
        passed = cs_arena_init(&native, pool, 128) == CS_ARENA_OK
            && cs_arena_alloc(&native, 1, 1, &x) == CS_ARENA_OK
            && cs_arena_alloc(&native, sizeof(record), _Alignof(record), &y) == CS_ARENA_OK;
        if (passed) {
            value = (record *)(void *)y.data;
            value->tag = UINT32_C(0x12345678); value->count = 17; value->link = pool;
            passed = value->tag == UINT32_C(0x12345678) && value->count == 17
                && value->link == pool && (uintptr_t)y.data % _Alignof(record) == 0;
        }
        free(pool);
        CHECK(passed);
    }
    return 1;
}

int main(int argc, char **argv)
{
    int passed;
    if (argc != 2) return 2;
    if (strcmp(argv[1], "layout") == 0) passed = layout();
    else if (strcmp(argv[1], "errors") == 0) passed = errors();
    else if (strcmp(argv[1], "matrix") == 0) passed = matrix();
    else if (strcmp(argv[1], "ownership") == 0) passed = ownership();
    else return 2;
    if (passed) printf("SPEC-0003 %s: PASS\n", argv[1]);
    return passed ? 0 : 1;
}
