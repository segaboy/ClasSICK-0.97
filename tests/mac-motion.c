/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "../platform/mac128k/motion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"motion check %u line %d\n",checks,__LINE__);exit(1);} } while(0)
typedef struct { uint32_t ch,reg,value; } write_event;
typedef struct { uint32_t ch,reg,value,write,a_mask,b_mask,mie; } access_event;
typedef struct {
    uint8_t regs[2][16],raw[2],pb,pending;
    unsigned calls,writes,snapshots,unstable,alias,bad_read,scripted;
    write_event log[256];
    unsigned accesses;
    access_event access[512];
} chip;
static chip hw;
static struct cs_mac_motion m;
static cs_window_inbox inbox;
static void log_access(chip *p,uint32_t ch,uint32_t reg,uint32_t value,uint32_t write)
{
    CHECK(p->accesses<512);
    p->access[p->accesses++]=(access_event){ch,reg,value,write,
        p->regs[0][15],p->regs[1][15],p->regs[0][9]&8u};
}
static uint8_t rd(void *c,uint32_t ch,uint32_t reg)
{
    chip *p=c;p->calls++;CHECK(ch<2 && reg<16);
    uint8_t v=p->regs[p->alias?0:ch][reg];
    if(reg==0)v=p->raw[ch];
    if(reg==3){CHECK(ch==0);v=p->pending;}
    if(reg==15 && p->bad_read==ch+1u)v=(uint8_t)(ch==0?0:8);
    log_access(p,ch,reg,v,0);return v;
}
static void wr(void *c,uint32_t ch,uint32_t reg,uint8_t v)
{
    chip *p=c;p->calls++;CHECK(ch<2 && reg<16 && p->writes<256);
    p->log[p->writes++]=(write_event){ch,reg,v};
    p->regs[p->alias?0:ch][reg]=v;
    log_access(p,ch,reg,v,1);
    if(reg==0 && v==0x10)p->pending&=(uint8_t)(ch==0?0xF7u:0xFEu);
}
static uint8_t pb(void *c)
{
    chip *p=c;p->calls++;p->snapshots++;
    if(p->scripted){const uint8_t v[3]={0x88,0x48,0x28};return v[(p->snapshots-1u)%3u];}
    return (uint8_t)(p->pb ^ ((p->unstable && p->snapshots%3u==0u)?0x10u:0u));
}
static cs_mac_motion_io io={&hw,rd,wr,pb};
/* Contract oracle: partial ordering and observed masks, not a duplicate driver. */
static void prepared(void)
{
    unsigned aw=512,bw=512,ar=512,br=512,enabled=512,reads=0;
    CHECK(hw.regs[0][9]==2 && hw.regs[0][15]==8 && hw.regs[1][15]==8);
    for(unsigned i=0;i<hw.accesses;i++){
        const access_event *a=&hw.access[i];
        CHECK(a->reg!=12 && a->reg!=13 && !a->mie);
        if(a->reg!=15)continue;
        if(a->write){
            if(a->ch==0 && a->value==8)aw=i;
            if(a->ch==1 && a->value==0)bw=i;
            if(a->ch==1 && a->value==8)enabled=i;
        }else{
            CHECK(a->a_mask==8 && a->b_mask==0);
            if(a->ch==0){CHECK(a->value==8);ar=i;}
            else {CHECK(a->value==0);br=i;}
            reads++;
        }
    }
    CHECK(reads==2 && aw<ar && aw<br && bw<ar && bw<br);
    CHECK(ar<enabled && br<enabled && enabled<hw.accesses);
}
static void fresh(uint32_t invert)
{
    memset(&hw,0,sizeof hw);memset(&m,0xCC,sizeof m);hw.pb=8;
    CHECK(cs_inbox_begin(&inbox)==CS_INBOX_OK);
    CHECK(cs_mac_motion_prepare(&m,&io,&inbox,invert)==CS_MAC_MOTION_OK);
    prepared();
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
    for(unsigned fault=0;fault<3;fault++){
        memset(&hw,0,sizeof hw);hw.pb=8;hw.alias=fault==2;hw.bad_read=fault<2?fault+1u:0;
        CHECK(cs_inbox_begin(&inbox)==CS_INBOX_OK);
        CHECK(cs_mac_motion_prepare(&m,&io,&inbox,0)==CS_MAC_MOTION_FAULT);
        CHECK(m.phase==CS_MAC_MOTION_FAILED && m.cause==CS_MAC_CAUSE_READBACK && hw.regs[0][9]==2);
        CHECK(m.stage==(fault==1?CS_MAC_STAGE_READBACK_B:CS_MAC_STAGE_READBACK_A));
        for(unsigned i=0;i<hw.accesses;i++)CHECK(!hw.access[i].mie && hw.access[i].reg!=12);
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
    /* First-loss record: hand-encoded expectations. */
    fresh(0);hw.unstable=1;n=hw.calls;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && hw.calls-n==11);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_NONE);
    fresh(0);baseline();hw.unstable=1;n=hw.calls;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && hw.calls-n==11);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_UNSTABLE && m.first_loss.rr3==0);
    CHECK(m.first_loss.snapshots[0]==0x080000u && m.first_loss.snapshots[1]==0x080000u);
    CHECK(m.first_loss.snapshots[2]==0x180000u && m.first_loss.previous==0x01000000u);
    CHECK(m.first_loss.inbox_phase==CS_INBOX_READY);
    struct cs_mac_motion_loss first=m.first_loss;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(!memcmp(&first,&m.first_loss,sizeof first));
    CHECK(cs_inbox_discard(&inbox,1)==CS_INBOX_OK);
    CHECK(cs_inbox_resume(&inbox,1)==CS_INBOX_BASELINE);hw.unstable=0;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED && m.baselines==2);
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK);
    hw.raw[0]=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && m.losses==2);
    CHECK(!memcmp(&first,&m.first_loss,sizeof first));
    CHECK(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK);
    CHECK(!memcmp(&first,&m.first_loss,sizeof first));
    fresh(0);CHECK(m.first_loss.reason==CS_MAC_LOSS_NONE && !m.first_loss.previous);
    baseline();m.have_phase=0;hw.unstable=1;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_NO_PHASE && m.first_loss.previous==0);
    CHECK(m.first_loss.snapshots[2]==0x180000u && m.first_loss.inbox_phase==CS_INBOX_READY);
    fresh(0);baseline();hw.pending=8;n=hw.calls;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && hw.calls-n==12);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_PENDING && m.first_loss.rr3==8);
    for(unsigned i=0;i<3;i++)CHECK(m.first_loss.snapshots[i]==0x080000u);
    CHECK(m.first_loss.previous==0x01000000u && m.first_loss.inbox_phase==CS_INBOX_READY);
    fresh(0);baseline();
    for(unsigned i=0;i<16;i++){hw.pb=(uint8_t)((i&1u)?8:0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);}
    hw.pb=0;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_OVERFLOW && m.first_loss.rr3==0);
    for(unsigned i=0;i<3;i++)CHECK(m.first_loss.snapshots[i]==0u);
    CHECK(m.first_loss.previous==0x01000000u && m.first_loss.inbox_phase==CS_INBOX_READY);
    fresh(0);baseline();CHECK(cs_inbox_uncertain(&inbox,1)==CS_INBOX_LOST);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && m.first_loss.reason==CS_MAC_LOSS_NONE);
    fresh(0);baseline();hw.pending=2;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT && m.first_loss.reason==CS_MAC_LOSS_NONE);
    /* Malformed inbox can refuse uncertain(): hard fault, not a loss record. */
    fresh(0);baseline();inbox.count=17;hw.unstable=1;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT && m.first_loss.reason==CS_MAC_LOSS_NONE);
    fresh(0);
    const struct cs_mac_motion_loss empty={0};
    CHECK(!memcmp(&m.first_loss,&empty,sizeof empty));
    /* Full raw bytes are retained, including bits not used by admission. */
    hw.pb=0xC8;hw.raw[0]=0x80;hw.raw[1]=0x40;baseline();
    hw.pending=9;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && m.first_loss.reason==CS_MAC_LOSS_PENDING);
    for(unsigned i=0;i<3;i++)CHECK(m.first_loss.snapshots[i]==0xC88040u);
    CHECK(m.first_loss.rr3==9 && m.first_loss.previous==0x01000000u);
    fresh(0);hw.raw[0]=0x80;hw.raw[1]=0x40;baseline();hw.scripted=1;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    CHECK(m.first_loss.snapshots[0]==0x888040u);
    CHECK(m.first_loss.snapshots[1]==0x488040u);
    CHECK(m.first_loss.snapshots[2]==0x288040u);
    printf("Mac motion: %u checks,32 phase/inversion cases; synthetic IO, no target execution.\n",checks);
    return 0;
}
