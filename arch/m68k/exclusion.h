/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Isolated MC68000 supervisor short-exclusion callbacks; see SPEC-0068. */
#ifndef CS_ARCH_M68K_EXCLUSION_H
#define CS_ARCH_M68K_EXCLUSION_H

#include <stdint.h>

/* Snapshot SR (low16 of token, upper16 zero), then raise IPL to 7. */
uint32_t cs_m68k_foreground_enter(void *cookie);
/* Write the snapshot word from the matching enter back to SR. */
void cs_m68k_foreground_leave(void *cookie, uint32_t token);

#endif
