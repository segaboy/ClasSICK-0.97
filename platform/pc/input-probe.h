/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_INPUT_PROBE_H
#define CLASSICK_PC_INPUT_PROBE_H
#include <stdint.h>
#include "window-session.h"
/* Diagnostic-only register mailbox. Never in normal images. */
#define CS_PC_INPUT_PROBE_MAGIC UINT64_C(0x435350524F424531)
enum { CS_PC_INPUT_PROBE_WORDS=8 };
/* Opt-in compile-time measurement: 0 keeps first-LOST, 1 stops on the first
 * PACKET with nonzero dx or dy (magic CSPMOT01). Nonreturning, diagnostic only. */
#ifndef CLASSICK_PC_INPUT_PROBE_MOTION
#define CLASSICK_PC_INPUT_PROBE_MOTION 0
#endif
#if (CLASSICK_PC_INPUT_PROBE_MOTION+0)!=0 && (CLASSICK_PC_INPUT_PROBE_MOTION+0)!=1
#error CLASSICK_PC_INPUT_PROBE_MOTION must be 0 or 1
#endif
_Static_assert(CLASSICK_PC_INPUT_PROBE_MOTION==0 || CLASSICK_PC_INPUT_PROBE_MOTION==1,
    "Input-probe mode must be numeric 0 or 1");
#define CS_PC_INPUT_PROBE_MOTION_MAGIC UINT64_C(0x4353504D4F543031)
/* Ignores context/trace; s must be the live const session. Mode0 stops on
 * LOST; mode1 on a PACKET with nonzero motion. Other paths return0. */
int cs_pc_input_probe_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s);
/* Native backend loads values[0..7] into r8..r15 and spins forever. */
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8]);
#endif
