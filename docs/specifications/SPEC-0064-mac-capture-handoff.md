# SPEC-0064 - Mac capture-to-foreground handoff

2026-10-23;SRC-0369..0371 / ADR-0073. Original engineering contract for
paired-native-preview-2. Contract only;no implementation or C ABI is admitted.
This does not amend the verified sequential SPEC-0063 helper or install an ISR.
Its foreground operations may render through the existing input owner;masking
interrupts across the whole operation would block capture during drawing.
[Lead/source review](../development/mac-scc-binding-review.md),
[planned TEST-0077](../../provenance/records/TEST-0077-mac-capture-handoff-plan.md).

## 1. Chosen boundary

A separate fixed-size inbox sits between an unadmitted capture adapter and SPEC-0063. The producer is single, non-nesting, and on one CPU. Foreground operations are serialized. This is not a general multithread or lock-free design.

The inbox holds 16 complete normalized records. Each record is dx, dy, and buttons. Capacity, epoch, and one private by-value claimed sample are budgeted apart from the SPEC-0063 descriptor (measured 248 bytes on 68000 and 256 bytes on x64). Additional inbox plus claimed storage is budgeted at or below 256 bytes on 68000. The per-native-frame goal is 512 bytes. Inbox size, frame stack, and ROM or RAM fit are unmeasured.

The adapter that normalizes and captures before enqueue is unadmitted. This contract does not interpret RR0 pins. The producer publishes only a complete normalized sample or an explicit loss. It never dereferences the owner and never calls the helper or the owner.

Foreground, inside a short exclusion, copies or dequeues at most one complete record into private claimed storage, or applies a sticky loss or a stop discard. It then restores the prior exclusion state. Only after release may it call SPEC-0063.

## 2. Invariants

1. The critical section excludes every producer entry and covers compiler and publication ordering. The producer cannot preempt while it is held. Foreground inbox writes occur only inside it. Delivery requires a foreground context that otherwise permits capture:restoring an already-blocking outer mask is insufficient. The MC68000 primitive, level, assembly, barriers, save, RTE, and timing are unadmitted.
2. The full record payload is published before the committed count. A claim never returns a partial record.
3. Overflow and explicit uncertainty are sticky. Unread records stay in place. A saturating diagnostic counter never decides freshness, order, or acceptance. Uncertainty applies even to an empty inbox.
4. Order is preserved. Records are not coalesced. dx/dy are unchanged normalized displacement;pointer/window clamping belongs to the existing foreground owner,per record. No producer displacement clamping is introduced.
5. Epoch is nonzero and must not wrap. A stale epoch has no effect. Restart when the epoch is already at its maximum has no effect. The inbox and dedicated sequential helper start at epoch1 and advance together only on explicit restart;the old producer must already be quiescent. Both exhaustion checks precede effects.
6. Exclusion contains no owner call, helper call, rendering, MMIO loop, wait, or unbounded scan. Release restores the prior exclusion state and does not blindly enable interrupts globally.
7. Claim is the acceptance point. At most that one in-flight delivery may finish if a newer detectable loss appears during rendering. Foreground must observe the loss and finish the cancel path before accepting a later record. There is no rollback and no asynchronous cancel of an owner call already running.
8. Unknown hardware transitions are not detectable in general. There is no hidden edge-count guarantee. Silence is not zero motion.
9. The first owner fault is retained. After fault, the only owner call still permitted is the cancel inside SPEC-0063 stop when that owner is still valid and READY. A faulted or non-ready owner is not called.
10. Begin, claim, deliver, rebaseline, stop, and restart run at a foreground boundary. The producer has no stop callback. The producer is quiescent before owner or storage is freed or rebound.
11. One future SCC/VIA read owner is serialized. The SCC control selector is shared across channels. This contract selects no MMIO program.

## 3. Phases

STOPPED: admission closed, inbox empty, no claim waiting. The helper is ended or absent.

WAIT_BASELINE: cancellation has finished;ordinary admission closed. A new capture barrier must precede a fresh baseline publication.

PRIMING: the sole producer publishes one zero-motion actual-button baseline as the first inbox record,then immediately stages later complete ordinary records in the same16-slot ring. A single flag distinguishes baseline pending/claimed. Foreground may claim only the baseline here;later records wait for its completion. No second baseline and no silent discard between baseline observation and READY. The hardware adapter must establish the baseline's phase reference and account for later edges;that adapter remains unadmitted. Overflow/uncertainty during PRIMING enters LOSS just as in READY.

READY: complete ordinary samples may be enqueued and claimed one at a time.

LOSS: sticky overflow or explicit uncertainty. Unread data remains. Good samples are refused. Extra loss notes only saturate counters.

FAULT: first helper or owner failure kept. Admission closed. No later sample is accepted.

## 4. Operation state table

