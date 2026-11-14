/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Experimental C16 keyboard binding to the actual shared window owner and
 * compositor. Compiler runtime modules remain unreviewed for distribution. */
#include "../../arch/i286/c16.h"
#include "../../apps/window-scene/input.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_shared_window_near, sizeof(void *) == 2);
static unsigned char canvas[21888],saved[48];
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void video_mode(void);
#pragma aux video_mode = "mov ax,0010h" "int 10h" modify [ax bx cx dx si di];
static unsigned short read_key(void);
#pragma aux read_key = "xor ah,ah" "int 16h" value [ax] modify [ax bx cx dx si di];
static void fail(unsigned char value) { post(value); for (;;) { } }

static void white_display(void)
{
    unsigned i;unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    for(i=0;i<28000u;++i)video[i]=0xffu;
}

static void present(int left,int top,int right,int bottom)
{
    unsigned x,y,first,last,width;
    unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    unsigned char __far *dst;
    const unsigned char *src;
    if(left<0)left=0;if(top<0)top=0;if(right>512)right=512;if(bottom>342)bottom=342;
    if(left>=right || top>=bottom)return;
    first=(unsigned)left/8u;last=((unsigned)right+7u)/8u;
    width=last-first;
    src=canvas+(unsigned)top*64u+first;
    dst=video+((unsigned)top+4u)*80u+first+8u;
    for(y=(unsigned)top;y<(unsigned)bottom;++y) {
        for(x=0;x<width;++x)dst[x]=(unsigned char)~src[x];
        if(y+1u<(unsigned)bottom) {src+=64u;dst+=80u;}
    }
}

static void publish(const cs_window_input *o,unsigned count,unsigned button)
{
    unsigned char __far *r=(unsigned char __far *)0x60000000UL;
    r[0]='S';r[1]='2';r[2]='8';r[3]='6';r[4]=(unsigned char)count;r[5]=(unsigned char)button;
    r[6]=(unsigned char)o->pointer.x;r[7]=(unsigned char)(o->pointer.x>>8);
    r[8]=(unsigned char)o->pointer.y;r[9]=(unsigned char)(o->pointer.y>>8);
    r[10]=(unsigned char)o->window.x;r[11]=(unsigned char)(o->window.x>>8);
    r[12]=(unsigned char)o->window.y;r[13]=(unsigned char)(o->window.y>>8);
    r[14]=(unsigned char)o->window.pattern;r[15]=(unsigned char)o->window.dragging;
    r[16]=(unsigned char)o->counter.presses;
}

void start(void)
{
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;cs_cursor_bits bits;
    cs_cursor_art art;cs_window_config config;cs_product_state product;cs_window_input owner;
    cs_pointer_sample sample;unsigned key,scan,count=0,button=0;int result,old_x,old_y;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)fail(0xe0);
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)fail(0xe1);
    if(cs_window_input_begin(&owner,&cursor,token,&art,&config,&product,0)!=CS_WINDOW_INPUT_OK)fail(0xe2);
#ifdef CS_P286_PREPOSITION_TEST
    sample.dx=-156;sample.dy=-91;sample.buttons=0;
    result=cs_window_input_sample(&owner,&sample);
    if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)fail(0xe3);
#endif
    video_mode();white_display();present(0,0,512,342);publish(&owner,count,button);post(0xb0);
    for(;;) {
        key=read_key();scan=key>>8;sample.dx=0;sample.dy=0;
        if(scan==0x4b)sample.dx=-16;else if(scan==0x4d)sample.dx=16;
        else if(scan==0x48)sample.dy=-16;else if(scan==0x50)sample.dy=16;
        else if((key&0xffu)==0x20u)button^=1u;else continue;
        sample.buttons=button;old_x=(int)owner.pointer.x;old_y=(int)owner.pointer.y;
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xc0u+count));
#endif
        result=cs_window_input_sample(&owner,&sample);
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xd0u+count));
#endif
        if(result!=CS_WINDOW_INPUT_OK && result!=CS_WINDOW_INPUT_CHANGED)fail(0xe4);
        present((int)owner.render.dirty.left,(int)owner.render.dirty.top,
                (int)owner.render.dirty.right,(int)owner.render.dirty.bottom);
        /* A cursor-only move can report an empty scene dirty rectangle. The
         * saved old and new cursor footprints still need EGA publication. */
        present(old_x-7,old_y-7,old_x+8,old_y+8);
        present((int)owner.pointer.x-7,(int)owner.pointer.y-7,
                (int)owner.pointer.x+8,(int)owner.pointer.y+8);
#ifdef CS_P286_TIMING_MARKERS
        post((unsigned char)(0xe0u+count));
#endif
        ++count;publish(&owner,count,button);post((unsigned char)(0xb0u+(count&0x3fu)));
    }
}
