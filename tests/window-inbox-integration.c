/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Real foreground owner,synthetic inbox only. */
#include <stdio.h>
#include <stdlib.h>
#include "../apps/window-scene/inbox.h"
#include "../apps/window-scene/capture.h"
static uint8_t pixels[21888],payload[48];
static cs_window_inbox box;
static cs_window_capture capture;
static cs_window_input owner;
static cs_cursor_state cursor;
static cs_cursor_bits bits;
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void pump(void)
{
    int item=cs_inbox_claim(&box,box.epoch);
    CHECK(item==CS_INBOX_BASELINE_ITEM || item==CS_INBOX_SAMPLE_ITEM);
    if(item==CS_INBOX_BASELINE_ITEM)
        CHECK(cs_capture_rebaseline(&capture,capture.epoch,box.last.buttons)==CS_CAPTURE_OK);
    else{
        CHECK(capture.phase==CS_CAPTURE_READY && capture.count==0);
        CHECK(cs_capture_push(&capture,capture.epoch,box.last.dx,box.last.dy,box.last.buttons)==CS_CAPTURE_OK);
        CHECK(cs_capture_drain(&capture,capture.epoch)==CS_CAPTURE_DELIVERED);
    }
    CHECK(cs_inbox_complete(&box,box.epoch,0)==CS_INBOX_OK);
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
    CHECK(cs_capture_begin(&capture,&owner)==CS_CAPTURE_OK);
    CHECK(cs_inbox_begin(&box)==CS_INBOX_OK);
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
    CHECK(cs_inbox_discard(&box,1)==CS_INBOX_OK);
    CHECK(cs_capture_uncertain(&capture,1)==CS_CAPTURE_LOST);
    CHECK(cs_capture_drain(&capture,1)==CS_CAPTURE_RESET);
    CHECK(owner.window.dragging==0 && owner.pointer.suppress==1);
    CHECK(cs_inbox_resume(&box,1)==CS_INBOX_BASELINE);
    CHECK(cs_inbox_baseline(&box,1,1)==CS_INBOX_OK);pump();
    event(0,0,1);CHECK(owner.counter.presses==2 && owner.pointer.suppress==1);
    event(0,0,0);CHECK(owner.pointer.suppress==0);
    event(0,0,1);CHECK(owner.counter.presses==3 && owner.window.dragging==1);
    CHECK(cs_inbox_stop(&box,1)==CS_INBOX_STOPPED);
    CHECK(cs_capture_stop(&capture,1)==CS_CAPTURE_STOPPED);
    CHECK(owner.window.dragging==0 && owner.counter.presses==3);
    printf("window-inbox real owner:%u assertions;drag clamp reversal/loss/release suppression\n",checks);return 0;
}
