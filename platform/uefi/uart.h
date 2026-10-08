/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_UEFI_UART_H
#define CLASSICK_UEFI_UART_H
#include "keyboard.h"
#include "../pc/uart.h"
enum { CS_UART_TRACE_OFFSET=256,CS_UART_TRACE_BYTES=640,CS_UART_TRACE_HEADER=128,
    CS_UART_TRACE_MAGIC=0x31555043 };
_Static_assert(CS_UART_TRACE_OFFSET>=CS_KBD_TRACE_OFFSET+CS_KBD_TRACE_BYTES,"UART trace follows keyboard");
_Static_assert(CS_UART_TRACE_OFFSET+CS_UART_TRACE_BYTES<=CS_LOADER_TRACE_BYTES,"UART trace fits bundle");
uint32_t cs_native_uart_loop(const cs_uefi_handoff *,uint32_t ready,
    const cs_native_devices *,const cs_ps2_io *,const cs_uart_io *,
    uint32_t base,uint32_t divisor,uint32_t seconds,unsigned char *keyboard_trace,unsigned char *uart_trace);
#endif
