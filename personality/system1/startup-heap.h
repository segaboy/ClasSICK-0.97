/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_SYSTEM1_STARTUP_HEAP_H
#define CLASSICK_SYSTEM1_STARTUP_HEAP_H
#include "../../core/memory/manager.h"

/* SPEC-0146. Guest addresses for the two fixed startup zones. */
#define CS_STARTUP_SYS_ZONE UINT32_C(0xa000)
#define CS_STARTUP_APP_ZONE UINT32_C(0x10000)
#define CS_STARTUP_HEAP_END UINT32_C(0x1a700)
#define CS_STARTUP_MEM_ERR 0x220u

typedef struct {
    uint32_t system_next, application_next;
} cs_startup_heap;

void cs_startup_heap_init(cs_startup_heap *heap);
/* Borrow the full SPEC-0137 low-memory window, beginning at guest $100. */
void cs_startup_heap_globals(volatile uint8_t *page100);
/* Mutates the supplied candidate only on allocation. A port using a fallible
 * RAM backend commits this candidate after completing the required RAM writes.
 * Return 0 on invalid configuration or unsupported trap; leave outputs alone. */
int cs_startup_new_ptr(cs_startup_heap *next, uint32_t current, uint32_t limit,
    uint16_t word, uint32_t size, cs_mm_reply *reply);
#endif
