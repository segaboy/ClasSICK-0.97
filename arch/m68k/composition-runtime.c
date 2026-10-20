/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original arithmetic;SPEC-0054/SRC-0081. */
#include "runtime.h"

/* Isolated composition closure only. Magnitudes use unsigned arithmetic so
 * INT32_MIN is safe. Remainder magnitude is <=INT32_MAX for every nonzero b.
 * Zero divisor uses the existing unsigned helper's trap;MIN/-1 yields zero. */
int32_t __modsi3(int32_t a,int32_t b)
{
    uint32_t ua=(uint32_t)a,ub=(uint32_t)b,r;
    if(a<0)ua=0u-ua;
    if(b<0)ub=0u-ub;
    r=__umodsi3(ua,ub);
    return a<0?-(int32_t)r:(int32_t)r;
}
