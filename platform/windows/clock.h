/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOWS_CLOCK_H
#define CLASSICK_WINDOWS_CLOCK_H
#include "../../core/time/clock.h"
typedef struct {
    uint64_t origin, previous;
    uint32_t frequency;
    cs_clock clock;
} cs_win_clock;
enum {
    CS_WIN_TIME_OK=0, CS_WIN_TIME_ERR_ARGUMENT=1, CS_WIN_TIME_ERR_STATE=2,
    CS_WIN_TIME_ERR_UNAVAILABLE=3, CS_WIN_TIME_ERR_FREQUENCY=4,
    CS_WIN_TIME_ERR_BACKWARD=5, CS_WIN_TIME_ERR_OVERFLOW=6
};
int cs_win_clock_convert(uint64_t elapsed, uint32_t frequency, cs_time *out);
/* Raw transitions also form the deterministic host-provider test seam. */
int cs_win_clock_start(cs_win_clock *clock, int available, int64_t frequency, int64_t counter);
int cs_win_clock_sample(cs_win_clock *clock, int available, int64_t counter, cs_time *out);
int cs_win_clock_init(cs_win_clock *clock);
int cs_win_clock_read(cs_win_clock *clock, cs_time *out);
#endif
