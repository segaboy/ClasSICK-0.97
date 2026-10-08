/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "clock.h"
#include <stddef.h>
static int valid(cs_time value) { return value.nanoseconds < CS_NANOSECONDS_PER_SECOND; }
static int before(cs_time a, cs_time b)
{ return a.seconds < b.seconds || (a.seconds == b.seconds && a.nanoseconds < b.nanoseconds); }
static void save(cs_time *out, uint32_t seconds, uint32_t nanoseconds)
{ out->seconds=seconds; out->nanoseconds=nanoseconds; }

cs_time_result cs_time_add(cs_time base, cs_time duration, cs_time *out)
{
    uint32_t seconds, nanoseconds, carry;
    if (out == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(base) || !valid(duration)) return CS_TIME_ERR_VALUE;
    nanoseconds=base.nanoseconds + duration.nanoseconds;
    carry=nanoseconds >= CS_NANOSECONDS_PER_SECOND ? 1u : 0u;
    if (duration.seconds > UINT32_MAX-base.seconds) return CS_TIME_ERR_OVERFLOW;
    seconds=base.seconds + duration.seconds;
    if (carry != 0 && seconds == UINT32_MAX) return CS_TIME_ERR_OVERFLOW;
    save(out,seconds+carry,nanoseconds-carry*CS_NANOSECONDS_PER_SECOND);
    return CS_TIME_OK;
}
cs_time_result cs_time_elapsed(cs_time start, cs_time end, cs_time *out)
{
    uint32_t seconds, nanoseconds;
    if (out == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(start) || !valid(end)) return CS_TIME_ERR_VALUE;
    if (before(end,start)) return CS_TIME_ERR_BACKWARD;
    seconds=end.seconds-start.seconds;
    if (end.nanoseconds < start.nanoseconds) {
        --seconds;
        nanoseconds=CS_NANOSECONDS_PER_SECOND-start.nanoseconds+end.nanoseconds;
    } else nanoseconds=end.nanoseconds-start.nanoseconds;
    save(out,seconds,nanoseconds);
    return CS_TIME_OK;
}
cs_time_result cs_time_reached(cs_time now, cs_time deadline, int *out)
{
    if (out == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(now) || !valid(deadline)) return CS_TIME_ERR_VALUE;
    *out=before(now,deadline) ? 0 : 1;
    return CS_TIME_OK;
}
cs_time_result cs_clock_init(cs_clock *clock, cs_time start)
{
    if (clock == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(start)) return CS_TIME_ERR_VALUE;
    save(&clock->last,start.seconds,start.nanoseconds);
    return CS_TIME_OK;
}
cs_time_result cs_clock_observe(cs_clock *clock, cs_time sample)
{
    if (clock == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(clock->last)) return CS_TIME_ERR_STATE;
    if (!valid(sample)) return CS_TIME_ERR_VALUE;
    if (before(sample,clock->last)) return CS_TIME_ERR_BACKWARD;
    save(&clock->last,sample.seconds,sample.nanoseconds);
    return CS_TIME_OK;
}
cs_time_result cs_clock_advance(cs_clock *clock, cs_time duration)
{
    cs_time sum;
    cs_time_result result;
    if (clock == NULL) return CS_TIME_ERR_ARGUMENT;
    if (!valid(clock->last)) return CS_TIME_ERR_STATE;
    result=cs_time_add(clock->last,duration,&sum);
    if (result != CS_TIME_OK) return result;
    save(&clock->last,sum.seconds,sum.nanoseconds);
    return CS_TIME_OK;
}
