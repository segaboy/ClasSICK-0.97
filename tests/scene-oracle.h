/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_TEST_SCENE_ORACLE_H
#define CLASSICK_TEST_SCENE_ORACLE_H
#include <stdint.h>
/* Independent SPEC-0009 point classifier: walks the painter list for one pixel,
   with no fills, clipping, band translation or implementation helpers. */
static int oracle_inside(long x,long y,long left,long top,long right,long bottom)
{
    return right>=left && bottom>=top && x>=left && x<right && y>=top && y<bottom;
}
static uint32_t oracle_scene(long width,long height,uint32_t step,long x,long y)
{
    static const uint32_t bars[8]={0xFFFFFFFFu,0xFFFF00FFu,0x00FFFFFFu,0x00FF00FFu,
        0xFF00FFFFu,0xFF0000FFu,0x0000FFFFu,0x000000FFu};
    long u=(width<height?width:height)/32,m,pw,ph,px,py,ix,iy,iw,ih,tx,tw,ty,tb;
    uint32_t color=0x1E3A4CFFu;
    if(u<1) u=1;
    m=2*u;
    if(oracle_inside(x,y,0,0,m,m)) color=0xFF0000FFu;
    if(oracle_inside(x,y,width-m,0,width,m)) color=0x00FF00FFu;
    if(oracle_inside(x,y,0,height-m,m,height)) color=0x0000FFFFu;
    if(oracle_inside(x,y,width-m,height-m,width,height)) color=0xFFFFFFFFu;
    pw=width/2; ph=height/3; px=(width-pw)/2; py=(height-ph)/2;
    if(oracle_inside(x,y,px,py,px+pw,py+ph)) color=0xF2F2F2FFu;
    ix=px+u; iy=py+u; iw=pw-2*u; ih=ph-2*u;
    if(iw<=0 || ih<=0) return color;
    if(oracle_inside(x,y,ix,iy,ix+iw,iy+ih)) color=0x2B2B2BFFu;
    for(long i=0;i<8;++i)
        if(oracle_inside(x,y,ix+iw*i/8,iy,ix+iw*(i+1)/8,iy+ih/2)) color=bars[i];
    tx=ix+u; tw=ix+iw-u-tx; ty=iy+ih/2+u; tb=iy+ih-u;
    if(oracle_inside(x,y,tx,ty,tx+tw,tb)) color=0x505050FFu;
    if(tw<=0) return color;
    for(long i=0;i<16;++i)
        if(oracle_inside(x,y,tx+tw*i/16,ty,tx+tw*(i+1)/16-u/2,tb))
            color=(uint32_t)i<step%17u?0x7FD35FFFu:0x3A3A3AFFu;
    return color;
}
#endif
