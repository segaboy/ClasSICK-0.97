/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Compile only; no target entry point. */
#define CS_C16_REQUIRE_16BIT
#include "../../arch/i286/c16.h"
CS_C16_ASSERT(cs_probe_high_mask,
    (0x80010007UL & ~7UL) == 0x80010000UL);
#ifdef CS_C16_FALSE_ASSERT
CS_C16_ASSERT(cs_probe_expected_refusal, sizeof(int) == 4);
#endif
int cs_probe_c16_chunk(uint32_t n, size_t *out)
{
    return cs_c16_chunk_size(n, out);
}
uint32_t cs_probe_c16_flags(uint32_t n, uint32_t mask)
{
    return cs_c16_clear_flags(n, mask);
}
int cs_probe_c16_offset(uint32_t n, uint16_t alignment)
{
    return cs_c16_offset_multiple(n, alignment);
}
