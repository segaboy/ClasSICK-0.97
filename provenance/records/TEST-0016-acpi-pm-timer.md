# TEST-0016 — ACPI walk, PM timer extension and native timer probe

Date 2026-10-08. SPEC-0010 / ADR-0014 / IMPL-0014; original synthetic fixtures only.
No firmware, VM, ACPI table from a real machine, or I/O port access was executed.

Suites: `acpi.discovery/checks/walk` build original RSDP/XSDT/APIC/FADT images
from published offsets. They cover legacy and extended timers, 24/32-bit width,
four unusable-extended fallbacks, HW-reduced and absent timers, every signature,
checksum, length, revision, entry-count, zero-entry, duplicate and not-found
result, and the 256-entry limit. They also cover reader refusal at seven points
and read-volume bounds. `pmtimer.extension/conversion/errors` cover 256,000
wrapping steps across both widths. They check conversion against a
native-division oracle for 200,009 values and every error, with state preserved.
`native-timer.gating/reader/probe` (x64) cover the exit, ready and RSDP gates
without port reads, and the map-type reader for all twelve tested descriptor
types plus window and straddle refusal. They also check probe traces for
success, stall after exactly 20,000,000 reads, out-of-width values, unsupported
timers, checksum failure, tables in conventional memory and an invalid map. The
loader `configuration` suite covers GUID selection, the 256-entry bound and
handoff capture.

Pinned Windows CI history at `claude/ci-staging` before `main` advanced:
[37810231122](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37810231122)
failed at ac6867f. Windows GCC rejected a test-only `unsigned long` counter
narrowing from `size_t` (LLP64). The 222a837 fix changed only tests/acpi.c.
[37810609408](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37810609408)
then passes at `222a837e`. Results: 224-file guard; Clang x64 78 checks per Debug
twin/Release/ASan+UBSan, actual i686 65; scoped GCC x64 63 per Debug
twin/Release, actual i686 50. Four EFI twins with 42 named symbols, the exact
port-read bytes, three DIR64 relocations and one 8-byte writable slot pass. 21
image corruptions and four omitted-object links (transition, exceptions,
presenter, ACPI) reject. Of 41 named fingerprints, 27 are unchanged. Eight change
as expected (loader tests from the new handoff field, the scene and native-gate
tests, both EFI images) and six are new. See
[the snapshot](../../docs/development/pm-timer-evidence.md).

Development-only Linux GCC/Clang/ASan runs also pass (not evidence). Unverified:
real ACPI placement, table types and contents on VirtualBox, the counter rate,
read quirks, pause behavior, the 60-second loop, keyboard, diagnostics, media,
firmware eligibility and B2. Human provenance review is pending.
