/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Bounded shared-compositor C16 fixture. The selected runtime library, if any,
 * is a separate experimental dependency and is not approved for release. */
#include "../../arch/i286/c16.h"
#include "../../apps/window-scene/input.h"

#if !defined(__WATCOMC__) || !defined(_M_I86)
#error Requires the pinned 16-bit Open Watcom target
#endif
CS_C16_ASSERT(cs_compositor_near, sizeof(void *) == 2);

static unsigned char canvas[21888];
static unsigned char saved[48];
static void post(unsigned char value);
#pragma aux post = "out 80h,al" parm [al] modify exact [];
static void video_mode(void);
#pragma aux video_mode = "mov ax,0010h" "int 10h" modify [ax bx cx dx si di];
static void fail(unsigned char value) { post(value); for (;;) { } }

void start(void)
{
    cs_surface surface;cs_cursor_state cursor;cs_cursor_token token;
    cs_cursor_bits bits;cs_cursor_art art;cs_window window;cs_window_config config;
    cs_product_state product;cs_window_scene_effect effect;
    unsigned char __far *video=(unsigned char __far *)0xa0000000UL;
    unsigned char __far *record=(unsigned char __far *)0x60000000UL;
    unsigned y,x;
    config.x=80;config.y=70;config.width=224;config.height=144;config.title=18;
    product.value=97;product.inputs=0;product.progress=16;product.phases=6;product.success=1;product.pixels=0;
    if(cs_window_begin(&window,&config)!=CS_WINDOW_OK)fail(0xe0);
    if(cs_surface_init(&surface,canvas,sizeof(canvas),512,342,64,CS_SURFACE_MONO1_MSB)!=CS_SURFACE_OK)fail(0xe1);
    cs_window_cursor_bits_init(&bits);art.bits=&bits;art.width=15;art.height=15;art.hot_x=7;art.hot_y=7;
    if(cs_cursor_init(&cursor)!=CS_CURSOR_OK || cs_cursor_bind(&cursor,&surface,saved,sizeof(saved),&token)!=CS_CURSOR_OK)fail(0xe2);
    if(cs_window_scene_compose(&cursor,token,&art,&product,0,&window,0,100,80,&effect)!=CS_WINDOW_SCENE_OK)fail(0xe3);
    video_mode();
    for(y=0;y<342;++y)for(x=0;x<64;++x)
        video[(y+4u)*80u+x+8u]=(unsigned char)~canvas[y*64u+x];
    record[0]='C';record[1]='2';record[2]='8';record[3]='6';record[4]=0x5a;
    post(0xab);for(;;) { }
}
