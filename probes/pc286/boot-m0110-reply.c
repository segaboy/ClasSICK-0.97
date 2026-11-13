/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Original bounded portable-parser diagnostic; no keyboard device I/O. */
#include "../../arch/i286/c16.h"
#include "../../core/input/m0110-reply.h"
#if !defined(__WATCOMC__) || !defined(_M_I86)
#error This diagnostic requires the pinned 16-bit Open Watcom target
#endif
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void fail(unsigned char code) { post(code); for (;;) { } }
void start(void)
{
    cs_m0110_event e;
    if (cs_m0110_reply(1,&e)!=CS_M0110_EVENT || e.position!=0 || e.pressed!=1) fail(0xe1);
#ifdef CS_KEY_MUTATE_RELEASE
    if (cs_m0110_reply(0x81,&e)!=CS_M0110_EVENT || e.position!=0 || e.pressed!=1) fail(0xe3);
#else
    if (cs_m0110_reply(0x81,&e)!=CS_M0110_EVENT || e.position!=0 || e.pressed!=0) fail(0xe3);
#endif
    if (cs_m0110_reply(3,&e)!=CS_M0110_EVENT || e.position!=1 || e.pressed!=1 ||
        cs_m0110_reply(0x83,&e)!=CS_M0110_EVENT || e.position!=1 || e.pressed!=0) fail(0xe2);
    e.position=23; e.pressed=47;
#ifdef CS_KEY_MUTATE_INVALID
    if (cs_m0110_reply(2,&e)!=CS_M0110_EVENT) fail(0xe4);
#else
    if (cs_m0110_reply(2,&e)!=CS_M0110_INVALID) fail(0xe4);
#endif
    if (e.position!=23 || e.pressed!=47 || cs_m0110_reply(0x7b,&e)!=CS_M0110_EMPTY ||
        e.position!=23 || e.pressed!=47 || cs_m0110_reply(256,&e)!=CS_M0110_ARGUMENT) fail(0xe5);
    post(0xa3); for (;;) { }
}
