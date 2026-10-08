/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_UART_H
#define CLASSICK_PC_UART_H
#include <stddef.h>
#include <stdint.h>
enum { CS_UART_ACTIVE=0,CS_UART_READY=1,CS_UART_DONE=2,CS_UART_ARGUMENT=3,
    CS_UART_STATE=4,CS_UART_TIMEOUT=5,CS_UART_DEVICE=6,CS_UART_LINE=7,CS_UART_FULL=8,
    CS_UART_NOT_STARTED=9,CS_UART_CAPACITY=512,CS_UART_TICKS=357955,CS_UART_POLLS=100000,
    CS_UART_PHASE_READY=20 };
typedef uint8_t (*cs_uart_read)(void *,uint16_t);
typedef void (*cs_uart_write)(void *,uint16_t,uint8_t);
typedef struct { cs_uart_read read; cs_uart_write write; void *context; } cs_uart_io;
typedef struct {
    unsigned char *bytes;
    uint64_t since,last;
    uint32_t base,divisor,result,phase,head,count,sent,dropped_records,dropped_bytes;
    uint32_t lsr,line_bits,started,closing,polls,scratch,temt,waiting;
} cs_uart;
/* Live disjoint state/512-byte storage/callbacks; SPEC-0013. */
uint32_t cs_uart_begin(cs_uart *,unsigned char *,uint32_t base,uint32_t divisor);
uint32_t cs_uart_enqueue(cs_uart *,const unsigned char *,size_t);
uint32_t cs_uart_finish(cs_uart *);
uint32_t cs_uart_poll(cs_uart *,const cs_uart_io *,uint64_t now);
#endif
