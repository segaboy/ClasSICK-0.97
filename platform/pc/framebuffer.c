/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "framebuffer.h"

static cs_fb_result check(volatile unsigned char *base,size_t size,uint32_t width,
    uint32_t height,uint32_t pitch,uint32_t format)
{
    uint64_t span;
    if(base==NULL) return CS_FB_ARGUMENT;
    if(format!=CS_FB_RGB_RESERVED && format!=CS_FB_BGR_RESERVED) return CS_FB_FORMAT;
    if(width==0 || height==0 || width>CS_FB_MAX_DIMENSION || height>CS_FB_MAX_DIMENSION
            || pitch<width || pitch>CS_FB_MAX_PITCH) return CS_FB_DIMENSION;
    /* Profile bounds keep this below 2^32; compare before narrowing to size_t. */
    span=(uint64_t)pitch*4u*(uint64_t)height;
    if(span>(uint64_t)SIZE_MAX || span>(uint64_t)PTRDIFF_MAX) return CS_FB_OVERFLOW;
    if(size<(size_t)span) return CS_FB_STORAGE;
    return CS_FB_OK;
}

cs_fb_result cs_fb_init(cs_fb_target *out,volatile unsigned char *base,size_t size,
    uint32_t width,uint32_t height,uint32_t pitch,uint32_t format)
{
    cs_fb_result result;
    if(out==NULL) return CS_FB_ARGUMENT;
    result=check(base,size,width,height,pitch,format);
    if(result!=CS_FB_OK) return result;
    out->base=base; out->size=size; out->width=width; out->height=height;
    out->pitch=pitch; out->format=format;
    return CS_FB_OK;
}

cs_fb_result cs_fb_present(const cs_fb_target *target,const cs_surface *source,
    int32_t left,int32_t top)
{
    cs_surface checked;
    cs_fb_result result;
    int64_t x0,x1,y0,y1;
    int swap;
    if(target==NULL || source==NULL) return CS_FB_ARGUMENT;
    result=check(target->base,target->size,target->width,target->height,
        target->pitch,target->format);
    if(result!=CS_FB_OK) return result;
    if(cs_surface_init(&checked,source->storage,source->storage_size,source->width,
            source->height,source->stride,source->format)!=CS_SURFACE_OK
            || checked.format!=CS_SURFACE_RGBA8) return CS_FB_SOURCE;
    x0=left<0?0:(int64_t)left;
    y0=top<0?0:(int64_t)top;
    x1=(int64_t)left+(int64_t)checked.width;
    y1=(int64_t)top+(int64_t)checked.height;
    if(x1>(int64_t)target->width) x1=(int64_t)target->width;
    if(y1>(int64_t)target->height) y1=(int64_t)target->height;
    if(x0>=x1 || y0>=y1) return CS_FB_OK;
    /* RGB places red in byte 0; BGR places blue there. Byte 3 is always zero. */
    swap=target->format==CS_FB_BGR_RESERVED;
    for(int64_t y=y0;y<y1;++y) {
        const unsigned char *in=checked.storage+(size_t)(y-(int64_t)top)*checked.stride
            +(size_t)(x0-(int64_t)left)*4u;
        volatile unsigned char *out=target->base+(size_t)y*(size_t)target->pitch*4u+(size_t)x0*4u;
        for(int64_t x=x0;x<x1;++x) {
            out[0]=swap?in[2]:in[0];
            out[1]=in[1];
            out[2]=swap?in[0]:in[2];
            out[3]=0;
            in+=4; out+=4;
        }
    }
    return CS_FB_OK;
}
