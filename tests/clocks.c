/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "clock.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"clock:%d: %s\n",__LINE__,#x); return 0; } } while (0)
static int same(cs_time a, cs_time b) { return a.seconds==b.seconds && a.nanoseconds==b.nanoseconds; }
static int arithmetic(void)
{
    cs_time out={0}; int reached=7;
    CHECK(cs_time_add((cs_time){7,900000000},(cs_time){2,200000000},&out)==CS_TIME_OK && same(out,(cs_time){10,100000000}));
    CHECK(cs_time_elapsed((cs_time){7,900000000},out,&out)==CS_TIME_OK && same(out,(cs_time){2,200000000}));
    CHECK(cs_time_reached((cs_time){0,249999999},(cs_time){0,250000000},&reached)==CS_TIME_OK && reached==0);
    CHECK(cs_time_reached((cs_time){0,250000000},(cs_time){0,250000000},&reached)==CS_TIME_OK && reached==1);
    CHECK(cs_time_reached((cs_time){1,0},(cs_time){0,250000000},&reached)==CS_TIME_OK && reached==1);
    CHECK(cs_time_add((cs_time){UINT32_MAX,999999998},(cs_time){0,1},&out)==CS_TIME_OK && same(out,(cs_time){UINT32_MAX,999999999}));
    return 1;
}
static int errors(void)
{
    cs_time invalid={0,1000000000}, out={17,19}, before=out;
    cs_clock clock={{23,29}}, prior=clock; int reached=31;
    CHECK(cs_time_add(invalid,invalid,NULL)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_time_elapsed(invalid,invalid,NULL)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_time_reached(invalid,invalid,NULL)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_time_add(invalid,invalid,&out)==CS_TIME_ERR_VALUE && same(out,before));
    CHECK(cs_time_elapsed(invalid,invalid,&out)==CS_TIME_ERR_VALUE && same(out,before));
    CHECK(cs_time_reached(invalid,invalid,&reached)==CS_TIME_ERR_VALUE && reached==31);
    CHECK(cs_time_elapsed((cs_time){1,0},(cs_time){0,999999999},&out)==CS_TIME_ERR_BACKWARD && same(out,before));
    CHECK(cs_time_add((cs_time){UINT32_MAX,0},(cs_time){1,0},&out)==CS_TIME_ERR_OVERFLOW && same(out,before));
    CHECK(cs_time_add((cs_time){UINT32_MAX,999999999},(cs_time){0,1},&out)==CS_TIME_ERR_OVERFLOW && same(out,before));
    CHECK(cs_clock_init(NULL,invalid)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_clock_observe(NULL,invalid)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_clock_advance(NULL,invalid)==CS_TIME_ERR_ARGUMENT);
    CHECK(cs_clock_init(&clock,invalid)==CS_TIME_ERR_VALUE && same(clock.last,prior.last));
    CHECK(cs_clock_observe(&clock,invalid)==CS_TIME_ERR_VALUE && same(clock.last,prior.last));
    CHECK(cs_clock_advance(&clock,invalid)==CS_TIME_ERR_VALUE && same(clock.last,prior.last));
    CHECK(cs_clock_observe(&clock,(cs_time){23,28})==CS_TIME_ERR_BACKWARD && same(clock.last,prior.last));
    clock.last=invalid;
    CHECK(cs_clock_observe(&clock,invalid)==CS_TIME_ERR_STATE && same(clock.last,invalid));
    CHECK(cs_clock_advance(&clock,invalid)==CS_TIME_ERR_STATE && same(clock.last,invalid));
    CHECK(cs_clock_init(&clock,(cs_time){UINT32_MAX,999999999})==CS_TIME_OK);
    prior=clock;
    CHECK(cs_clock_advance(&clock,(cs_time){0,1})==CS_TIME_ERR_OVERFLOW && same(clock.last,prior.last));
    return 1;
}
static int matrix(void)
{
    const uint32_t seconds[]={0,1,2,3,7,16,65535,UINT32_MAX-1u,UINT32_MAX};
    const uint32_t nanos[]={0,1,2,9999999,249999999,250000000,999999998,999999999};
    const uint64_t unit=UINT64_C(1000000000), limit=(uint64_t)UINT32_MAX*unit+unit-1u;
    size_t cases=0;
    for(size_t a=0;a<9;++a) for(size_t b=0;b<8;++b)
    for(size_t c=0;c<9;++c) for(size_t d=0;d<8;++d) {
        cs_time x={seconds[a],nanos[b]}, y={seconds[c],nanos[d]};
        struct { uint32_t left; cs_time value; uint32_t right; } guarded={71,{73,79},83};
        cs_time sentinel=guarded.value;
        uint64_t xt=(uint64_t)x.seconds*unit+x.nanoseconds, yt=(uint64_t)y.seconds*unit+y.nanoseconds;
        uint64_t sum=xt+yt; int reached=17;
        cs_time_result result=cs_time_add(x,y,&guarded.value);
        if(sum>limit) CHECK(result==CS_TIME_ERR_OVERFLOW && same(guarded.value,sentinel));
        else CHECK(result==CS_TIME_OK && guarded.value.seconds==sum/unit && guarded.value.nanoseconds==sum%unit);
        guarded.value=sentinel;
        result=cs_time_elapsed(x,y,&guarded.value);
        if(yt<xt) CHECK(result==CS_TIME_ERR_BACKWARD && same(guarded.value,sentinel));
        else CHECK(result==CS_TIME_OK && guarded.value.seconds==(yt-xt)/unit && guarded.value.nanoseconds==(yt-xt)%unit);
        CHECK(cs_time_reached(x,y,&reached)==CS_TIME_OK && reached==(xt>=yt ? 1 : 0));
        CHECK(guarded.left==71 && guarded.right==83);
        ++cases;
    }
    printf("SPEC-0005 independent wide-integer oracle: %zu boundary pairs\n",cases);
    return 1;
}
static int deterministic(void)
{
    cs_clock a={{0}}, b={{0}}; cs_time elapsed;
    CHECK(cs_clock_init(&a,(cs_time){7,900000000})==CS_TIME_OK);
    CHECK(cs_clock_init(&b,(cs_time){1,0})==CS_TIME_OK);
    CHECK(cs_clock_advance(&a,(cs_time){0,200000000})==CS_TIME_OK && same(a.last,(cs_time){8,100000000}));
    CHECK(cs_clock_advance(&a,(cs_time){0,0})==CS_TIME_OK);
    CHECK(cs_clock_observe(&a,a.last)==CS_TIME_OK);
    CHECK(cs_clock_observe(&a,(cs_time){9,0})==CS_TIME_OK);
    CHECK(same(b.last,(cs_time){1,0}));
    CHECK(cs_time_elapsed((cs_time){7,900000000},a.last,&elapsed)==CS_TIME_OK && same(elapsed,(cs_time){1,100000000}));
    CHECK(cs_clock_init(&a,(cs_time){0,0})==CS_TIME_OK);
    CHECK(cs_clock_advance(&b,(cs_time){2,3})==CS_TIME_OK && same(b.last,(cs_time){3,3}));
    CHECK(same(a.last,(cs_time){0,0}));
    return 1;
}
int main(int argc,char **argv)
{
    int passed=0; if(argc!=2) return 2;
    if(strcmp(argv[1],"arithmetic")==0) passed=arithmetic();
    else if(strcmp(argv[1],"errors")==0) passed=errors();
    else if(strcmp(argv[1],"matrix")==0) passed=matrix();
    else if(strcmp(argv[1],"deterministic")==0) passed=deterministic();
    else return 2;
    if(passed) puts("SPEC-0005 portable clock: PASS");
    return passed ? 0 : 1;
}
