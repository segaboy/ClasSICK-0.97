# Pure native drawing-port selection - 2026-10-17

SPEC-0043 / ADR-0049 / IMPL-0046 / TEST-0051; SRC-0258..0261.
One original freestanding C core validates a prepared table of one or two ports
and returns a selection plan. The same code builds on68000/x86. Every operation
validates the complete table and the current value supplied afresh. NIL get is
valid; NIL set refuses. Only the plan's fields can change; all failed validation
leaves every input/output byte unchanged. No guest address is dereferenced.

This is a library foundation, not a changed scene or installed manager. Existing
native drawing workloads, all64 native ROM input fingerprints, old guest fixture,
UART bodies, release assets/tags and the24 untracked originals/copies stay exact.
SPEC-0042 guest binding still needs owner integrity, environment-span and lifecycle
contracts. Admitted trusted corruption belongs to owner quarantine; a pure value
validation failure returns0. C22b now states that distinction. No target run/release.

## Worker artifacts and integration
The frozen contract/API hashes preceded both calls. Claude returned the core;
Copilot returned independent tests without implementation, implementation logs or
implementation session history. Both were told Windows11, forbidden tools/writes
and excluded inputs; both recorded zero tool calls and executed no tests. Captures
remain outside Git. Codex reviewed and integrated usable artifacts rather than
recreating them. Grok/Argon qualifications were reused; no redundant worker call.

Claude core adopted unchanged (198 lines). Copilot returned549 test lines against400 budget and malformed JSON (one extra bracket); mechanical format repair retained the original content. Lead fixed the include path, one alias setup that corrupted numeric inputs, random-only coverage counters and an unreachable conditional; added two low-reservation boundaries and distinct partial native port/view overlaps. Original uint64 numeric oracle, complete input/backing snapshots, directed and randomized cases retained. No formatting retry.

Claude 37.31s; Copilot 196.52s. Initial one call each; one subsequent qualified-author Claude build correction. No duplicate reviewer call. Provider counters and list-cost basis are in the [receipt](native-selection-evidence.json); they are not subscription bills or measured ChatGPT quota savings. Account balances not exposed by these calls stay unknown.

## 68000 build correction
The first CI attempt `056165e` failed the new68000 import/data audit at O0; its
diagnostic did not print symbols. A dynamic uintptr_t modulo in alignment
validation was the suspected compiler-helper source, not a confirmed symbol.
Claude supplied one focused33.89s correction: natural alignment uses a
power-of-two guard and mask, preserving the frozen API/results. Final core200
lines; no contract/test-oracle changes. The audit now prints any offending symbols.
All local checks below were rerun after the correction, including eight mutation
controls. A local command quoting error caused redundant unchanged-code checks;
those are not counted as corrected-code verification. Three worker model calls
total (two Claude,one Copilot),all zero tools/writes/tests. No new Grok/Argon call.
The owner explicitly approved repository pushes/CI/evidence after the first
automatic-review publication rejection. No target run or release permission changed.

## Actual verification
810 local CTests pass: clang-hosted-debug-a 285, clang-hosted-sanitized 285, gcc-i686 240. 10 strict width/optimization/endian object audits pass with no undefined imports or mutable global data. 8 deliberately broken-core controls are rejected by the independent test artifact. CI [39074280105](https://github.com/segaboy/ClasSICK-0.97/actions/runs/39074280105) at `5da633e25a80b435792c3835b9f007b74a13bbec` passes both jobs: 3013 CTests across11 profiles,10 selection width/endian audits and6 native68000 selection objects (O0/O2/Os matching twins), with zero undefined imports/mutable data. No target execution. Hosted tests are executions of original test programs; cross-compilation is not a target boot. Full CPU/stack/BE-current-cell/quarantine/commit cases remain unexecuted plans.

Native views/backings are borrowed live storage, with a flat uintptr_t span model
on the supported targets; this cannot prove liveness of arbitrary native pointers.
Plan lifetime ends when its borrowed binding/view/backing changes. Guest owner
integrity/epoch authentication and all unknown owner/map/control aliases remain
outside this pure API. No historical behavior is inferred from these choices.

Next specify P1/P2/P4 together before guest GetPort/SetPort binding; selected-port
drawing needs its later adapter. OneNote/wiki remain retired. Fresh owner go-ahead
is still required for each emulator/VM/hardware/reference trial.
