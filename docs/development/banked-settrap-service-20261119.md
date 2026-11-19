<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Installing and calling a 68k routine on the native 286

Under BootHush, the native 286 runtime executed an original 68k program that
installed a RAM address in its trap directory using SetTrapAddress (A047),
cleared A0, retrieved the installed address with GetTrapAddress (A146), then
called it with JSR (A0). The routine set D3 to 42 and returned with RTS to
0x9214. The following NOP completed at 0x9216. This is an explicit subroutine
call to a retrieved target. Automatic dispatch to a guest-patched trap,
startup from the original disks and physical 286 testing remain open.

## Transaction

[SPEC-0129](../specifications/SPEC-0129-banked-settrap-service.md) extends the
banked API with an explicit Get/Set entry. The existing Get-only entry continues
to reject A047. Set validates the current Set slot, checks the supplied index
and address using the shared trap directory's validation, and stages the
completed CPU frame. After the six-byte guest stack write succeeds, it
publishes the directory entry and CPU frame under exclusive ownership.

The 128 KiB guest mapped eight physical banks. Three held parts of the
six-byte scratch area. Denial on the final byte preserved all three banks,
the full directory and the frame. Retrying installed address 0x9340 in slot
23. Both successful native builds independently exposed the final stack
bytes at physical 0x38FFC, 0x41000 and 0x42000 (27 10 00, 00 and 92 14).
The last four bytes are the return PC left by JSR. The program also checked
the surrounding canaries and restored stack pointer.

## Reproduction

Run from the repository with a fresh output path. The script verifies the
selected C16 toolchain members and BootHush executable, then records tool
and source hashes before compilation and rechecks them before publishing
the report.

```powershell
& C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe -B scripts/Test-BankedServices.py --cc .tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain --emulator C:/Repos/.task-temp/boothush-floppy-writes-20261118-b/a/boothush-active.exe --output C:/Repos/.task-temp/banked-trap-set-fresh
```

The 2026-11-19 run is in
`C:/Repos/.task-temp/banked-trap-set-20261119-a/report.json`, SHA-256
`defe8baf7cecf03e8d33958b06af53490f1b39625c5b49dc6d7ea9bf16f7149d`.
BootHush SHA-256 is
`3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4`.
Each of the Release, ASan/UBSan and C99 host profiles passed 701,769 checks
for Set and 619,707 for the existing Get profile. The generic service
suites for decode, directory, calls, failures and probe also passed.

| Native 286 build | Loaded bytes | POST | Modeled cycles |
| --- | ---: | --- | ---: |
| Optimized | 19,372 | B8 | 7,316,712 |
| Unoptimized service wrapper | 20,846 | B8 | 7,514,536 |
| Get publishes frame before write | 19,382 | E1 | 2,336,954 |
| Get splits scratch writes | 19,408 | E2 | 2,574,336 |
| Get skips signature check | 19,356 | E3 | 2,898,717 |
| Set publishes directory before write | 19,400 | E4 | 5,054,159 |
| Set publishes frame before write | 19,388 | E5 | 5,138,870 |
| Set splits scratch writes | 19,441 | E6 | 5,217,536 |
| Set skips signature check | 19,356 | E7 | 5,362,752 |

The last seven markers are distinct diagnostic failures induced by the
harness. All nine builds linked native USE16/80286 objects without a runtime
library. Each build used two fresh boots to compare all loaded program bytes
against the built payload within the CLI inspection limit. The original
loader verified the full payload checksum each boot, the reported machine
state matched between replays, and the source floppy hashes stayed fixed.
Cycle counts include startup and diagnostics. Real-time performance and
execution from the installed PC preview remain unmeasured.
