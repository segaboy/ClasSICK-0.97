/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen-API behavioral oracle. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../apps/window-scene/inbox.h"
static unsigned checks;
static cs_window_inbox box;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line%d:%s\n",__LINE__,#x);exit(1);}}while(0)
#define UNCHANGED(call,result) do{unsigned char snap[sizeof box];memcpy(snap,&box,sizeof box);CHECK((call)==(result));CHECK(memcmp(snap,&box,sizeof box)==0);}while(0)
static void sample(int32_t x,int32_t y,uint32_t b)
{CHECK(box.last.dx==x && box.last.dy==y && box.last.buttons==b);}
static void fresh(void)
{
    memset(&box,0xA5,sizeof box);CHECK(cs_inbox_begin(&box)==CS_INBOX_OK);
    CHECK(box.valid==CS_INBOX_VALID && box.epoch==1 && box.phase==CS_INBOX_WAIT);
    CHECK(box.head==0 && box.count==0 && box.claimed==0 && box.cause==0);
    CHECK(box.losses==0 && box.delivered==0 && box.loss_reason==CS_INBOX_STARTUP);
    sample(0,0,0);
}
static void ready(void)
{
    fresh();CHECK(cs_inbox_baseline(&box,1,0)==CS_INBOX_OK);
    CHECK(box.phase==CS_INBOX_PRIMING && box.count==1);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_BASELINE_ITEM);sample(0,0,0);
    CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);
    CHECK(box.phase==CS_INBOX_READY && box.delivered==0);
}
static void refusals(void)
{
    fresh();CHECK(cs_inbox_begin(NULL)==CS_INBOX_ARGUMENT);
    CHECK(cs_inbox_push(NULL,1,0,0,8)==CS_INBOX_ARGUMENT);
    CHECK(cs_inbox_claim(NULL,1)==CS_INBOX_ARGUMENT);
    UNCHANGED(cs_inbox_push(&box,1,1,2,0),CS_INBOX_BASELINE);
    UNCHANGED(cs_inbox_claim(&box,1),CS_INBOX_BASELINE);
    UNCHANGED(cs_inbox_complete(&box,1,0),CS_INBOX_BASELINE);
    UNCHANGED(cs_inbox_restart(&box),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_baseline(&box,0,0),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_fault(&box,0,-1),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_baseline(&box,1,8),CS_INBOX_ARGUMENT);
    UNCHANGED(cs_inbox_fault(&box,1,0),CS_INBOX_ARGUMENT);
    ready();UNCHANGED(cs_inbox_claim(&box,1),CS_INBOX_EMPTY);
    UNCHANGED(cs_inbox_complete(&box,1,0),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_baseline(&box,1,0),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_discard(&box,1),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_resume(&box,1),CS_INBOX_STATE);
    CHECK(cs_inbox_push(&box,1,INT32_MIN,INT32_MAX,7)==CS_INBOX_OK);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_SAMPLE_ITEM);sample(INT32_MIN,INT32_MAX,7);
    UNCHANGED(cs_inbox_claim(&box,1),CS_INBOX_STATE);
    CHECK(cs_inbox_push(&box,1,9,-9,0)==CS_INBOX_OK);sample(INT32_MIN,INT32_MAX,7);
    CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);
    CHECK(cs_inbox_stop(&box,1)==CS_INBOX_STOPPED);
    UNCHANGED(cs_inbox_stop(&box,1),CS_INBOX_STOPPED);
    UNCHANGED(cs_inbox_claim(&box,1),CS_INBOX_STOPPED);
    UNCHANGED(cs_inbox_uncertain(&box,1),CS_INBOX_STOPPED);
    CHECK(cs_inbox_fault(&box,1,-53)==CS_INBOX_FAULTED);
    CHECK(box.phase==CS_INBOX_ENDED && box.cause==-53);
    UNCHANGED(cs_inbox_fault(&box,1,-99),CS_INBOX_FAULTED);
    CHECK(cs_inbox_restart(&box)==CS_INBOX_OK);
    CHECK(box.epoch==2 && box.phase==CS_INBOX_WAIT && box.cause==0);
    UNCHANGED(cs_inbox_push(&box,1,9,9,0),CS_INBOX_STATE);
    CHECK(cs_inbox_stop(&box,2)==CS_INBOX_STOPPED);
    box.epoch=UINT32_MAX-1u;CHECK(cs_inbox_restart(&box)==CS_INBOX_OK);
    CHECK(box.epoch==UINT32_MAX);CHECK(cs_inbox_stop(&box,UINT32_MAX)==CS_INBOX_STOPPED);
    UNCHANGED(cs_inbox_restart(&box),CS_INBOX_EXHAUSTED);
}
static void invalid_metadata(void)
{
    for(unsigned which=0;which<9;++which){
        ready();
        switch(which){
        case 0:box.valid=0;break;case 1:box.epoch=0;break;case 2:box.phase=0;break;
        case 3:box.head=16;break;case 4:box.count=17;break;case 5:box.claimed=3;break;
        case 6:box.phase=CS_INBOX_WAIT;box.count=1;break;
        case 7:box.claimed=CS_INBOX_CLAIM_BASELINE;break;
        default:box.phase=CS_INBOX_FAULT;box.claimed=CS_INBOX_CLAIM_SAMPLE;break;}
        UNCHANGED(cs_inbox_claim(&box,box.epoch),CS_INBOX_STATE);
        UNCHANGED(cs_inbox_push(&box,box.epoch,0,0,8),CS_INBOX_ARGUMENT);
    }
}
static void ordered_wrap(void)
{
    ready();uint32_t total=0;
    /* Independent FIFO transcript:each round has16 distinct signed records,
     * consumed with pauses and fresh tail appends after every eight claims. */
    for(int32_t round=0;round<100;++round){
        for(int32_t i=0;i<16;++i)
            CHECK(cs_inbox_push(&box,1,round*100+i,-i,(uint32_t)i%8u)==CS_INBOX_OK);
        for(int32_t i=0;i<8;++i){
            CHECK(cs_inbox_claim(&box,1)==CS_INBOX_SAMPLE_ITEM);sample(round*100+i,-i,(uint32_t)i%8u);
            CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);++total;
        }
        for(int32_t i=16;i<24;++i)
            CHECK(cs_inbox_push(&box,1,round*100+i,-i,(uint32_t)i%8u)==CS_INBOX_OK);
        for(int32_t i=8;i<24;++i){
            CHECK(cs_inbox_claim(&box,1)==CS_INBOX_SAMPLE_ITEM);sample(round*100+i,-i,(uint32_t)i%8u);
            CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);++total;
        }
        CHECK(box.count==0 && box.delivered==total);
    }
    CHECK(total==2400);
}
static void losses_and_priming(void)
{
    ready();for(int32_t i=0;i<16;++i)CHECK(cs_inbox_push(&box,1,i,-i,1)==CS_INBOX_OK);
    unsigned char records[sizeof box.records];memcpy(records,box.records,sizeof records);
    CHECK(cs_inbox_push(&box,1,99,99,2)==CS_INBOX_LOST);
    CHECK(box.phase==CS_INBOX_LOSS && box.count==16 && box.loss_reason==CS_INBOX_OVERFLOW && box.losses==1);
    CHECK(memcmp(records,box.records,sizeof records)==0);
    UNCHANGED(cs_inbox_push(&box,1,2,3,4),CS_INBOX_LOST);
    UNCHANGED(cs_inbox_uncertain(&box,1),CS_INBOX_LOST);
    UNCHANGED(cs_inbox_claim(&box,1),CS_INBOX_LOST);
    UNCHANGED(cs_inbox_complete(&box,1,0),CS_INBOX_STATE);
    UNCHANGED(cs_inbox_resume(&box,1),CS_INBOX_STATE);
    CHECK(cs_inbox_discard(&box,1)==CS_INBOX_OK);
    CHECK(box.phase==CS_INBOX_LOSS && box.count==0);
    CHECK(cs_inbox_resume(&box,1)==CS_INBOX_BASELINE);
    CHECK(box.phase==CS_INBOX_WAIT && box.losses==1);
    CHECK(cs_inbox_uncertain(&box,1)==CS_INBOX_LOST);
    CHECK(box.losses==2);CHECK(cs_inbox_discard(&box,1)==CS_INBOX_OK);
    CHECK(cs_inbox_resume(&box,1)==CS_INBOX_BASELINE);
    CHECK(cs_inbox_baseline(&box,1,7)==CS_INBOX_OK);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_BASELINE_ITEM);sample(0,0,7);
    CHECK(cs_inbox_push(&box,1,-1,2,6)==CS_INBOX_OK);sample(0,0,7);
    CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);
    CHECK(box.phase==CS_INBOX_READY && box.count==1 && box.delivered==0);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_SAMPLE_ITEM);sample(-1,2,6);
    CHECK(cs_inbox_uncertain(&box,1)==CS_INBOX_LOST);
    UNCHANGED(cs_inbox_discard(&box,1),CS_INBOX_STATE);
    CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_LOST);
    CHECK(box.delivered==1 && box.phase==CS_INBOX_LOSS);
    CHECK(cs_inbox_discard(&box,1)==CS_INBOX_OK);CHECK(cs_inbox_resume(&box,1)==CS_INBOX_BASELINE);
    CHECK(cs_inbox_baseline(&box,1,1)==CS_INBOX_OK);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_push(&box,1,42,0,0)==CS_INBOX_OK);
    CHECK(cs_inbox_uncertain(&box,1)==CS_INBOX_LOST);
    CHECK(cs_inbox_complete(&box,1,-47)==CS_INBOX_FAULTED);
    CHECK(box.phase==CS_INBOX_FAULT && box.cause==-47 && box.count==1 && box.claimed==0);
    UNCHANGED(cs_inbox_fault(&box,1,-48),CS_INBOX_FAULTED);
    UNCHANGED(cs_inbox_push(&box,1,0,0,0),CS_INBOX_FAULTED);
    CHECK(cs_inbox_stop(&box,1)==CS_INBOX_STOPPED);CHECK(box.cause==-47);
}
static void saturation_and_stop(void)
{
    ready();box.delivered=UINT32_MAX;box.losses=UINT32_MAX;
    CHECK(cs_inbox_push(&box,1,0,0,0)==CS_INBOX_OK);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_SAMPLE_ITEM);
    CHECK(cs_inbox_complete(&box,1,0)==CS_INBOX_OK);CHECK(box.delivered==UINT32_MAX);
    CHECK(cs_inbox_uncertain(&box,1)==CS_INBOX_LOST);CHECK(box.losses==UINT32_MAX);
    CHECK(cs_inbox_discard(&box,1)==CS_INBOX_OK);CHECK(cs_inbox_resume(&box,1)==CS_INBOX_BASELINE);
    CHECK(cs_inbox_baseline(&box,1,0)==CS_INBOX_OK);
    CHECK(cs_inbox_claim(&box,1)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_stop(&box,1)==CS_INBOX_STOPPED);
    CHECK(box.count==0 && box.claimed==0 && box.head==0);
    UNCHANGED(cs_inbox_complete(&box,1,0),CS_INBOX_STOPPED);
    CHECK(cs_inbox_restart(&box)==CS_INBOX_OK);
    CHECK(box.epoch==2 && box.losses==0 && box.delivered==0);
}
int main(void)
{
    refusals();invalid_metadata();ordered_wrap();losses_and_priming();saturation_and_stop();
    printf("window-inbox contract:%u assertions;2400 ordered records;bytes=%zu\n",checks,sizeof box);return 0;
}
