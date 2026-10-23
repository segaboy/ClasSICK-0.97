/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* C99 dialect probe, isolated: long long, for-declarations, mixed
 * declarations. No headers, so <stdint.h> is qualified separately. */
typedef char c99_ll64[(sizeof(long long) == 8) ? 1 : -1];
const unsigned c99_ll_size = sizeof(long long);

unsigned long c99_be(const unsigned char *p, unsigned n)
{
    unsigned long v = 0;
    for (unsigned i = 0; i < n; ++i) v = (v << 8) | p[i];
    unsigned long done = v;
    return done;
}
/* 64-bit helpers stay unresolved imports, if any are emitted. */
unsigned long long c99_mul64(unsigned long long a, unsigned long long b)
{ return a * b; }
unsigned long long c99_div64(unsigned long long a, unsigned long long b)
{ return a / b; }
