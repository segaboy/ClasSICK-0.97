/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_TEST_SCENE_ORACLE_H
#define CLASSICK_TEST_SCENE_ORACLE_H
#include <stdint.h>
/* Independent SPEC-0037 point classifier; no renderer or image buffer. */
static int oracle_inside(long x,long y,long left,long top,long right,long bottom)
{
    return right>=left && bottom>=top && x>=left && x<right && y>=top && y<bottom;
}
static int oracle_digit(uint32_t digit,long x,long y,long scale)
{
    static const uint16_t bits[10]={0x7B6Fu,0x2492u,0x73E7u,0x73CFu,0x5BC9u,
        0x79CFu,0x79EFu,0x7249u,0x7BEFu,0x7BCFu};
    if(x<0 || y<0 || x>=3*scale || y>=5*scale)return 0;
    return (bits[digit]>>(14u-(unsigned)(y/scale)*3u-(unsigned)(x/scale)))&1u;
}
static uint32_t oracle_scene_inputs(long width,long height,uint32_t step,uint32_t inputs,long x,long y)
{
    if(width>512)x-=(width-512)/2;
    if(height>342)y-=(height-342)/2;
    int black=0;
    if(!oracle_inside(x,y,0,0,512,342))return 0xFFFFFFFFu;
    if(oracle_inside(x,y,8,8,504,334) && !oracle_inside(x,y,12,12,500,330))black=1;
    if(oracle_inside(x,y,36,28,84,84)){
        if(oracle_inside(x,y,40,28,80,32) || oracle_inside(x,y,36,32,84,70) ||
           oracle_inside(x,y,40,70,80,74) || oracle_inside(x,y,54,74,66,80) ||
           oracle_inside(x,y,42,80,78,84))black=1;
        if(oracle_inside(x,y,40,34,80,66))black=0;
        if(oracle_inside(x,y,50,40,72,44) || oracle_inside(x,y,46,44,52,60) ||
           oracle_inside(x,y,50,60,72,64) || oracle_inside(x,y,48,42,52,46) ||
           oracle_inside(x,y,48,58,52,62))black=1;
    }
    if(oracle_inside(x,y,112,34,424,54)){
        static const uint16_t text[13]={0x7927u,0x6497u,0x0C7Fu,0x01D7u,0x79CFu,
            0x7497u,0x7927u,0x5BADu,0,0x7B6Fu,0x0002u,0x7BCFu,0x7249u};
        long at=(x-112)/24,col=(x-112)%24;
        if(col<12 && (text[at]>>(14u-(unsigned)((y-34)/4)*3u-(unsigned)(col/4))&1u))black=1;
    }
    if(oracle_inside(x,y,238,65,274,80)){
        static const uint16_t app[3]={0x2BEDu,0x6BA4u,0x6BA4u};
        long at=(x-238)/12,col=(x-238)%12;
        if(col<9 && (app[at]>>(14u-(unsigned)((y-65)/3)*3u-(unsigned)(col/3))&1u))black=1;
    }
    if(y>=86 && y<116){
        if(oracle_digit(0,x-214,y-86,6)||oracle_digit(9,x-246,y-86,6)||oracle_digit(7,x-278,y-86,6))black=1;
    }
    if(oracle_inside(x,y,48,128,464,208)){
        black=!oracle_inside(x,y,52,132,460,204);
        if(oracle_inside(x,y,200,144,312,192) && !oracle_inside(x,y,208,152,304,184))black=1;
    }
    if(y>=214 && y<222 && x>=64 && x<424 && (x-64)%64<40)black=1;
    if(y>=232 && y<260 && x>=48 && x<464){
        long index=(x-48)/26,col=(x-48)%26;
        if(index<16 && col<20)black=(uint32_t)index<step%17u || col<2 || col>=18 || y<234 || y>=258;
    }
    if(y>=286 && y<306){
        const long starts[2]={80,352};const uint32_t values[2]={97,inputs};
        for(unsigned field=0;field<2;++field)if(x>=starts[field] && x<starts[field]+80){
            unsigned index=(unsigned)((x-starts[field])/20);uint32_t n=values[field];
            for(unsigned power=index;power<3;++power)n/=10;
            if(oracle_digit(n%10u,(x-starts[field])%20,y-286,4))black=1;
        }
    }
    return black?0x000000FFu:0xFFFFFFFFu;
}
static inline uint32_t oracle_scene(long width,long height,uint32_t step,long x,long y)
{return oracle_scene_inputs(width,height,step,0,x,y);}
#endif
