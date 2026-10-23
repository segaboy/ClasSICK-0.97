/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOW_FOREGROUND_H
#define CLASSICK_WINDOW_FOREGROUND_H
#include "inbox.h"
#include "capture.h"

/* Original coordinator contract: serialized foreground, separate single
 * non-nesting producer. Infallible hooks exclude that producer and order all
 * inbox publication;leave restores its token exactly. Foreground otherwise
 * permits capture. No helper/owner/render call inside exclusion. No CPU/ISR
 * primitive is supplied here. All borrowed objects/hooks are live,aligned,
 * nonoverlapping and stable;no foreground/hook/owner re-entry. */
typedef struct {
    uint32_t (*enter)(void *);
    void (*leave)(void *,uint32_t);
    void *cookie;
} cs_foreground_hooks;
enum { CS_FG_OK=0,CS_FG_EMPTY=1,CS_FG_BASELINED=2,CS_FG_DELIVERED=3,
       CS_FG_RESET=4,CS_FG_STOPPED=5,CS_FG_ARGUMENT=6,CS_FG_STATE=7,
       CS_FG_FAULT=8,CS_FG_EXHAUSTED=9 };
enum { CS_FG_ACTIVE=1,CS_FG_FAILED=2,CS_FG_ENDED=3 };
enum { CS_FG_BEGIN=1,CS_FG_CLAIM=2,CS_FG_BASELINE=3,CS_FG_PUSH=4,
       CS_FG_DRAIN=5,CS_FG_LOSS=6,CS_FG_COMPLETE=7,CS_FG_STOP=8,
       CS_FG_RESTART=9,CS_FG_INTEGRATION=10 };
#define CS_FG_VALID 0x43464731u
typedef struct {
    uint32_t valid,epoch,phase,stage;
    int32_t cause;
    cs_window_inbox *inbox;
    cs_window_capture *capture;
    cs_foreground_hooks hooks;
} cs_window_foreground;
_Static_assert(sizeof(cs_window_foreground)<=128,"Bounded coordinator");
/* Fresh/quiescent lifetime;no reads of old descriptors. Ready owner;producer
 * stays off until successful begin AND a later fresh hardware baseline. */
int cs_foreground_begin(cs_window_foreground*,cs_window_inbox*,
    cs_window_capture*,cs_window_input*,const cs_foreground_hooks*);
/* At most one claim/delivery or one loss-recovery episode. No loop/drain-all.
 * A claimed delivery may finish during newer loss;complete respects that loss.
 * No automatic retry/repair. First raw owner/helper failure and stage retained. */
int cs_foreground_step(cs_window_foreground*);
/* Producer quiescent,foreground boundary. Close inbox first,unmasked helper
 * stop next. Terminal even on failure;repeat no hooks/owner calls. */
int cs_foreground_stop(cs_window_foreground*);
/* ENDED only,producer quiescent,ready owner. Preflight both epochs below max
 * before any effect. Helper restart precedes inbox reopening. Failed attempted
 * restart stays closed and may retain unequal descriptor epochs;fresh lifetime
 * required then,no rollback or retry. New-attempt first cause replaces old. */
int cs_foreground_restart(cs_window_foreground*,cs_window_input*);
#endif
