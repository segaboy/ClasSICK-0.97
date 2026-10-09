# TEST-0024 — Native qualification diagnostics

2026-10-09. SPEC-0016 / ADR-0020 / IMPL-0020; SRC-0064. Hosted and unloaded
checks only: nothing ran natively, in a VM or under firmware.

## New and changed suites

- `ps2.startup`: the 20-write transcript now includes D0 before AA and after the
  post-test drain; both port values recorded (0xCF/0xCF); resend checks start at
  the first keyboard byte (index 15). `ps2.errors`: reply/ACK phases
  4,8,10,15,19,21,24,25,27,29,31,33 reject error-flagged bytes and ignore
  auxiliary bytes; timeouts for every phase below 34 with the self-test (10) and
  BAT (25) limits; renumbered PROTOCOL/DEVICE/CONTROLLER and invalid-phase cases.
- `ps2.machine` (new): self-test port changes 0xCD, 0xCE and 0xCC stop with
  MACHINE at phase 15 after exactly ten writes, and 0x4F (bits 2–7 only)
  reaches READY; direct reply-phase recording; a controller that never answers
  D0 completes startup with both ports unread, and the D0 phases advance on their
  tick and call bounds; no comparison with an unread first value; sticky MACHINE
  and an out-of-range result rejected. The fixture counts any D1 or F0–FF write to
  0x64 as a violation in every suite.
- `loader.transactions`: handoff version 2, size, revision and vendor; the mock
  overwrites the vendor string and revision at exit, so equality proves capture
  before exit. `loader.vendor` (new): 0, 5, 30 and 31 units complete; 32 and 39
  units truncated, with and without a later NUL; control, non-ASCII and DEL units
  become `?`; a null pointer gives ABSENT with nothing copied.
- `native-keyboard.qualification` (new): probe PASS for partner types 2, 4 and 7
  with the partner bytes unchanged, the cell restored and the full scene drawn;
  UNAVAILABLE for types 1, 3, 6 and 9, missing WB, the runtime bit, a partner
  past the window and an unmapped partner (type 0xFFFFFFFF); ALIAS before
  startup (cell placed so it is its own partner) stops with MACHINE before any
  controller access or drawing; ALIAS that appears when AA is written stops with
  MACHINE after READY and before drawing; a self-test Gate A20 change maps PS/2
  MACHINE to loop MACHINE; a silent D0 still completes with ports unread; record contents for completed, failed and rejected
  runs, null-trace and null-handoff cases.
- `native-uart.output`: exact v2 transcript (banner, `fw`, `own`, `keyboard
  ready`, `qual`, `alias`, frames, result; 11 records), including the UART's own
  printable filter. `native-uart.qualification` (new): exact transcript for
  ALIAS before startup and the tail for a Gate A20 change, with no `keyboard
  ready` line and the framebuffer untouched. `native-uart.failure`: a stopped
  clock ends the drain at the transport's no-progress bound; a slow THRE with a
  stopped clock reaches the 4,000,000-turn drain cap. `native-uart.gating`: a
  null qualification pointer is rejected with no I/O.

## Local results (container, not the pinned toolchain)

Ubuntu GCC 13.3.0 Debug and Clang 18.1.3 Debug and Release each run 95 CTest
checks (91 at the parent commit). All functional suites pass. The checks that
fail are the same as at the parent commit in this container: GCC
`native-uart.freestanding`, `native-keyboard.freestanding` and
`loader.freestanding` (ELF `_GLOBAL_OFFSET_TABLE_`, CMake 3.28 policy), Clang
`loader.freestanding` (policy). GCC O2 with ASan/UBSan passes all 79 functional
checks; its freestanding audits fail only because that local configuration also
passed sanitizer flags to the freestanding objects.

Freestanding compiles of the changed C files with GCC and Clang at O0/O2/O3/Os
under the EFI warning set (`-Werror -Wconversion -Wsign-conversion`,
`-Wframe-larger-than=2048`) succeed with no memset/memcpy or other helper
imports; Clang's keyboard and native UART imports equal the existing
allowlists. With the pinned LLVM-MinGW 20260908 Linux build (IMPL-0020) the
parent reproduces `8e25d694…ad831f2`; the new O2 twins are 43,520 bytes,
`920ed92c…ce5d16`, and O0 twins 60,928 bytes, `6a7469c9…ad05f1`, both accepted
by `Test-UEFIImage.ps1` with 3 relocations and 59 required symbols. The writer
packages the O2 payload as raw image `e2e6a431…4069e9`, accepted by
`Test-BootMedia.ps1`.

## Pinned Windows CI

Pending.
