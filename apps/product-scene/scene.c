/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original product artwork and lettering. */
#include "scene.h"

typedef struct {const cs_surface *s;int32_t width,height,top,x,y;} canvas;
static const uint16_t title[13]={0x7927u,0x6497u,0x0C7Fu,0x01D7u,0x79CFu,
    0x7497u,0x7927u,0x5BADu,0u,0x7B6Fu,0x0002u,0x7BCFu,0x7249u};
static const uint16_t digits[10]={0x7B6Fu,0x2492u,0x73E7u,0x73CFu,0x5BC9u,
    0x79CFu,0x79EFu,0x7249u,0x7BEFu,0x7BCFu};

static int fill(const canvas *c,int32_t x,int32_t y,int32_t w,int32_t h,uint32_t black)
{
    int32_t left=x+c->x,right=left+w,top=y+c->y,bottom=top+h;
    if(left<0)left=0;
    if(right>c->width)right=c->width;
    if(top<0)top=0;
    if(bottom>c->height)bottom=c->height;
    if(left>c->s->width)left=c->s->width;
    if(right>c->s->width)right=c->s->width;
    if(top<c->top)top=c->top;
    if(bottom>c->top+c->s->height)bottom=c->top+c->s->height;
    if(left>=right || top>=bottom)return 1;
    top-=c->top;bottom-=c->top;
    if(c->s->format==CS_SURFACE_RGBA8){
        cs_rect rect={left,top,right,bottom};
        return cs_surface_fill(c->s,rect,black?UINT32_C(0x000000FF):UINT32_C(0xFFFFFFFF))==CS_SURFACE_OK;
    }
    size_t first=(size_t)left/8u,last=(size_t)(right-1)/8u;
    uint8_t lm=(uint8_t)(UINT32_C(0xFF)>>((uint32_t)left&7u));
    uint8_t rm=(uint8_t)(UINT32_C(0xFF)<<(7u-((uint32_t)(right-1)&7u)));
    for(int32_t row=top;row<bottom;++row){
        uint8_t *p=c->s->storage+(size_t)row*c->s->stride;
        for(size_t col=first;col<=last;++col){
            uint8_t mask=0xFFu;if(col==first)mask&=lm;if(col==last)mask&=rm;
            p[col]=black?(uint8_t)(p[col]|mask):(uint8_t)(p[col]&(uint8_t)~mask);
        }
    }
    return 1;
}
static int glyph(const canvas *c,int32_t x,int32_t y,uint16_t bits,int32_t scale)
{
    int ok=1;
    for(unsigned row=0;row<5;++row)for(unsigned col=0;col<3;++col)
        if(bits&(UINT16_C(1)<<(14u-row*3u-col)))
            ok&=fill(c,x+(int32_t)col*scale,y+(int32_t)row*scale,scale,scale,1);
    return ok;
}
static int number(const canvas *c,int32_t x,uint32_t value)
{
    int ok=1;
    for(unsigned i=0;i<4;++i){
        ok&=glyph(c,x+(int32_t)(3u-i)*20,286,digits[value%10u],4);value/=10u;
    }
    return ok;
}
static int validate(const cs_surface *s,int32_t width,int32_t height,int32_t top,cs_surface *out)
{
    return s && s->width<=CS_PRODUCT_MAX_DIMENSION && s->height<=CS_PRODUCT_MAX_DIMENSION && width>0 && height>0 && width<=CS_PRODUCT_MAX_DIMENSION &&
        height<=CS_PRODUCT_MAX_DIMENSION && top>=0 && top<height &&
        cs_surface_init(out,s->storage,s->storage_size,s->width,s->height,s->stride,s->format)==CS_SURFACE_OK;
}
static void setup(canvas *c,const cs_surface *s,int32_t w,int32_t h,int32_t top)
{
    c->s=s;c->width=w;c->height=h;c->top=top;
    c->x=w>CS_PRODUCT_WIDTH?(w-CS_PRODUCT_WIDTH)/2:0;
    c->y=h>CS_PRODUCT_HEIGHT?(h-CS_PRODUCT_HEIGHT)/2:0;
}
int cs_product_scene(const cs_surface *s,int32_t width,int32_t height,int32_t top,
    const cs_product_state *state)
{
    cs_surface checked;canvas c;int ok=1;
    if(!state || state->phases>6 || state->success>1 || !validate(s,width,height,top,&checked))return 0;
    setup(&c,&checked,width,height,top);
    /* Clear margins too, while preserving descriptor padding/tail bytes. */
    canvas background;setup(&background,&checked,width,height,top);background.x=0;background.y=0;
    ok&=fill(&background,0,0,width,height,0);
    ok&=fill(&c,8,8,496,4,1);ok&=fill(&c,8,330,496,4,1);
    ok&=fill(&c,8,8,4,326,1);ok&=fill(&c,500,8,4,326,1);
    /* Original stepped monitor/C approximation of owner-supplied branding. */
    ok&=fill(&c,40,28,40,4,1);ok&=fill(&c,36,32,48,38,1);ok&=fill(&c,40,70,40,4,1);
    ok&=fill(&c,40,34,40,32,0);ok&=fill(&c,54,74,12,6,1);ok&=fill(&c,42,80,36,4,1);
    ok&=fill(&c,50,40,22,4,1);ok&=fill(&c,46,44,6,16,1);ok&=fill(&c,50,60,22,4,1);
    ok&=fill(&c,48,42,4,4,1);ok&=fill(&c,48,58,4,4,1);
    for(unsigned i=0;i<13;++i)ok&=glyph(&c,112+(int32_t)i*24,34,title[i],4);
    static const uint16_t app[3]={0x2BEDu,0x6BA4u,0x6BA4u};
    for(unsigned i=0;i<3;++i)ok&=glyph(&c,238+(int32_t)i*12,65,app[i],3);
    uint32_t value=state->value%1000u;
    ok&=glyph(&c,214,86,digits[value/100u],6);
    ok&=glyph(&c,246,86,digits[(value/10u)%10u],6);
    ok&=glyph(&c,278,86,digits[value%10u],6);
    ok&=fill(&c,48,128,416,80,1);ok&=fill(&c,52,132,408,72,0);
    if(state->pixels){
        for(unsigned y=0;y<64;++y)for(unsigned x=0;x<128;++x)
            if(state->pixels[y*16u+x/8u]&(0x80u>>(x&7u)))ok&=fill(&c,192+(int32_t)x,136+(int32_t)y,1,1,1);
    }else{
        ok&=fill(&c,200,144,112,48,1);ok&=fill(&c,208,152,96,32,0);
    }
    for(unsigned i=0;i<6;++i)if(i<state->phases)ok&=fill(&c,64+(int32_t)i*64,214,40,8,1);
    uint32_t progress=state->progress>16u?16u:state->progress;
    for(unsigned i=0;i<16;++i){
        ok&=fill(&c,48+(int32_t)i*26,232,20,28,1);
        if(i>=progress)ok&=fill(&c,50+(int32_t)i*26,234,16,24,0);
    }
    ok&=number(&c,80,state->value);ok&=number(&c,352,state->inputs);
    if(!state->success){ok&=fill(&c,20,124,16,88,1);ok&=fill(&c,476,124,16,88,1);}
    return ok;
}
int cs_product_scene_input(const cs_surface *s,uint32_t inputs)
{
    cs_surface checked;canvas c;
    if(!s || !validate(s,s->width,s->height,0,&checked))return 0;
    setup(&c,&checked,s->width,s->height,0);
    return fill(&c,352,286,80,20,0) && number(&c,352,inputs);
}
