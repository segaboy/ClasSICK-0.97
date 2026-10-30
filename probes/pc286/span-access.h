/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC286_SPAN_ACCESS_H
#define CLASSICK_PC286_SPAN_ACCESS_H
#include "span-bind.h"

/* PROPOSED SPEC-0069 section 1 surface over SPEC-0082 bound spans: checked
 * big-endian read/write of widths 1/2/4. Values only: no native pointer is
 * returned, retained or exposed. Frozen order (pc286-span-access-contract.md):
 *   read : ARGUMENT (NULL out, no READ bit) -> env (ARGUMENT, MAP, BACKING,
 *          STALE) -> WIDTH -> ADDRESS (alignment) -> window (ARGUMENT for
 *          access bits outside 7, ADDRESS span/region, PERMISSION) -> BINDING
 *   write: env -> WIDTH -> ADDRESS (alignment) -> window (WRITE) -> BINDING
 * BACKING/STALE are deliberate insertions after MAP. Refusal leaves *out and
 * every bound byte unchanged. Production guest.c is unchanged. */
cs_bx_result cs_bx_read(const cs_bx_env *env, uint32_t at, unsigned width,
                        uint32_t access, uint32_t *out);
cs_bx_result cs_bx_write(const cs_bx_env *env, uint32_t at, unsigned width,
                         uint32_t value);
#endif
