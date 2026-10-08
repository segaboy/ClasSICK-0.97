/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "clock.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static int valid(const cs_win_clock *clock)
{
    return clock->frequency != 0 && clock->origin <= clock->previous
        && clock->previous <= (uint64_t)INT64_MAX
        && clock->clock.last.nanoseconds < CS_NANOSECONDS_PER_SECOND;
}
int cs_win_clock_convert(uint64_t elapsed, uint32_t frequency, cs_time *out)
{
    uint64_t seconds, fraction;
    if (out == NULL) return CS_WIN_TIME_ERR_ARGUMENT;
    if (frequency == 0) return CS_WIN_TIME_ERR_FREQUENCY;
    seconds=elapsed/frequency;
    if (seconds > UINT32_MAX) return CS_WIN_TIME_ERR_OVERFLOW;
    fraction=((elapsed%frequency)*CS_NANOSECONDS_PER_SECOND)/frequency;
    out->seconds=(uint32_t)seconds;
    out->nanoseconds=(uint32_t)fraction;
    return CS_WIN_TIME_OK;
}
int cs_win_clock_start(cs_win_clock *clock, int available, int64_t frequency, int64_t counter)
{
    if (clock == NULL) return CS_WIN_TIME_ERR_ARGUMENT;
    if (!available) return CS_WIN_TIME_ERR_UNAVAILABLE;
    if (frequency <= 0 || (uint64_t)frequency > UINT32_MAX) return CS_WIN_TIME_ERR_FREQUENCY;
    if (counter < 0) return CS_WIN_TIME_ERR_UNAVAILABLE;
    clock->origin=(uint64_t)counter;
    clock->previous=(uint64_t)counter;
    clock->frequency=(uint32_t)frequency;
    clock->clock.last.seconds=0;
    clock->clock.last.nanoseconds=0;
    return CS_WIN_TIME_OK;
}
int cs_win_clock_sample(cs_win_clock *clock, int available, int64_t counter, cs_time *out)
{
    cs_time sample;
    cs_clock next;
    int result;
    if (clock == NULL || out == NULL) return CS_WIN_TIME_ERR_ARGUMENT;
    if (!valid(clock)) return CS_WIN_TIME_ERR_STATE;
    if (!available || counter < 0) return CS_WIN_TIME_ERR_UNAVAILABLE;
    if ((uint64_t)counter < clock->previous) return CS_WIN_TIME_ERR_BACKWARD;
    result=cs_win_clock_convert((uint64_t)counter-clock->origin,clock->frequency,&sample);
    if (result != CS_WIN_TIME_OK) return result;
    next=clock->clock;
    if (cs_clock_observe(&next,sample) != CS_TIME_OK) return CS_WIN_TIME_ERR_BACKWARD;
    clock->clock=next;
    clock->previous=(uint64_t)counter;
    out->seconds=sample.seconds;
    out->nanoseconds=sample.nanoseconds;
    return CS_WIN_TIME_OK;
}
int cs_win_clock_init(cs_win_clock *clock)
{
    LARGE_INTEGER frequency, counter;
    if (clock == NULL) return CS_WIN_TIME_ERR_ARGUMENT;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter))
        return CS_WIN_TIME_ERR_UNAVAILABLE;
    return cs_win_clock_start(clock,1,frequency.QuadPart,counter.QuadPart);
}
int cs_win_clock_read(cs_win_clock *clock, cs_time *out)
{
    LARGE_INTEGER counter;
    if (clock == NULL || out == NULL) return CS_WIN_TIME_ERR_ARGUMENT;
    if (!valid(clock)) return CS_WIN_TIME_ERR_STATE;
    if (!QueryPerformanceCounter(&counter)) return CS_WIN_TIME_ERR_UNAVAILABLE;
    return cs_win_clock_sample(clock,1,counter.QuadPart,out);
}
