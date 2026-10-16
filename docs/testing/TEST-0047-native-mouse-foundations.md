# TEST-0047 - Native button, report and capture conformance

SPEC-0039 / ADR-0045 / IMPL-0043;2026-10-16; SRC-0227..0232.
Primary authored cases cover button transitions and complete/partial reports.
Independent tests enumerate every tagged header and all256x256 payload pairs
(8,388,608 report combinations), untagged headers, header-based signs, overflow,
command-looking payload, invalid no-effect arguments/states, corruption resets,
initially held buttons, repeat/release/cancel and saturation. Mac authored register
transcripts cover2048 ten-sample traces through the actual existing loop and
shared core. Hidden Windows --verify-mouse exercises actual message dispatch,
capture/cancel/focus paths, counters and unchanged non-counter pixels. This is
synthetic hosted input, not human, emulator or freestanding PC qualification.
Strict-warning compiler, sanitizer, freestanding import checks, native68000 ROM
twins/budget and complete CI regressions are credited only when executed.
[Results](../development/mouse-input-evidence.md).
