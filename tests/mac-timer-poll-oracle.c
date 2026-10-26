/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "platform/mac128k/timer-poll.h"
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define C(x) do { ++checks; if(!(x)){printf("FAIL:%d:%s\n",__LINE__,#x);return 1;} } while(0)
typedef struct {
    unsigned writes,reads,pbs,ier_reads,mie_writes,bad_reg7,rr0reads,recurrence,unstable,alias,bad_acr;
    uint8_t rr15[2],lo,hi,acr,ifr,pb,a,b;
    uint32_t log[64];
} env;
static uint8_t vr(void *v,unsigned reg)
{
    env *e=v; ++e->reads;
    if(reg==14){++e->ier_reads;return 0;}
    if(reg==4){e->ifr=(uint8_t)(e->ifr&0xBFu);return 0;}
    if(reg==6)return e->lo;
    if(reg==7)return e->hi;
    if(reg==11)return e->bad_acr?0:e->acr;
    if(reg==13){if(e->recurrence && e->rr0reads>=4)e->ifr|=0x40;return e->ifr;}
    return 0;
}
static void vw(void *v,unsigned reg,uint8_t val)
{
    env *e=v;if(e->writes<64)e->log[e->writes]=(uint32_t)reg<<8|val;++e->writes;
    if(reg==4)e->lo=val;
    if(reg==5){e->hi=val;e->ifr=(uint8_t)(e->ifr&0xBFu);}
    if(reg==7)++e->bad_reg7;
    if(reg==11)e->acr=val;
}
static uint8_t sr(void *v,uint32_t ch,uint32_t reg)
{
    env *e=v;++e->reads;
    if(reg==15)return e->rr15[e->alias?0:ch];
    if(reg==0){++e->rr0reads;return ch==0?e->a:e->b;}
    return 0;
}
static void sw(void *v,uint32_t ch,uint32_t reg,uint8_t val)
{
    env *e=v;if(e->writes<64)e->log[e->writes]=0x10000000u|ch<<16|reg<<8|val;++e->writes;
    if(reg==15)e->rr15[e->alias?0:ch]=val;
    if(reg==9 && (val&8u))++e->mie_writes;
}
static uint8_t pb(void *v){env *e=v;++e->pbs;return (uint8_t)(e->pb^(e->unstable && (e->pbs&1u)==0?0x10:0));}
static int setup(cs_mac_timer_poll *p,cs_window_inbox *in,env *e)
{
    cs_mac_motion_io s={e,sr,sw,pb};cs_mac_io v={e,vr,vw};
    memset(e,0,sizeof(*e));e->pb=8;
    if(cs_inbox_begin(in)!=CS_INBOX_OK)return -1;
    return cs_mac_timer_poll_prepare(p,&s,&v,in,0,4096);
}
static int tick(cs_mac_timer_poll *p,env *e){e->ifr|=0x40;e->rr0reads=0;return cs_mac_timer_poll_service(p);}
int main(void)
{
    cs_mac_timer_poll p,before;cs_window_inbox in;env e;unsigned i,n;cs_mac_motion_io s;cs_mac_io v;
    static const uint32_t setup_writes[]={0xE40,0x100009C0,0x10000902,0x10000100,0x10000F08,0x10010100,0x10010F00,0x10000200,0x10000F00,0xB40,0x400,0x510};
    C(setup(&p,&in,&e)==CS_MAC_MOTION_OK);C(e.writes==12);
    for(i=0;i<12;i++)C(e.log[i]==setup_writes[i]);
    C(e.ier_reads==0 && e.mie_writes==0 && e.bad_reg7==0);C(p.motion.phase==CS_MAC_MOTION_PREPARED);
    C(cs_mac_timer_poll_arm(&p)==CS_MAC_MOTION_OK);C(e.log[e.writes-1]==0xEC0);
    n=e.pbs;C(cs_mac_timer_poll_service(&p)==CS_MAC_MOTION_EMPTY);C(e.pbs==n && p.motion.services==0);
    e.ifr=2;C(tick(&p,&e)==CS_MAC_MOTION_BASELINED);C(e.ifr==2 && in.count==1 && p.motion.baselines==1);
    C(cs_inbox_claim(&in,in.epoch)==CS_INBOX_BASELINE_ITEM);C(in.last.buttons==0 && in.last.dx==0);
    C(cs_inbox_complete(&in,in.epoch,0)==CS_INBOX_OK);C(in.phase==CS_INBOX_READY);
    e.a=8;C(tick(&p,&e)==CS_MAC_MOTION_OK);C(cs_inbox_claim(&in,in.epoch)==CS_INBOX_SAMPLE_ITEM);C(in.last.dx==-1 && in.last.dy==0 && in.last.buttons==0);C(cs_inbox_complete(&in,in.epoch,0)==CS_INBOX_OK);
    e.pb=24;C(tick(&p,&e)==CS_MAC_MOTION_EMPTY);C(in.count==0);
    e.unstable=1;C(tick(&p,&e)==CS_MAC_MOTION_LOST);C(in.phase==CS_INBOX_LOSS && p.motion.have_phase==0 && p.decoder.have_phase==0);
    before=p;n=e.pbs;C(tick(&p,&e)==CS_MAC_MOTION_EMPTY);C(e.pbs==n && p.motion.losses==before.motion.losses);
    C(cs_inbox_discard(&in,in.epoch)==CS_INBOX_OK);C(cs_inbox_resume(&in,in.epoch)==CS_INBOX_BASELINE);
    e.unstable=0;e.pb=16;C(tick(&p,&e)==CS_MAC_MOTION_BASELINED);C(cs_inbox_claim(&in,in.epoch)==CS_INBOX_BASELINE_ITEM);C(in.last.buttons==1 && in.last.dx==0);C(cs_inbox_complete(&in,in.epoch,0)==CS_INBOX_OK);
    e.recurrence=1;C(tick(&p,&e)==CS_MAC_MOTION_LOST);C(p.motion.losses==2 && p.motion.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    C(cs_mac_timer_poll_stop(&p)==CS_MAC_MOTION_OK);n=e.writes;C(cs_mac_timer_poll_stop(&p)==CS_MAC_MOTION_OK && e.writes==n);
    C(setup(&p,&in,&e)==CS_MAC_MOTION_OK);C(cs_mac_timer_poll_arm(&p)==CS_MAC_MOTION_OK);C(tick(&p,&e)==CS_MAC_MOTION_BASELINED);C(cs_inbox_claim(&in,in.epoch)==CS_INBOX_BASELINE_ITEM);C(cs_inbox_complete(&in,in.epoch,0)==CS_INBOX_OK);
    e.pb=24;e.a=8;C(tick(&p,&e)==CS_MAC_MOTION_LOST);C(p.motion.first_loss.reason==CS_MAC_TIMER_LOSS_GRAY);
    C(setup(&p,&in,&e)==CS_MAC_MOTION_OK);C(cs_mac_timer_poll_arm(&p)==CS_MAC_MOTION_OK);C(tick(&p,&e)==CS_MAC_MOTION_BASELINED);C(cs_inbox_claim(&in,in.epoch)==CS_INBOX_BASELINE_ITEM);C(cs_inbox_complete(&in,in.epoch,0)==CS_INBOX_OK);
    for(i=0;i<16;i++){e.pb=(uint8_t)((i&1u)?8:0);C(tick(&p,&e)==CS_MAC_MOTION_OK);}
    e.pb=0;C(tick(&p,&e)==CS_MAC_MOTION_LOST);C(in.phase==CS_INBOX_LOSS && p.motion.first_loss.reason==CS_MAC_LOSS_OVERFLOW);
    C(setup(&p,&in,&e)==CS_MAC_MOTION_OK);C(cs_mac_timer_poll_arm(&p)==CS_MAC_MOTION_OK);before=p;n=e.reads;in.epoch++;
    C(cs_mac_timer_poll_service(&p)==CS_MAC_MOTION_STATE);C(memcmp(&p,&before,sizeof(p))==0 && e.reads==n);
    s.cookie=&e;s.read=sr;s.write=sw;s.pb=pb;v.context=&e;v.read=vr;v.write=vw;
    C(cs_inbox_begin(&in)==CS_INBOX_OK);before=p;n=e.writes;C(cs_mac_timer_poll_prepare(&p,&s,&v,&in,0,0)==CS_MAC_MOTION_ARGUMENT);C(memcmp(&p,&before,sizeof(p))==0 && e.writes==n);
    e.alias=1;C(cs_mac_timer_poll_prepare(&p,&s,&v,&in,0,4096)==CS_MAC_MOTION_FAULT);C(p.motion.phase==CS_MAC_MOTION_FAILED && p.motion.stage==CS_MAC_STAGE_READBACK_A);
    e.alias=0;e.bad_acr=1;C(cs_mac_timer_poll_prepare(&p,&s,&v,&in,0,4096)==CS_MAC_MOTION_FAULT);C(p.motion.phase==CS_MAC_MOTION_FAILED);
    C(e.ier_reads==0 && e.mie_writes==0 && e.bad_reg7==0);
    printf("timer poll oracle:%u assertions\n",checks);return 0;
}
