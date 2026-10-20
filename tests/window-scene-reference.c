/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Contract-first full-image oracle. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/scene.h"
static unsigned checks,frames;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static void pixel(const cs_surface *s,int32_t x,int32_t y,int black)
{
    if(x<0 || x>=s->width || y<0 || y>=s->height)return;
    uint8_t *v=s->storage+(size_t)y*s->stride;
    if(s->format==CS_SURFACE_MONO1_MSB){
        uint8_t mask=(uint8_t)(128u>>((unsigned)x&7u));v+=(size_t)x/8u;
        *v=black?(uint8_t)(*v|mask):(uint8_t)(*v&(uint8_t)~mask);
    }else{v+=(size_t)x*4u;v[0]=black?0u:255u;v[1]=v[0];v[2]=v[0];v[3]=255u;}
}
static void expected(const cs_surface *s,const cs_product_state *p,const cs_window *w,const cs_cursor_art *art,int32_t px,int32_t py)
{
    int32_t ox=s->width>512?(s->width-512)/2:0,oy=s->height>342?(s->height-342)/2:0;
    CHECK(cs_product_scene(s,s->width,s->height,0,p));
    for(int32_t y=0;y<w->height;++y)for(int32_t x=0;x<w->width;++x){
        int black=x==0 || x==w->width-1 || y==0 || y==w->height-1 || y==w->title-1;
        if(!black && y>=w->title)black=w->pattern?((unsigned)x>>2)&1u:(((unsigned)x>>3)^((unsigned)(y-w->title)>>3))&1u;
        pixel(s,ox+w->x+x,oy+w->y+y,black);
    }
    for(unsigned y=0;y<art->height;++y)for(unsigned x=0;x<art->width;++x){
        int32_t lx=px-(int32_t)art->hot_x+(int32_t)x,ly=py-(int32_t)art->hot_y+(int32_t)y;
        unsigned index=y*2u+x/8u,mask=128u>>(x&7u);
        if(lx>=0 && lx<512 && ly>=0 && ly<342 && (art->bits->mask[index]&mask))pixel(s,ox+lx,oy+ly,(art->bits->image[index]&mask)!=0u);
    }
}
static cs_cursor_bits original_art(void)
{
    cs_cursor_bits bits={{0},{0}};
    for(unsigned y=0;y<11u;++y)for(unsigned x=0;x<=y/2u;++x){
        uint8_t mask=(uint8_t)(128u>>x);bits.mask[y*2u]|=mask;
        if(x==0u || x==y/2u || y==10u)bits.image[y*2u]|=mask;
    }
    return bits;
}
static void fixture(int32_t width,int32_t height,cs_surface_format format)
{
    size_t stride=(format==CS_SURFACE_MONO1_MSB?((size_t)width+7u)/8u:(size_t)width*4u)+7u;
    size_t length=stride*(size_t)height+17u;
    uint8_t *a=malloc(length+128u),*b=malloc(length+128u),payload[CS_CURSOR_RGBA_PAYLOAD];
    cs_surface actual,reference;
    cs_cursor_state cursor;
    cs_cursor_bits bits=original_art();cs_cursor_art art={&bits,7u,11u,2u,4u};
    cs_cursor_token token;
    cs_window window;
    const cs_window_config config={80,70,224,144,18};
    cs_window_scene_effect effect;
    cs_product_state product={97u,0u,16u,6u,1u,NULL};
    uint32_t seed=0x970054u;
    CHECK(a && b);memset(a,0xA5,length+128u);memset(b,0xA5,length+128u);memset(payload,0xC3,sizeof(payload));
    CHECK(cs_surface_init(&actual,a+64u,length,width,height,stride,format)==CS_SURFACE_OK);
    CHECK(cs_surface_init(&reference,b+64u,length,width,height,stride,format)==CS_SURFACE_OK);
    CHECK(cs_window_begin(&window,&config)==CS_WINDOW_OK);
    CHECK(cs_cursor_init(&cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&cursor,&actual,payload,sizeof(payload),&token)==CS_CURSOR_OK);
    CHECK(cs_window_scene_compose(&cursor,token,&art,&product,NULL,&window,0u,256,171,&effect)==CS_WINDOW_SCENE_OK);
    token=effect.token;CHECK(effect.stage==CS_WINDOW_SCENE_VISIBLE && effect.cause==0 && token.revision==2u);
    CHECK(effect.dirty.left==0 && effect.dirty.top==0 && effect.dirty.right==width && effect.dirty.bottom==height);
    expected(&reference,&product,&window,&art,256,171);CHECK(memcmp(a,b,length+128u)==0);++frames;
    for(unsigned n=0u;n<250u;++n){
        cs_window old=window;
        uint32_t old_inputs=product.inputs,old_revision=token.revision;
        seed=seed*1664525u+1013904223u;
        if(n%3u==0u){window.x=(int32_t)((seed>>4)%289u);window.y=(int32_t)((seed>>16)%199u);}
        if(n%11u==0u)window.pattern^=1u;
        if(n%7u==0u)product.inputs=n==0u?UINT32_MAX:product.inputs+1u;
        int changed=old.x!=window.x || old.y!=window.y || old.pattern!=window.pattern || old_inputs!=product.inputs;
        int32_t x=n%5u==0u?0:n%5u==1u?511:(int32_t)(seed%512u);
        int32_t y=n%5u==0u?0:n%5u==1u?341:(int32_t)((seed>>11)%342u);
        CHECK(cs_window_scene_compose(&cursor,token,&art,&product,&old,&window,old_inputs,x,y,&effect)==CS_WINDOW_SCENE_OK);
        token=effect.token;CHECK(token.revision==old_revision+(changed?1u:0u));
        CHECK(effect.stage==CS_WINDOW_SCENE_VISIBLE && effect.cause==0);
        expected(&reference,&product,&window,&art,x,y);
        if(memcmp(a,b,length+128u)!=0){fprintf(stderr,"mismatch %dx%d format%d frame%u\n",width,height,(int)format,n);CHECK(0);}
        ++frames;
    }
    CHECK(cs_cursor_hide(&cursor,token)==CS_CURSOR_OK);
    CHECK(cs_cursor_unbind(&cursor,token)==CS_CURSOR_OK);
    free(a);free(b);
}
int main(void)
{
    fixture(512,342,CS_SURFACE_MONO1_MSB);fixture(640,480,CS_SURFACE_MONO1_MSB);
    fixture(1024,768,CS_SURFACE_RGBA8);fixture(515,345,CS_SURFACE_MONO1_MSB);
    fixture(513,343,CS_SURFACE_RGBA8);fixture(127,83,CS_SURFACE_MONO1_MSB);
    fixture(9,7,CS_SURFACE_RGBA8);fixture(8192,2,CS_SURFACE_MONO1_MSB);
    printf("Window composition reference PASS:%u complete frames,%u checks\n",frames,checks);return 0;
}
