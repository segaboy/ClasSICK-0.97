/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "runtime.h"

uint32_t __mulsi3(uint32_t a, uint32_t b)
{
    uint32_t product = 0;
    /* Remaining zero multiplier bits cannot contribute to the product. */
    while (b != 0u) {
        if ((b & 1u) != 0u) product += a;
        a <<= 1u;
        b >>= 1u;
    }
    return product;
}

static uint32_t divide(uint32_t a, uint32_t b, uint32_t *remainder)
{
    uint32_t q = 0, r = 0;
    if (b == 0u) __builtin_trap();
    for (unsigned i = 32; i != 0u; --i) {
        uint32_t carry = r >> 31u;
        r = (r << 1u) | ((a >> (i - 1u)) & 1u);
        if (carry != 0u || r >= b) {
            r -= b;
            q |= UINT32_C(1) << (i - 1u);
        }
    }
    *remainder = r;
    return q;
}

uint32_t __udivsi3(uint32_t a, uint32_t b)
{
    uint32_t r;
    return divide(a, b, &r);
}

uint32_t __umodsi3(uint32_t a, uint32_t b)
{
    uint32_t r;
    (void)divide(a, b, &r);
    return r;
}

int32_t __divsi3(int32_t a, int32_t b)
{
    uint32_t ua = (uint32_t)a, ub = (uint32_t)b, r, q;
    int negative = (a < 0) != (b < 0);
    if (a < 0) ua = 0u - ua;
    if (b < 0) ub = 0u - ub;
    q = divide(ua, ub, &r);
    if (q == UINT32_C(0x80000000)) return INT32_MIN;
    return negative ? -(int32_t)q : (int32_t)q;
}

void *memcpy(void *destination, const void *source, size_t count)
{
    unsigned char *d = destination;
    const unsigned char *s = source;
    for (size_t i = 0; i < count; ++i) d[i] = s[i];
    return destination;
}
