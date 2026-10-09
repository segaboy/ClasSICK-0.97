/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_KEYBOARD_H
#define CLASSICK_UEFI_KEYBOARD_H
#include "native.h"
#include "../pc/ps2.h"
enum { CS_KBD_OK=0, CS_KBD_ARGUMENT=1, CS_KBD_NOT_EXITED=2, CS_KBD_NOT_READY=3,
    CS_KBD_TIMER=4, CS_KBD_TARGET=5, CS_KBD_ARENA=6, CS_KBD_DRAW=7,
    CS_KBD_VALUE=8, CS_KBD_STALLED=9, CS_KBD_NO_8042=10, CS_KBD_KEYBOARD=11,
    CS_KBD_ESCAPED=12, CS_KBD_RUNNING=13, CS_KBD_MACHINE=14 };
enum { CS_KBD_TRACE_MAGIC=0x314B5043, CS_KBD_TRACE_VERSION=1,
    CS_KBD_TRACE_BYTES=112, CS_KBD_TRACE_OFFSET=128 };
/* SPEC-0016 qualification record at bundle trace offset 1024, followed by the
   8-byte owned probe cell at 1152. Alias outcomes: PASS, ALIAS (bit-20 masking
   observed, fail closed), UNAVAILABLE (partner not eligible RAM) or NOT_RUN. */
enum { CS_QUAL_TRACE_MAGIC=0x31515043, CS_QUAL_TRACE_VERSION=1, CS_QUAL_TRACE_BYTES=128,
    CS_QUAL_TRACE_OFFSET=1024, CS_QUAL_CELL_OFFSET=1152, CS_QUAL_CELL_BYTES=8,
    CS_QUAL_PASS=0, CS_QUAL_ALIAS=1, CS_QUAL_UNAVAILABLE=2, CS_QUAL_NOT_RUN=3 };
/* SPEC-0013: notice 0=timer turn, 1=termination, 2=frame snapshot. */
typedef void (*cs_keyboard_observer)(void *context,uint64_t ticks,
    const unsigned char *snapshot,uint32_t final);
/* qualification: null (no probe) or the identity view of trace_base+1024 with
   136 live disjoint bytes; record before the cell. SPEC-0016. */
uint32_t cs_native_keyboard_observed(const cs_uefi_handoff *handoff,uint32_t ready,
    const cs_native_devices *devices,const cs_ps2_io *keyboard,uint32_t seconds,unsigned char *trace,
    unsigned char *qualification,cs_keyboard_observer observer,void *context);
/* 112 disjoint live trace bytes; devices/ports/arena obey SPEC-0012. */
uint32_t cs_native_keyboard_loop(const cs_uefi_handoff *handoff,uint32_t ready,
    const cs_native_devices *devices,const cs_ps2_io *keyboard,uint32_t seconds,unsigned char *trace);
#endif
