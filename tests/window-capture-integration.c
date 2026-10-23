/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Real shared-owner ordering/recovery checks. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/capture.h"
static uint8_t pixels[21888],payload[48];
static cs_window_input owner;
static cs_window_capture capture;
static cs_cursor_state cursor;
static cs_cursor_bits bits;
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void event(int32_t dx,int32_t dy,uint32_t buttons)
{
    CHECK(cs_capture_push(&capture,capture.epoch,dx,dy,buttons)==CS_CAPTURE_OK);
    CHECK(cs_capture_drain(&capture,capture.epoch)==CS_CAPTURE_DELIVERED);
    CHECK(owner.phase==CS_WINDOW_INPUT_READY);
}
int main(void)
{
    cs_surface s;cs_cursor_token token;
    cs_window_config cfg={80,70,224,144,18};
    cs_product_state product={97,0,16,6,1,NULL};
    bits.mask[0]=0x80u;bits.image[0]=0x80u;
    cs_cursor_art art={&bits,1,1,0,0};
    CHECK(cs_surface_init(&s,pixels,sizeof pixels,512,342,64,CS_SURFACE_MONO1_MSB)==CS_SURFACE_OK);
    CHECK(cs_cursor_init(&cursor)==CS_CURSOR_OK);
    CHECK(cs_cursor_bind(&cursor,&s,payload,sizeof payload,&token)==CS_CURSOR_OK);
    CHECK(cs_window_input_begin(&owner,&cursor,token,&art,&cfg,&product,0)==CS_WINDOW_INPUT_OK);
    CHECK(cs_capture_begin(&capture,&owner)==CS_CAPTURE_OK);
    CHECK(owner.pointer.suppress==1 && capture.phase==CS_CAPTURE_WAIT);
    CHECK(cs_capture_rebaseline(&capture,1,0)==CS_CAPTURE_OK);
    CHECK(owner.pointer.suppress==0 && owner.counter.presses==0);
    event(-256,-171,0);CHECK(owner.pointer.x==0 && owner.pointer.y==0);
    CHECK(cs_capture_push(&capture,1,-1,0,0)==CS_CAPTURE_OK);
    CHECK(cs_capture_push(&capture,1,1,0,0)==CS_CAPTURE_OK);
    CHECK(cs_capture_drain(&capture,1)==CS_CAPTURE_DELIVERED);CHECK(owner.pointer.x==0);
    CHECK(cs_capture_drain(&capture,1)==CS_CAPTURE_DELIVERED);CHECK(owner.pointer.x==1);
    event(119,79,0);event(0,0,1);
    CHECK(owner.window.dragging==1 && owner.counter.presses==1);
    event(20,10,1);CHECK(owner.window.x==100 && owner.window.y==80);
    event(0,0,0);CHECK(owner.window.dragging==0 && owner.counter.presses==1);
    event(60,70,0);event(0,0,1);event(0,0,0);
    CHECK(owner.counter.presses==2 && owner.window.pattern==1);
    event(-60,-70,0);event(0,0,1);CHECK(owner.window.dragging==1 && owner.counter.presses==3);
    for(unsigned i=0;i<16;++i)CHECK(cs_capture_push(&capture,1,1,0,1)==CS_CAPTURE_OK);
    CHECK(cs_capture_push(&capture,1,1,0,1)==CS_CAPTURE_LOST);
    CHECK(owner.pointer.x==140 && owner.pointer.y==89 && owner.window.dragging==1);
    CHECK(cs_capture_drain(&capture,1)==CS_CAPTURE_RESET);
    CHECK(owner.window.dragging==0 && owner.pointer.suppress==1 && owner.counter.presses==3);
    CHECK(cs_capture_rebaseline(&capture,1,1)==CS_CAPTURE_OK);
    event(0,0,1);CHECK(owner.counter.presses==3 && owner.pointer.suppress==1);
    event(0,0,0);CHECK(owner.pointer.suppress==0);
    event(0,0,1);CHECK(owner.counter.presses==4 && owner.window.dragging==1);
    CHECK(cs_capture_stop(&capture,1)==CS_CAPTURE_STOPPED);
    CHECK(owner.window.dragging==0 && owner.counter.presses==4);
    CHECK(cs_capture_restart(&capture,&owner)==CS_CAPTURE_OK && capture.epoch==2);
    CHECK(cs_capture_push(&capture,1,99,99,0)==CS_CAPTURE_STATE);
    CHECK(cs_capture_rebaseline(&capture,2,0)==CS_CAPTURE_OK);
    CHECK(owner.counter.presses==4 && capture.count==0);
    printf("window-capture real owner: %u assertions passed;ordered clamp/drag/loss/suppression\n",checks);
    return 0;
}
