# SPEC-0084 - Opt-in retained decoder-phase witness

Lead-adopted original engineering diagnostic,2026-10-30. Source baseline0511263.
Owner authorizes the next paired-release task. Reuses SRC-0509/0510's executed
sampling study and SRC-0550/0551;no new hardware/reference claim. The retained
MWP1v2 previous word has DCD bits but lacks previous PB quadrature phases.
A rate sweep cannot supply those missing bits.

## Scope and producer

Add explicit --window-timer-phase-witness, requiring --window-drawing,
--window-timer-poll-probe,--window-timer-refresh-probe,--window-timer-live-pair
and --window-timer-divisor4. All existing checkpoint conflicts still refuse before
output. C macro CLASSICK_MAC_TIMER_PHASE_WITNESS defaults0,accepts0/1 only,and
requires pair/poll/refresh/live/divisor4 when1. Build profile is
native-window-timer-live-pair-divisor4-phase-witness-v1;specification SPEC-0084.
Manifest includes this spec/input hash and a phase_witness object recording
trace_version3,trace_bytes128,no_added_state_bytes=true,no_added_MMIO=true.
The unwitnessed live1/live4 modes and every existing default stay available.

Only in this opt-in profile,first_loss.previous preserves all existing fields
and adds the marker and earlier decoder phases shown below. The word is still
written only for the first loss. Do not sample hardware again. Capture the predecessor phases before the decoder call. This is essential
for queue overflow:polled_step has already changed its state when enqueue fails.
The witness must describe the predecessor for every loss reason,not that current
state. If prior motion.have_phase is0,write zero phase bits and report unknown.

| Bits | Meaning |
| --- | --- |
|31|Phase-witness marker,1 for a recorded loss|
|30|Reserved,0|
|29..28|Previous decoder X phase,0..3|
|27..26|Previous decoder Y phase,0..3|
|25|Reserved,0|
|24|Existing have-phase flag|
|16|Existing previous raw DCD A bit|
|8|Existing previous raw DCD B bit|
|0|Existing previous button bit|
|all other bits|Reserved,0|

Allowed mask BD010101. With a known previous phase,its bit0 must equal its
retained raw DCD bit. When no loss has been recorded,keep word0 (no marker).
The opt-in MWP1 trace remains128bytes at all existing offsets,tag4D575031,
version3;version2 stays unchanged elsewhere. Add no state fields or storage.
No persistent RAM/state growth is allowed. A bounded automatic temporary is
permitted and its native stack cost must be audited;instrumentation may perturb
timing. Keep first-loss immutability,all IO order/reads/writes,64NOP pair,IRQs,decoder,
loss/refusal/recovery behavior,128K reservations and ongoing foreground intact.
No implementation of a movement fix or relaxation is authorized by this spec.

## Reader contract

scripts/Read-MacPhaseWitness.py exposes decode(obj) and FILE.json CLI (0 success,
2 refused). Input is exactly {"words":[32 strings]},each an eight-digit unsigned
hex word,optionally0x-prefixed. Strictly refuse duplicate JSON keys,extra keys,
wrong types/lengths,non-hex,wrong tag/version/bytes,unmarked recorded loss,
reserved previous bits,unknown-have with nonzero phases,and known-phase/DCD
parity contradictions. Refuse version2 rather than filling in its missing bits.

Word order is the existing cs_mac_window_trace layout:
tag,version,bytes,source_irq,vbl,scc,combined,services,baselines,losses,queued,
peak,deliveries,pointer_x,pointer_y,window_x,window_y,pattern,phase,motion_stage,
raw_a,raw_b,buttons,motion_cause,cause,loss_reason,loss_rr3,loss_snap0,loss_snap1,
loss_snap2,loss_previous,loss_phase. Signed coordinates/causes decode as int32.

Return {format:"MWP1-v3-phase-witness",mapping:{all32named integers},previous:
{known:bool,x_phase:int_or_null,y_phase:int_or_null},current:null_or_
{x_phase:int,y_phase:int},model_gray_jump_axes:null_or_["x","y"],
consistent_with_gray_loss:bool_or_null}.
For a recorded loss,current phases come from loss_snap1's PB/RR0A/RR0B using
SPEC-0074's engineering normalization. A known predecessor permits independent
per-axis Gray checks using position table0,1,3,2;delta2 is illegal. Legal movement
on both axes is allowed. Only reason6 has a consistency boolean (true iff at
least one jump);otherwise null. Unknown predecessor gives null phase comparison,
not invented PB bits. No loss gives previous.known=false,current=null,axes=null.
A known reason6 with no illegal axis is returned as inconsistent,false:it is a
useful observation,not malformed input. No event-causality or hardware inference.

## Frozen engineering examples and verification

Known/button-up predecessor words in X-major,Y-minor order:
81000000,85000100,89000000,8D000100,
91010000,95010100,99010000,9D010100,
A1000000,A5000100,A9000000,AD000100,
B1010000,B5010100,B9010000,BD010100.
Button-down adds1. Unknown previous phase encodes80000000.
Without the opt-in flag,keep only the legacy01010101 bits.

For current AF/4C/4C,earlier logical phases(0,0),(2,0),(0,2),(2,2) imply jump
axes[y],[x,y],[],[x],respectively. These are frozen synthetic model examples,
not reconstructed hardware history. The third is inconsistent with reason6.

Independent tests must cover all16previous phase pairs,both buttons,unknown
predecessor,first-loss immutability,queue-overflow predecessor retention,
legacy-off behavior,these four Gray cases,
no-loss,and reader/refusal boundaries. Proposed cases are not passing tests.
Native proof must retain64NOP helper bytes,twins/seals/IRQ/budget checks,source
fingerprints and exact unwitnessed ROM baselines. No new VM/reference/hardware
permission follows;existing isolated-candidate scope alone governs any trial.
