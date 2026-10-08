/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/pmtimer.h"
#include <stdio.h>
#include <string.h>
static unsigned failures;
static unsigned long cases;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static uint64_t seed=0x9E3779B97F4A7C15u;
static uint64_t next(void)
{
    seed^=seed<<13; seed^=seed>>7; seed^=seed<<17;
    return seed;
}
static void extension(void)
{
    for(unsigned bits=24;bits<=32;bits+=8) {
        uint32_t mask=bits==32?0xFFFFFFFFu:0x00FFFFFFu;
        for(unsigned run=0;run<64;++run) {
            cs_pmtimer t;
            uint32_t raw=(uint32_t)next()&mask,delta;
            uint64_t expected=0;
            CHECK(cs_pmtimer_init(&t,bits,raw)==CS_PMTIMER_OK && t.ticks==0);
            for(unsigned i=0;i<2000;++i) {
                /* Steps up to one full period minus one, including exact wraps. */
                uint32_t step=(uint32_t)(i%7==0?mask:(uint32_t)next()&(i%3==0?mask:0xFFFFu));
                raw=(raw+step)&mask; expected+=step;
                CHECK(cs_pmtimer_sample(&t,raw,&delta)==CS_PMTIMER_OK && delta==step);
                ++cases;
            }
            CHECK(t.ticks==expected && t.last==raw);
        }
    }
}
static void conversion(void)
{
    static const uint64_t fixed[]={0,1,3579544,3579545,3579546,7159090,UINT64_C(3579545)*60,
        UINT64_C(3579545)*UINT32_MAX,UINT64_C(3579545)*UINT32_MAX+3579544};
    cs_time out;
    for(unsigned i=0;i<sizeof fixed/sizeof fixed[0]+200000;++i) {
        uint64_t ticks=i<sizeof fixed/sizeof fixed[0]?fixed[i]:next()%(UINT64_C(3579545)*UINT32_MAX);
        /* Oracle uses native 64-bit division; the implementation uses long division. */
        uint64_t seconds=ticks/3579545u,ns=(ticks%3579545u)*1000000000u/3579545u;
        CHECK(cs_pmtimer_time(ticks,&out)==CS_PMTIMER_OK && out.seconds==seconds && out.nanoseconds==ns);
        CHECK(out.nanoseconds<1000000000u);
        ++cases;
    }
    out.seconds=7; out.nanoseconds=9;
    CHECK(cs_pmtimer_time(UINT64_C(3579545)*UINT32_MAX+3579545,&out)==CS_PMTIMER_OVERFLOW);
    CHECK(cs_pmtimer_time(UINT64_MAX,&out)==CS_PMTIMER_OVERFLOW);
    CHECK(out.seconds==7 && out.nanoseconds==9);
}
static void errors(void)
{
    cs_pmtimer t,saved;
    uint32_t delta=77;
    CHECK(cs_pmtimer_init(NULL,24,0)==CS_PMTIMER_ARGUMENT);
    CHECK(cs_pmtimer_init(&t,16,0)==CS_PMTIMER_ARGUMENT && cs_pmtimer_init(&t,0,0)==CS_PMTIMER_ARGUMENT);
    CHECK(cs_pmtimer_init(&t,24,0x01000000u)==CS_PMTIMER_VALUE);
    CHECK(cs_pmtimer_init(&t,32,0xFFFFFFFFu)==CS_PMTIMER_OK && t.mask==0xFFFFFFFFu);
    CHECK(cs_pmtimer_init(&t,24,0x00FFFFFFu)==CS_PMTIMER_OK);
    CHECK(cs_pmtimer_sample(&t,0,&delta)==CS_PMTIMER_OK && delta==1 && t.ticks==1);
    saved=t;
    CHECK(cs_pmtimer_sample(&t,0x01000005u,&delta)==CS_PMTIMER_VALUE);
    CHECK(t.ticks==saved.ticks && t.last==saved.last && delta==1);
    CHECK(cs_pmtimer_sample(NULL,0,&delta)==CS_PMTIMER_ARGUMENT);
    CHECK(cs_pmtimer_sample(&t,0,NULL)==CS_PMTIMER_ARGUMENT);
    t.mask=0xFFFFu; CHECK(cs_pmtimer_sample(&t,0,&delta)==CS_PMTIMER_ARGUMENT);
    t=saved; t.last=0x02000000u; CHECK(cs_pmtimer_sample(&t,0,&delta)==CS_PMTIMER_ARGUMENT);
    t=saved; t.ticks=UINT64_MAX-2; t.last=0;
    CHECK(cs_pmtimer_sample(&t,2,&delta)==CS_PMTIMER_OK && t.ticks==UINT64_MAX);
    CHECK(cs_pmtimer_sample(&t,3,&delta)==CS_PMTIMER_OVERFLOW && t.ticks==UINT64_MAX && t.last==2);
    CHECK(cs_pmtimer_sample(&t,2,&delta)==CS_PMTIMER_OK && delta==0);
    CHECK(cs_pmtimer_time(0,NULL)==CS_PMTIMER_ARGUMENT);
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"extension")==0) extension();
    else if(strcmp(suite,"conversion")==0) conversion();
    else if(strcmp(suite,"errors")==0) errors();
    else { fprintf(stderr,"unknown suite\n"); return 2; }
    if(cases!=0) printf("PM timer %s: %lu cases\n",suite,cases);
    if(failures!=0) { fprintf(stderr,"%u failures\n",failures); return 1; }
    printf("PM timer %s: PASS\n",suite);
    return 0;
}
