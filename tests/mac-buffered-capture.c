/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#define main original_motion_fixture_main
#include "mac-motion.c"
#undef main
#include "../platform/mac128k/capture-queue.h"

int main(void)
{
    cs_mac_capture_queue q={0};
    cs_mac_capture v={{0},0},got={{0},0};
    unsigned before;
    /* Independent FIFO sequence, all fields and wraparound. */
    for(unsigned batch=0;batch<20;batch++){
        for(unsigned i=0;i<100;i++){
            for(unsigned j=0;j<4;j++)v.raw[j]=batch*1000+i*4+j;
            v.late=~v.raw[0];cs_mac_capture_push(&q,&v);
        }
        for(unsigned i=0;i<100;i++){
            CHECK(cs_mac_capture_pop(&q,&got)==1);
            for(unsigned j=0;j<4;j++)CHECK(got.raw[j]==batch*1000+i*4+j);
            CHECK(got.late==~got.raw[0]);
        }
        CHECK(cs_mac_capture_pop(&q,&got)==0 && !q.losses && !q.count);
    }
    for(unsigned i=0;i<CS_MAC_CAPTURE_CAPACITY;i++)cs_mac_capture_push(&q,&v);
    CHECK(!q.overflow && q.peak==CS_MAC_CAPTURE_CAPACITY);
    cs_mac_capture_push(&q,&v);cs_mac_capture_push(&q,&v);
    CHECK(q.overflow && q.losses==1 && q.count==CS_MAC_CAPTURE_CAPACITY);
    got.late=123;CHECK(cs_mac_capture_pop(&q,&got)==-1 && got.late==123);
    CHECK(!q.count && !q.head && !q.overflow);
    q.losses=UINT32_MAX;
    for(unsigned i=0;i<=CS_MAC_CAPTURE_CAPACITY;i++)cs_mac_capture_push(&q,&v);
    CHECK(q.losses==UINT32_MAX && cs_mac_capture_pop(&q,&got)==-1);

    fresh(0);baseline();
    /* Retained +X capture decoded after hardware has moved elsewhere.
     * The queued late RR3 is used instead of querying the later chip. */
    v.raw[0]=8;v.raw[1]=v.raw[2]=v.raw[3]=0x00180800;
    hw.raw[0]=0;hw.pb=0;before=hw.calls;
    CHECK(cs_mac_motion_samples(&m,v.raw,0)==CS_MAC_MOTION_OK);
    CHECK(hw.calls==before && m.last_raw_a==1 && m.last_buttons==0);
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_SAMPLE_ITEM);
    CHECK(inbox.last.dx==1 && inbox.last.dy==0 && !inbox.last.buttons);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK);
    /* Clean late edge: do not consume it or consult current RR3. */
    v.raw[0]=0;v.raw[1]=v.raw[2]=v.raw[3]=0x00080000;
    before=hw.calls;CHECK(cs_mac_motion_samples(&m,v.raw,8)==CS_MAC_MOTION_EMPTY);
    CHECK(hw.calls==before && m.last_raw_a==1 && inbox.count==0);
    v.raw[0]=8;CHECK(cs_mac_motion_samples(&m,v.raw,0)==CS_MAC_MOTION_OK);
    CHECK(m.last_raw_a==0);
    /* Lost capture cancels unread work and requires a fresh baseline. */
    CHECK(cs_mac_motion_capture_lost(&m)==CS_MAC_MOTION_LOST);
    CHECK(inbox.phase==CS_INBOX_LOSS && m.quiesced && !m.have_phase);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_CAPTURE && m.losses==1);
    CHECK(cs_inbox_discard(&inbox,1)==CS_INBOX_OK);
    CHECK(cs_inbox_resume(&inbox,1)==CS_INBOX_BASELINE);
    hw.pb=8;hw.raw[0]=hw.raw[1]=hw.pending=0;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED && !m.quiesced);
    CHECK(cs_mac_motion_samples(&m,v.raw,256)==CS_MAC_MOTION_ARGUMENT);
    CHECK(cs_mac_motion_samples(&m,v.raw,-2)==CS_MAC_MOTION_ARGUMENT);
    CHECK(cs_mac_motion_samples(&m,NULL,0)==CS_MAC_MOTION_ARGUMENT);
    CHECK(cs_mac_motion_capture_lost(NULL)==CS_MAC_MOTION_ARGUMENT);
    printf("Buffered capture: %u checks passed.\n",checks);
    return 0;
}
