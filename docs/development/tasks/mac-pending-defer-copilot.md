# Experimental one-pass pending-edge defer

Windows11,baseline38e938b. Codex sole writer/integrator/verifier. One bounded
fresh Copilot task. Read this frozen packet once only,no other reads,commands,
writes,web,MCP,delegation or tests. Original reviewed public source/contracts
only;no manuals,HQ,untracked originals,Apple/MS code,emulator source or samples.

Deliver exact unique old/new replacements for motion.h/.c and one new standalone
tests/mac-motion-defer.c. JSON {replacements:[{path,old,new}],files:[{path,content}],
handoff:{result,tests_executed:[],blockers:[],decisions_needed:[]}}. New/replacement
C<=320 lines,brief handoff<=160words. No build scripts/CMake/stable IDs.

This is an ORIGINAL OPT-IN ENGINEERING EXPERIMENT,not a diagnosed hardware bug,
physical timing guarantee or release qualification. Default sampling remains
unchanged. Earlier actual RR3zero/one-axis unstable trace did not isolate its
cause. We are testing a finite policy while preserving every accepted sample's
existing stability and pending/phase checks. No claim it fixes native input.

Use CLASSICK_MAC_PENDING_DEFER_PROBE,default0 in motion.h;reject values outside
0/1 at preprocessing. With flag0 original state layout/IO/behavior/binary must
remain unchanged. With1 append exactly two uint32 fields after first_loss:
defer_pending (0 or RR3 A=8/B=1),defer_snapshot (masked PB/RR0 endpoint).
Keep original size bounds. No extra IO,allocation,callbacks,trace fields or enum.

Existing service takes3snapshots/acks and computes stable. Preserve that. Only
under1,READY + have_phase + !quiesced may defer the FIRST nonstable observation
when all of the following hold:
- RR3 is zero,so no external latch was acknowledged in this pass.
- s1 raw DCDs equal remembered last_raw_a/b;exactly one final DCD differs.
- The other DCD stays at its remembered value in all3snapshots.
- PB3 button stays unchanged across3snapshots and matches last_buttons.
- The OTHER axis's PB direction bit stays unchanged across3snapshots.
- The changed axis's PB direction bit changes at most once across3snapshots:
  reject a-b-a toggling. One DCD transition is observed old->new->new or
  old->old->new. Mask only PB bits3..5 and both RR0 DCD bits.
Store expected RR3 bit and masked s3 endpoint;return EMPTY. Do not update
remembered phase/buttons,samples,losses,queue,baseline or deliver a record.

On the very NEXT service no second defer is allowed. It must still be READY
with have_phase,all3snapshots stable and equal to the saved masked endpoint,
and RR3 must equal the saved single-axis bit exactly. Clear deferred state
then proceed through original changed-phase/pending validation and queue push.
Only that original stable path can emit the one record. Otherwise clear state
and use existing uncertain/quiesce:UNSTABLE for nonstable,otherwise PENDING.
Unsupported RR3 and invalid/terminal/epoch state remain original hard faults;
do not turn them into loss. Clear deferred state on WAIT,LOSS,hard fail,stop
and fresh prepare. PRIMING never creates a defer. Lifecycle cannot replay it.

This permits one finite observation delay,not acceptance of unstable data,
motion coalescing,hidden-edge recovery,an unbounded loop,automatic rearm or a
new timestamp guarantee. A pending bit plus endpoint still cannot establish
physical chronology or rule out missed even transitions. Normal profile stays
default;native experiment needs its own reviewed compiler/ROM/target receipt.

Tests should provide injected own SCC/VIA callbacks and literal samples for:
A/B defer then one exact record;zero publication/unchanged old phase on defer;
wrong next pending/endpoint/button/other axis;second unstable pass->LOSS;no
defer in PRIMING;two-axis or direction a-b-a first instability->LOSS;hard-fault
precedence;WAIT/recovery/stop clears;IO order/count unchanged. Worker tests
remain proposed. Codex independently checks state transitions and native costs.
