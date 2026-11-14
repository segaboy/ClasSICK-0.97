/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc286/ega-mono.h"
#include <limits.h>
#include <stdio.h>

static uint8_t canvas[21888],guarded[28002];
struct rect {int left,top,right,bottom;};
static const struct rect cases[]={
    {0,0,512,342},{0,0,1,1},{511,0,512,1},{0,341,1,342},
    {511,341,512,342},{-5,-4,13,3},{7,20,9,22},{300,10,299,11},
    {INT_MIN,INT_MIN,INT_MAX,INT_MAX},{512,342,INT_MAX,INT_MAX},
    {INT_MIN,0,-1,342},{0,INT_MIN,512,-1},{16,25,16,26}
};

int main(void)
{
    unsigned k,i,row,col;
    for(i=0;i<sizeof(canvas);++i)canvas[i]=(uint8_t)(i*37u+11u);
    for(k=0;k<sizeof(cases)/sizeof(cases[0]);++k) {
        struct rect r=cases[k];
        for(i=0;i<sizeof(guarded);++i)guarded[i]=0x69u;
        if(!cs_pc286_ega_publish(canvas,guarded+1,r.left,r.top,r.right,r.bottom))return 1;
        if(guarded[0]!=0x69u || guarded[28001]!=0x69u)return 2;
        for(row=0;row<350u;++row)for(col=0;col<80u;++col) {
            uint8_t expected=0x69u;
            if(row>=4u && row<346u && col>=8u && col<72u && r.left<r.right && r.top<r.bottom) {
                int y=(int)row-4;int x=((int)col-8)*8;
                if(y>=r.top && y<r.bottom && x<r.right && x+8>r.left)
                    expected=(uint8_t)~canvas[(row-4u)*64u+col-8u];
            }
            if(guarded[1u+row*80u+col]!=expected) {
                fprintf(stderr,"case %u row %u column %u mismatch\n",k,row,col);return 3;
            }
        }
    }
    if(cs_pc286_ega_publish(NULL,guarded+1,0,0,1,1) ||
       cs_pc286_ega_publish(canvas,NULL,0,0,1,1))return 4;
    printf("PASS %u edge cases, guard bytes and null refusal\n",(unsigned)(sizeof(cases)/sizeof(cases[0])));
    return 0;
}
