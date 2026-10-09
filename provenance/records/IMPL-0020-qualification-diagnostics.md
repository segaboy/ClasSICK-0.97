# IMPL-0020 — Native qualification diagnostics

2026-10-09. SRC-0064 delegation; SRC-0034/0038/0043/0049 with SPEC-0016 addenda;
SPEC-0016 / ADR-0020 / TEST-0024; amends SPEC-0007, SPEC-0012 and SPEC-0013.
Claude (Anthropic; session configured as `claude-opus-5-5`) wrote the original
C, tests, CMake, PowerShell and documentation. Self-review only; human
provenance review pending. GPL-3.0-or-later. No independently staffed or legally
certified clean-room claim.

Inputs were the project's own code and specifications, the TEST-0023 findings
and the cited publisher documents: SMSC's output-port and command tables, Intel's
A20M# description and UEFI 2.11's system-table, memory-type and attribute text.
No firmware, driver, operating-system or emulator source, firmware image or
sample code was read. No Apple material is involved.

Paths and changes:
- `platform/uefi/loader.h/.c`: handoff version 2 with `firmware_revision`,
  `firmware_vendor_state` and a 32-byte vendor copy; a bounded bytewise
  `vendor_copy` runs once, after bundle zeroing and before the exit loop.
- `platform/pc/ps2.h/.c`: two D0 phases and replies, `port_before/port_after`,
  result MACHINE=9, READY at phase 34. Still one port operation per poll and a
  table-free source sequence. As at the parent commit, Clang O2 emits
  position-relative jump tables for it; they add no base relocations.
- `platform/uefi/keyboard.h/.c`: qualification record, partner eligibility
  check, volatile byte probe, probes before startup and at READY, loop result
  MACHINE=14 and a new `qualification` parameter on the observed loop. The
  unobserved `cs_native_keyboard_loop` passes null: no probes, no record.
- `platform/uefi/uart.h/.c`: banner v2, `fw`/`own`/`qual`/`alias` lines,
  required qualification pointer, 1-s / 4,000,000-turn termination drain.
- `platform/uefi/entry.c`: passes trace offset 1024 as the qualification view.
- Tests: `tests/ps2.c` and its fixture (D0 model, self-test port change, D1/F0–FF
  writes counted as violations), `tests/uefi-loader.c` (vendor suite, capture
  before exit), `tests/native-keyboard.c` and `tests/native-uart.c` (new
  qualification suites, v2 transcripts, drain cap), `tests/uart-fixture.h`
  (optional THRE period). CMake registers `ps2.machine`, `loader.vendor`,
  `native-keyboard.qualification` and `native-uart.qualification`.
- `scripts/Prepare-VMTrial.ps1`: required `-Candidate SPEC-0015|SPEC-0016`
  selects the admitted payload/raw-image hashes; machine settings are unchanged.

No new global symbol or import: the EFI required-symbol list (59 names), object
allowlists, omitted-object controls and relocation count (3) are unchanged.
The writable image data is still the single 8-byte slot.

Unloaded EFI results from a local mirror of `Verify-UEFIImage.ps1`'s
compile/link with the pinned LLVM-MinGW 20260908 Linux release (not CI evidence):
the unchanged parent commit reproduces audited payload `8e25d694…ad831f2`
byte for byte; this change gives O2 twins of 43,520 bytes, SHA-256
`920ed92c2b30609718d1ed95867e0cf79aab82ae5f9ce76ffee2c9b806ce5d16`, O0 twins of
60,928 bytes, `6a7469c98d75f977b7ce4044901180f14d12728708d0978edfe67eb621ad05f1`,
both accepted by `Test-UEFIImage.ps1`. The SPEC-0014 writer packages the O2
payload as raw image `e2e6a431722b39d6ed7c215bbc60727aef74ae776f74934f60ddfd0e6f4069e9`
(85 payload clusters), accepted by `Test-BootMedia.ps1`. The pinned Windows CI
result is recorded in TEST-0024.

Nothing was executed natively or in a VM. B2 remains partial. Macintosh/mini
vMac, 128K and three-edition goals are unchanged.
