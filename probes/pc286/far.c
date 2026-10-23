/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Far data and code probe, C90 plus target __near/__far. Compile only.
 * In the listing, check offset-only arithmetic, how compares are coded,
 * and whether calls are near or far. */
typedef char far_data4[(sizeof(unsigned char __far *) == 4) ? 1 : -1];
typedef char near_data2[(sizeof(unsigned char __near *) == 2) ? 1 : -1];

unsigned char __far *far_add(unsigned char __far *p, unsigned n)
{ return p + n; }
/* 32-bit index: does the compiler silently drop the high word? */
unsigned char far_load32(const unsigned char __far *p, unsigned long i)
{ return p[i]; }
unsigned char far_load(const unsigned char __far *p, unsigned i)
{ return p[i]; }
int far_lt(const unsigned char __far *a, const unsigned char __far *b)
{ return a < b; }
int far_eq(const unsigned char __far *a, const unsigned char __far *b)
{ return a == b; }
unsigned far_diff(const unsigned char __far *a, const unsigned char __far *b)
{ return (unsigned)(a - b); }

int __far far_callee(int x) { return x + 1; }
int __near near_callee(int x) { return x + 2; }
int far_caller(int x) { return far_callee(x); }
int near_caller(int x) { return near_callee(x); }
/* Deliberately unresolved far import: diagnostic only, never linked. */
extern int __far far_import(int x);
int import_caller(int x) { return far_import(x); }
int (__far *far_fnptr)(int) = far_callee;
const unsigned far_fnptr_size = sizeof(int (__far *)(int));
