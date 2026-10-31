/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original fast-collection contract checks. */
#if CLASSICK_MAC_FAST_COLLECT != 1
#error Build this test with CLASSICK_MAC_FAST_COLLECT=1
#endif
#define main existing_motion_fixture_main
#include "mac-motion.c"
#undef main

int main(void)
{
    unsigned k, n, pb_before;
    static const uint32_t ch[10]={0,0,1,0,1,0,1,0,0,1};
    static const uint32_t reg[10]={3,0,0,0,0,0,0,0,0,0};
    static const uint32_t write[10]={0,0,0,0,0,1,1,1,0,0};
    static const uint32_t val[3]={0x10,0x10,0x38};
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
