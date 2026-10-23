/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Abstract exclusion model,not CPU timing. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/inbox.h"
#include "../apps/window-scene/capture.h"
static cs_window_inbox box;
static cs_window_capture capture;
static cs_window_input owner;
static unsigned checks,depth,samples,cancels,action,initialized;
static int sample_rc,cancel_rc;
static cs_pointer_sample transcript[128];
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void enter(void){CHECK(depth==0);depth=1;}
static void leave(void){CHECK(depth==1);depth=0;}
static void produce(int32_t x,int32_t y,uint32_t b)
{CHECK(depth==0);CHECK(cs_inbox_push(&box,box.epoch,x,y,b)==CS_INBOX_OK);}
int cs_window_input_cancel(cs_window_input *p)
{
    CHECK(depth==0 && p==&owner);CHECK(capture.count==0);++cancels;
    if(initialized)CHECK(box.phase==CS_INBOX_LOSS || box.phase==CS_INBOX_ENDED);
    return cancel_rc;
}
int cs_window_input_sample(cs_window_input *p,const cs_pointer_sample *s)
{
    CHECK(depth==0 && p==&owner && samples<128);
    CHECK(box.claimed!=CS_INBOX_NONE);transcript[samples++]=*s;
    if(action==1){
        CHECK(box.phase==CS_INBOX_PRIMING);
        produce(10,-11,1);produce(20,22,0);
    }else if(action==2){
        CHECK(cs_inbox_uncertain(&box,box.epoch)==CS_INBOX_LOST);
    }else if(action==3){
        for(int32_t i=0;i<16;++i)produce(i,0,0);
        CHECK(cs_inbox_push(&box,box.epoch,17,0,0)==CS_INBOX_LOST);
    }
    action=0;return sample_rc;
}
static void fresh(void)
{
    memset(&box,0xA5,sizeof box);memset(&owner,0,sizeof owner);
    owner.valid=CS_WINDOW_INPUT_VALID;owner.phase=CS_WINDOW_INPUT_READY;
    depth=samples=cancels=action=initialized=0;
    sample_rc=CS_WINDOW_INPUT_CHANGED;cancel_rc=CS_WINDOW_INPUT_CHANGED;
    CHECK(cs_capture_begin(&capture,&owner)==CS_CAPTURE_OK);
    CHECK(cancels==1);enter();CHECK(cs_inbox_begin(&box)==CS_INBOX_OK);leave();initialized=1;
    CHECK(box.phase==CS_INBOX_WAIT && capture.phase==CS_CAPTURE_WAIT);
}
static int poll(void)
{
    enter();int item=cs_inbox_claim(&box,box.epoch);leave();
    if(item!=CS_INBOX_BASELINE_ITEM && item!=CS_INBOX_SAMPLE_ITEM)return item;
    int r;
    if(item==CS_INBOX_BASELINE_ITEM)r=cs_capture_rebaseline(&capture,capture.epoch,box.last.buttons);
    else{
        CHECK(capture.phase==CS_CAPTURE_READY && capture.count==0);
        r=cs_capture_push(&capture,capture.epoch,box.last.dx,box.last.dy,box.last.buttons);
        CHECK(r==CS_CAPTURE_OK);r=cs_capture_drain(&capture,capture.epoch);
    }
    int32_t cause=(r==CS_CAPTURE_OK || r==CS_CAPTURE_DELIVERED)?0:
        (capture.cause!=0?capture.cause:(int32_t)r);
    enter();int result=cs_inbox_complete(&box,box.epoch,cause);leave();return result;
}
static void recover(void)
{
    enter();CHECK(cs_inbox_discard(&box,box.epoch)==CS_INBOX_OK);leave();
    CHECK(box.phase==CS_INBOX_LOSS);
    int r=CS_CAPTURE_RESET;
    if(capture.phase==CS_CAPTURE_READY){
        CHECK(cs_capture_uncertain(&capture,capture.epoch)==CS_CAPTURE_LOST);
        r=cs_capture_drain(&capture,capture.epoch);
    }else CHECK(capture.phase==CS_CAPTURE_WAIT);
    enter();
    if(r==CS_CAPTURE_RESET)CHECK(cs_inbox_resume(&box,box.epoch)==CS_INBOX_BASELINE);
    else CHECK(cs_inbox_fault(&box,box.epoch,capture.cause)==CS_INBOX_FAULTED);
    leave();
}
static void stop(void)
{
    enter();CHECK(cs_inbox_stop(&box,box.epoch)==CS_INBOX_STOPPED);leave();
    int r=cs_capture_stop(&capture,capture.epoch);
    if(r==CS_CAPTURE_OWNER){enter();CHECK(cs_inbox_fault(&box,box.epoch,capture.cause)==CS_INBOX_FAULTED);leave();}
    else CHECK(r==CS_CAPTURE_STOPPED);
}
static void baseline(uint32_t buttons)
{CHECK(depth==0);CHECK(cs_inbox_baseline(&box,box.epoch,buttons)==CS_INBOX_OK);}
int main(void)
{
    fresh();baseline(0);action=1;CHECK(poll()==CS_INBOX_OK);
    CHECK(box.phase==CS_INBOX_READY && box.count==2 && capture.phase==CS_CAPTURE_READY);
    CHECK(poll()==CS_INBOX_OK);CHECK(poll()==CS_INBOX_OK);
    CHECK(samples==3 && transcript[1].dx==10 && transcript[1].dy==-11 && transcript[1].buttons==1);
    CHECK(transcript[2].dx==20 && transcript[2].dy==22 && transcript[2].buttons==0);
    /* Loss before baseline owner call:helper already WAIT,so no duplicate cancel. */
    fresh();baseline(1);CHECK(cs_inbox_uncertain(&box,1)==CS_INBOX_LOST);
    CHECK(poll()==CS_INBOX_LOST);recover();CHECK(cancels==1 && samples==0);
    /* Loss during successful baseline:helper now READY,so cancel once. */
    baseline(1);action=2;CHECK(poll()==CS_INBOX_LOST);
    CHECK(capture.phase==CS_CAPTURE_READY);recover();CHECK(cancels==2 && samples==1);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_BASELINE);CHECK(cancels==2);
    baseline(0);CHECK(poll()==CS_INBOX_OK);produce(7,0,0);produce(8,0,0);
    unsigned before=samples;action=2;CHECK(poll()==CS_INBOX_LOST);
    CHECK(samples==before+1 && transcript[before].dx==7 && box.count==1);
    CHECK(poll()==CS_INBOX_LOST);CHECK(samples==before+1);recover();
    CHECK(box.count==0 && cancels==3);baseline(0);CHECK(poll()==CS_INBOX_OK);
    CHECK(box.count==0); /* discarded8 is never replayed */
    /* Queue may fill while baseline redraw is running;no blocking render mask. */
    fresh();baseline(0);action=3;CHECK(poll()==CS_INBOX_LOST);
    CHECK(box.count==16 && samples==1);recover();CHECK(cancels==2);
    /* First raw owner failure wins over simultaneous detectable loss. */
    fresh();baseline(0);CHECK(poll()==CS_INBOX_OK);produce(1,0,0);produce(2,0,0);
    sample_rc=-47;action=2;CHECK(poll()==CS_INBOX_FAULTED);
    CHECK(box.phase==CS_INBOX_FAULT && box.cause==-47 && box.count==1);
    CHECK(capture.phase==CS_CAPTURE_FAULT && capture.cause==-47);
    CHECK(cs_inbox_push(&box,1,0,0,0)==CS_INBOX_FAULTED);
    cancel_rc=-55;stop();CHECK(box.phase==CS_INBOX_ENDED && box.cause==-47);
    CHECK(capture.phase==CS_CAPTURE_ENDED && capture.cause==-47);
    unsigned old=cancels;stop();CHECK(cancels==old);
    cancel_rc=CS_WINDOW_INPUT_CHANGED;sample_rc=CS_WINDOW_INPUT_CHANGED;initialized=0;
    CHECK(cs_capture_restart(&capture,&owner)==CS_CAPTURE_OK);
    enter();CHECK(cs_inbox_restart(&box)==CS_INBOX_OK);leave();initialized=1;
    CHECK(capture.epoch==2 && box.epoch==2 && box.cause==0);
    CHECK(cs_inbox_push(&box,1,0,0,0)==CS_INBOX_STATE);
    baseline(0);CHECK(poll()==CS_INBOX_OK);produce(-1,0,0);
    CHECK(poll()==CS_INBOX_OK);stop();
    CHECK(depth==0);
    printf("window-inbox interleavings:%u assertions;mock callbacks outside exclusion\n",checks);return 0;
}
