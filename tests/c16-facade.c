/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Host tests, not 286 execution. */
#include "../arch/i286/c16.h"
#include <stdio.h>

static unsigned checks;
static int within(uint32_t value, uintmax_t ceiling)
{
    return (uintmax_t)value <= ceiling;
}
#define CHECK(x) do { ++checks; if (!(x)) { \
    fprintf(stderr, "C16 host check failed at line %d\n", __LINE__); return 1; \
} } while (0)
int main(void)
{
    size_t out = 17u;
    const uint32_t limits[] = {0u, 1u, 32767u, 32768u, 65535u, 65536u,
                              70000u, UINT32_MAX};
    unsigned i;
    CHECK(cs_c16_clear_flags(UINT32_C(0x80010007), 7u) == UINT32_C(0x80010000));
    CHECK(cs_c16_clear_flags(UINT32_MAX, UINT32_C(0x80010000)) == UINT32_C(0x7ffeffff));
    CHECK(cs_c16_clear_flags(UINT32_MAX, 0u) == UINT32_MAX);
    CHECK(cs_c16_clear_flags(UINT32_MAX, UINT32_MAX) == 0u);
    CHECK(!cs_c16_chunk_size(1u, NULL));
    for (i = 0; i < sizeof(limits) / sizeof(limits[0]); ++i) {
        uint32_t n = limits[i];
        int expected = within(n, (uintmax_t)SIZE_MAX) &&
                       within(n, (uintmax_t)PTRDIFF_MAX);
        out = 17u;
        CHECK(cs_c16_chunk_size(n, &out) == expected);
        CHECK(out == (expected ? (size_t)n : 17u));
    }
    for (i = 0; i < 17u; ++i) {
        CHECK(cs_c16_offset_multiple(i, 1u));
        CHECK(cs_c16_offset_multiple(i, 2u) == (i % 2u == 0u));
        CHECK(cs_c16_offset_multiple(i, 4u) == (i % 4u == 0u));
    }
    CHECK(cs_c16_offset_multiple(UINT32_MAX - 3u, 4u));
    CHECK(!cs_c16_offset_multiple(UINT32_MAX, 2u));
    CHECK(!cs_c16_offset_multiple(0u, 0u));
    CHECK(!cs_c16_offset_multiple(0u, 3u));
    CHECK(!cs_c16_offset_multiple(0u, UINT16_MAX));
    printf("C16 host facade: %u checks passed; no target execution.\n", checks);
    return 0;
}
