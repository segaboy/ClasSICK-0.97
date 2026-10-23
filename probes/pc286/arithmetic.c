/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Mask, promotion and 32-bit helper probe, plain C90, no headers.
 * The const values are visible results for the lead to check in the listing.
 * They are constant-folded and prove nothing about execution. */

/* Old guest.c idiom: ~7u has unsigned int width. With a 16-bit int it is
 * 0xFFF8 and zero-extends, so high flag bits slip through. Expected: 0. */
const unsigned long mask_old_bit16 = 0x10000UL & ~7u;
const unsigned long mask_old_bit31 = 0x80000000UL & ~7u;
/* Corrected idiom keeps the high bits, so a stray bit is rejected. */
const unsigned long mask_new_bit16 = 0x10000UL & ~7UL;
const unsigned long mask_new_bit31 = 0x80000000UL & ~(unsigned long)7;
typedef char mask_new_rejects16[((0x10000UL & ~7UL) != 0) ? 1 : -1];
typedef char mask_new_rejects31[((0x80000000UL & ~7UL) != 0) ? 1 : -1];
typedef char mask_low_ok[((5UL & ~7UL) == 0) ? 1 : -1];

/* With a 16-bit int, unsigned short promotes to unsigned int and wraps. Expected: 0. */
const unsigned long promo_wrap = (unsigned short)0xFFFFu + 1u;
/* A 32-bit length stored in a 16-bit size type truncates. Expected: 0. */
const unsigned long size_trunc = (unsigned)0x20000UL;
/* 0x1000000u must become unsigned long, not truncate. */
typedef char limit_is_wide[(0x1000000u > 0xFFFFu) ? 1 : -1];
const unsigned limit_type_size = sizeof(0x1000000u);

/* Non-foldable 32-bit operations. Look in the listing for inline code or
 * helper calls. Any helper call stays an unresolved import, and this probe
 * is never linked against a runtime. */
unsigned long ar_add(unsigned long a, unsigned long b) { return a + b; }
unsigned long ar_sub(unsigned long a, unsigned long b) { return a - b; }
unsigned long ar_shl(unsigned long a, unsigned n) { return a << n; }
unsigned long ar_shr(unsigned long a, unsigned n) { return a >> n; }
unsigned long ar_mul(unsigned long a, unsigned long b) { return a * b; }
unsigned long ar_div(unsigned long a, unsigned long b) { return a / b; }
unsigned long ar_mod(unsigned long a, unsigned long b) { return a % b; }
long ar_sdiv(long a, long b) { return a / b; }
int ar_ult(unsigned long a, unsigned long b) { return a < b; }

/* Big-endian load and store, same shape as cs_guest_read/write. */
unsigned long ar_be32(const unsigned char *p)
{
    unsigned long v = 0;
    unsigned i;
    for (i = 0; i < 4u; ++i) v = (v << 8) | p[i];
    return v;
}
void ar_st32(unsigned char *p, unsigned long v)
{
    unsigned i;
    for (i = 0; i < 4u; ++i) p[i] = (unsigned char)(v >> ((3u - i) * 8u));
}
