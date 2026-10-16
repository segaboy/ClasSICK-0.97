# TEST-0044 - Original loaded rectangle calls

SPEC-0036 / IMPL-0040, 2026-10-16. Engineering verification, not historical parity.

1. Independent per-pixel oracle:1500 deterministic edge/random paint/erase
   rectangles, signed extremes, clipping, empty/inverted inputs, bit boundaries,
   preserved unrelated bits/guard bytes, immutable arguments, null/unknown
   operation refusal and diagnostic saturation.
2. Actual original resource PREPARE/LOAD/RUN/RETIRE:13 successful instructions,
   D0=97, one paint/erase,2304 exact expected black pixels, balanced root return,
   unchanged caller/gateway and16 below-A5 bytes scrubbed. Ordinary execution
   still stops at the first A-line.
3. Trap atomicity: independently loaded identical programs stopped just before
   a failing trap have byte-identical RAM/port/frame to the failed-call case.
   Unknown, auto-pop, reserved, OS/exit variants, odd/high-byte/low-memory/cell/
   token/cross-span Rect pointers, missing parameter, scratch floor and invalid
   final continuation refuse without that call's effects.
4. Retained effects: budget after paint, later unknown trap and forged token jump
   preserve actual prior drawing; no repeat/resume. A Rect overlapping exception
   scratch uses its pre-commit bytes. An empty guest erase succeeds/counts.
5. Native aliases/null/misalignment refuse before report mutation/claim. Both
   previously patched directory slots refuse admission. Valid subsequent entry
   remains available after ordinary pre-entry refusal.
6. Full scene copy uses actual guest-produced pixels; an independent pixel
   overlay checks all output bytes, row padding and tail. Native counter update
   leaves the port image unchanged. Optional host preview is not a boot capture.

Run Clang/GCC debug/optimized32/64 and sanitizer profiles, existing complete
regressions, native import/endian audits, independent GNU68000 assembly encoding,
no-runtime ROM twins and static layout/stack checks. CI builds the separate
drawing candidate; no emulator/VM/reference run follows without fresh go-ahead.
Completed measurements and exact revisions belong in
[evidence](../development/drawing-app-evidence.md), not inferred from this plan.
