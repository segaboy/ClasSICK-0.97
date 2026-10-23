# TEST-0076 - Ordered capture verification

2026-10-22;SPEC-0063;IMPL-0065;SRC-0366..0368.
343 final local profile CTests,12 native object audits,three actual compiled defects
rejected and valid control passed. Native68000 compiler/closure checks pending CI.
Two API-only tests frozen before implementation inspection;later raw-byte snapshot
correction follows the first GCC Release CI failure without changing expectations.

|TEST-0075 case|Actually executed scope / limits|
|---|---|
|MP1|Sequential begin/WAIT,no capture admission;no MMIO in helper. Owner cancel is foreground-only.|
|MP2|Ordered normalized X records through stub and real owner.|
|MP3|Ordered Y records and full int32 values;real owner coordinate transform.|
|MP4|Motion/press/held/release order;real title drag/body toggle/counter.|
|MP5|Pointer boundary -1,+1 endsx1. Actual window drag verified;new dragged-window boundary-reversal subcase not executed.|
|MP6|Overflow retains stale prefix until foreground cancellation;three faulty controls include premature discard.|
|MP7|Real owner held rebaseline remains suppressed until fresh release;no invented press.|
|MP8|Stop/restart discards old records;stale epochs refused. Exhaustion is an injected UINT32_MAX endpoint,not billions of restarts.|
|MP9|Injected saturated delivered/loss diagnostics do not gate freshness.|
|MP10|Sample/baseline/cancel failures retain first raw stage/cause;no replay/automatic repair.|
|MP11|Sequential stop/operation boundaries only;no interrupt/preempted partial-publication proof.|
|MP12|Explicit uncertainty cancels. Hidden hardware edge loss cannot be tested or claimed detected here.|

All observations are E1 original software evidence,no IRQ/physical/historical
claim. Exact source-pinned CI and full report:[evidence](../../docs/development/window-capture-evidence.json).


First CI62926d1:68000 passes,GCC Release oracle fails on struct-copy padding.
Reproduced atGCC-O3;raw byte snapshots fix that harness error while preserving
full-byte no-effect checks.343 final local CTests include3 GCC64 Release checks;
three compiled defects still reject. Production code unchanged.


Source-pinned CI `d6e851fc42743430db371c0f599862231662defe` succeeds for Windows and68000.
Actual target compiler checks:78 objects/6 closed links,matching twins,O0/O2/Os,
descriptor248B/helper largest frame32B.
No target execution or all-IRQ/full128K claim. [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39882048452).
