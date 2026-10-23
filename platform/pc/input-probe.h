/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_INPUT_PROBE_H
#define CLASSICK_PC_INPUT_PROBE_H
#include <stdint.h>
#include "window-session.h"
/* Diagnostic-only first-LOST register mailbox. Never in normal images. */
#define CS_PC_INPUT_PROBE_MAGIC UINT64_C(0x435350524F424531)
enum { CS_PC_INPUT_PROBE_WORDS=8 };
/* Ignores context/trace; s must be the live const session. Returns 0 unless
 * delivery.kind is LOST, in which case it never returns. */
int cs_pc_input_probe_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s);
/* Native backend loads values[0..7] into r8..r15 and spins forever. */
__attribute__((noreturn)) void cs_pc_input_probe_halt(const uint64_t values[8]);
#endif
