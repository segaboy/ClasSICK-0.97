/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Diagnostic-only SCC snapshot checks. */
#if CLASSICK_MAC_MOTION_TIMELINE != 1
#error Build this test with CLASSICK_MAC_MOTION_TIMELINE=1
#endif
/* Reuse the original injected register fixture; its main is not executed. */
#define main existing_motion_fixture_main
#include "mac-motion.c"
#undef main

/* Reproduce the three sequential reads in the captured 0x1725 record. */
static unsigned observed_index;
static uint8_t observed_pb(void *cookie)
{
    static const uint8_t values[3]={0xBF,0xAF,0xAF};
    (void)cookie;
    CHECK(observed_index<3u);
    return values[observed_index++];
}
static uint8_t observed_read(void *cookie,uint32_t ch,uint32_t reg)
{
    static const uint8_t a[3]={0x4C,0x44,0x44};
    if(reg==0u){
        CHECK(observed_index>=1u && observed_index<=3u);
        return ch==0u?a[observed_index-1u]:0x44;
    }
    return rd(cookie,ch,reg);
}

int main(void)
{
    unsigned i;
    fresh(0);
    CHECK(m.timeline.magic==CS_MAC_TIMELINE_MAGIC && m.timeline.count==0u);
    baseline();
    CHECK(m.timeline.count==0u);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY);
    CHECK(m.timeline.count==0u);

    hw.raw[0]=8; hw.pending=8;
    sample(-1,0,0);
    CHECK(m.timeline.count==1u && m.timeline.pending==1u);
    CHECK(m.timeline.entries[0].service==3u && m.timeline.entries[0].rr3==8u);
    CHECK(m.timeline.entries[0].snapshots[0]==UINT32_C(0x00080800));
    CHECK(m.timeline.entries[0].snapshots[1]==UINT32_C(0x00080800));
    CHECK(m.timeline.entries[0].snapshots[2]==UINT32_C(0x00080800));
    CHECK(m.timeline.entries[0].previous==UINT32_C(0x01000000));
    CHECK(m.timeline.entries[0].inbox_phase==CS_INBOX_READY);
    CHECK(m.timeline.entries[0].result==UINT32_MAX);

    /* An uncommitted result remains unknown when a later service occurs. */
    hw.raw[0]=0; hw.pending=8;
    sample(1,0,0);
    CHECK(m.timeline.count==2u && m.timeline.entries[0].result==UINT32_MAX);
    cs_mac_motion_timeline_commit(&m,CS_MAC_MOTION_OK);
    CHECK(m.timeline.entries[1].result==CS_MAC_MOTION_OK && m.timeline.pending==0u);
    cs_mac_motion_timeline_commit(&m,CS_MAC_MOTION_LOST);
    CHECK(m.timeline.entries[1].result==CS_MAC_MOTION_OK);

    for(i=0;i<5u;i++) {
        hw.raw[0]=(uint8_t)(hw.raw[0]?0:8); hw.pending=8;
        sample(hw.raw[0]?-1:1,0,0);
        cs_mac_motion_timeline_commit(&m,CS_MAC_MOTION_OK);
    }
    CHECK(m.timeline.count==6u && m.timeline.overflow==1u);
    CHECK(m.timeline.entries[5].result==CS_MAC_MOTION_OK);

    fresh(0); baseline(); hw.unstable=1;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(m.timeline.count==1u && m.timeline.entries[0].result==UINT32_MAX);
    CHECK(m.timeline.entries[0].snapshots[0]!=m.timeline.entries[0].snapshots[2]);
    cs_mac_motion_timeline_commit(&m,CS_MAC_MOTION_LOST);
    CHECK(m.timeline.entries[0].result==CS_MAC_MOTION_LOST);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    cs_mac_motion_timeline_commit(NULL,CS_MAC_MOTION_OK);

    fresh(0); baseline(); hw.pending=8; observed_index=0;
    m.io.pb=observed_pb; m.io.read=observed_read;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    CHECK(observed_index==3u && m.first_loss.reason==CS_MAC_LOSS_UNSTABLE);
    CHECK(m.first_loss.snapshots[0]==UINT32_C(0x00BF4C44));
    CHECK(m.first_loss.snapshots[1]==UINT32_C(0x00AF4444));
    CHECK(m.first_loss.snapshots[2]==UINT32_C(0x00AF4444));
    CHECK(inbox.phase==CS_INBOX_LOSS && inbox.count==0u && !m.have_phase);
    CHECK(hw.regs[0][9]==2u);
    puts("mac-motion-timeline: passed");
    return 0;
}
