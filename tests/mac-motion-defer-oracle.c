/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Frozen before experimental author review. */
#include "platform/mac128k/motion.h"
#include <stdio.h>
#include <string.h>
typedef struct {uint8_t pb,a,b;} point;
static struct cs_mac_motion m;
static cs_window_inbox in;
static point sequence[3];
static uint8_t regs[2][16],pending;
static unsigned cursor,calls,checks,failures;
static void ok(int yes,const char *why){++checks;if(!yes){++failures;fprintf(stderr,"FAIL %s\n",why);}}
static uint8_t rd(void *c,uint32_t ch,uint32_t r)
{(void)c;++calls;if(r==0)return ch?sequence[cursor-1].b:sequence[cursor-1].a;if(r==3)return pending;return regs[ch][r];}
static void wr(void *c,uint32_t ch,uint32_t r,uint8_t v)
{(void)c;++calls;regs[ch][r]=v;if(r==0&&v==0x10)pending&=(uint8_t)(ch?0xFE:0xF7);}
static uint8_t pb(void *c){(void)c;++calls;return sequence[cursor++].pb;}
static void program(uint8_t p,point a,point b,point c)
{pending=p;sequence[0]=a;sequence[1]=b;sequence[2]=c;cursor=0;calls=0;}
static const point zero={8,0,0},ax0={24,0,0},ax1={24,8,0},ax2={8,8,0},by1={40,0,0},by2={40,0,8};
static void fresh(int ready)
{
    cs_mac_motion_io io={0,rd,wr,pb};memset(&m,0,sizeof m);memset(&in,0,sizeof in);memset(regs,0,sizeof regs);
    ok(cs_inbox_begin(&in)==CS_INBOX_OK,"begin");
    ok(cs_mac_motion_prepare(&m,&io,&in,0)==CS_MAC_MOTION_OK,"prepare");
    ok(cs_mac_motion_arm(&m)==CS_MAC_MOTION_OK,"arm");program(0,zero,zero,zero);
    ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED,"baseline");
    if(ready){ok(cs_inbox_claim(&in,1)==CS_INBOX_BASELINE_ITEM,"claim baseline");ok(cs_inbox_complete(&in,1,0)==CS_INBOX_OK,"complete baseline");}
}
static void defer_a(void)
{
    program(0,ax0,ax1,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY,"eligible A defer");
    ok(calls==11,"first pass original IO count");ok(m.defer_pending==8&&m.defer_snapshot==0x00080800UL,"literal A endpoint");
    ok(m.have_phase==1&&m.last_raw_a==0&&m.last_raw_b==0&&m.last_buttons==0,"remembered phase unchanged");
    ok(m.samples==0&&m.losses==0&&in.count==0,"no publication on defer");
}
static void clear(void){ok(m.defer_pending==0&&m.defer_snapshot==0,"defer state cleared");}
int main(void)
{
    fresh(1);defer_a();program(8,ax2,ax2,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK,"stable A completion");
    ok(calls==12&&pending==0,"second pass original IO and acknowledgement");clear();
    ok(in.count==1&&m.samples==1&&m.losses==0,"exactly one queued sample");
    ok(cs_inbox_claim(&in,1)==CS_INBOX_SAMPLE_ITEM,"claim A");ok(in.last.dx==-1&&in.last.dy==0&&in.last.buttons==0,"literal A record");
    ok(cs_inbox_complete(&in,1,0)==CS_INBOX_OK,"complete A");program(0,ax2,ax2,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY&&m.samples==1&&in.count==0,"no replay");
    fresh(1);program(0,zero,by1,by2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY,"eligible B defer");
    ok(m.defer_pending==1&&m.defer_snapshot==0x00280008UL&&m.last_raw_b==0,"literal B endpoint");
    program(1,by2,by2,by2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK,"stable B completion");clear();
    ok(cs_inbox_claim(&in,1)==CS_INBOX_SAMPLE_ITEM,"claim B");ok(in.last.dx==0&&in.last.dy==-1&&!in.last.buttons,"literal B record");
    for(unsigned failure=0;failure<6;++failure){
        point end=ax2,b=ax2;uint8_t request=8;fresh(1);defer_a();
        if(failure==0)request=0;
        if(failure==1)request=9;
        if(failure==2)end.pb=24;
        if(failure==3)end.pb=0;
        if(failure==4)end.b=8;
        b=end;if(failure==5)b.pb^=16;
        program(request,end,b,end);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST,"failed next pass is loss");clear();
        ok(in.phase==CS_INBOX_LOSS&&m.losses==1&&!m.have_phase&&in.count==0,"no uncertain publication");
        ok(m.first_loss.reason==(failure==5?CS_MAC_LOSS_UNSTABLE:CS_MAC_LOSS_PENDING),"retry failure reason");
    }
    fresh(1);program(0,(point){8,0,0},(point){8,8,8},(point){8,8,8});ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST,"two-axis first instability refuses");clear();
    fresh(1);program(0,zero,(point){24,8,0},ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST,"direction a-b-a refuses");clear();
    fresh(0);program(0,ax0,ax1,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST,"PRIMING cannot defer");clear();
    fresh(1);defer_a();program(2,ax2,ax2,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT,"unsupported pending remains hard fault");clear();ok(m.losses==0,"fault not loss");
    fresh(1);defer_a();in.epoch=2;program(8,ax2,ax2,ax2);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT,"wrong epoch hard fault");clear();
    fresh(1);defer_a();ok(cs_inbox_uncertain(&in,1)==CS_INBOX_LOST,"external loss");program(0,zero,zero,zero);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST,"LOSS clears pending");clear();
    fresh(1);defer_a();ok(cs_inbox_uncertain(&in,1)==CS_INBOX_LOST,"loss before reset");ok(cs_inbox_discard(&in,1)==CS_INBOX_OK,"discard");ok(cs_inbox_resume(&in,1)==CS_INBOX_BASELINE,"resume");program(0,zero,zero,zero);ok(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED,"WAIT rebaseline");clear();
    fresh(1);defer_a();ok(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK,"stop");clear();{unsigned n=calls;ok(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK&&calls==n,"repeat stop no IO");}
    printf("%u frozen checks, %u failures\n",checks,failures);return failures?1:0;
}
