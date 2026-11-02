/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Reuse the original injected register fixture, not its incompatible baseline
 * expectations; this mode intentionally changes exactly one decision. */
#define main original_motion_fixture_main
#include "mac-motion.c"
#undef main

int main(void)
{
    const uint8_t pendings[3]={1,8,9};
    for(unsigned phases=0;phases<4u;phases++)for(unsigned p=0;p<3u;p++){
        fresh(0);hw.raw[0]=(uint8_t)((phases&1u)?8u:0u);
        hw.raw[1]=(uint8_t)((phases&2u)?8u:0u);baseline();
        hw.pending=pendings[p];
        CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY);
        CHECK(!hw.pending && m.losses==0 && m.samples==0 && m.have_phase &&
              inbox.phase==CS_INBOX_READY && inbox.count==0);
    }
    /* Matched changes still move; unreported changes and button changes lose. */
    fresh(0);baseline();hw.raw[0]=8;hw.pending=8;sample(-1,0,0);
    fresh(0);baseline();hw.raw[0]=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    fresh(0);baseline();hw.pb=0;hw.pending=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    fresh(0);baseline();hw.unstable=1;hw.pending=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    fresh(0);baseline();m.quiesced=1;hw.pending=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    fresh(0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    hw.pending=8;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST);
    for(unsigned bad=0;bad<3u;bad++){
        fresh(0);baseline();hw.pending=8;
        if(bad==0)inbox.count=17;else if(bad==1)inbox.head=16;else inbox.claimed=CS_INBOX_CLAIM_BASELINE;
        CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT);
    }
    fresh(0);baseline();hw.pending=2;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT);
    printf("Pending-idle experiment: %u checks; synthetic IO, hidden transitions remain unknown.\n",checks);
    return 0;
}
