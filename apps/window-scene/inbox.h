/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#ifndef CLASSICK_WINDOW_INBOX_H
#define CLASSICK_WINDOW_INBOX_H
#include "../../core/input/pointer.h"
/* SPEC-0065: pure operation-level state machine, NOT an ISR/publication proof.
 * Caller supplies live/aligned storage, single non-nesting producer, serialized
 * foreground, and short exclusion/compiler ordering around foreground inbox
 * operations. No helper/owner/render call while excluded. No callbacks/MMIO.
 * baseline/push/uncertain are producer operations; all others foreground.
 * last is the ONE by-value claimed record: producer never reads/writes it.
 * Read it outside exclusion only while claimed!=NONE; complete before another
 * claim. Data fields are diagnostic, not a hostile-pointer security boundary. */
enum { CS_INBOX_CAPACITY=16 };
enum { CS_INBOX_OK=0, CS_INBOX_EMPTY=1, CS_INBOX_BASELINE_ITEM=2,
    CS_INBOX_SAMPLE_ITEM=3, CS_INBOX_LOST=4, CS_INBOX_BASELINE=5,
    CS_INBOX_STOPPED=6, CS_INBOX_ARGUMENT=7, CS_INBOX_STATE=8,
    CS_INBOX_FAULTED=9, CS_INBOX_EXHAUSTED=10 };
enum { CS_INBOX_WAIT=1, CS_INBOX_PRIMING=2, CS_INBOX_READY=3,
    CS_INBOX_LOSS=4, CS_INBOX_FAULT=5, CS_INBOX_ENDED=6 };
enum { CS_INBOX_NONE=0, CS_INBOX_CLAIM_BASELINE=1, CS_INBOX_CLAIM_SAMPLE=2 };
enum { CS_INBOX_OVERFLOW=1, CS_INBOX_UNCERTAIN=2, CS_INBOX_STARTUP=3 };
#define CS_INBOX_VALID 0x43494231u
typedef struct {
    uint32_t valid,epoch,phase,head,count,claimed,loss_reason,losses,delivered;
    int32_t cause;
    cs_pointer_sample records[CS_INBOX_CAPACITY],last;
} cs_window_inbox;
_Static_assert(sizeof(cs_window_inbox)<=256,"Bounded inbox and claimed record");
/* Once per fresh/quiescent lifetime,AFTER dedicated helper begin cancellation.
 * Initializes WAIT/epoch1,metadata/last only;never reads old inbox bytes. */
int cs_inbox_begin(cs_window_inbox*);
/* ENDED-only,quiescent producer;preflight exhaustion in both objects BEFORE
 * helper restart,then restart inbox to same epoch. No helper calls here. */
int cs_inbox_restart(cs_window_inbox*);
/* WAIT-only producer fresh post-barrier baseline,zero dx/dy,actual buttons.
 * Immediately PRIMING;later records stage even while foreground rebaselines. */
int cs_inbox_baseline(cs_window_inbox*,uint32_t epoch,uint32_t buttons);
int cs_inbox_push(cs_window_inbox*,uint32_t epoch,int32_t dx,int32_t dy,uint32_t buttons);
int cs_inbox_uncertain(cs_window_inbox*,uint32_t epoch);
/* Claim oldest exactly once. Returns BASELINE_ITEM or SAMPLE_ITEM;last stable
 * during unmasked delivery. Pending claim refuses another with STATE. */
int cs_inbox_claim(cs_window_inbox*,uint32_t epoch);
/* After the one foreground helper call sequence. cause=0 for successful owner
 * OK/CHANGED;otherwise pass raw nonzero failure. Success respects concurrent
 * LOSS;failure takes priority. No helper/owner call inside this function. */
int cs_inbox_complete(cs_window_inbox*,uint32_t epoch,int32_t cause);
/* LOSS,no claim:discard inbox prefix BEFORE unmasked helper loss cancellation.
 * Keep LOSS sticky through cancellation. resume only after successful cancel
 * (or helper already WAIT,no duplicate cancel),and after count/head are zero. */
int cs_inbox_discard(cs_window_inbox*,uint32_t epoch);
int cs_inbox_resume(cs_window_inbox*,uint32_t epoch);
/* Nonzero raw cause closes admission/clears claim but retains unread prefix.
 * Preserve first cause. If ENDED,stay ENDED(e.g.stop cancel failure). */
int cs_inbox_fault(cs_window_inbox*,uint32_t epoch,int32_t cause);
/* At foreground boundary before helper stop;discard unread/claimed work.
 * In-flight owner call finishes first. Remains ENDED;repeat has no effects. */
int cs_inbox_stop(cs_window_inbox*,uint32_t epoch);
#endif
