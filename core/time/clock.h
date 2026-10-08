/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_CLOCK_H
#define CLASSICK_CLOCK_H
#include <stdint.h>
#define CS_NANOSECONDS_PER_SECOND UINT32_C(1000000000)
typedef struct { uint32_t seconds, nanoseconds; } cs_time;
typedef struct { cs_time last; } cs_clock;
typedef enum {
    CS_TIME_OK=0, CS_TIME_ERR_ARGUMENT=1, CS_TIME_ERR_STATE=2,
    CS_TIME_ERR_VALUE=3, CS_TIME_ERR_BACKWARD=4, CS_TIME_ERR_OVERFLOW=5
} cs_time_result;
cs_time_result cs_time_add(cs_time base, cs_time duration, cs_time *out);
cs_time_result cs_time_elapsed(cs_time start, cs_time end, cs_time *out);
cs_time_result cs_time_reached(cs_time now, cs_time deadline, int *out);
cs_time_result cs_clock_init(cs_clock *clock, cs_time start);
cs_time_result cs_clock_observe(cs_clock *clock, cs_time sample);
cs_time_result cs_clock_advance(cs_clock *clock, cs_time duration);
#endif
