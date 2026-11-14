/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Exercise real surface validation with 16-bit size_t and ptrdiff_t. */
#include "../../arch/i286/c16.h"
#include "../../core/graphics/surface.h"
#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires 16-bit Open Watcom
#endif
CS_C16_ASSERT(surface_size_t_is_16, sizeof(size_t) == 2);
static unsigned char byte;
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void check(cs_surface_result actual, cs_surface_result expected, unsigned index)
{
    if (actual != expected) { post((unsigned char)(0xc0u + index)); for (;;) {} }
}
void start(void)
{
    cs_surface s;
    check(cs_surface_init(&s,0,1,8,1,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_ARGUMENT,0);
    check(cs_surface_init(&s,&byte,1,8,1,1,(cs_surface_format)0),CS_SURFACE_ERR_FORMAT,1);
    check(cs_surface_init(&s,&byte,1,0,1,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_DIMENSION,2);
    check(cs_surface_init(&s,&byte,1,8,1,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_OK,3);
    check(cs_surface_init(&s,&byte,1,9,1,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_STRIDE,4);
    check(cs_surface_init(&s,&byte,1,9,1,2,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_STORAGE,5);
    check(cs_surface_init(&s,&byte,65535u,524281L,1,65535u,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_OVERFLOW,6);
    check(cs_surface_init(&s,&byte,65535u,524280L,1,65535u,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_OVERFLOW,7);
    check(cs_surface_init(&s,&byte,65535u,16384L,1,65535u,CS_SURFACE_RGBA8),CS_SURFACE_ERR_OVERFLOW,8);
    check(cs_surface_init(&s,&byte,32767u,1,32767L,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_OK,9);
    check(cs_surface_init(&s,&byte,32768u,1,32768L,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_OVERFLOW,10);
    check(cs_surface_init(&s,&byte,65535u,1,65536L,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_OVERFLOW,11);
    check(cs_surface_init(&s,&byte,65535u,1,2147483647L,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_OVERFLOW,12);
#ifdef CS_BAD_EXPECTATION
    check(cs_surface_init(&s,&byte,1,8,1,1,CS_SURFACE_MONO1_MSB),CS_SURFACE_ERR_STORAGE,13);
#endif
    post(0xaf);for (;;) {}
}
