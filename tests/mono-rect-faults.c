/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/pc/mono-present.h"
#include <stdio.h>
#include <string.h>
static uint8_t fb[640*480*4],saved_fb[sizeof fb],canvas[21888],row[640*4],saved_row[sizeof row];
static unsigned calls,fail_at;
cs_fb_result cs_test_fb_present(const cs_fb_target *t,const cs_surface *s,int32_t x,int32_t y)
{
    ++calls;
    if(calls==fail_at)return CS_FB_STORAGE;
    return cs_fb_present(t,s,x,y);
}
int main(void)
{
    cs_surface source;cs_fb_target target;cs_pc_mono_report report;
    const cs_rect rects[2]={{3,4,9,8},{20,10,22,13}};
    memset(fb,0xA5,sizeof fb);memset(row,0xCC,sizeof row);
    if(cs_surface_init(&source,canvas,sizeof canvas,512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK ||
        cs_fb_init(&target,fb,sizeof fb,640,480,640,CS_FB_RGB_RESERVED)!=CS_FB_OK)return 1;
    fail_at=6;
    if(cs_pc_mono_present_rects(&target,&source,row,sizeof row,rects,2,&report)!=CS_MONO_PRESENT ||
        calls!=6 || report.phase!=CS_MONO_FAILED || report.rows!=5 || report.required!=7 || report.cause!=CS_FB_STORAGE)return 1;
    for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){
        int changed=(x>=67 && x<73 && y>=73 && y<77) || (x>=84 && x<86 && y==79);
        size_t at=((size_t)y*640+x)*4u;
        for(unsigned b=0;b<4;++b)if(fb[at+b]!=(changed?(b==3?0:255):0xA5))return 1;
    }
    cs_rect bad[2]={{0,0,5,5},{9,9,8,10}};
    uint8_t old_report[sizeof report];memcpy(old_report,&report,sizeof report);
    memcpy(saved_fb,fb,sizeof fb);memcpy(saved_row,row,sizeof row);calls=0;
    if(cs_pc_mono_present_rects(&target,&source,row,sizeof row,bad,2,&report)!=CS_MONO_ARGUMENT || calls!=0 ||
        memcmp(old_report,&report,sizeof report)!=0 || memcmp(saved_fb,fb,sizeof fb)!=0 || memcmp(saved_row,row,sizeof row)!=0)return 1;
    puts("Dirty rectangles: five completed rows retained; failing/later rows untouched; bad later rect refused before effects.");return 0;
}
