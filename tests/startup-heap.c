/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../personality/system1/startup-heap.h"
#include "../personality/system1/startup-globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); \
} } while (0)

static void allocation(cs_startup_heap *heap, uint32_t current, uint32_t limit,
    uint16_t trap, uint32_t size, uint32_t expected)
{
    cs_startup_heap previous = *heap;
    cs_mm_reply reply = {22, 33, 44};
    CHECK(cs_startup_new_ptr(heap, current, limit, trap, size, &reply));
    CHECK(reply.value == expected && reply.moved == 0);
    CHECK(reply.code == (expected ? CS_MM_NO_ERROR : CS_MM_FULL));
    if (!expected) {
        CHECK(heap->system_next == previous.system_next);
        CHECK(heap->application_next == previous.application_next);
    }
}

static void refused(cs_startup_heap *heap, uint32_t current, uint32_t limit,
    uint16_t trap)
{
    cs_startup_heap previous = *heap;
    cs_mm_reply reply = {22, 33, 44};
    CHECK(!cs_startup_new_ptr(heap, current, limit, trap, 2, &reply));
    CHECK(reply.code == 22 && reply.value == 33 && reply.moved == 44);
    CHECK(heap->system_next == previous.system_next);
    CHECK(heap->application_next == previous.application_next);
}

static void globals(void)
{
    static const struct { unsigned address; uint32_t value; } locations[] = {
        {0x114, 0x1a700}, {0x118, 0x10000}, {0x130, 0x1a700},
        {0x2a6, 0xa000}, {0x2aa, 0x10000}, {0x31a, 0xffffff}
    };
    uint8_t actual[CS_MAC128K_GLOBALS_BYTES + 2];
    uint8_t expected[sizeof actual];
    memset(actual, 0xcc, sizeof actual);
    memset(expected, 0xcc, sizeof expected);
    for (unsigned i = 0; i < sizeof locations / sizeof locations[0]; ++i) {
        unsigned at = 1 + locations[i].address - CS_MAC128K_GLOBALS_BASE;
        for (unsigned n = 0; n < 4; ++n)
            expected[at + n] = (uint8_t)(locations[i].value >> (24 - 8*n));
    }
    cs_startup_heap_globals(actual + 1);
    /* MemErr describes a completed operation. Seeding the zones preserves it. */
    CHECK(actual[1 + 0x220 - CS_MAC128K_GLOBALS_BASE] == 0xcc);
    CHECK(actual[1 + 0x221 - CS_MAC128K_GLOBALS_BASE] == 0xcc);
    CHECK(memcmp(actual, expected, sizeof actual) == 0);
}

int main(void)
{
    const uint32_t app = 0x10000, sys = 0xa000, end = 0x1a700;
    cs_startup_heap heap;
    globals();
    cs_startup_heap_init(&heap);
    allocation(&heap, app, end, 0xa11e, 5, 0x10080);
    allocation(&heap, app, end, 0xa31e, 0, 0x10086);
    allocation(&heap, sys, end, 0xa11e, 3, 0xa080);
    allocation(&heap, app, end, 0xa71e, 0, 0xa084);
    allocation(&heap, app, end, 0xa51e, 2, 0xa086);
    allocation(&heap, app, end, 0xa11e, UINT32_MAX, 0);
    allocation(&heap, app, end, 0xa31e, UINT32_C(0x80000000), 0);
    allocation(&heap, app, end, 0xa31e, UINT32_C(0x7fffffff), 0);
    allocation(&heap, app, 0x1008a, 0xa31e, 3, 0);
    allocation(&heap, app, 0x1008a, 0xa31e, 1, 0x10088);
    allocation(&heap, app, 0x1008a, 0xa11e, 0, 0);
    allocation(&heap, app, 0x10080, 0xa11e, 1, 0);
    allocation(&heap, app, 0x10080, 0xa51e, 0x5f78, 0xa088);
    allocation(&heap, sys, end, 0xa11e, 0, 0);
    refused(&heap, app + 2, end, 0xa11e);
    refused(&heap, app, end + 2, 0xa11e);
    refused(&heap, app, end - 1, 0xa11e);
    refused(&heap, app, end, 0xa01e);
    refused(&heap, app, end, 0xa91e);
    allocation(&heap, app, end, 0xa11e, end - 0x1008a, 0x1008a);
    allocation(&heap, app, end, 0xa11e, 1, 0);
    return 0;
}
