/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_PC_PMTIMER_H
#define CLASSICK_PC_PMTIMER_H
#include <stddef.h>
#include <stdint.h>
#include "../../core/time/clock.h"

/* SPEC-0010 ACPI PM timer extension: 3,579,545 Hz, 24- or 32-bit wrapping counter. */
typedef enum { CS_PMTIMER_OK=0, CS_PMTIMER_ARGUMENT=1, CS_PMTIMER_VALUE=2,
    CS_PMTIMER_OVERFLOW=3 } cs_pmtimer_result;
enum { CS_PMTIMER_HZ=3579545 };
typedef struct { uint64_t ticks; uint32_t last,mask; } cs_pmtimer;

/* Bits must be 24 or 32; raw must fit. Ticks start at zero. */
cs_pmtimer_result cs_pmtimer_init(cs_pmtimer *timer,uint32_t bits,uint32_t raw);
/* Adds (raw-last) modulo the width. One wrap between samples at most is assumed. */
cs_pmtimer_result cs_pmtimer_sample(cs_pmtimer *timer,uint32_t raw,uint32_t *delta);
/* Converts ticks to truncated SPEC-0005 time without 64-bit division helpers. */
cs_pmtimer_result cs_pmtimer_time(uint64_t ticks,cs_time *out);
#endif
