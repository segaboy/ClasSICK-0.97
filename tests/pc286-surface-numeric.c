/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent wide-arithmetic reference for the numeric C16 model. */
#include "../probes/pc286/surface-numeric.h"
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
static int reference(int present, int format, int32_t w, int32_t h, uint16_t stride, uint16_t storage) {
    uint64_t row, span;
    if (!present) return 1;
    if (format != 1 && format != 2) return 2;
    if (w <= 0 || h <= 0) return 3;
    if (format == 1) row = ((uint64_t)(uint32_t)w + 7u) / 8u;
    else row = (uint64_t)(uint32_t)w * 4u;
    if (row > 65535u) return 5;
    if ((uint64_t)stride < row) return 4;
    span = (uint64_t)stride * (uint32_t)h;
    if (span > 32767u) return 5;
    if ((uint64_t)storage < span) return 6;
    return 0;
}
static uint32_t state=0x12cb9137u;
static uint32_t next(void) { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
static int check(int p, int f, int32_t w, int32_t h, uint16_t s, uint16_t n) {
    int a=(int)cs_pc286_surface_numeric(p,f,w,h,s,n), b=reference(p,f,w,h,s,n);
    if(a!=b) { fprintf(stderr,"Mismatch: %d %d %ld %ld %u %u got=%d expected=%d\n",p,f,(long)w,(long)h,(unsigned)s,(unsigned)n,a,b); return 1; }
    return 0;
}
int main(void) {
    static const int32_t widths[]={INT32_MIN,-1,0,1,7,8,9,512,16383,16384,524279,524280,524281,INT32_MAX};
    static const int32_t heights[]={INT32_MIN,-1,0,1,2,342,32766,32767,32768,65535,INT32_MAX};
    static const uint16_t bounds[]={0,1,63,64,65,32766,32767,32768,65534,65535};
    unsigned i,j,k,l,p,f; unsigned long count=0;
    for(p=0;p<2;p++) for(f=0;f<4;f++)
      for(i=0;i<sizeof widths/sizeof widths[0];i++) for(j=0;j<sizeof heights/sizeof heights[0];j++)
       for(k=0;k<sizeof bounds/sizeof bounds[0];k++) for(l=0;l<sizeof bounds/sizeof bounds[0];l++) {
        if(check((int)p,(int)f,widths[i],heights[j],bounds[k],bounds[l])) return 1;
        count++;
       }
    for(i=0;i<2000000u;i++) {
        uint32_t a=next(), b=next(), c=next(), d=next();
        int32_t w=(int32_t)(a & 0x7fffffffU), h=(int32_t)(b & 0x7fffffffU);
        if(i%3u==0) w=(int32_t)(a%540000u);
        if(i%3u==1) h=(int32_t)(b%70000u);
        if(i%17u==0) w=-w;
        if(i%19u==0) h=-h;
        if(check(1,(int)(1u+(i&1u)),w,h,(uint16_t)c,(uint16_t)d)) return 1;
        count++;
    }
    {
        static uint8_t storage[2048];
        cs_surface out;
        for(i=0;i<100000u;i++) {
            uint32_t a=next(), b=next(), c=next(), d=next();
            int present=(int)(a & 1u), format=(int)(b % 4u);
            int32_t w=(int32_t)(1u+(a % 512u));
            int32_t h=(int32_t)(1u+(b % 20u));
            uint16_t stride=(uint16_t)(c % 81u), size=(uint16_t)(d % 2049u);
            cs_surface_result actual=cs_surface_init(&out, present ? storage : NULL,
                size, w, h, stride, (cs_surface_format)format);
            cs_surface_result modeled=cs_pc286_surface_numeric(present,format,w,h,stride,size);
            if(actual!=modeled) {
                fprintf(stderr,"Production parity mismatch at %u: actual=%d model=%d\n",i,(int)actual,(int)modeled);
                return 1;
            }
        }
    }
    printf("PASS %lu numeric comparisons and 100000 production parity cases\n",count); return 0;
}
