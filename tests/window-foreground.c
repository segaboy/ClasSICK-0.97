/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Abstract exclusion,not CPU/IRQ timing. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/foreground.h"
static cs_window_foreground fg;
static cs_window_inbox box;
static cs_window_capture cap;
static cs_window_input owner;
static unsigned checks,depth,enters,leaves,samples,cancels,action,cancel_action;
static int sample_rc,cancel_rc;
static cs_pointer_sample seen[64];
static uint32_t token;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static uint32_t lock(void *cookie)
{ CHECK(cookie==&depth && depth==0);depth=1;++enters;token=0xCAFE0000u+enters;return token; }
static void unlock(void *cookie,uint32_t saved)
{ CHECK(cookie==&depth && depth==1 && saved==token);depth=0;++leaves; }
static const cs_foreground_hooks hooks={lock,unlock,&depth};
int cs_window_input_cancel(cs_window_input *p)
{
    CHECK(p==&owner && depth==0 && cap.count==0);++cancels;
    if(cancel_action){
        CHECK(box.phase==CS_INBOX_LOSS);
        CHECK(cs_inbox_uncertain(&box,box.epoch)==CS_INBOX_LOST);
        cancel_action=0;
    }
    return cancel_rc;
}
int cs_window_input_sample(cs_window_input *p,const cs_pointer_sample *s)
{
    CHECK(p==&owner && depth==0 && samples<64 && box.claimed!=CS_INBOX_NONE);
    seen[samples++]=*s;
    if(action==1 || action==3)CHECK(cs_inbox_uncertain(&box,box.epoch)==CS_INBOX_LOST);
    if(action==2){
        CHECK(cs_inbox_push(&box,box.epoch,11,-12,1)==CS_INBOX_OK);
        CHECK(cs_inbox_push(&box,box.epoch,-13,14,0)==CS_INBOX_OK);
    }
    action=0;return sample_rc;
}
static void fresh(void)
{
    memset(&fg,0xA5,sizeof fg);memset(&box,0xA5,sizeof box);memset(&cap,0xA5,sizeof cap);
    memset(&owner,0,sizeof owner);owner.valid=CS_WINDOW_INPUT_VALID;owner.phase=CS_WINDOW_INPUT_READY;
    depth=enters=leaves=samples=cancels=action=cancel_action=0;sample_rc=cancel_rc=CS_WINDOW_INPUT_OK;
    CHECK(cs_foreground_begin(&fg,&box,&cap,&owner,&hooks)==CS_FG_OK);
    CHECK(depth==0 && enters==leaves && cancels==1 && samples==0);
    CHECK(fg.phase==CS_FG_ACTIVE && box.phase==CS_INBOX_WAIT && cap.phase==CS_CAPTURE_WAIT);
    CHECK(fg.epoch==1 && box.epoch==1 && cap.epoch==1 && box.count==0 && cap.count==0);
}
static void baseline(uint32_t buttons)
{
    CHECK(cs_inbox_baseline(&box,box.epoch,buttons)==CS_INBOX_OK);
    CHECK(cs_foreground_step(&fg)==CS_FG_BASELINED);
    CHECK(box.phase==CS_INBOX_READY && cap.phase==CS_CAPTURE_READY && cap.count==0);
}
static void basic(void)
{
    fresh();CHECK(cs_foreground_step(&fg)==CS_FG_EMPTY);CHECK(samples==0 && cancels==1);
    baseline(0);CHECK(samples==1 && seen[0].dx==0 && seen[0].buttons==0);
    CHECK(cs_inbox_push(&box,box.epoch,7,-8,1)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&box,box.epoch,-9,10,0)==CS_INBOX_OK);
    sample_rc=CS_WINDOW_INPUT_CHANGED;
    CHECK(cs_foreground_step(&fg)==CS_FG_DELIVERED);
    CHECK(samples==2 && seen[1].dx==7 && seen[1].dy==-8 && seen[1].buttons==1 && box.count==1);
    CHECK(cs_foreground_step(&fg)==CS_FG_DELIVERED);
    CHECK(samples==3 && seen[2].dx==-9 && seen[2].dy==10 && seen[2].buttons==0 && box.count==0);
    CHECK(cs_foreground_step(&fg)==CS_FG_EMPTY && samples==3 && depth==0 && enters==leaves);
}
static void priming_and_loss(void)
{
    fresh();CHECK(cs_inbox_baseline(&box,box.epoch,1)==CS_INBOX_OK);action=2;
    CHECK(cs_foreground_step(&fg)==CS_FG_BASELINED && samples==1 && box.count==2);
    CHECK(cs_foreground_step(&fg)==CS_FG_DELIVERED && seen[1].dx==11);
    CHECK(cs_foreground_step(&fg)==CS_FG_DELIVERED && seen[2].dx==-13);
    CHECK(cs_inbox_push(&box,box.epoch,15,0,0)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&box,box.epoch,16,0,0)==CS_INBOX_OK);action=1;
    CHECK(cs_foreground_step(&fg)==CS_FG_DELIVERED);
    CHECK(box.phase==CS_INBOX_LOSS && box.count==1 && samples==4);
    cancel_action=1;
    CHECK(cs_foreground_step(&fg)==CS_FG_RESET);
    CHECK(box.phase==CS_INBOX_WAIT && box.count==0 && cancels==2 && samples==4);
    CHECK(cs_foreground_step(&fg)==CS_FG_EMPTY && cancels==2);
    baseline(0);CHECK(samples==5);
    fresh();CHECK(cs_inbox_uncertain(&box,box.epoch)==CS_INBOX_LOST);
    CHECK(cs_foreground_step(&fg)==CS_FG_RESET && cancels==1 && samples==0);
    CHECK(cs_inbox_baseline(&box,box.epoch,0)==CS_INBOX_OK);action=1;
    CHECK(cs_foreground_step(&fg)==CS_FG_BASELINED && cap.phase==CS_CAPTURE_READY);
    CHECK(box.phase==CS_INBOX_LOSS);
    CHECK(cs_foreground_step(&fg)==CS_FG_RESET && cancels==2);
}
static void fault_and_stop(void)
{
    unsigned calls,locks;
    fresh();baseline(0);
    CHECK(cs_inbox_push(&box,box.epoch,1,0,1)==CS_INBOX_OK);
    CHECK(cs_inbox_push(&box,box.epoch,2,0,0)==CS_INBOX_OK);
    sample_rc=37;action=3;
    CHECK(cs_foreground_step(&fg)==CS_FG_FAULT);
    CHECK(fg.cause==37 && box.cause==37 && box.phase==CS_INBOX_FAULT && box.count==1);
    CHECK(fg.stage==CS_FG_DRAIN && fg.phase==CS_FG_FAILED && cap.cause==37);
    calls=samples+cancels;locks=enters;
    CHECK(cs_foreground_step(&fg)==CS_FG_FAULT && samples+cancels==calls && enters==locks);
    cancel_rc=41;CHECK(cs_foreground_stop(&fg)==CS_FG_FAULT);
    CHECK(fg.phase==CS_FG_ENDED && fg.cause==37 && fg.stage==CS_FG_DRAIN && box.phase==CS_INBOX_ENDED);
    calls=samples+cancels;locks=enters;
    CHECK(cs_foreground_stop(&fg)==CS_FG_STOPPED && samples+cancels==calls && enters==locks);
    fresh();baseline(0);CHECK(cs_inbox_push(&box,box.epoch,3,0,1)==CS_INBOX_OK);
    CHECK(cs_capture_push(&cap,cap.epoch,99,0,0)==CS_CAPTURE_OK);calls=samples+cancels;
    CHECK(cs_foreground_step(&fg)==CS_FG_FAULT && samples+cancels==calls);
    CHECK(box.count==1 && box.phase==CS_INBOX_FAULT);
    fresh();baseline(0);CHECK(cs_inbox_push(&box,box.epoch,3,0,1)==CS_INBOX_OK);
    cap.phase=CS_CAPTURE_WAIT;calls=samples+cancels;
    CHECK(cs_foreground_step(&fg)==CS_FG_FAULT && samples+cancels==calls);
    CHECK(box.count==1 && box.claimed==CS_INBOX_NONE && box.phase==CS_INBOX_FAULT);
}
static void stop_restart(void)
{
    unsigned char f[sizeof fg],b[sizeof box],c[sizeof cap];unsigned calls,locks;
    fresh();baseline(0);CHECK(cs_inbox_push(&box,box.epoch,1,1,1)==CS_INBOX_OK);
    CHECK(cs_foreground_stop(&fg)==CS_FG_STOPPED);
    CHECK(box.phase==CS_INBOX_ENDED && box.count==0 && cap.phase==CS_CAPTURE_ENDED && cancels==2);
    fg.epoch=box.epoch=cap.epoch=UINT32_MAX;
    memcpy(f,&fg,sizeof fg);memcpy(b,&box,sizeof box);memcpy(c,&cap,sizeof cap);calls=cancels;
    CHECK(cs_foreground_restart(&fg,&owner)==CS_FG_EXHAUSTED && cancels==calls);
    CHECK(memcmp(f,&fg,sizeof fg)==0 && memcmp(b,&box,sizeof box)==0 && memcmp(c,&cap,sizeof cap)==0);
    fg.epoch=box.epoch=cap.epoch=10;
    cap.count=1;memcpy(f,&fg,sizeof fg);memcpy(b,&box,sizeof box);memcpy(c,&cap,sizeof cap);calls=cancels;
    CHECK(cs_foreground_restart(&fg,&owner)==CS_FG_STATE && cancels==calls);
    CHECK(memcmp(f,&fg,sizeof fg)==0 && memcmp(b,&box,sizeof box)==0 && memcmp(c,&cap,sizeof cap)==0);
    cap.count=0;
    CHECK(cs_foreground_restart(&fg,&owner)==CS_FG_OK && cancels==calls+1);
    CHECK(fg.epoch==11 && box.epoch==11 && cap.epoch==11 && fg.cause==0 && box.phase==CS_INBOX_WAIT);
    CHECK(cs_foreground_stop(&fg)==CS_FG_STOPPED);cancel_rc=43;
    CHECK(cs_foreground_restart(&fg,&owner)==CS_FG_FAULT);
    CHECK(fg.phase==CS_FG_ENDED && fg.cause==43 && fg.stage==CS_FG_RESTART && box.phase==CS_INBOX_ENDED);
    calls=cancels;locks=enters;
    CHECK(cs_foreground_stop(&fg)==CS_FG_STOPPED && cancels==calls && enters==locks);
    CHECK(cs_foreground_restart(&fg,&owner)==CS_FG_STATE && cancels==calls);
}
static void preflight(void)
{
    cs_foreground_hooks bad=hooks;unsigned char f[sizeof fg],b[sizeof box],c[sizeof cap];
    unsigned calls;
    fresh();CHECK(cs_foreground_stop(&fg)==CS_FG_STOPPED);
    memcpy(f,&fg,sizeof fg);memcpy(b,&box,sizeof box);memcpy(c,&cap,sizeof cap);calls=cancels;
    CHECK(cs_foreground_restart(&fg,0)==CS_FG_ARGUMENT && cancels==calls);
    CHECK(memcmp(f,&fg,sizeof fg)==0 && memcmp(b,&box,sizeof box)==0 && memcmp(c,&cap,sizeof cap)==0);
    bad.leave=0;CHECK(cs_foreground_begin(&fg,&box,&cap,&owner,&bad)==CS_FG_ARGUMENT);
    CHECK(memcmp(f,&fg,sizeof fg)==0 && memcmp(b,&box,sizeof box)==0 && memcmp(c,&cap,sizeof cap)==0);
    CHECK(cs_foreground_step(0)==CS_FG_ARGUMENT && cs_foreground_stop(0)==CS_FG_ARGUMENT);
    CHECK(depth==0 && enters==leaves);
    cancel_rc=31;
    CHECK(cs_foreground_begin(&fg,&box,&cap,&owner,&hooks)==CS_FG_FAULT);
    CHECK(fg.phase==CS_FG_FAILED && fg.cause==31 && box.phase==CS_INBOX_FAULT && box.cause==31);
    CHECK(fg.stage==CS_FG_BEGIN && box.count==0 && box.claimed==CS_INBOX_NONE);
}
int main(void)
{basic();priming_and_loss();fault_and_stop();stop_restart();preflight();printf("foreground:%u checks\n",checks);return 0;}
