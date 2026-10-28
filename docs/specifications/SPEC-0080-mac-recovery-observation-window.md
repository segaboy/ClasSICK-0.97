# SPEC-0080 - Mac input recovery observation window

Original engineering diagnostic;2026-10-28. SRC-0525/0526,IMPL-0088,TEST-0101.
The owner requests this next step without other agents. Existing trial/release
and source/role gates retain their scope;this contract admits no hardware change.

The SPEC-0078 checkpoint freezes on the first loss,so it cannot establish whether
foreground cancellation and subsequent sampling recover. A separate opt-in
profile retains that first-loss record while allowing foreground work to continue.

## Gate

Caller-owned16-byte state contains valid,armed,first_service,last_service uint32
fields. Begin initializes fresh bytes without reading prior contents;null returns1.
Step accepts a monotonically nondecreasing saturating service count and a0/1
activity indication. Null/invalid indication/backward count returns-1;invalid
state returns-2. Refusals have no effect. No allocation,globals,IO or callbacks.

The first observed indication arms the window at that service count. Later
indications do not restart it. Indication means any accumulated loss,queued
nonzero motion or queued zero-motion/button report. It is not proof of a physical
gesture. Services are timer-service counts,not the activity record's pair count.

Return0 while waiting. Return1 after at least512 further services since arming,
and at least500 services overall. Return2 at50000 services if return1 is not yet
available,including no-input and late-input fallback. Completed window takes
precedence when both apply. No wrap arithmetic;UINT32_MAX uses the same fallback.
The fallback may truncate a late window. No wall-time or sample-rate promise.

## Native binding

--window-timer-recovery-probe requires --window-timer-activity-probe and its
existing timer/idle prerequisites. CLASSICK_MAC_TIMER_RECOVERY_PROBE defaults0,
accepts only0/1 and cannot be enabled without those prerequisites. Only the new
profile links the gate. Begin runs while quiescent before timer arm. Observe runs
the gate under the existing IPL7 exclusion;while waiting,it returns to the normal
foreground loop. Error is a diagnostic fault,never a successful checkpoint.

On completion it copies the same80-byte MWA1 snapshot and invokes the existing
audited74-byte MAR1 leaf. No new assembly or register-layout changes. Gate state
has a distinct,checked own-RAM address in the manifest;MAR1 omits it and cannot
alone identify fallback versus a complete window. Keep that limit in readouts.
Record first-loss fields stay immutable while counts may advance. No changes to
SCC/VIA programming,read sequence,cadence,decoder,loss policy or foreground logic.

Old profiles retain their exact bytes. New twins,source/symbol/IRQ/opcode audits,
RAM/stack costs and old hash comparison precede any trial. This step builds and
host-tests the diagnostic;target recovery/input qualification is separate.

## Regression and limits

The independently scripted fake supplies the captured previous FF4444 and pair
9F4C44/8F4444. It checks loss2,retained full previous PB and no invented delta.
After the existing discard/resume handshake,a deliberately stable baseline and
one adjacent DCD transition must queue their specified results. A later Gray loss
must retain the first loss. This continuation is synthetic,not an observed trace
or a physical timing model. Supplying the desired register returns does not
validate hardware latching,atomicity,wiring,frequency or capture.

Hosted recovery is not target recovery. All Mac/PC GUI/human movement,drag,
release/restoration,physical/full128K,historical,edition,parity,B2criterion6,
native-app,System1/ROM and human provenance gates remain.
