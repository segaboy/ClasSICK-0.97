/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "pmtimer.h"

cs_pmtimer_result cs_pmtimer_init(cs_pmtimer *timer,uint32_t bits,uint32_t raw)
{
    uint32_t mask;
    if(timer==NULL || (bits!=24u && bits!=32u)) return CS_PMTIMER_ARGUMENT;
    mask=bits==32u?UINT32_C(0xFFFFFFFF):UINT32_C(0x00FFFFFF);
    if((raw&~mask)!=0) return CS_PMTIMER_VALUE;
    timer->ticks=0; timer->last=raw; timer->mask=mask;
    return CS_PMTIMER_OK;
}

cs_pmtimer_result cs_pmtimer_sample(cs_pmtimer *timer,uint32_t raw,uint32_t *delta)
{
    uint32_t step;
    if(timer==NULL || delta==NULL
            || (timer->mask!=UINT32_C(0xFFFFFFFF) && timer->mask!=UINT32_C(0x00FFFFFF))
            || (timer->last&~timer->mask)!=0) return CS_PMTIMER_ARGUMENT;
    if((raw&~timer->mask)!=0) return CS_PMTIMER_VALUE;
    step=(raw-timer->last)&timer->mask;
    if(timer->ticks>UINT64_MAX-step) return CS_PMTIMER_OVERFLOW;
    timer->ticks+=step; timer->last=raw; *delta=step;
    return CS_PMTIMER_OK;
}

cs_pmtimer_result cs_pmtimer_time(uint64_t ticks,cs_time *out)
{
    /* Restoring binary long division by the 22-bit rate; remainder stays below 2^23. */
    uint64_t quotient=0,remainder=0,scaled;
    if(out==NULL) return CS_PMTIMER_ARGUMENT;
    for(int bit=63;bit>=0;--bit) {
        remainder=(remainder<<1)|((ticks>>bit)&1u);
        quotient<<=1;
        if(remainder>=CS_PMTIMER_HZ) { remainder-=CS_PMTIMER_HZ; quotient|=1u; }
    }
    if(quotient>UINT32_MAX) return CS_PMTIMER_OVERFLOW;
    /* remainder*1e9 < 3.6e15: divide the same way to keep the result truncated. */
    scaled=remainder*CS_NANOSECONDS_PER_SECOND;
    remainder=0; ticks=0;
    for(int bit=63;bit>=0;--bit) {
        remainder=(remainder<<1)|((scaled>>bit)&1u);
        ticks<<=1;
        if(remainder>=CS_PMTIMER_HZ) { remainder-=CS_PMTIMER_HZ; ticks|=1u; }
    }
    out->seconds=(uint32_t)quotient;
    out->nanoseconds=(uint32_t)ticks;
    return CS_PMTIMER_OK;
}
