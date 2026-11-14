/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "ega-mono.h"

int cs_pc286_ega_publish(const uint8_t *canvas, uint8_t CS_PC286_FAR *plane,
                         int left, int top, int right, int bottom)
{
    unsigned first,last,width,x,y;
    const uint8_t *src;
    uint8_t CS_PC286_FAR *dst;
    if(!canvas || !plane)return 0;
    if(left<0)left=0;
    if(top<0)top=0;
    if(right>512)right=512;
    if(bottom>342)bottom=342;
    if(left>=right || top>=bottom)return 1;
    first=(unsigned)left/8u;last=((unsigned)right+7u)/8u;width=last-first;
    src=canvas+(unsigned)top*64u+first;
    dst=plane+((unsigned)top+4u)*80u+first+8u;
    for(y=(unsigned)top;y<(unsigned)bottom;++y) {
        for(x=0;x<width;++x)dst[x]=(uint8_t)~src[x];
        if(y+1u<(unsigned)bottom) {src+=64u;dst+=80u;}
    }
    return 1;
}
