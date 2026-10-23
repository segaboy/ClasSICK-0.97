/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* C11 dialect probe, isolated: _Static_assert and _Alignof. */
_Static_assert(sizeof(int) == 2, "int must be 16 bits");
_Static_assert(sizeof(long) == 4, "long must be 32 bits");
_Static_assert((0x10000UL & ~7UL) != 0, "wide mask keeps bit 16");
_Static_assert(sizeof(char __far *) == 4, "far data pointer is 4 bytes");
const unsigned c11_align_long = _Alignof(long);
const unsigned c11_align_short = _Alignof(short);
const unsigned c11_align_farp = _Alignof(char __far *);
