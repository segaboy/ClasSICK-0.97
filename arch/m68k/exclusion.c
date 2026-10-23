/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Supervisor-only; not bound to foreground, ROM, boot or any handler. */
#include "exclusion.h"

#if !defined(__m68k__)
#error "arch/m68k/exclusion.c requires an m68k target"
#endif

uint32_t cs_m68k_foreground_enter(void *cookie)
{
    uint16_t sr;

    (void)cookie;
    /* Snapshot precedes the mask change inside one block; ORI keeps S/T bits. */
    __asm__ volatile ("move.w %%sr,%0\n\tori.w #0x0700,%%sr"
                      : "=d" (sr) : : "cc", "memory");
    return (uint32_t)sr;
}

void cs_m68k_foreground_leave(void *cookie, uint32_t token)
{
    uint16_t sr = (uint16_t)(token & 0xFFFFu);

    (void)cookie;
    __asm__ volatile ("move.w %0,%%sr" : : "d" (sr) : "cc", "memory");
}
