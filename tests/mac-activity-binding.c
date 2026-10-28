/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Reuse the reviewed fake hardware and all legacy IO-order assertions. */
#define main previous_refresh_main
#include "mac-timer-refresh.c"
#undef main
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE != 1
#error activity binding test requires the opt-in observer
#endif
int main(void)
{
    int previous = previous_refresh_main();
    T t; cs_pointer_sample o;
    ready(&t);
    K(t.p.activity.tag == UINT32_C(0x4D574131));
    K(t.p.activity.bytes == 80u && t.p.activity.pairs == 0u);
    K(svc(&t,0x40,0) == CS_MAC_MOTION_BASELINED); (void)take(&t,&o);
    K(t.p.activity.previous_accepted == UINT32_C(0x80080000));
    t.f.pb = 0x18;
    K(svc(&t,0x40,0) == CS_MAC_MOTION_EMPTY);
    K(t.p.activity.pb_x_changes == 1u && t.p.activity.nonzero_queued == 0u);
    t.f.live[0] = 8;
    K(svc(&t,0x40,0) == CS_MAC_MOTION_OK); (void)take(&t,&o);
    K(o.dx == 1 && o.dy == 0 && t.p.activity.nonzero_queued == 1u);
    t.f.live[0] = 0;
    K(svc(&t,0x40,0) == CS_MAC_MOTION_OK); (void)take(&t,&o);
    K(o.dx == -1 && o.dy == 0 && t.p.activity.nonzero_queued == 2u);
    t.f.pb = 0x10;
    K(svc(&t,0x40,0) == CS_MAC_MOTION_OK); (void)take(&t,&o);
    K(o.dx == 0 && o.dy == 0 && o.buttons == 1u);
    K(t.p.activity.buttononly_queued == 1u && t.p.activity.button_changes == 1u);
    t.f.pb = 0; t.f.live[0] = 8;
    K(svc(&t,0x40,0) == CS_MAC_MOTION_LOST);
    K(t.p.activity.first_previous == UINT32_C(0x80100000));
    K(t.p.activity.first_s0 == UINT32_C(0x00000800));
    K(t.p.activity.first_s1 == UINT32_C(0x00000800));
    K(t.p.activity.first_reason == 6u && t.p.activity.first_service == 6u);
    K(t.p.activity.previous_accepted == 0u && t.p.activity.pairs == 6u);
    K(t.p.activity.pb_x_changes == 2u && t.p.activity.dcd_a_changes == 3u);
    K(t.p.activity.pb_y_changes == 0u && t.p.activity.dcd_b_changes == 0u);
    K(t.p.activity.stable_pairs == 6u && t.p.activity.losses == 1u);
    K(t.p.motion.first_loss.previous == UINT32_C(0x01000001));
    printf("activity binding incl legacy:%u assertions,%d failures\n",checks,fails);
    return previous || fails;
}
