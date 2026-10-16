# First loaded rectangle calls - 2026-10-16

SPEC-0036 / ADR-0041 / IMPL-0040 / TEST-0044; SRC-0202..0206.

An original resource/CODE application now calls the selected PaintRect and
EraseRect cases through an explicit owned-code coordinator. Its40-byte program
executes13 instructions, returns D0=97, paints once and erases once, yielding
exactly2304 expected black pixels. It explicitly retires its single CODE handle
and scrubs16 below-A5 bytes. Actual retained pixels replace the previous native
checksum visualization in the framed result panel. The checksum remains in
result diagnostics; no host drawing substitutes for the application's output.

## Local verification

All2181 CTests in nine Clang/GCC debug/optimized32/64/sanitizer profiles pass.
The complete matrix includes1500 original edge/random pixel cases per profile,
real loaded guest programs, malformed/flagged calls, stack/continuation/root
checks, native aliases, patch refusal, atomic failure versus exact budget-stop
state, retained drawing, scratch/argument overlap and explicit retirement.
After refining only the panel presentation and its oracle, all54 affected
drawing checks and30 drawing object audits were repeated;2127 unaffected checks
were retained. Across the checkpoint60 object/import/endian audits cover the
final modules, with debug executable twins matching for both compilers.

Original signed/clipped byte rendering and an independent per-pixel oracle
agree. Complete padded/tail scene bytes match the independently overlaid image;
counter updates preserve the image. The hosted preview is a conformance output,
not an emulator screenshot. Ordinary cs_image_run and generic guarded CPU
stepping retain their A-line refusals. No new interpreter addressing mode.

The final original65536-byte ROM twins match, SHA-256
`494aa494b09ff616b8c755b5a07d61dc84594990336d718b65295e611d7d895a`.
Used ROM45192, native data/BSS2168, workspace45592, conservative nonrecursive
frame sum plus assembly margin10376 within24320 reserved native stack bytes;
largest frame604. Original assembly independently matches the40-byte fixture.
Complete linking, image/reset/exception/seal/corruption and static layout checks
pass. These are candidate costs, not a full128K OS or physical-machine proof.
The57 input fingerprints are frozen in [JSON](drawing-app-evidence.json).

## Bounded worker and source review

Claude: one request, three allowed Reads, four model turns and103 original C
lines; no commands/tests/edits. Codex supplies the independent oracle, original
guest program, owner bridge, integration and compiler verification. A local
working-directory launch error occurred before any Claude request and was
corrected; no model-request retry. Grok: one request, one whole327-line frozen
allowed packet read despite requested chunks, no demonstrated bypass within
that packet. Its returned text matches the complete packet after line-prefix
removal. Helpers/CPU internals were omitted and are not credited as reviewed by
Grok. Both requests obeyed standing SRC-0149 source scope; no raw drafts/manuals,
HQ, credentials or prohibited implementation was transferred. No quota/billing
or measured-savings claim. Returned read payloads and usage identities retained.

SRC-0071 public API/field facts are later-manual evidence only; source hash and
selected routine/trap-table page images were checked. Profile geometry, native
port, preservation and completion are independent engineering choices. No
private structure, example implementation, Apple asset or emulator source used.
Human provenance/clean-room certification is not claimed.

## Current qualification boundary

CI qualification is pending at this preparation checkpoint. The new ROM has not
been run in an emulator or on hardware; each trial needs fresh explicit owner
go-ahead. It is a new candidate, not a release. Published alpha0.1.0-alpha.1
assets/tag, all24 untracked research originals/existing copies and old records
remain unchanged. No new duplicate research-copy set or UART body inspection.

This is a fixed128x64 mono case with black/white patterns, not globally installed
QuickDraw. No guest GrafPort/InitGraf/regions, callbacks, recording, port state,
Get/Set/direct QuickDraw equivalence, fonts/windows/events/desktop or native SDK.
The native-app gate stays closed; QQ-1..13, R-001/AL-2, both mandatory System1/ROM
directions and historical/source/UART/physical/full128K/parity/edition/B2/human
and input reliability gates remain. System stays unreleased; wiki queue only.
