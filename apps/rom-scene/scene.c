/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../../platform/mac128k/rom.h"

static int box(const cs_surface *s,int32_t x,int32_t y,int32_t w,int32_t h,uint32_t v)
{
    /* Scene rectangles are bounded, not clipped. Preserve neighboring bits. */
    if(x<0 || y<0 || w<=0 || h<=0 || w>s->width || h>s->height ||
            x>s->width-w || y>s->height-h || v>1u) return 0;
    size_t first=(size_t)x/8u, last=(size_t)(x+w-1)/8u;
    uint8_t left=(uint8_t)(UINT32_C(0xFF)>>((uint32_t)x & 7u));
    uint8_t right=(uint8_t)(UINT32_C(0xFF)<<(7u-((uint32_t)(x+w-1) & 7u)));
    uint8_t *row=s->storage+(size_t)y*s->stride+first;
    for(int32_t j=0;j<h;++j) {
        for(size_t col=first;col<=last;++col) {
            uint8_t mask=0xFFu;
            if(col==first) mask&=left;
            if(col==last) mask&=right;
            uint8_t *p=row+(col-first);
            *p=v!=0u ? (uint8_t)(*p|mask) : (uint8_t)(*p & (uint8_t)~mask);
        }
        if(j+1<h) row+=s->stride;
    }
    return 1;
}

/* Original three-column, five-row lettering, independently authored for this scene. */
/* ClasSICK 0.97, including original lowercase letter forms. */
static const uint16_t letters[13]={0x7927u,0x6497u,0x0C7Fu,0x01D7u,0x79CFu,
    0x7497u,0x7927u,0x5BADu,0u,0x7B6Fu,0x0002u,0x7BCFu,0x7249u};
static const uint16_t digits[10]={0x7B6Fu,0x2492u,0x73E7u,0x73CFu,0x5BC9u,
    0x79CFu,0x79EFu,0x7249u,0x7BEFu,0x7BCFu};

static int glyph(const cs_surface *s,int32_t x,int32_t y,uint16_t bits,int32_t scale)
{
    int ok=1;
    for(unsigned row=0;row<5u;++row)
        for(unsigned col=0;col<3u;++col)
            if((bits & (UINT16_C(1) << (14u-row*3u-col)))!=0u)
                ok &= box(s,x+(int32_t)col*scale,y+(int32_t)row*scale,scale,scale,1);
    return ok;
}

static int number(const cs_surface *s,int32_t x,int32_t y,uint32_t value)
{
    int ok=1;
    for(unsigned i=0;i<4u;++i) {
        ok &= glyph(s,x+(int32_t)(3u-i)*20,y,digits[value%10u],4);
        value/=10u;
    }
    return ok;
}

int cs_mac_scene(const cs_surface *s,uint32_t ticks,uint32_t presses)
{
    int ok=1;
    uint32_t lit;
    cs_surface checked;
    if(s==NULL || s->format!=CS_SURFACE_MONO1_MSB || s->width!=512 || s->height!=342)
        return 0;
    if(cs_surface_init(&checked,s->storage,s->storage_size,s->width,s->height,
            s->stride,s->format)!=CS_SURFACE_OK) return 0;
    s=&checked;
    if(ticks>CS_MAC_TOTAL_TICKS) ticks=CS_MAC_TOTAL_TICKS;
    lit=ticks==CS_MAC_TOTAL_TICKS ? 16u : ticks/226u;
    ok &= box(s,0,0,512,342,0);
    ok &= box(s,8,8,496,4,1); ok &= box(s,8,330,496,4,1);
    ok &= box(s,8,8,4,326,1); ok &= box(s,500,8,4,326,1);
    for(unsigned i=0;i<13u;++i) ok &= glyph(s,52+(int32_t)i*32,30,letters[i],6);
    ok &= glyph(s,214,86,digits[0],6); ok &= glyph(s,246,86,digits[9],6);
    ok &= glyph(s,278,86,digits[7],6);
    ok &= box(s,48,128,416,80,1);
    ok &= box(s,52,132,408,72,0);
    for(unsigned i=0;i<8u;++i)
        ok &= box(s,60+(int32_t)i*48,140,32,56,(i+(presses & 1u)) & 1u);
    for(unsigned i=0;i<16u;++i) {
        ok &= box(s,48+(int32_t)i*26,232,20,28,1);
        if(i>=lit) ok &= box(s,50+(int32_t)i*26,234,16,24,0);
    }
    ok &= number(s,80,286,ticks);
    ok &= number(s,352,286,presses);
    return ok;
}
