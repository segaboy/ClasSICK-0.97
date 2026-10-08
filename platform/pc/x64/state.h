/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_X64_STATE_H
#define CLASSICK_X64_STATE_H
#include <stddef.h>
#include <stdint.h>
enum { CS_X64_STATE_BYTES=8192, CS_X64_VECTOR_BYTES=8192,
    CS_X64_KERNEL_STACK_BYTES=65536, CS_X64_IST_BYTES=4096,
    CS_X64_GDTR=40, CS_X64_IDTR=50, CS_X64_TSS=64,
    CS_X64_RECORD=176, CS_X64_READY=240, CS_X64_IDT=256 };
typedef enum { CS_X64_OK=0, CS_X64_ARGUMENT=1, CS_X64_LIMIT=2,
    CS_X64_OVERLAP=3 } cs_x64_result;
typedef struct {
    uint64_t state_base,vector_base,stack_top,ist_top[4];
} cs_x64_layout;
/* Live writable storage/layout are disjoint. Logical ranges need not be hosted
   pointers; native use requires truthful live identity-mapped addresses. */
cs_x64_result cs_x64_tables_init(void *storage,size_t capacity,const cs_x64_layout *layout);
#endif
