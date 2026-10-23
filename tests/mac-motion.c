/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/motion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"motion check %u line %d\n",checks,__LINE__);exit(1);} } while(0)
typedef struct { uint32_t ch,reg,value; } write_event;
typedef struct {
    uint8_t regs[2][16],raw[2],pb,pending;
    unsigned calls,writes,snapshots,unstable,alias,bad_read;
    write_event log[256];
} chip;
static chip hw;
static struct cs_mac_motion m;
static cs_window_inbox inbox;
static uint8_t rd(void *c,uint32_t ch,uint32_t reg)
{
    chip *p=c;p->calls++;CHECK(ch<2 && reg<16);
    if(reg==0)return p->raw[ch];
    if(reg==3){CHECK(ch==0);return p->pending;}
    if(reg==12 && p->bad_read)return 0;
    return p->regs[p->alias?0:ch][reg];
}
static void wr(void *c,uint32_t ch,uint32_t reg,uint8_t v)
{
    chip *p=c;p->calls++;CHECK(ch<2 && reg<16 && p->writes<256);
    p->log[p->writes++]=(write_event){ch,reg,v};
    p->regs[p->alias?0:ch][reg]=v;
    if(reg==0 && v==0x10)p->pending&=(uint8_t)(ch==0?0xF7u:0xFEu);
}
static uint8_t pb(void *c)
{
    chip *p=c;p->calls++;p->snapshots++;
    return (uint8_t)(p->pb ^ ((p->unstable && p->snapshots%3u==0u)?0x10u:0u));
}
static cs_mac_motion_io io={&hw,rd,wr,pb};
static void fresh(uint32_t invert)
{
    memset(&hw,0,sizeof hw);memset(&m,0xCC,sizeof m);hw.pb=8;
    CHECK(cs_inbox_begin(&inbox)==CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&m,&io,&inbox,invert)==CS_MAC_MOTION_OK);
    CHECK(hw.regs[0][9]==2 && hw.regs[0][12]==0x35 && hw.regs[1][12]==0x6A);
    CHECK(!m.have_phase && inbox.phase==CS_INBOX_WAIT);
    CHECK(cs_mac_motion_arm(&m)==CS_MAC_MOTION_OK);
    CHECK(hw.log[hw.writes-1].reg==9 && hw.log[hw.writes-1].value==0x0A);
}
static void baseline(void)
{
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    CHECK(inbox.phase==CS_INBOX_PRIMING && inbox.count==1);
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK);
    CHECK(inbox.phase==CS_INBOX_READY && m.have_phase && m.baselines==1);
}
static void sample(int dx,int dy,unsigned buttons)
{
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_SAMPLE_ITEM);
    CHECK(inbox.last.dx==dx && inbox.last.dy==dy && inbox.last.buttons==buttons);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK);
}
int main(void)
{
    /* Frozen physical phase table, independently encoded before author read. */
    const int direction[2][2][2]={{{1,-1},{-1,1}},{{-1,1},{1,-1}}};
    for(unsigned inv=0;inv<4;inv++)for(unsigned axis=0;axis<2;axis++)
    for(unsigned phase=0;phase<2;phase++)for(unsigned second=0;second<2;second++){
        fresh(inv);
        unsigned raw=phase ^ ((inv>>axis)&1u);
        hw.raw[axis]=(uint8_t)(raw?0:8);baseline();
        hw.raw[axis]=(uint8_t)(raw?8:0);hw.pending=(uint8_t)(axis?1:8);
        hw.pb=(uint8_t)(8u|(second<<(axis?5:4)));
        sample(axis?0:direction[axis][phase][second],axis?direction[axis][phase][second]:0,0);
        CHECK(m.samples==1 && m.peak_queued==1 && !hw.pending);
        CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && !inbox.count);
    }
    fresh(0);hw.unstable=1;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && inbox.phase==CS_INBOX_WAIT);
    hw.unstable=0;baseline();
    hw.pb=0;sample(0,0,1);hw.pb=8;sample(0,0,0);
    hw.raw[0]=hw.raw[1]=8;hw.pb=24;hw.pending=9;sample(1,1,0);
    hw.unstable=1;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(inbox.phase==CS_INBOX_LOSS && !m.have_phase);
    unsigned n=inbox.count;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && inbox.count==n);
    CHECK(cs_inbox_discard(&inbox,1)==CS_INBOX_OK);
    CHECK(cs_inbox_resume(&inbox,1)==CS_INBOX_BASELINE);hw.unstable=0;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED && m.baselines==2);
    /* Overflow retains loss, never replays a partial prefix. */
    fresh(0);baseline();
    for(unsigned i=0;i<16;i++){hw.pb=(uint8_t)((i&1u)?8:0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);}
    hw.pb=0;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(inbox.count==16 && inbox.phase==CS_INBOX_LOSS && !m.have_phase && m.losses==1);
    for(unsigned pending=0;pending<2;pending++){
        fresh(0);baseline();hw.raw[0]=(uint8_t)(pending?0:8);hw.pending=(uint8_t)(pending?8:0);
        CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && inbox.loss_reason==CS_INBOX_UNCERTAIN);
    }
    fresh(0);baseline();m.services=UINT32_MAX;m.samples=UINT32_MAX;m.losses=UINT32_MAX;
    hw.pb=0;sample(0,0,1);CHECK(m.services==UINT32_MAX && m.samples==UINT32_MAX);
    hw.pending=2;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT);
    CHECK(m.phase==CS_MAC_MOTION_FAILED && m.cause==CS_MAC_CAUSE_PENDING && hw.regs[0][9]==2);
    unsigned stage=m.stage;int cause=m.cause;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_STATE && m.stage==stage && m.cause==cause);
    CHECK(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK);
    CHECK(!hw.regs[0][1] && !hw.regs[1][1] && !hw.regs[0][15] && !hw.regs[1][15]);
    n=hw.calls;CHECK(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK && hw.calls==n);
    fresh(0);inbox.epoch=2;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT && hw.regs[0][9]==2);
    fresh(0);CHECK(cs_inbox_stop(&inbox,1)==CS_INBOX_STOPPED);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT && hw.regs[0][9]==2);
    for(unsigned fault=0;fault<2;fault++){
        memset(&hw,0,sizeof hw);hw.pb=8;hw.alias=fault;hw.bad_read=!fault;
        CHECK(cs_inbox_begin(&inbox)==CS_INBOX_OK);
        CHECK(cs_mac_motion_prepare(&m,&io,&inbox,0)==CS_MAC_MOTION_FAULT);
        CHECK(m.phase==CS_MAC_MOTION_FAILED && m.cause==CS_MAC_CAUSE_READBACK && hw.regs[0][9]==2);
        n=hw.calls;CHECK(cs_mac_motion_arm(&m)==CS_MAC_MOTION_STATE && hw.calls==n);
    }
    memset(&m,0xCC,sizeof m);unsigned char old[sizeof m];memcpy(old,&m,sizeof m);
    n=hw.calls;CHECK(cs_mac_motion_prepare(&m,&io,&inbox,4)==CS_MAC_MOTION_ARGUMENT);
    CHECK(!memcmp(old,&m,sizeof m) && hw.calls==n);
    CHECK(cs_mac_motion_prepare(0,&io,&inbox,0)==CS_MAC_MOTION_ARGUMENT);
    cs_mac_motion_io invalid=io;invalid.pb=0;
    CHECK(cs_mac_motion_prepare(&m,&invalid,&inbox,0)==CS_MAC_MOTION_ARGUMENT && hw.calls==n);
    inbox.phase=CS_INBOX_READY;
    CHECK(cs_mac_motion_prepare(&m,&io,&inbox,0)==CS_MAC_MOTION_STATE && hw.calls==n);
    printf("Mac motion: %u checks,32 phase/inversion cases; synthetic IO, no target execution.\n",checks);
    return 0;
}
