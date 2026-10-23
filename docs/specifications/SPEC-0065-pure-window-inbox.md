# SPEC-0065 - Pure normalized window inbox

2026-10-23;original implementation slice of SPEC-0064. Low-level inbox only,
not a production foreground bridge,CPU exclusion primitive,ISR or SCC adapter.
API:apps/window-scene/inbox.h. Hosted synthetic coordination with unchanged
SPEC-0063 is a test harness,not hardware admission. Native68000/x64 same C.

Single CPU/non-nesting producer and serialized foreground are caller contracts.
All foreground inbox operations occur under caller short exclusion with proper
compiler ordering;helper/owner calls occur outside it in an otherwise capture-
permitting context. This module neither masks nor enables interrupts and
provides no thread/volatile-only/lock-free publication guarantee. Producer
operations never touch last. Foreground may read that sole by-value claim
unmasked while the producer runs,until completion/stop/fault at a foreground
boundary. No metadata mutation by callers except explicit diagnostic test
injection. Live/aligned storage is a precondition,not arbitrary pointer safety.
No allocation,mutable globals/statics,callbacks,MMIO or external imports.

Descriptor includes16 complete int32 dx/dy,uint32 buttons records,one claimed
record and metadata,<=256B on supported ABIs. Native per-function frame<=512B;
actual measurements required. Additional caller/owner/interrupt/ROM/RAM cost
remains outside this bound;not a complete128K or latency claim.

Fresh begin checks null then initializes valid/epoch1,WAIT,head/count/claimed0,
reasonSTARTUP,losses/delivered/cause0,last zero;unused records/padding unspecified.
No old-byte read;return OK. Caller already completed dedicated helper begin/cancellation.
Restart requires a valid canonical ENDED object,refuses epochUINT32_MAX with
EXHAUSTED/no effects;else same reset with epoch+1/OK. Old producer quiescent.
Caller preflights both epochs before any helper restart,then keeps them equal.

All later calls validate null(ARGUMENT),then tag/nonzero matching epoch,
recognized phase,head<16,count<=16,claimed in0..2(STATE/no effects on failure).
Restart validates object without caller epoch. Canonical WAIT/ENDED require
head/count/claimed0;FAULT requires claimed0;PRIMING forbids SAMPLE claim and
requires count>0 if no claim;READY forbids BASELINE claim. LOSS accepts either
pending claim. Public corruption is not a supported operation. Buttons>7 in
baseline/push return ARGUMENT before object reads;fault cause0 does likewise.
Other precondition refusals never change any byte,including padding/unused
records. No saturation counter is a freshness or admission gate.

|Operation|Required state|Effects and result|
|---|---|---|
|baseline|WAIT|One complete zero-motion actual-buttons record atslot0,count1;PRIMING,clear loss_reason;OK. Sole producer must establish fresh post-barrier reference;software cannot prove freshness/coherence.|
|push|PRIMING/READY|Append unchanged dx/dy/buttons at(head+count)mod16,payload before count;OK. No clamp/coalescing. Full ring instead LOSS/reasonOVERFLOW,losses saturating++,LOST;retain every unread record/count.|
|uncertain|WAIT/PRIMING/READY|LOSS/reasonUNCERTAIN,losses saturating++,LOST;retain unread prefix and any claim,even empty.|
|claim|PRIMING/READY,no claim|If empty EMPTY/no effects;otherwise scalar-copy oldest to last,advance/decrement before return. Mark BASELINE in PRIMING,SAMPLE in READY. Return BASELINE_ITEM/SAMPLE_ITEM.|
|complete|pending claim,phase PRIMING/READY/LOSS|Clear claim. Nonzero cause ->FAULT/preserve first cause,FAULTED,retain queue. Success increments saturating delivered for SAMPLE only;BASELINE PRIMING->READY. LOSS stays LOSS/LOST;otherwise OK. Last retained.|
|discard|LOSS,no claim|head/count0,keep LOSS/last/reason/cause;OK. No helper call. Repeated discard OK.|
|resume|LOSS,no claim,head/count0|WAIT/BASELINE,retain last/reason/counters/cause. Must follow completed helper cancellation or helper already WAIT. No automatic retry.|
|fault|any valid phase,nonzero cause|Preserve first cause,clear claim,retain prefix. ENDED stays ENDED,otherwise FAULT;FAULTED.|
|stop|any valid phase|ENDED,head/count/claimed0,preserve diagnostics/last. STOPPED. Repeated stop no effects. Caller finishes in-flight owner call first.|

Ordinary unavailable operations in LOSS/WAIT/FAULT/ENDED return respectively
LOST/BASELINE/FAULTED/STOPPED without effects. For claim in PRIMING/READY with
pending claim,complete without a claim,baseline in PRIMING/READY,discard/resume
in PRIMING/READY,noncanonical empty-prefix resume,or restart before ENDED:return
STATE/no effects. claim in LOSS never accepts another record even when a
previous claim remains. push continues in PRIMING/READY while claim held.
uncertain/baseline/push in LOSS are LOST/no effects;additional LOSS reports do
not increment diagnostics or replace the reason. stop/fault exceptions above
take precedence over unavailable-operation returns. complete with a pending
claim in LOSS may finish that one accepted item and never clears LOSS.

Required coordination in the original test harness:dedicated helper empty/READY
at each ordinary claim,one push/drain,then inbox complete. Baseline claim uses
helper rebaseline once;producer can stage later records during it. Any helper
non-success becomes a nonzero cause;OK/CHANGED owner success maps tozero.
First owner failure wins over concurrent loss. Loss discards inbox under short
exclusion,then unmasked uncertain/drain if helper READY;WAIT skips duplicate
cancel. Resume only after success. Stop inbox before unmasked helper stop;
record stop failure without reopening ENDED. No unrelated producer/consumer
or direct pointer/window leaf calls permitted in this harness.

IC-01..14 from TEST-0077 are mapped to actual checks in TEST-0078 as executed.
Hosted model checks can demonstrate the abstract exclusion/injection boundaries
and record order,not actual CPU masking,compiler interrupt safety,pin polarity,
physical no-loss capture,fresh hardware baseline or timing. M1-M7,PC GUI/human/
CWP1 and all compatibility/release gates persist. No boot input changes or
emulator/VM/reference/hardware trial in this slice.
