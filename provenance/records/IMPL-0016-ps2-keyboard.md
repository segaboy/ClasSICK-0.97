# IMPL-0016 — Bounded polling PS/2 keyboard

2026-10-08. SRC-0048–0052; SPEC-0012 / ADR-0016 / TEST-0018.
OpenAI Codex (GPT-6) assisted with original C, assembly, CMake, PowerShell,
fixtures and documentation. Implementation-lead self-review; human provenance
review pending. GPL-3.0-or-later. No independently staffed clean-room claim.

The reviewed manufacturer/ACPI/ISA interface scope is catalogued before
publication. Specification work defined inputs, state transitions, bounds and
errors; implementation follows the original contract. Discovery snippet exposure
and exclusions are recorded in sources.md. No external driver, OS header,
implementation algorithm, firmware body or protected payload is adopted.

Paths: platform/pc/ps2.h/.c; acpi.h/.c controller-present gate;
platform/pc/x64/keyboard-io.S; platform/uefi/keyboard.h/.c and entry.c;
tests/ps2-fixture.h, ps2.c, acpi-keyboard.c, native-keyboard.c; CMake, EFI audit,
Verify-PS2Keyboard wrapper and workflow. Core event contracts are reused unchanged.
The SPEC-0011 timed-only function remains covered by its original tests.

Design: caller-owned state, one status plus at most one data operation per poll,
phase deadlines/call caps, table-free command sequence, two retries per device
byte, explicit parser prefixes/held keys and saturating diagnostics. An eight-event
FIFO is allocated in the owned arena. The successor loop starts its duration at
READY, accepts initial Space/Escape presses, and records exactly 112 trace bytes
at offset 128. Original INB/OUTB marshaling is audited as six bytes each.

Unverified: real controller model/set-2 compliance, self-test retaining A20 and
machine state, SMM/USB absence, actual port access, native timing/input, firmware
eligibility, media, UART, faults and B2. This is original PC demonstration behavior,
not Macintosh keyboard compatibility or completed full keyboard support.
