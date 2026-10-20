/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original compiler-runtime arithmetic tests. */
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
int32_t __modsi3(int32_t a,int32_t b);
static unsigned checks;
static void check(int32_t a,int32_t b)
{
    int32_t expected=(a==INT32_MIN && b==-1)?0:a%b;
    ++checks;
    if(__modsi3(a,b)!=expected){fprintf(stderr,"signed remainder mismatch\n");exit(1);}
}
int main(void)
{
    const int32_t edges[]={INT32_MIN,INT32_MIN+1,-1000,-10,-1,0,1,10,1000,INT32_MAX-1,INT32_MAX};
    uint32_t seed=0x970054u;
    for(unsigned i=0u;i<sizeof(edges)/sizeof(edges[0]);++i)
        for(unsigned j=0u;j<sizeof(edges)/sizeof(edges[0]);++j)if(edges[j])check(edges[i],edges[j]);
    for(unsigned i=0u;i<10000u;++i){
        int32_t a,b;
        seed=seed*1664525u+1013904223u;a=(int32_t)(seed&INT32_MAX);if(seed&0x80000000u)a=-a;
        seed=seed*1664525u+1013904223u;b=(int32_t)(seed&INT32_MAX);if(!b)b=1;if(seed&0x80000000u)b=-b;
        check(a,b);
    }
    printf("Composition remainder PASS:%u cases;zero-divisor trap not executed\n",checks);return 0;
}
