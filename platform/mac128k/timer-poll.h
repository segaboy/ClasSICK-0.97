/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_TIMER_POLL_H
#define CS_MAC_TIMER_POLL_H
#include <stdint.h>
#include "motion.h"
#include "polled-phase.h"
#include "rom.h"
#ifndef CLASSICK_MAC_TIMER_ACTIVITY_PROBE
#define CLASSICK_MAC_TIMER_ACTIVITY_PROBE 0
#endif
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE != 0 && CLASSICK_MAC_TIMER_ACTIVITY_PROBE != 1
#error CLASSICK_MAC_TIMER_ACTIVITY_PROBE must be 0 or 1
#endif
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
#include "activity.h"
#endif

/* SPEC-0084 opt-in diagnostic: first_loss.previous also records marker bit31
 * and the predecessor decoder X/Y phases (bits 29..26). No state or MMIO. */
#ifndef CLASSICK_MAC_TIMER_PHASE_WITNESS
#define CLASSICK_MAC_TIMER_PHASE_WITNESS 0
#endif
#if (CLASSICK_MAC_TIMER_PHASE_WITNESS)!=0 && (CLASSICK_MAC_TIMER_PHASE_WITNESS)!=1
#error CLASSICK_MAC_TIMER_PHASE_WITNESS must be 0 or 1
#endif
#if CLASSICK_MAC_TIMER_PHASE_WITNESS && !(CLASSICK_MAC_TIMER_PAIR_PROBE && CLASSICK_MAC_TIMER_POLL_PROBE && CLASSICK_MAC_TIMER_REFRESH_PROBE && CLASSICK_MAC_TIMER_LIVE_PAIR && CLASSICK_MAC_TIMER_DIVISOR == 4)
#error Phase witness requires pair,poll,refresh,live pair and divisor 4
#endif

/* Opt-in engineering experiment; default 0 keeps behavior/API/layout. At 1,
 * prepare leaves WR15 DCD enable (8) set on both channels and each admitted
 * WAIT/PRIMING/READY service issues ResetExtStatus (WR0 0x10) to A then B
 * before the first PB/RR0 triple. WR1 and SCC master stay off. Endpoint
 * polling only; not a historical or proven root-cause fix. */
#ifndef CLASSICK_MAC_TIMER_REFRESH_PROBE
#define CLASSICK_MAC_TIMER_REFRESH_PROBE 0
#endif
#if (CLASSICK_MAC_TIMER_REFRESH_PROBE)!=0 && (CLASSICK_MAC_TIMER_REFRESH_PROBE)!=1
#error CLASSICK_MAC_TIMER_REFRESH_PROBE must be 0 or 1
#endif

/* Opt-in timer-driven polling controller; the normal driver is unchanged.
 * Caller owns SCC/VIA T1/ACR exclusively. prepare/arm/stop run under IPL7 and
 * service runs in a serialized IPL7 ISR, with foreground inbox exclusion
 * already established. motion holds diagnostics, copied SCC callbacks, inbox
 * and epoch only; the original motion driver functions are never called.
 * reload is an engineering T1 count, not a frequency. The stability and T1
 * recurrence checks cannot detect every missed cycle or prove physical
 * coherence of the sampled levels. SCC master interrupt enable stays off. */
enum { CS_MAC_TIMER_LOSS_OVERRUN = 5, CS_MAC_TIMER_LOSS_GRAY = 6 };

typedef struct {
    struct cs_mac_motion motion;
    cs_mac_polled_phase decoder;
    cs_mac_io via;
    uint32_t reload;
#if CLASSICK_MAC_TIMER_ACTIVITY_PROBE
    cs_mac_activity activity;
#endif
} cs_mac_timer_poll;
_Static_assert(sizeof(void *) > 4 || sizeof(cs_mac_timer_poll) <= 320,
    "Bounded timer poll controller");

/* prepare refusals (ARGUMENT) have no effect. arm/service/stop reject bad
 * lifecycle or epoch with STATE;caller objects/callbacks are trusted.
 * Hardware failure returns FAULT with
 * phase FAILED, T1 and SCC master off. */
int cs_mac_timer_poll_prepare(cs_mac_timer_poll *p, const cs_mac_motion_io *scc,
    const cs_mac_io *via, cs_window_inbox *inbox, uint32_t invert,
    uint32_t reload);
int cs_mac_timer_poll_arm(cs_mac_timer_poll *p);
int cs_mac_timer_poll_service(cs_mac_timer_poll *p);
int cs_mac_timer_poll_stop(cs_mac_timer_poll *p);
#endif
