/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CS_MAC_MOTION_H
#define CS_MAC_MOTION_H
#include <stdint.h>
#include "../../apps/window-scene/inbox.h"

/* Original Mac SCC/VIA quadrature motion core. See SPEC-0072. No MMIO,
 * no rendering/owner calls, no allocation, no globals. IO callbacks perform
 * one complete selector/data transaction each; the target guarantees chip
 * access spacing and recovery. Producer operations are serialized by the
 * caller; foreground inbox operations are excluded around service(). */
/* Opt-in engineering experiment; default 0 keeps the original layout/behavior. */
#ifndef CLASSICK_MAC_PENDING_DEFER_PROBE
#define CLASSICK_MAC_PENDING_DEFER_PROBE 0
#endif
#if (CLASSICK_MAC_PENDING_DEFER_PROBE)!=0 && (CLASSICK_MAC_PENDING_DEFER_PROBE)!=1
#error CLASSICK_MAC_PENDING_DEFER_PROBE must be 0 or 1
#endif

/* Opt-in engineering experiment; default 0. Unstable PB4/5-only passes with
 * no DCD/button/RR3 evidence return EMPTY instead of loss. */
#ifndef CLASSICK_MAC_PHASE_IDLE_PROBE
#define CLASSICK_MAC_PHASE_IDLE_PROBE 0
#endif
#if (CLASSICK_MAC_PHASE_IDLE_PROBE)!=0 && (CLASSICK_MAC_PHASE_IDLE_PROBE)!=1
#error CLASSICK_MAC_PHASE_IDLE_PROBE must be 0 or 1
#endif

/* Opt-in experiment (SPEC-0081); default 0 keeps the IO layout unchanged. */
#ifndef CLASSICK_MAC_TIMER_PAIR_PROBE
#define CLASSICK_MAC_TIMER_PAIR_PROBE 0
#endif
#if (CLASSICK_MAC_TIMER_PAIR_PROBE)!=0 && (CLASSICK_MAC_TIMER_PAIR_PROBE)!=1
#error CLASSICK_MAC_TIMER_PAIR_PROBE must be 0 or 1
#endif

enum { CS_MAC_SCC_A = 0, CS_MAC_SCC_B = 1 };

typedef struct {
    void *cookie;
    uint8_t (*read)(void *cookie, uint32_t channel, uint32_t reg);
    void (*write)(void *cookie, uint32_t channel, uint32_t reg, uint8_t v);
    uint8_t (*pb)(void *cookie);
#if CLASSICK_MAC_TIMER_PAIR_PROBE
    /* Writes PB,RR0A,RR0B,PB,RR0A,RR0B in that order; no other effects. */
    void (*pair)(void *cookie, uint8_t out[6]);
#endif
} cs_mac_motion_io;

enum { CS_MAC_MOTION_OK = 0, CS_MAC_MOTION_EMPTY = 1,
       CS_MAC_MOTION_BASELINED = 2, CS_MAC_MOTION_LOST = 3,
       CS_MAC_MOTION_ARGUMENT = 4, CS_MAC_MOTION_STATE = 5,
       CS_MAC_MOTION_FAULT = 6 };
enum { CS_MAC_MOTION_PREPARED = 1, CS_MAC_MOTION_ARMED = 2,
       CS_MAC_MOTION_FAILED = 3, CS_MAC_MOTION_STOPPED = 4 };
enum { CS_MAC_STAGE_NONE = 0, CS_MAC_STAGE_RESET = 1,
       CS_MAC_STAGE_CONFIG = 2, CS_MAC_STAGE_READBACK_A = 3,
       CS_MAC_STAGE_READBACK_B = 4, CS_MAC_STAGE_ARM = 5,
       CS_MAC_STAGE_PENDING = 6, CS_MAC_STAGE_INBOX = 7 };
enum { CS_MAC_CAUSE_NONE = 0, CS_MAC_CAUSE_READBACK = 1,
       CS_MAC_CAUSE_PENDING = 2, CS_MAC_CAUSE_INBOX_STATE = 3,
       CS_MAC_CAUSE_INBOX_RESULT = 4 };

enum { CS_MAC_LOSS_NONE = 0, CS_MAC_LOSS_NO_PHASE = 1,
       CS_MAC_LOSS_UNSTABLE = 2, CS_MAC_LOSS_PENDING = 3,
       CS_MAC_LOSS_OVERFLOW = 4 };

/* First actual transition to loss since prepare; observation only.
 * snapshots[i]=PB<<16|RR0A<<8|RR0B; previous=have_phase<<24|last_raw_a<<16|
 * last_raw_b<<8|last_buttons. reason is committed last. */
struct cs_mac_motion_loss {
    uint32_t reason, rr3, snapshots[3], previous, inbox_phase;
};
_Static_assert(sizeof(struct cs_mac_motion_loss)==28,"Loss record size");

struct cs_mac_motion {
    cs_mac_motion_io io;          /* copied */
    cs_window_inbox *inbox;       /* borrowed */
    uint32_t epoch, phase, have_phase, last_raw_a, last_raw_b, last_buttons;
    uint32_t invert, stage, quiesced; /* quiesced: SCC master off after loss */
    int32_t cause;
    uint32_t services, samples, baselines, losses, peak_queued; /* saturate */
    struct cs_mac_motion_loss first_loss;
#if CLASSICK_MAC_PENDING_DEFER_PROBE
    uint32_t defer_pending;       /* 0, or expected RR3 (A=8/B=1) */
    uint32_t defer_snapshot;      /* masked PB/RR0 endpoint */
#endif
};

int cs_mac_motion_prepare(struct cs_mac_motion *m, const cs_mac_motion_io *io,
    cs_window_inbox *inbox, uint32_t invert);
int cs_mac_motion_arm(struct cs_mac_motion *m);
int cs_mac_motion_service(struct cs_mac_motion *m);
int cs_mac_motion_stop(struct cs_mac_motion *m);

_Static_assert(sizeof(struct cs_mac_motion)<=192,"Bounded motion controller");
#endif
