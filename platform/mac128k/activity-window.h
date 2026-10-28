/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_ACTIVITY_WINDOW_H
#define CS_MAC_ACTIVITY_WINDOW_H
#include <stdint.h>
#ifndef CLASSICK_MAC_TIMER_DIVISOR
#define CLASSICK_MAC_TIMER_DIVISOR 1
#endif
#if CLASSICK_MAC_TIMER_DIVISOR != 1 && CLASSICK_MAC_TIMER_DIVISOR != 4 && CLASSICK_MAC_TIMER_DIVISOR != 16
#error CLASSICK_MAC_TIMER_DIVISOR must be1,4 or16
#endif
#define CS_MAC_ACTIVITY_WINDOW_VALID UINT32_C(0x41574731)
#define CS_MAC_ACTIVITY_WINDOW_DELAY (UINT32_C(512)*CLASSICK_MAC_TIMER_DIVISOR)
#define CS_MAC_ACTIVITY_WINDOW_FALLBACK (UINT32_C(50000)*CLASSICK_MAC_TIMER_DIVISOR)
#define CS_MAC_ACTIVITY_WINDOW_MINIMUM (UINT32_C(500)*CLASSICK_MAC_TIMER_DIVISOR)
typedef struct { uint32_t valid,armed,first_service,last_service; } cs_mac_activity_window;
_Static_assert(sizeof(cs_mac_activity_window)==16,"Activity window size");
enum { CS_MAC_AW_WAIT=0, CS_MAC_AW_COMPLETE=1, CS_MAC_AW_FALLBACK=2,
       CS_MAC_AW_ARGUMENT=-1, CS_MAC_AW_STATE=-2 };
/* Fresh caller-owned state;0 success,1 null. No read of prior bytes. */
int cs_mac_activity_window_begin(cs_mac_activity_window *s);
/* Monotone service counts;activity0/1. Refusal has no effects. No device access. */
int cs_mac_activity_window_step(cs_mac_activity_window *s,uint32_t services,uint32_t activity);
#endif
