# PC mouse-controller checkpoint - 2026-10-16

SPEC-0040 / ADR-0046 / IMPL-0044 / TEST-0048; SRC-0233..0237.
SPEC-0040 implements single-owner8042 keyboard/mouse startup and routing, plus an explicit native x64 PC mouse build profile. Shared button semantics and512x342 scene match the existing Mac/Windows foundation. Hosted/compiler/EFI candidate evidence only; no emulator/VM/hardware/reference trial or release. Default legacy keyboard contract/trace, UART source/test bodies, Mac ROM inputs,24 originals/copies and frozen wiki remain preserved.

The coordinator owns all controller I/O after the existing keyboard startup.
It disables/tests/enables AUX, sends each mouse command with D4, accepts only
correct-source ACK/BAT/ID/configuration replies and bounds every wait/retry/drain.
READY routes keyboard bytes into the existing FIFO and mouse reports through
the shared decoder/button core. Corruption cancels held state and requires an
uncredited baseline. Unsupported/missing mouse startup stops before drawing.

The explicit EFI native profile uses that coordinator and the real native scene.
Mouse presses update the combined counter; Space retains its progress offset
and Escape stops. Logical512x342 stays centered. Trace v2 uses the same112 bytes;
the unchanged serial carrier's legacy `space=` field is combined input in v2.
The legacy CMake profile remains default0, with its original v1 contract/tests.
No UART implementation/test body was inspected/modified or transferred.

## Worker disposition and controls

One combined Claude draft exceeded360s without a deliverable. One narrower
retry (code and exact binding patches, no test artifacts) finished in145.73s.
It supplied161 bus-code lines,21 header lines, a38-line spec and native edits.
Use delta patches and fewer output artifacts for future primaries; low effort
and deadlines alone do not guarantee completion. Supported partial output was
captured on retry. Copilot supplied273 independent test lines in168.78s,13 over
the requested line budget. Both successful calls read their one permitted
packet, made no writes/commands/test runs/web/MCP/subagents and saw no manuals,
HQ, research originals, credentials or prohibited implementation. Raw transcripts
remain outside Git. Existing Grok/Antigravity qualifications reused, no new calls.

Lead corrections count all AUX replies, reject phase/result mismatches and mark
native queue faults in bus diagnostics. Independent fixture corrections distinguish
overflow header0xFA from ordinary payload bytes and include expected AUX replies;
licenses/strict conversions/port assertions and2 forged-state controls added.
Codex supplies complete-startup/native integration tests, build wiring and source
checks. The first broad compiler scripts exposed lead flag-array/import-check
wiring errors; corrected. Native ARM compile correctly rejects the existing
x64-only UEFI ABI; the final audit treats both byte orders as negative controls,
without weakening the ABI. Core/decoder math and old tests are reused unchanged.
Provider counters and list-cost estimates in [JSON](pc-mouse-evidence.json)
do not measure subscription billing or ChatGPT quota savings.

## Actual verification

All786 local CTests pass: Clang debug277, Clang ASan/UBSan277 and GCC i686232.
The2 final forged-state controls were rerun in all3 profiles,3 extra passes.
Full original keyboard/AUX startup, independent exact transcripts/wrong-source
floods/deadlines/retries, shared scene input, legacy keyboard/UART, pixels/guards
and previous8,388,608-case decoder coverage pass. These are authored hosted
models, not physical/controller/emulator observations.

34 import/global-data object audits pass: Clang/GCC32/64/O0/O2 and AArch64
little/big-endian portable components, plus native x64 producer profiles.
Both unsupported native ARM ABI compilations reject as expected. Original EFI
O0/O2 twins match,27 corruption rejections and18 omitted-object controls pass.
Optimized candidate SHA-256 `1f0d222b355f54b01a2822a8c857ba0f78b3dc9192744b562a6a3c2bdc2fd4c1`.
All61 native Mac ROM inputs remain byte-identical; that ROM is unchanged.
[Candidate identity record](pc-mouse-builds.json). CI pending at this checkpoint.

## Remaining gates

No current emulator/VM/hardware/reference trial, release, physical/human input
or historical qualification. The strict PS/2 mouse profile needs a fresh approved
native boot trial; current automated/human delivery and full pointer/Event Manager
parity remain open. No cursor UI, USB/legacy/SMM/hotplug or universal PC fallback.
Mac/Windows native button behavior and all protected originals/old rows/releases/
tags remain intact. OneNote and all wiki maintenance stay permanently retired.
