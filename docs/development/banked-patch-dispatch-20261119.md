<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# A 68k guest intercepts an OS trap on the 286

The native C16 runtime executed an original 68k caller on BootHush PC/AT.
The caller saved its old RAM handler address using GetTrapAddress, installed a
replacement using SetTrapAddress, and issued A017 and A117. For each trap
the execution loop looked up slot 23, pushed the caller's continuation and
started the replacement at 0x9480. The replacement called its predecessor
at 0x9340 via JSR (A2). D3 reached 42 on each invocation, confirming that
the old handler ran. Both returns used actual guest stack bytes. The caller
resumed at 0x9410 and 0x9412 respectively, with A7 restored to 0x11002.
The second call returned the handler's A0 value 0x1234. The first preserved
the caller's A0 value 0x9480. Both preserved D1 and returned a negative low
word in D0 with SR 0x2718.

The 128 KiB banked backend spans eight scattered physical banks. Denying the
last byte of the trap's stack push preserved the CPU frame and prior scratch
bytes and left the patch call stack empty. The same A-line succeeded on retry.
After the final return, external inspections read 94 82 00, 00 and 94 12 at
physical 0x38FFC, 0x41000 and 0x42000. These include the continuations of
the chained subroutine and the second trap. The hosted fixture additionally
executes a trap inside a handler, checks the two epilogues independently, and
exercises stack tampering, aliasing, denied guards, overlapping code/stack,
remapped Get, excessive depth and unsupported words.

## Qualification

Use the command in the [Set/Get receipt](banked-settrap-service-20261119.md)
with a fresh output directory. The final report is
`C:/Repos/.task-temp/banked-patch-dispatch-20261119-c/report.json`, SHA-256
`475ac3dfdb1de7abbb98fadde32c4968ed6a22a6eba084283f2449201d821c7b`.
It verifies source and tool identities before and after qualification and
records generated object, payload and floppy hashes. The pinned BootHush hash is
`3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4`.

| Native build | Bytes | Modeled cycles | Required POST |
| --- | ---: | ---: | --- |
| Size optimized | 21,904 | 12,894,737 | B8 |
| Service wrapper unoptimized | 24,405 | 13,258,019 | B8 |

Each of eleven builds ran twice to verify every loaded payload byte under
BootHush's inspection limit. Both normal builds reached B8. Seven earlier
Get/Set faults reached E1 through E7. A patch-entry build that published the
frame before a failed stack write reached EA. A patch-return build that
omitted the bit-8 A0 preservation rule reached EB. The loader verified each
payload and repeated boots reported identical machine state. Times include
diagnostics and startup.

In Release, ASan/UBSan and C99 host builds, the expanded Set/patch fixture
passed 701,858 checks and the legacy Get fixture passed 619,707. All five
generic trap suites passed. The normal two CTest entries also passed,
recorded at `C:/Repos/.task-temp/classick-ports-ctest-20261119-a/patch-dispatch-ctest-final.log`,
SHA-256 `feb13dd1475048adabc7577e63c8e2a5f191eaa4c3ea81a9e9ba4353ce102795`.
The harness and CMake reruns include the updated absolute-short MOVE decoder
in `core/cpu/m68k-bus.c` (SHA-256
`08d387d8afb2a27421adc7aaf4576b9d5c374028c8e4047bccb39dae6df80afc`).

[SPEC-0135](../specifications/SPEC-0135-banked-patch-dispatch.md) defines
the admitted register-based OS trap forms and caller-owned state. The run
used an original diagnostic disk. Installed System/Finder execution,
native-token chaining, Toolbox calling conventions, physical hardware
behavior and the January 1984 routine inventory remain open.