| Operation | Starts from | Inside exclusion | After release | Phase becomes |
|---|---|---|---|---|
| begin | fresh quiescent storage | epoch 1, empty inbox, claim clear, admission closed | SPEC-0063 begin; hardware stays off | WAIT_BASELINE, else FAULT |
| claim | READY, count above 0, claim empty | copy the oldest record, advance the inbox | no helper call | READY, claim held |
| deliver | claimed ordinary record | no inbox mutation during helper calls | helper push/drain exactly once with interrupts available;then inspect/publish status under short exclusion | READY,sticky LOSS,or FAULT |
| observe-loss | LOSS,no owner call in flight | keep admission closed;discard inbox unread prefix/claim | if helper READY,uncertain then drain once;if WAIT,no duplicate cancel | WAIT_BASELINE,else FAULT |
| rebaseline | PRIMING,baseline pending | claim baseline only,mark claimed | helper rebaseline once;then short exclusion to accept completion | READY unless sticky LOSS;failure FAULT |
| stop | any live phase, foreground boundary | close admission; discard unread and undelivered claim | SPEC-0063 stop | STOPPED |
| restart | STOPPED,both epochs below max,producer quiescent | closed admission;prepare next epoch | helper restart,then publish matching inbox epoch under exclusion | WAIT_BASELINE,FAULT,or no effect |

Fresh begin never reads old fields:quiescent fresh storage is a caller precondition,not a detected refusal of arbitrary live bytes. Restart is STOPPED-only,uses helper restart rather than helper begin,and requires old producer quiescence. Hardware enable is unadmitted and cannot precede successful owner cancellation. The IRQ path never calls SPEC-0063 push or uncertain. Stop is terminal even if cancellation fails;repeated stop has no calls/effects. An exhausted restart has no effects in either descriptor.

The foreground owns a dedicated SPEC-0063 instance exclusively. At every
ordinary acceptance boundary it is READY with an empty helper queue:one push
followed by one drain consumes the claimed record. Unexpected helper phase or
queue occupancy is an integration fault,not permission to drop/retry/replay a
record. First helper/owner failure closes inbox admission under short exclusion
and retains its cause;unread diagnostics are not subsequently delivered. No
automatic repair/restart. Foreground failure is checked before sticky LOSS so
that loss cannot erase the first owner fault. Owner calls never hold exclusion.

Loss recovery discards the IRQ inbox explicitly;the helper cannot discard a
different queue. While helper is READY,uncertain then drain causes one cancel.
If loss preceded any baseline submission and helper is still WAIT,do not issue
a second cancel. If rebaseline completed during new loss,helper is now READY
and must take the normal loss-cancel path before any later ordinary claim.
Keep inbox LOSS throughout foreground cancellation;publish WAIT_BASELINE under
exclusion only on success. Additional loss reports in that episode do not
start more cancellation calls. A fault closes admission instead. Empty helper
drain is never mistaken for cancellation. No direct owner/leaf calls are added.

## 5. Scheduling boundary

Drawing cannot sit inside the exclusion, because capture must continue during the owner call. A record claimed before a loss can therefore still be delivered after the loss is published. Only that claimed record is in flight. Newer unread samples stay in the inbox and are not delivered on that pass. The next foreground operation observes sticky LOSS, routes the single helper cancel path, and waits for a new baseline. Stop is not injected from the interrupt.

## 6. Timelines

Loss during delivery:

1. Foreground claims record N and releases exclusion.
2. The producer either enqueues a newer complete sample or, on a full inbox or explicit uncertainty, sets sticky LOSS and overwrites nothing.
3. Foreground delivers only N.
4. Foreground observes LOSS before any later claim, discards the IRQ inbox prefix under short exclusion,and routes one uncertain/drain cancellation through the helper.
5. The phase is WAIT_BASELINE. Another record waits for rebaseline.

Rebaseline:

1. Cancellation succeeded;inbox WAIT_BASELINE. Only the sole producer's fresh post-barrier observation can create the baseline;no old queued sample,foreground guess or invented atomic SCC/VIA snapshot qualifies.
2. Producer publishes baseline(zero motion,actual buttons) first,enters PRIMING,and stages subsequent motion/button records immediately in order. The hardware phase/coherence/barrier proof remains open;uncertainty instead enters LOSS.
3. Foreground claims baseline under short exclusion,releases,and calls helper rebaseline once. PRIMING producer remains able to enqueue while that call runs.
4. Success then short exclusion:PRIMING becomes READY;sticky LOSS stays LOSS. Failure becomes FAULT regardless of concurrent loss. Ordinary records are not claimed before this decision.
5. Loss before the baseline call sees helper WAIT and needs no duplicate cancel. Loss during successful rebaseline sees helper READY and cancels before another ordinary claim. New barrier/fresh baseline required;no auto-retry loop.
6. Held left remains suppressed until a later released observation. An unverified hardware baseline cannot be labeled trusted READY merely because software tests pass.

