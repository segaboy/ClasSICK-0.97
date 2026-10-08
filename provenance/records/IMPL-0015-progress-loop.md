# IMPL-0015 — Cooperative timed progress loop

Date 2026-10-08. SRC-0047; SPEC-0011 / ADR-0015 / TEST-0017. Claude (Anthropic;
session configured as `claude-opus-5-5`) assisted with the original C, CMake,
PowerShell, tests and documentation. Implementation-lead self-review; human
provenance review pending. GPL-3.0-or-later.

Paths: platform/uefi/native.h/.c (`cs_native_progress_loop`), entry.c (60-second
call after the probe); tests/native-loop.c; scripts Verify-ProgressLoop.ps1 and
Test-UEFIImage.ps1; CMake, workflow and records. Presenter, scene, ACPI and
counter algorithms are unchanged.

Design: one synchronous loop with fixed state on the stack. It recomputes time
from accumulated ticks after every read and redraws the full frame only on step
changes. Late-sample and stall diagnostics live in a fixed trace record; there
are no callbacks, interrupts or globals. No external source was consulted beyond
the internal specifications. Development checks used unpinned Linux GCC/Clang
(not evidence); pinned results are in TEST-0017.

Unverified: native execution, real timer rate, the visible bar and its 60-second
duration against an independent clock, VM pauses, keyboard and B2.
