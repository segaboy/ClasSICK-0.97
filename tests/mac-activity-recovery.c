/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Scripted register returns,not a latch/timing/physical hardware model.
 * Reuse the reviewed queue-backed fake;do not run its macro0 legacy main. */
#define main unused_legacy_poll_main
#include "mac-timer-poll.c"
#undef main
#include "../platform/mac128k/activity-window.h"
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE != 1 || CLASSICK_MAC_TIMER_REFRESH_PROBE != 1
#error recovery regression requires activity and refresh
#endif
int main(void)
{
    T t;cs_pointer_sample o;cs_mac_activity_window gate;
    uint32_t i,pbn,rr0n;
    ready(&t);K(cs_mac_activity_window_begin(&gate)==0);
    K(svc(&t,0x40,0,0xFF,0x44,0x44,0xFF,0x44,0x44)==CS_MAC_MOTION_BASELINED);
    K(take(&t,&o)==CS_INBOX_BASELINE_ITEM&&o.dx==0&&o.dy==0&&o.buttons==0);
    for(i=1;i<17816u;++i){
        if(svc(&t,0x40,0,0xFF,0x44,0x44,0xFF,0x44,0x44)!=CS_MAC_MOTION_EMPTY){K(0);break;}
    }
    K(t.p.activity.pairs==17816u&&t.p.activity.losses==0u);
    K(svc(&t,0x40,0,0x9F,0x4C,0x44,0x8F,0x44,0x44)==CS_MAC_MOTION_LOST);
    K(t.in.phase==CS_INBOX_LOSS&&t.in.count==0u);
    K(t.p.activity.first_previous==UINT32_C(0x80FF4444));
    K(t.p.activity.first_s0==UINT32_C(0x009F4C44)&&t.p.activity.first_s1==UINT32_C(0x008F4444));
    K(t.p.activity.first_reason==2u&&t.p.activity.first_service==17817u);
    K(t.p.activity.pb_x_changes==1u&&t.p.activity.pb_y_changes==1u);
    K(!t.p.activity.dcd_a_changes&&!t.p.activity.dcd_b_changes&&!t.p.activity.button_changes);
    K(!t.p.activity.nonzero_queued&&!t.p.activity.buttononly_queued&&t.p.activity.losses==1u);
    K(!t.p.decoder.have_phase&&!t.p.motion.have_phase&&!t.p.activity.previous_accepted);
    K(cs_mac_activity_window_step(&gate,t.p.motion.services,1)==CS_MAC_AW_WAIT);
    pbn=t.f.pbn;rr0n=t.f.rr0n;
    /* LOSS stays sticky until foreground handshake;services still advance. */
    for(i=0;i<64u;++i){
        K(svc(&t,0x40,0,0,0,0,0,0,0)==CS_MAC_MOTION_EMPTY);
        K(cs_mac_activity_window_step(&gate,t.p.motion.services,1)==CS_MAC_AW_WAIT);
    }
    K(t.f.pbn==pbn&&t.f.rr0n==rr0n&&t.p.activity.pairs==17817u);
    K(cs_inbox_discard(&t.in,t.in.epoch)==CS_INBOX_OK);
    K(cs_inbox_resume(&t.in,t.in.epoch)==CS_INBOX_BASELINE);
    K(svc(&t,0x40,0,0x8F,0x44,0x44,0x8F,0x44,0x44)==CS_MAC_MOTION_BASELINED);
    K(take(&t,&o)==CS_INBOX_BASELINE_ITEM&&!o.dx&&!o.dy&&!o.buttons);
    K(svc(&t,0x40,0,0x8F,0x4C,0x44,0x8F,0x4C,0x44)==CS_MAC_MOTION_OK);
    K(take(&t,&o)==CS_INBOX_SAMPLE_ITEM&&o.dx==-1&&o.dy==0&&o.buttons==0);
    K(t.p.activity.nonzero_queued==1u&&t.p.activity.losses==1u&&t.p.motion.baselines==2u);
    /* Remaining quiet services reach the gate deadline,not the pair count. */
    for(i=0;i<446u;++i){
        int want=i==445u?CS_MAC_AW_COMPLETE:CS_MAC_AW_WAIT;
        K(svc(&t,0x40,0,0x8F,0x4C,0x44,0x8F,0x4C,0x44)==CS_MAC_MOTION_EMPTY);
        K(cs_mac_activity_window_step(&gate,t.p.motion.services,1)==want);
    }
    K(t.p.motion.services==18329u&&t.p.activity.pairs==18265u);
    /* Later synthetic Gray loss increments totals,never replaces first loss. */
    K(svc(&t,0x40,0,0x9F,0x44,0x44,0x9F,0x44,0x44)==CS_MAC_MOTION_LOST);
    K(t.p.activity.losses==2u&&t.p.activity.first_reason==2u);
    K(t.p.activity.first_previous==UINT32_C(0x80FF4444)&&t.p.activity.first_service==17817u);
    K(t.p.activity.first_s0==UINT32_C(0x009F4C44)&&t.p.activity.first_s1==UINT32_C(0x008F4444));
    printf("captured loss plus synthetic recovery:%u checks,%d failures\n",checks,fails);
    return fails?1:0;
}
