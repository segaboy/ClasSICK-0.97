/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#define main original_motion_fixture_main
#include "mac-motion.c"
#undef main

static void finish_baseline(void)
{
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_BASELINE_ITEM);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK);
    CHECK(inbox.phase==CS_INBOX_READY);
}

int main(void)
{
    fresh(0);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    CHECK(m.quiesced && hw.regs[0][9]==2 && inbox.phase==CS_INBOX_PRIMING);
    hw.pending=8;unsigned reads=hw.calls;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && hw.calls==reads && hw.pending==8);
    CHECK(cs_inbox_claim(&inbox,1)==CS_INBOX_BASELINE_ITEM);
    reads=hw.calls;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && hw.calls==reads && hw.pending==8);
    CHECK(cs_inbox_complete(&inbox,1,0)==CS_INBOX_OK && inbox.phase==CS_INBOX_READY);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_LOST && m.quiesced && hw.regs[0][9]==2);
    CHECK(m.first_loss.reason==CS_MAC_LOSS_PENDING);
    CHECK(cs_inbox_discard(&inbox,1)==CS_INBOX_OK);
    CHECK(cs_inbox_resume(&inbox,1)==CS_INBOX_BASELINE);
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED && m.quiesced);
    reads=hw.calls;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && hw.calls==reads);
    finish_baseline();
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && !m.quiesced && hw.regs[0][9]==0x0A);
    CHECK(hw.log[hw.writes-1].reg==9 && hw.log[hw.writes-1].value==0x0A);
    /* A retained, coherent edge is delivered after baseline before rearm. */
    fresh(0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    hw.raw[0]=8;hw.pending=8;reads=hw.calls;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_EMPTY && hw.calls==reads && hw.pending==8);
    finish_baseline();sample(-1,0,0);
    CHECK(!m.quiesced && hw.regs[0][9]==0x0A);
    /* Corrupt in-flight metadata must not use the no-IO fast path. */
    fresh(0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    inbox.count=0;hw.pending=8;
    CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_FAULT && hw.regs[0][9]==2);
    fresh(0);CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_BASELINED);
    CHECK(cs_mac_motion_stop(&m)==CS_MAC_MOTION_OK && hw.regs[0][9]==2);
    reads=hw.calls;CHECK(cs_mac_motion_service(&m)==CS_MAC_MOTION_STATE && hw.calls==reads);
    printf("Rearm-ready experiment: %u checks; synthetic IO, physical timing unknown.\n",checks);
    return 0;
}
