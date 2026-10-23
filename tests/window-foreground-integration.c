/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Real foreground owner,synthetic inbox only. */
#include <stdio.h>
#include <stdlib.h>
#include "../apps/window-scene/foreground.h"
static uint8_t pixels[21888],payload[48];
static cs_window_inbox box;
static cs_window_capture capture;
static cs_window_input owner;
static cs_cursor_state cursor;
static cs_cursor_bits bits;
static unsigned checks,depth;
static cs_window_foreground foreground;
static uint32_t enter(void *p){(void)p;if(depth)abort();depth=1;return 0xACE1u;}
static void leave(void *p,uint32_t t){(void)p;if(depth!=1 || t!=0xACE1u)abort();depth=0;}
static const cs_foreground_hooks hooks={enter,leave,NULL};
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void pump(void)
{
    int r=cs_foreground_step(&foreground);
    CHECK(r==CS_FG_BASELINED || r==CS_FG_DELIVERED);
    CHECK(depth==0);
}
static void event(int32_t x,int32_t y,uint32_t b)
{CHECK(cs_inbox_push(&box,box.epoch,x,y,b)==CS_INBOX_OK);pump();}
int main(void)
{
    cs_surface s;cs_cursor_token token;cs_window_config cfg={80,70,224,144,18};
    cs_product_state product={97,0,16,6,1,NULL};
    bits.mask[0]=0x80u;bits.image[0]=0x80u;cs_cursor_art art={&bits,1,1,0,0};
    CHECK(cs_surface_init(&s,pixels,sizeof pixels,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CHECK(cs_cursor_init(&cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&cursor,&s,payload,sizeof payload,&token)==CS_CURSOR_OK);
    CHECK(cs_window_input_begin(&owner,&cursor,token,&art,&cfg,&product,0)==CS_WINDOW_INPUT_OK);
    CHECK(cs_foreground_begin(&foreground,&box,&capture,&owner,&hooks)==CS_FG_OK);
    CHECK(cs_inbox_baseline(&box,1,0)==CS_INBOX_OK);pump();
    CHECK(owner.pointer.suppress==0 && owner.counter.presses==0);
    event(-176,-92,0);CHECK(owner.pointer.x==80 && owner.pointer.y==79);
    event(0,0,1);CHECK(owner.window.dragging==1 && owner.window.grab_x==0);
    event(-80,0,1);CHECK(owner.window.x==0 && owner.pointer.x==0);
    CHECK(cs_inbox_push(&box,1,-1,0,1)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&box,1,1,0,1)==CS_INBOX_OK);
    pump();CHECK(owner.pointer.x==0 && owner.window.x==0);
    pump();CHECK(owner.pointer.x==1 && owner.window.x==1);
    CHECK(owner.counter.presses==1);event(0,0,0);CHECK(owner.window.dragging==0);
    event(120,0,0);event(0,0,1);CHECK(owner.window.dragging==1 && owner.counter.presses==2);
    for(unsigned i=0;i<16;++i)CHECK(cs_inbox_push(&box,1,1,0,1)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&box,1,1,0,1)==CS_INBOX_LOST);
    CHECK(owner.pointer.x==121 && owner.window.dragging==1);
    CHECK(cs_foreground_step(&foreground)==CS_FG_RESET);
    CHECK(owner.window.dragging==0 && owner.pointer.suppress==1);
    CHECK(box.phase==CS_INBOX_WAIT);
    CHECK(cs_inbox_baseline(&box,1,1)==CS_INBOX_OK);pump();
    event(0,0,1);CHECK(owner.counter.presses==2 && owner.pointer.suppress==1);
    event(0,0,0);CHECK(owner.pointer.suppress==0);
    event(0,0,1);CHECK(owner.counter.presses==3 && owner.window.dragging==1);
    CHECK(cs_foreground_stop(&foreground)==CS_FG_STOPPED);
    CHECK(owner.window.dragging==0 && owner.counter.presses==3);
    printf("window-foreground real owner:%u assertions;drag clamp reversal/loss/release suppression\n",checks);return 0;
}
