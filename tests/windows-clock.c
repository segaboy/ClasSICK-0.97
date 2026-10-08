/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/windows/clock.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"win-clock:%d: %s\n",__LINE__,#x); return 0; } } while (0)
static int same(cs_time a,cs_time b) { return a.seconds==b.seconds && a.nanoseconds==b.nanoseconds; }
static int same_clock(cs_win_clock a,cs_win_clock b)
{ return a.origin==b.origin && a.previous==b.previous && a.frequency==b.frequency && same(a.clock.last,b.clock.last); }
static int conversion(void)
{
    /* Independently calculated rational fixtures, including truncation. */
    const struct { uint64_t ticks; uint32_t freq,seconds,nanos; } fixtures[]={
        {0,1,0,0},{1,1,1,0},{1,2,0,500000000},{1,3,0,333333333},
        {2,3,0,666666666},{7,3,2,333333333},{1,7,0,142857142},
        {1234,1000,1,234000000},{UINT64_C(10000001),10000000,1,100},
        {UINT32_MAX,UINT32_MAX,1,0},{UINT32_MAX-1u,UINT32_MAX,0,999999999},
        {UINT32_MAX,1,UINT32_MAX,0},
        {UINT64_C(18446744065119617024),UINT32_MAX,UINT32_MAX-1u,999999999}};
    cs_time out={17,19}, prior=out;
    CHECK(cs_win_clock_convert(0,0,NULL)==CS_WIN_TIME_ERR_ARGUMENT);
    CHECK(cs_win_clock_convert(0,0,&out)==CS_WIN_TIME_ERR_FREQUENCY && same(out,prior));
    CHECK(cs_win_clock_convert((uint64_t)UINT32_MAX+1u,1,&out)==CS_WIN_TIME_ERR_OVERFLOW && same(out,prior));
    CHECK(cs_win_clock_convert(UINT64_MAX,UINT32_MAX,&out)==CS_WIN_TIME_ERR_OVERFLOW && same(out,prior));
    for(size_t i=0;i<sizeof(fixtures)/sizeof(fixtures[0]);++i) {
        CHECK(cs_win_clock_convert(fixtures[i].ticks,fixtures[i].freq,&out)==CS_WIN_TIME_OK);
        CHECK(out.seconds==fixtures[i].seconds && out.nanoseconds==fixtures[i].nanos);
    }
    return 1;
}
static int transitions(void)
{
    cs_win_clock clock={0}, before=clock; cs_time out={31,37}, prior=out;
    CHECK(cs_win_clock_start(NULL,0,0,-1)==CS_WIN_TIME_ERR_ARGUMENT);
    CHECK(cs_win_clock_start(&clock,0,0,-1)==CS_WIN_TIME_ERR_UNAVAILABLE && same_clock(clock,before));
    CHECK(cs_win_clock_start(&clock,1,0,0)==CS_WIN_TIME_ERR_FREQUENCY && same_clock(clock,before));
    CHECK(cs_win_clock_start(&clock,1,-1,0)==CS_WIN_TIME_ERR_FREQUENCY && same_clock(clock,before));
    CHECK(cs_win_clock_start(&clock,1,(int64_t)UINT32_MAX+1,0)==CS_WIN_TIME_ERR_FREQUENCY && same_clock(clock,before));
    CHECK(cs_win_clock_start(&clock,1,3,-1)==CS_WIN_TIME_ERR_UNAVAILABLE && same_clock(clock,before));
    CHECK(cs_win_clock_sample(&clock,0,0,&out)==CS_WIN_TIME_ERR_STATE && same(out,prior));
    CHECK(cs_win_clock_sample(NULL,0,0,&out)==CS_WIN_TIME_ERR_ARGUMENT);
    CHECK(cs_win_clock_start(&clock,1,3,100)==CS_WIN_TIME_OK);
    CHECK(cs_win_clock_sample(&clock,1,101,&out)==CS_WIN_TIME_OK && same(out,(cs_time){0,333333333}));
    before=clock; prior=out;
    CHECK(cs_win_clock_sample(&clock,0,102,&out)==CS_WIN_TIME_ERR_UNAVAILABLE && same_clock(clock,before) && same(out,prior));
    CHECK(cs_win_clock_sample(&clock,1,-1,&out)==CS_WIN_TIME_ERR_UNAVAILABLE && same_clock(clock,before) && same(out,prior));
    CHECK(cs_win_clock_sample(&clock,1,100,&out)==CS_WIN_TIME_ERR_BACKWARD && same_clock(clock,before) && same(out,prior));
    CHECK(cs_win_clock_sample(&clock,1,101,&out)==CS_WIN_TIME_OK && same(out,prior));
    CHECK(cs_win_clock_sample(&clock,1,107,&out)==CS_WIN_TIME_OK && same(out,(cs_time){2,333333333}));
    CHECK(cs_win_clock_start(&clock,1,UINT32_MAX,0)==CS_WIN_TIME_OK);
    CHECK(cs_win_clock_sample(&clock,1,3,&out)==CS_WIN_TIME_OK && out.nanoseconds==0);
    before=clock; prior=out;
    CHECK(cs_win_clock_sample(&clock,1,2,&out)==CS_WIN_TIME_ERR_BACKWARD && same_clock(clock,before) && same(out,prior));
    CHECK(cs_win_clock_start(&clock,1,1,0)==CS_WIN_TIME_OK);
    before=clock;
    CHECK(cs_win_clock_sample(&clock,1,(int64_t)UINT32_MAX+1,&out)==CS_WIN_TIME_ERR_OVERFLOW && same_clock(clock,before) && same(out,prior));
    CHECK(cs_win_clock_sample(&clock,1,7,&out)==CS_WIN_TIME_OK && same(out,(cs_time){7,0}));
    for(unsigned i=0;i<4;++i) {
        CHECK(cs_win_clock_start(&clock,1,1,0)==CS_WIN_TIME_OK);
        if(i==0) clock.frequency=0;
        if(i==1) clock.origin=1;
        if(i==2) clock.previous=UINT64_MAX;
        if(i==3) clock.clock.last.nanoseconds=1000000000;
        before=clock; prior=out;
        CHECK(cs_win_clock_sample(&clock,1,7,&out)==CS_WIN_TIME_ERR_STATE && same_clock(clock,before) && same(out,prior));
    }
    return 1;
}
static int live(void)
{
    cs_win_clock clock={0}; cs_time out;
    CHECK(cs_win_clock_init(NULL)==CS_WIN_TIME_ERR_ARGUMENT);
    CHECK(cs_win_clock_read(NULL,&out)==CS_WIN_TIME_ERR_ARGUMENT);
    CHECK(cs_win_clock_read(&clock,&out)==CS_WIN_TIME_ERR_STATE);
    CHECK(cs_win_clock_init(&clock)==CS_WIN_TIME_OK);
    CHECK(cs_win_clock_read(&clock,NULL)==CS_WIN_TIME_ERR_ARGUMENT);
    for(unsigned i=0;i<1000;++i) {
        cs_time previous=clock.clock.last; int reached=0;
        CHECK(cs_win_clock_read(&clock,&out)==CS_WIN_TIME_OK);
        CHECK(cs_time_reached(out,previous,&reached)==CS_TIME_OK && reached==1);
    }
    return 1;
}
int main(int argc,char **argv)
{
    int passed=0; if(argc!=2) return 2;
    if(strcmp(argv[1],"conversion")==0) passed=conversion();
    else if(strcmp(argv[1],"transitions")==0) passed=transitions();
    else if(strcmp(argv[1],"live")==0) passed=live();
    else return 2;
    if(passed) puts("SPEC-0005 Windows clock: PASS");
    return passed ? 0 : 1;
}
