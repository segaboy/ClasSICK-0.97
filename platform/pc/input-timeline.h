/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_INPUT_TIMELINE_H
#define CLASSICK_PC_INPUT_TIMELINE_H
#include <stdint.h>
#include "input-probe.h"
#ifndef CLASSICK_PC_INPUT_TIMELINE
#define CLASSICK_PC_INPUT_TIMELINE 0
#endif
#if (CLASSICK_PC_INPUT_TIMELINE+0)!=0 && (CLASSICK_PC_INPUT_TIMELINE+0)!=1
#error CLASSICK_PC_INPUT_TIMELINE must be 0 or 1
#endif
_Static_assert(CLASSICK_PC_INPUT_TIMELINE==0 || CLASSICK_PC_INPUT_TIMELINE==1,
    "PC input timeline must be numeric 0 or 1");
/* Diagnostic-only, opt-in READY AUX timeline observer (magic CSPTIM01). Never
 * linked into normal images; reads the live session, never writes it. */
#define CS_PC_INPUT_TIMELINE_MAGIC UINT64_C(0x43535054494D3031)
#define CS_PC_INPUT_TIMELINE_STARTED 0x4D495443u
enum { CS_PC_INPUT_TIMELINE_ENTRIES=6 };
/* Caller-owned recorder. Only begin and observe write it. Not copied or
 * moved after begin. entries[] are valid below count only. */
typedef struct {
    uint32_t magic, primed, was_ready, count, gaps, unreliable, baseline;
    uint32_t auxiliary, errors, used;
    uint64_t entries[CS_PC_INPUT_TIMELINE_ENTRIES];
} cs_pc_input_timeline;
/* Returns 0 on success, 1 on NULL. */
int cs_pc_input_timeline_begin(cs_pc_input_timeline *recorder);
/* context must be a begun recorder; trace is ignored. NULL/unstarted recorder
 * or NULL/uninitialized session returns 0 with no effects. Halts through
 * cs_pc_input_probe_halt after 6 entries, on a session fault, or on a
 * post-READY drained marker. This observer must be called on a failed poll
 * to see its fault; an ordinary successful-poll-only caller cannot do that. */
int cs_pc_input_timeline_observe(void *context,const unsigned char *trace,
    const cs_pc_window_session *s);
#endif