Stop:

1. At a foreground boundary, under exclusion, close admission and discard unread records and any claimed record whose owner call has not started. A claimed record already being delivered finishes before this foreground stop boundary;there is no asynchronous stop callback.
2. Release exclusion.
3. Call SPEC-0063 stop. That helper cancels once only when the owner is valid and READY.
4. Remain STOPPED on success or owner failure. A repeated stop changes nothing and calls nothing.
5. Failure does not restart or loop. The first raw cause is kept. Recovery is an explicit restart from STOPPED, unless the epoch is exhausted.

## 7. Hardware admission checklist

Every item stays open. This contract admits none of them.

- Exclusion primitive, level, assembly, barriers, save, and RTE.
- Layout and compiler proof that payload publication precedes the committed count.
- Measured inbox bytes, native frame stack, ROM, and RAM.
- Capture adapter, normalization, pin meaning, and polarity.
- Baseline coherence. No atomic SCC/VIA snapshot is admitted.
- Shared SCC control selector and a single serialized SCC/VIA reader.
- Fact-sheet spacing constraint for a later slice: even SCC byte reads, odd SCC byte writes, at least 2.2 microseconds apart. No access program is given here.
- Distinct facts, with no enable recipe: MIE WR9 D3, external or status WR1 D0, per-source WR15 enable, and RR3A only.
- Distinct facts: MIE gating, IP reset, IUS reset, and CPU exception return.
- WR9 D5 software acknowledge stays reserved 0 on NMOS. IUS is set only when an actual acknowledge occurs. No unconditional IUS reset is assumed.
- Fact-sheet WR0 conflict, unresolved: figure command 111 versus prose label 110 for reset-highest-IUS, while error-reset is also labeled 110. No opcode is selected.
- MMIO addresses, polarity, CPU vector choice, part and profile, pending clear, rebaseline spacing, and cycle bounds.
- RR0 latch behavior. RR0 is not an edge queue. Cross-axis atomicity and SCC/VIA atomicity are unavailable.

## 8. Future verification cases

These fourteen cases are named and UNEXECUTED. No new test is claimed. No gate is closed. Existing PC qualification is reused only as background. No target, VM, emulator, release, asset, tag, or wiki change is made. The native external-app gate stays closed.

- IC-01 Begin leaves hardware disabled until helper cancellation has succeeded.
- IC-02 A record can be claimed only after its full payload and then its committed count.
- IC-03 A sample past capacity sticks LOSS, keeps every unread record, and overwrites none.
- IC-04 Explicit uncertainty on an empty inbox sticks LOSS, independent of the saturating counter.
- IC-05 Claim copies one record and issues no helper, owner, render, wait, or scan inside exclusion.
- IC-06 Release restores the prior exclusion state and does not globally enable interrupts;delivery requires an otherwise capture-permitting foreground context.
- IC-07 The claimed delivery finishes after a newer loss; the next record waits until one cancel path completes.
- IC-08 Foreground stop discards unread data and the undelivered claim, then uses one helper stop, with no producer callback.
- IC-09 Rebaseline accepts only a post-barrier fresh zero-motion observation; queued motion is not trusted drag; held left stays suppressed.
- IC-10 A second loss view does not cancel twice, and an empty drain is not treated as a loss cancel.
- IC-11 A stale epoch is refused, maximum-epoch restart has no effect, and the first fault is retained.
- IC-12 The producer is quiescent before rebind, does not nest, and does not preempt inside exclusion.

- IC-13 A baseline published before foreground rebaseline stages later records;loss before/during rebaseline takes the correct WAIT/READY cancellation path without a blind interval.
- IC-14 Ordered -1,+1 at a pointer boundary remains distinct;neither producer clamping nor coalescing changes the existing owner result.

## 9. Software-only next slice

Proposed, and not an authorization to implement: on the current Windows x64 host, run a software stand-in of this inbox against the existing SPEC-0063 helper. Use an original deterministic interleaving model with one producer,capacity16,sticky loss,one claimed slot,PRIMING and the section6 timelines;then native68000/x64 object/layout/frame checks. This is not an actual CPU exclusion/timing proof. Use synthetic normalized samples only. Leave the helper unchanged. Choose no acknowledge opcode, open no adapter, and state no 68000 timing. Keep IC-01 through IC-12 unexecuted until that later slice actually runs them. Distinguish actual native-object size/frame facts from hosted simulations;neither proves full128K fit or IRQ latency.

## 10. Limits

No code, header, fixture, or hardware recipe is included. Only reviewed published-interface facts and original engineering enter this contract;no manual bytes were sent to the worker. No Apple/Microsoft implementation,ROM,media,emulator source,disassembly,HQ,untracked original or UART body entered its inputs. No test was executed for this contract.
