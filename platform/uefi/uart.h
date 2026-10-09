/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_UART_H
#define CLASSICK_UEFI_UART_H
#include "keyboard.h"
#include "../pc/uart.h"
enum { CS_UART_TRACE_OFFSET=256,CS_UART_TRACE_BYTES=640,CS_UART_TRACE_HEADER=128,
    CS_UART_TRACE_MAGIC=0x31555043 };
/* SPEC-0013 v2: the termination drain may empty a full 512-byte ring at 9600 baud. */
enum { CS_UART_DRAIN_TICKS=3579545, CS_UART_DRAIN_POLLS=4000000 };
_Static_assert(CS_UART_TRACE_OFFSET>=CS_KBD_TRACE_OFFSET+CS_KBD_TRACE_BYTES,"UART trace follows keyboard");
_Static_assert(CS_UART_TRACE_OFFSET+CS_UART_TRACE_BYTES<=CS_LOADER_TRACE_BYTES,"UART trace fits bundle");
_Static_assert(CS_QUAL_TRACE_OFFSET>=CS_UART_TRACE_OFFSET+CS_UART_TRACE_BYTES
    && CS_QUAL_CELL_OFFSET==CS_QUAL_TRACE_OFFSET+CS_QUAL_TRACE_BYTES
    && CS_QUAL_CELL_OFFSET+CS_QUAL_CELL_BYTES<=CS_LOADER_TRACE_BYTES,"Qualification follows UART trace");
/* SPEC-0013 v2 / SPEC-0016: qualification is required (see keyboard.h). */
uint32_t cs_native_uart_loop(const cs_uefi_handoff *,uint32_t ready,
    const cs_native_devices *,const cs_ps2_io *,const cs_uart_io *,
    uint32_t base,uint32_t divisor,uint32_t seconds,unsigned char *keyboard_trace,unsigned char *uart_trace,
    unsigned char *qualification);
#endif
