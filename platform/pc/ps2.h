/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_PS2_H
#define CLASSICK_PC_PS2_H
#include "../../core/input/input.h"
enum { CS_PS2_ACTIVE=0, CS_PS2_READY=1, CS_PS2_ARGUMENT=2, CS_PS2_STATE=3,
    CS_PS2_TIMEOUT=4, CS_PS2_CONTROLLER=5, CS_PS2_DEVICE=6, CS_PS2_PROTOCOL=7,
    CS_PS2_QUEUE=8, CS_PS2_PHASE_READY=30, CS_PS2_PHASE_POLLS=1000000,
    CS_PS2_SHORT_TICKS=357955, CS_PS2_TEST_TICKS=1789773, CS_PS2_BAT_TICKS=3579545 };
typedef uint8_t (*cs_ps2_read)(void *context,uint16_t port);
typedef void (*cs_ps2_write)(void *context,uint16_t port,uint8_t value);
typedef struct { cs_ps2_read read; cs_ps2_write write; void *context; } cs_ps2_io;
typedef struct {
    uint64_t since,last_tick;
    uint32_t phase,result,polls,started,configuration,retries;
    uint32_t drained,auxiliary,errors,dropped,resends,events;
    uint32_t held,release,extended,pause,last_byte;
} cs_ps2;
/* Live disjoint state/callbacks/queue; monotonic extended PM ticks. SPEC-0012. */
uint32_t cs_ps2_begin(cs_ps2 *state);
uint32_t cs_ps2_poll(cs_ps2 *state,const cs_ps2_io *io,uint64_t now,cs_input_queue *queue);
#endif
