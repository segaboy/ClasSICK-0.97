/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original fast-collection contract checks. */
#if CLASSICK_MAC_FAST_COLLECT != 1
#error Build this test with CLASSICK_MAC_FAST_COLLECT=1
#endif
#define main existing_motion_fixture_main
#include "mac-motion.c"
#undef main

/* Compile the production collector body against the instrumented chip. */
static uint8_t scc_read(void *c,uint32_t ch,uint32_t reg){return rd(c,ch,reg);}
static void scc_write(void *c,uint32_t ch,uint32_t reg,uint8_t v){wr(c,ch,reg,v);}
static uint8_t motion_pb(void *c){return pb(c);}
#include "../platform/mac128k/fast-collect.inc"

static void actual_collector_case(uint8_t pending)
{
    uint8_t out[10];
    unsigned start, snapshots, at;
    fresh(0);memset(out,0xA5,sizeof out);
    hw.raw[0]=0x20;hw.raw[1]=0x40;hw.pb=0x88;hw.pending=pending;
    start=hw.accesses;snapshots=hw.snapshots;
    cs_mac_native_collect(&hw,out);
    CHECK(out[0]==pending && hw.access[start].reg==3 && !hw.access[start].write);
    if(pending&0xF6u){
        CHECK(hw.accesses==start+1u && hw.snapshots==snapshots);
        for(unsigned i=1;i<10u;i++)CHECK(out[i]==0xA5);
        return;
    }
    CHECK(hw.snapshots==snapshots+3u);
    for(unsigned i=0;i<3u;i++){
        CHECK(out[1u+3u*i]==0x88 && out[2u+3u*i]==0x20 && out[3u+3u*i]==0x40);
    }
    at=start+1u;
    for(unsigned i=0;i<2u;i++){
        CHECK(!hw.access[at].write && hw.access[at].ch==0 && hw.access[at].reg==0);at++;
        CHECK(!hw.access[at].write && hw.access[at].ch==1 && hw.access[at].reg==0);at++;
    }
    if(pending&8u){CHECK(hw.access[at].write && hw.access[at].ch==0 && hw.access[at].value==0x10);at++;}
    if(pending&1u){CHECK(hw.access[at].write && hw.access[at].ch==1 && hw.access[at].value==0x10);at++;}
    CHECK(hw.access[at].write && hw.access[at].ch==0 && hw.access[at].value==0x38);at++;
    CHECK(!hw.access[at].write && hw.access[at].ch==0 && hw.access[at].reg==0);at++;
    CHECK(!hw.access[at].write && hw.access[at].ch==1 && hw.access[at].reg==0);at++;
    CHECK(hw.accesses==at && hw.pending==0);
}

int main(void)
{
    unsigned k, n, pb_before;
    static const uint32_t ch[10]={0,0,1,0,1,0,1,0,0,1};
    static const uint32_t reg[10]={3,0,0,0,0,0,0,0,0,0};
    static const uint32_t write[10]={0,0,0,0,0,1,1,1,0,0};
    static const uint32_t val[3]={0x10,0x10,0x38};
    io.collect=cs_mac_native_collect;
    for(unsigned i=0;i<5u;i++){
        static const uint8_t cases[5]={0,1,8,9,0x0A};
        actual_collector_case(cases[i]);
    }
    fresh(0); baseline(); hw.raw[0]=hw.raw[1]=8; hw.pending=9; hw.pb=8;
    k=hw.accesses; pb_before=hw.snapshots;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_OK);
    CHECK(hw.accesses-k==10 && hw.snapshots-pb_before==3u && !hw.pending);
    for(unsigned i=0,j=0;i<10u;i++){
        const access_event *a=&hw.access[k+i];
        CHECK(a->ch==ch[i] && a->reg==reg[i] && a->write==write[i]);
        if(a->write)CHECK(a->value==val[j++]);
    }
    fresh(0); baseline(); hw.pending=2; k=hw.accesses; pb_before=hw.snapshots;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT);
    CHECK(hw.accesses-k==2 && hw.snapshots==pb_before);
    CHECK(!hw.access[k].write && hw.access[k].reg==3);
    CHECK(hw.access[k+1u].write && hw.access[k+1u].reg==9);
    CHECK(cs_inbox_begin(&inbox)==CS_INBOX_OK);
    cs_mac_motion_io invalid=io; invalid.collect=NULL; n=hw.calls;
    CHECK(cs_mac_motion_prepare(&m,&invalid,&inbox,0)==CS_MAC_MOTION_ARGUMENT);
    CHECK(hw.calls==n);
    puts("mac-motion-fast: passed");
    return 0;
}
