/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Generated expected values are supplied by the hosted independent oracle. */
#include "../../arch/i286/c16.h"
#include <stdint.h>
#include "arithmetic-cases.h"
#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires 16-bit Open Watcom
#endif
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char value) {post(value);for (;;) {}}
void start(void)
{
    unsigned i;
    for(i=0;i<ARITH_COUNT;i++) {
        volatile uint32_t a=arith[i].a,b=arith[i].b;
        volatile int32_t x=(int32_t)a,y=(int32_t)b;
        if(a*b!=arith[i].product)fail(0xc1);
        if(a/b!=arith[i].uq || a%b!=arith[i].ur)fail(0xc2);
        if(x/y!=(int32_t)arith[i].sq || x%y!=(int32_t)arith[i].sr)fail(0xc3);
    }
#ifdef CS_BAD_EXPECTATION
    if(arith[0].uq!=arith[0].uq+1u)fail(0xcd);
#endif
    post(0xad);for (;;) {}
}
