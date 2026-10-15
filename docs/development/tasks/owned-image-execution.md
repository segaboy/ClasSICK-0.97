# Task brief: owned-image execution contract - 2026-10-15

Owner SRC-0179: "OK. Carry out your plan for the next task. GO."
Baseline public ff3d3f7, compiled99d4a02. Draft SPEC-0034 /
ADR-0038, profile `classick-owned-image-execution-v1`, unimplemented.

## Outcome and budget

Claude authors the first complete engineering contract; Grok checks only the
selected fetch/write/stop risk; Codex integrates, decides, verifies and publishes.
One writer to repository files: Codex. Initial budget two CLI invocations, not
two complete overlapping reviews. Extra calls only for a specific blocking gap.
Use small named file ranges, not one assembled giant packet. No exact-one-Read
rule. No implementation, test fixture, TEST/IMPL/build, actual guest/native app,
reset/boot binding or emulator/VM/reference run. Ordinary Git/HQ/wiki records only.

## Allowed inputs for this drafting task

All paths relative to C:/Repos/ClasSICK-0.97. Read this brief, then these ranges.
Line ranges are one-based, inclusive; keep each tool Read <=110 lines.

- `docs/specifications/SPEC-0033-owned-code-image-lifecycle.md`, lines300..340:
  existing execution prerequisites and boundaries.
- `docs/specifications/SPEC-0028-bounded-guest-sessions.md`, lines1..99:
  root admission, stack/sentinel, exit and fault snapshots.
- `core/cpu/m68k-bus.h`, lines1..40: current bus/dispatch API.
- `core/cpu/m68k-session.h`, lines1..66: current session/driver/exit API.

At most five planned Reads including this brief; at most one additional Read
of `docs/specifications/SPEC-0029-typed-toolbox-dispatch.md`, lines73..116,
if deciding root-only ExitToShell. Do not read other files or browse/search.
These are original project contracts/APIs. No manuals/raw research/HQ/Apple or
Microsoft code, ROMs/binaries/resources/images/disassembly/emulator source,
credentials, manual samples or private heap reconstruction.

## Reviewed baseline facts to use, not rediscover

SPEC33 owns a LOADED data-only image, all selected segments pinned locked; first
CODE1 entry and literal resource-start routine offsets are engineering choices,
AL-2 unverified. Shared32 MM slots, CODE64 entries/16 segments/32768 bytes each.
Non-EXEC app/world/stack and immutable map/backing bindings are sealed. Its gate
allows EMPTY only, so existing sessions cannot simply run a LOADED image.
It retains generation/heap epoch, actual record/cell/extent identity, exclusive
image handles, world/stack reservation and truthful FUTURE/REFUSE inventory.
Associated resource owners must be CLOSED/reference-free on this same runtime.
No guest low-memory launch globals/CPU frame/sentinel are populated by SPEC33.
Loaded rows contain segmentID / 0x4EF9 / resource-start+literal routine offset.
Parameter32 bytes/slack/below-A5 are zero; QD+0 and Finder+16 are not initialized
services. Successful LOAD ends source/staging borrows. RELEASE validates owned
cells/records/table/zeroed world then clears world and reverse-disposes handles.
Unexpected image faults are terminal; ordinary PARTIAL has explicit RELEASE.
Low-level APIs do not intercept every host entry; truthful coordinator coverage
and absence of hidden references remain caller preconditions, not authentication.

## Design requirements / decisions to propose

Draft a narrow first execution profile, not general application compatibility.
Define explicit future changes to SPEC33's non-EXEC/EMPTY-only/zero-world assumptions;
never claim its current API already permits execution. Keep backing/map identities
stable; prefer a separate per-access instruction/data policy over flipping whole
heap EXEC flags or trying to fit16 segments into an eight-region overlay map.
Current bus API has no arbitrary per-memory-access filter: name this missing
primitive and its session/typed-service integration as implementation prerequisites.
Do not duplicate an instruction decoder to predict writes. Require checks before
every opcode/extension fetch and explicit/implicit read/write, strict24 and aliases.
Specify writable globals/stack versus protected segments/headers/padding/table/
handle cells/low memory; prevent A7 escapes and writes through aliases/BlockMove/
trap/native service paths. Regions alone are not ownership proof.
Use one root and bounded budget, fresh admission, entry row and scalar identity,
defined initial registers/status/A5/A7/return sentinel. No direct native pointer
execution. Decide whether root-only unpatched ExitToShell can fit this first
profile; refuse all unsupported traps, MM operations, dynamic LoadSeg/UnloadSeg,
callbacks/patches/interrupts/re-entry unless fully mediated. No arbitrary executor.
Define clean return/exit vs budget/CPU/policy fault, retained effects/frame/report,
root/callback/patch/pending-exit quiescence, single-use/no-resume authority and
post-run world expectations. Existing SPEC33 RELEASE cannot silently accept mutated
globals: propose an explicit audited release/handoff rule without weakening it.
Specify state/refusal/fault order, native alias/output protection, unexecuted future
test obligations and the first smallest dependency to implement later.

## Deliverable

Return only a standalone Markdown draft SPEC-0034, about1000..1400 words, maximum
1600. Use concise normative sections/tables. Clearly mark engineering decisions,
new prerequisites, unimplemented status and all unexecuted obligations. No code or
fixtures. Retain historical/native/source/UART/physical/full128K/parity/edition/B2/
human gates, all24 untracked originals and both untested System1/ROM boot targets.
No extra agents/chats/automation/OneNote; no edits or commands by the CLI worker.
