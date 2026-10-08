# IMPL-0014 — ACPI PM timer discovery, counter extension and native probe

Date 2026-10-08. SRC-0045 / SRC-0046 / SRC-0036 / SRC-0038; SPEC-0010 / ADR-0014 /
TEST-0016. Claude (Anthropic; session configured as `claude-opus-5-5`) assisted
with the original C, assembly, CMake, PowerShell, tests and documentation.
Implementation-lead self-review; human provenance review pending. GPL-3.0-or-later.

Paths: platform/pc/acpi.h/.c, pmtimer.h/.c, x64/io.S; platform/uefi/loader.h/.c
(RSDP capture and handoff field), native.h/.c, entry.c; tests/acpi.c, pmtimer.c,
native-timer.c, acpi-fixture.h, uefi-loader.c (configuration suite); scripts
Verify-PMTimer.ps1, Verify-UEFIImage.ps1, Test-UEFIImage.ps1; CMake, workflow,
records. apps/boot-scene/scene.c now copies the target descriptor field by field,
and native.c avoids aggregate initializers. This keeps every optimization level
free of memcpy/memset helpers: an i686 -O0 scene object previously imported
`memcpy`, though the audited x64 EFI objects and -O2 objects never did. Rendering
behavior is unchanged.

Original design: the walk uses a reader callback, 64-byte checksum chunks and
fixed bounds. The counter keeps the last raw value and a 64-bit tick sum. Time
conversion uses restoring binary division by the 22-bit rate. The native reader
maps physical to integer addresses through an explicit window and refuses
anything outside one reserved/ACPI map descriptor. The single port read is
four bytes of x64 assembly. No tables, timer values or ports are hard-coded
outside tests.

Exposure: SRC-0046 ACPI table prose/field tables and SRC-0036 timer prose; UEFI
GUID layout from SRC-0038. No other OS, firmware, ACPI library, ASL or sample
code, and no Apple material, was used. Development checks used unpinned Linux
GCC 13.3 / Clang 18.1 / LLD 18 (not evidence); pinned results are in TEST-0016.

Unverified: native execution, real table placement and types, the counter on
VirtualBox, chipset read quirks, accuracy and pauses, the 60-second loop and B2.
