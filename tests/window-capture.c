/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen-API call-order/fault oracle. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/capture.h"
static unsigned checks,cancels,samples;
static int sample_result,cancel_result;
static uint32_t expected_count=UINT32_MAX;
static cs_window_capture c,*active=&c;
static cs_window_input owner;
static cs_pointer_sample calls[1024];
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
int cs_window_input_cancel(cs_window_input *o)
{ CHECK(o==&owner);CHECK(active->count==0);++cancels;return cancel_result; }
int cs_window_input_sample(cs_window_input *o,const cs_pointer_sample *s)
{
    CHECK(o==&owner);CHECK(samples<1024u);
    CHECK(active->last.dx==s->dx && active->last.dy==s->dy && active->last.buttons==s->buttons);
    if(expected_count!=UINT32_MAX)CHECK(active->count==expected_count);
    calls[samples++]=*s;return sample_result;
}
static void fresh(void)
{
    memset(&c,0xA5,sizeof c);memset(&owner,0,sizeof owner);
    owner.valid=CS_WINDOW_INPUT_VALID;owner.phase=CS_WINDOW_INPUT_READY;
    sample_result=CS_WINDOW_INPUT_CHANGED;cancel_result=CS_WINDOW_INPUT_CHANGED;
    cancels=0;samples=0;expected_count=UINT32_MAX;
    CHECK(cs_capture_begin(&c,&owner)==CS_CAPTURE_OK);
    CHECK(c.valid==CS_CAPTURE_VALID && c.epoch==1 && c.phase==CS_CAPTURE_WAIT);
    CHECK(c.count==0 && c.head==0 && cancels==1 && samples==0);
    CHECK(c.loss_reason==CS_CAPTURE_STARTUP && c.losses==0 && c.delivered==0 && c.cause==0);
}
static void ready(uint32_t buttons)
{
    fresh();CHECK(cs_capture_rebaseline(&c,1,buttons)==CS_CAPTURE_OK);
    CHECK(c.phase==CS_CAPTURE_READY && samples==1);
    CHECK(calls[0].dx==0 && calls[0].dy==0 && calls[0].buttons==buttons);
    samples=0;
}
static void same(cs_window_capture old)
{CHECK(memcmp(&old,&c,sizeof c)==0);}
static void refuses(void)
{
    fresh();cs_window_capture old=c;
    CHECK(cs_capture_push(&c,1,1,0,0)==CS_CAPTURE_BASELINE);same(old);
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_BASELINE);same(old);
    CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_BASELINE);same(old);
    CHECK(cs_capture_rebaseline(&c,0,0)==CS_CAPTURE_STATE);same(old);
    CHECK(cs_capture_push(NULL,1,0,0,8)==CS_CAPTURE_ARGUMENT);
    CHECK(cs_capture_push(&c,1,0,0,8)==CS_CAPTURE_ARGUMENT);same(old);
    CHECK(cs_capture_rebaseline(&c,1,8)==CS_CAPTURE_ARGUMENT);same(old);
    CHECK(cs_capture_restart(&c,&owner)==CS_CAPTURE_STATE);same(old);
    CHECK(cs_capture_begin(NULL,&owner)==CS_CAPTURE_ARGUMENT);
    CHECK(cs_capture_begin(&c,NULL)==CS_CAPTURE_ARGUMENT);same(old);
    owner.phase=CS_WINDOW_INPUT_FAULTED;
    CHECK(cs_capture_begin(&c,&owner)==CS_CAPTURE_STATE);same(old);
    owner.phase=CS_WINDOW_INPUT_READY;
    CHECK(cs_capture_begin(&c,(cs_window_input *)(void *)&c)==CS_CAPTURE_ARGUMENT);same(old);
    ready(0);old=c;
    CHECK(cs_capture_rebaseline(&c,1,0)==CS_CAPTURE_STATE);same(old);
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_EMPTY);same(old);
    CHECK(cs_capture_push(&c,2,1,0,0)==CS_CAPTURE_STATE);same(old);
    for(unsigned mode=0;mode<6;++mode){
        c=old;
        switch(mode){case 0:c.valid=0;break;case 1:c.epoch=0;break;
        case 2:c.phase=99;break;case 3:c.head=16;break;case 4:c.count=17;break;default:c.owner=NULL;break;}
        cs_window_capture bad=c;
        CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_STATE);same(bad);
        CHECK(cs_capture_push(&c,1,0,0,0)==CS_CAPTURE_STATE);same(bad);
    }
    CHECK(cancels==1 && samples==0);
}
static void ordered(void)
{
    ready(0);
    const cs_pointer_sample steps[]={{-1,0,0},{1,0,0},{0,-1,0},{0,0,1},{5,3,1},{0,0,0},{INT32_MIN,INT32_MAX,7}};
    for(unsigned i=0;i<7;++i)CHECK(cs_capture_push(&c,1,steps[i].dx,steps[i].dy,steps[i].buttons)==CS_CAPTURE_OK);
    CHECK(samples==0 && c.count==7);
    for(unsigned i=0;i<7;++i){expected_count=6u-i;CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_DELIVERED);}
    CHECK(samples==7 && c.delivered==7 && c.count==0);
    for(unsigned i=0;i<7;++i)CHECK(calls[i].dx==steps[i].dx && calls[i].dy==steps[i].dy && calls[i].buttons==steps[i].buttons);
    expected_count=UINT32_MAX;cs_window_capture old=c;CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_EMPTY);same(old);
    /* Producer must not consult owner state or call it. */
    owner.valid=0;owner.phase=CS_WINDOW_INPUT_FAULTED;
    CHECK(cs_capture_push(&c,1,1,1,0)==CS_CAPTURE_OK);
    CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_LOST);CHECK(samples==7 && cancels==1);
}
static void overflow(void)
{
    ready(0);
    for(int32_t i=0;i<16;++i)CHECK(cs_capture_push(&c,1,i,-i,(uint32_t)i%8u)==CS_CAPTURE_OK);
    cs_pointer_sample before[16];memcpy(before,c.records,sizeof before);
    CHECK(cs_capture_push(&c,1,99,99,0)==CS_CAPTURE_LOST);
    CHECK(c.phase==CS_CAPTURE_LOSS && c.count==16 && c.loss_reason==CS_CAPTURE_OVERFLOW && c.losses==1);
    CHECK(memcmp(before,c.records,sizeof before)==0 && samples==0 && cancels==1);
    cs_window_capture old=c;
    CHECK(cs_capture_push(&c,1,0,0,0)==CS_CAPTURE_LOST);same(old);
    CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_LOST);same(old);
    CHECK(cs_capture_rebaseline(&c,1,0)==CS_CAPTURE_LOST);same(old);
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_RESET);
    CHECK(c.phase==CS_CAPTURE_WAIT && c.count==0 && c.head==0 && cancels==2 && samples==0);
    old=c;CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_BASELINE);same(old);CHECK(cancels==2);
    CHECK(cs_capture_rebaseline(&c,1,1)==CS_CAPTURE_OK);CHECK(samples==1 && calls[0].buttons==1);
    CHECK(cs_capture_push(&c,1,9,0,1)==CS_CAPTURE_OK);expected_count=0;
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_DELIVERED);CHECK(samples==2 && calls[1].dx==9);
    ready(0);c.losses=UINT32_MAX;
    CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_LOST);CHECK(c.losses==UINT32_MAX && c.count==0);
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_RESET);CHECK(cancels==2);
}
static void wrap(void)
{
    ready(0);
    for(int32_t i=0;i<16;++i)CHECK(cs_capture_push(&c,1,i,0,0)==CS_CAPTURE_OK);
    for(unsigned i=0;i<9;++i)CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_DELIVERED);
    for(int32_t i=16;i<25;++i)CHECK(cs_capture_push(&c,1,i,0,0)==CS_CAPTURE_OK);
    for(unsigned i=0;i<16;++i)CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_DELIVERED);
    CHECK(samples==25 && c.count==0);
    for(unsigned i=0;i<25;++i)CHECK(calls[i].dx==(int32_t)i);
    c.delivered=UINT32_MAX;
    for(int32_t i=0;i<128;++i){CHECK(cs_capture_push(&c,1,i,0,0)==CS_CAPTURE_OK);CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_DELIVERED);}
    CHECK(c.delivered==UINT32_MAX && samples==153);
}
static void failures(void)
{
    ready(0);CHECK(cs_capture_push(&c,1,1,2,0)==CS_CAPTURE_OK);CHECK(cs_capture_push(&c,1,3,4,0)==CS_CAPTURE_OK);
    sample_result=CS_WINDOW_INPUT_ARGUMENT;expected_count=1;
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_OWNER);
    CHECK(c.phase==CS_CAPTURE_FAULT && c.cause==CS_WINDOW_INPUT_ARGUMENT && c.stage==CS_CAPTURE_SAMPLE && c.count==1 && samples==1);
    cs_window_capture old=c;CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_OWNER);same(old);
    CHECK(cs_capture_push(&c,1,0,0,0)==CS_CAPTURE_OWNER);same(old);
    cancel_result=CS_WINDOW_INPUT_FAULT;
    CHECK(cs_capture_stop(&c,1)==CS_CAPTURE_OWNER);CHECK(c.phase==CS_CAPTURE_ENDED && c.count==0 && cancels==2);
    CHECK(c.cause==CS_WINDOW_INPUT_ARGUMENT && c.stage==CS_CAPTURE_SAMPLE);
    old=c;CHECK(cs_capture_stop(&c,1)==CS_CAPTURE_STOPPED);same(old);CHECK(cancels==2);
    cancel_result=CS_WINDOW_INPUT_OK;sample_result=CS_WINDOW_INPUT_OK;expected_count=UINT32_MAX;
    CHECK(cs_capture_restart(&c,&owner)==CS_CAPTURE_OK);CHECK(c.epoch==2 && c.phase==CS_CAPTURE_WAIT && c.cause==0 && c.delivered==0 && c.losses==0);
    old=c;CHECK(cs_capture_push(&c,1,1,1,0)==CS_CAPTURE_STATE);same(old);
    CHECK(cs_capture_rebaseline(&c,2,0)==CS_CAPTURE_OK);
    CHECK(cs_capture_push(&c,2,1,1,0)==CS_CAPTURE_OK);CHECK(cs_capture_drain(&c,2)==CS_CAPTURE_DELIVERED);
    ready(0);CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_LOST);cancel_result=-123;
    CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_OWNER);CHECK(c.cause==-123 && c.stage==CS_CAPTURE_LOSS_CANCEL);
    owner.phase=CS_WINDOW_INPUT_FAULTED;CHECK(cs_capture_stop(&c,1)==CS_CAPTURE_STOPPED);CHECK(cancels==2 && c.cause==-123);
    fresh();sample_result=-17;CHECK(cs_capture_rebaseline(&c,1,0)==CS_CAPTURE_OWNER);
    CHECK(c.phase==CS_CAPTURE_FAULT && c.cause==-17 && c.stage==CS_CAPTURE_REBASELINE);
    memset(&c,0xA5,sizeof c);owner.valid=CS_WINDOW_INPUT_VALID;owner.phase=CS_WINDOW_INPUT_READY;cancel_result=-8;
    CHECK(cs_capture_begin(&c,&owner)==CS_CAPTURE_OWNER);CHECK(c.phase==CS_CAPTURE_FAULT && c.cause==-8 && c.stage==CS_CAPTURE_BEGIN_CANCEL);
}
static void stopping(void)
{
    ready(0);CHECK(cs_capture_push(&c,1,4,5,0)==CS_CAPTURE_OK);
    CHECK(cs_capture_stop(&c,1)==CS_CAPTURE_STOPPED);CHECK(c.count==0 && c.head==0 && samples==0 && cancels==2);
    cs_window_capture old=c;CHECK(cs_capture_drain(&c,1)==CS_CAPTURE_STOPPED);same(old);
    CHECK(cs_capture_push(&c,1,0,0,0)==CS_CAPTURE_STOPPED);same(old);
    CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_STOPPED);same(old);
    CHECK(cs_capture_rebaseline(&c,1,0)==CS_CAPTURE_STOPPED);same(old);
    c.epoch=UINT32_MAX;old=c;CHECK(cs_capture_restart(&c,&owner)==CS_CAPTURE_EXHAUSTED);same(old);
    ready(0);CHECK(cs_capture_uncertain(&c,1)==CS_CAPTURE_LOST);
    CHECK(cs_capture_stop(&c,1)==CS_CAPTURE_STOPPED);CHECK(cancels==2 && c.count==0 && c.losses==1);
}
int main(void)
{
    CHECK(sizeof c<=256);refuses();ordered();overflow();wrap();failures();stopping();
    printf("window-capture contract oracle: %u assertions passed\n",checks);return 0;
}
