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
/* Diagnostic-only: with MOTION=1, ignore PACKET deliveries while the total decoded-packet
 * counter is <=1. Observes the first nonzero packet whose total counter is >=2. */
#ifndef CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET
#define CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET 0
#endif
#if (CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET+0)!=0 && (CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET+0)!=1
#error CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET must be 0 or 1
#endif
_Static_assert(CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET==0 || CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET==1,
    "Skip-first-packet must be numeric 0 or 1");
#if (CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET+0)==1 && (CLASSICK_PC_INPUT_PROBE_MOTION+0)!=1
#error CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET=1 requires CLASSICK_PC_INPUT_PROBE_MOTION=1
#endif
/* Diagnostic-only: 1 captures the second-or-later PACKET even when dx and dy
 * are both zero (magic CSPRPT01, raw-mode register layout). Requires
 * RAW_PACKET_PROBE=1, MOTION=1 and SKIP_FIRST_PACKET=1. */
#ifndef CLASSICK_PC_INPUT_PROBE_REPORT
#define CLASSICK_PC_INPUT_PROBE_REPORT 0
#endif
#if (CLASSICK_PC_INPUT_PROBE_REPORT+0)!=0 && (CLASSICK_PC_INPUT_PROBE_REPORT+0)!=1
#error CLASSICK_PC_INPUT_PROBE_REPORT must be 0 or 1
#endif
_Static_assert(CLASSICK_PC_INPUT_PROBE_REPORT==0 || CLASSICK_PC_INPUT_PROBE_REPORT==1,
    "Report probe must be numeric 0 or 1");
#if (CLASSICK_PC_INPUT_PROBE_REPORT+0)==1 && ((CLASSICK_PC_RAW_PACKET_PROBE+0)!=1 || \
    (CLASSICK_PC_INPUT_PROBE_MOTION+0)!=1 || (CLASSICK_PC_INPUT_PROBE_SKIP_FIRST_PACKET+0)!=1)
#error CLASSICK_PC_INPUT_PROBE_REPORT=1 requires RAW_PACKET_PROBE=1, MOTION=1 and SKIP_FIRST_PACKET=1
#endif
#define CS_PC_INPUT_PROBE_MOTION_MAGIC UINT64_C(0x4353504D4F543031)
#define CS_PC_INPUT_PROBE_REPORT_MAGIC UINT64_C(0x4353505250543031)
/* Ignores context/trace; s must be the live const session. Mode0 stops on
 * LOST; mode1 on a PACKET with nonzero motion (REPORT: any packet after the
 * first). Other paths return0. */
int cs_pc_input_probe_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s);
/* Native backend loads values[0..7] into r8..r15 and spins forever. */
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8]);
#endif
