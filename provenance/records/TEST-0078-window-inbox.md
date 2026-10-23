# TEST-0078 - Pure inbox verification and remaining IC scope

2026-10-23;SPEC-0065/IMPL-0066/SRC-0372..0374. Actual hosted/profile/compiler
checks in [evidence](../../docs/development/window-inbox-evidence.json).
Three tests:API/order/refusal oracle;mock-owner deterministic interleavings
through real SPEC-0063;real drawing owner clamp/drag/loss/release behavior.
Raw-byte no-effect snapshots avoid structure-copy padding comparisons. Final
tests differ from initial freeze as disclosed;not a strict final author-blind
claim. Three compiled faulty variants reject,valid control passes. Initial
unit-only oracle did not cover baseline-success concurrent loss;that control
is correctly assigned to the existing interleaving oracle. No hardware run.

|Original case|Actual status|Scope/limitation|
|---|---|---|
|IC-01|partial|Harness helper cancellation precedes inbox begin;hardware enable/CPU delivery unexecuted.|
|IC-02|partial|Complete scalar FIFO observed;actual interrupt publication/barriers/partial-write preemption unexecuted.|
|IC-03|software-pass|16-record capacity/overflow retains unread prefix;three compiled defect controls separately recorded.|
|IC-04|software-pass|Empty-inbox uncertainty and saturated diagnostics do not gate delivery.|
|IC-05|partial|Pure claim has no imports;mock owner callbacks run outside abstract exclusion. No real mask proof.|
|IC-06|unexecuted|Actual save/restore/exclusion primitive and capture-permitting CPU context not implemented.|
|IC-07|software-pass|One claimed record completes during injected newer loss;later record blocked/discarded before cancel/rebaseline.|
|IC-08|software-pass|Boundary stop drops queued/claimed records;repeat no callback;first fault survives failed stop cancel.|
|IC-09|partial|Real owner held-button suppression/release observed;fresh physical barrier/coherent baseline not proved.|
|IC-10|software-pass|Loss before baseline skips duplicate WAIT cancel;loss during successful baseline cancels READY once.|
|IC-11|software-pass|Stale epoch/no-wrap exhaustion and first raw fault retention;epochs injected at endpoints.|
|IC-12|unexecuted|Actual producer non-nesting/quiescence/CPU preemption proof remains caller/hardware work.|
|IC-13|software-pass|Producer stages later records during baseline callback;before/during loss and full-inbox baseline cases exercised.|
|IC-14|software-pass|Real pointer AND dragged-window boundary -1,+1 remain ordered and end atx1;no coalescing/clamping upstream.|

Software-pass means the described synthetic scenario passed,not physical
capture/CPU timing. Partial/unexecuted components remain open. TEST-0077's
dated planned record and TEST-0076's prior results remain unchanged.
