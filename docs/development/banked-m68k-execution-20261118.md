<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native286 bank-backed guarded interpreter - 2026-11-18

Canonical receipt for SPEC-0120 / IMPL-0150 / TEST-0211, extending the original
SPEC-0117 bank provider and the shared guarded decoder including SPEC-0118.
This is actual native286 execution of an independently authored 68000 program
through scattered physical RAM banks. It is not selected System Disk execution
or a simulated desktop milestone. No protected media/code, third-party source,
private HQ, excluded research, held UART body, WSL or other emulator was used.

## Implementation and boundary

`core/cpu/m68k-backend.h` adds a trusted native backend descriptor and guarded
entry. `core/cpu/m68k-bus.c` shares the existing decoder, guard events, staged
CPU frame and stack rules between that entry and the unchanged flat APIs.
Backend reads and numeric span checks replace borrowed flat pointers. A scalar
store is preflighted, then all final state and COMMIT checks run, then one
atomic backend write precedes frame publication. A failed final write has no
RAM/frame effect. No per-byte callback is added to the native flat path.

The new entry is strict24 with a mandatory guard and bounded stack, S=1/T=0/I=7.
All A-lines are refused before dispatch. It supplies no gateway, native service,
EXEC override, return token, image lease or application-start authority.
Existing `cs_guest_map`, `cs_guest_view` and flat interpreter entry signatures
are unchanged. This is a separately admitted execution route, not a migration
of managers, call/session, image, Resource Manager or Toolbox callers.

`platform/pc286/m68k-banked.[ch]` owns its complete bank service and generation
inline in one near context. The entire declared conventional RAM arena,
including gaps, must exclude current full CS and DS/SS envelopes. Generic
controls, complete backend context and guard context have checked disjoint
native spans; typed controls retain the admitted ABI alignment checks. Callers
provide live aligned objects and stable exclusive ordinary RAM, immutable
callbacks/policy and no reentry, DMA, retirement or rebind during a step.
This is not arbitrary native-pointer validation or a security boundary.

The bank provider also exposes numeric-only validate/span operations. These
permit a 32-bit logical range without creating a near pointer or touching its
backing. Native binding/lifetime/backing failures map to the existing guest
MAP result; no production error enum is extended. Final writes revalidate the
complete binding and all byte destinations before publishing any byte.

## Exact evidence

All paths below are local temporary evidence, outside Git. Reports retain
commands, declared source/spec/helper/tool hashes and actual native results.
Inputs were hashed before the first compilation and verified unchanged after
qualification. Earlier receipts continue to identify their earlier snapshots.

| Final report | SHA-256 |
| --- | --- |
| `C:/Repos/.task-temp/banked-m68k-20261118-d/report.json` | `92e4415c3cc7e5c2d6e21f8a6888f7754b5c926f6036625c21acdfd33808e98d` |
| `C:/Repos/.task-temp/banked-m68k-flat-20261118-a/report.json` | `d14838b3d12fd040933f0307dda8c553d020682b6b5ba3ea3f204f5d73e58d0a` |
| `C:/Repos/.task-temp/pc286-banked-guest-20261118-e/report.json` | `47fe0e51b26d2d17a6d53a625ed93ef9bd4653336055132ccd247328aa7711b3` |

| Qualified input | SHA-256 |
| --- | --- |
| `core/cpu/m68k-bus.c` | `97efb6f036de39c8e69784a9e8a8342adfe50a658130f2ec8ebd7a9541f67edf` |
| `core/cpu/m68k-backend.h` | `ff6ec3ba2db9a128387936cd2ded9a9dc514e0f11f56923f8156718226c9715e` |
| `platform/pc286/banked-guest.c` | `6d1ada88d2410312b93457e648c29a4f1fadfe7ae5e832f5a57db3da1f112d98` |
| `platform/pc286/m68k-banked.c` | `1fbaccc69e0d547f0a0177d99125ab7482f20e5093d8eb62f091de53f2d20e31` |
| `probes/pc286/boot-banked-m68k.c` | `7e3b5d8d13c89cdba50683a477aab2fd5a24f9bb7dac9c9a3ae0e2780ddc781f` |
| `scripts/Test-BankedM68k.py` | `32919f3dfcf28c4c2550a502d97c423248c307afe8c6e29d6d2e1091518c1ef4` |
| `scripts/Test-M68kStartup.py` | `5a7e91cc51b59e9d2326157c71d1aed1c8b44dcf67d54d9170b46161fa1adefc` |
| `SPEC-0120` | `338c15e675c7a6a0b553d43b67ebbc40c7cb6670ef2e49f593e01c83171877ce` |
| BootHush executable | `7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a` |
| Windows Clang executable | `93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4` |
| Python executable | `50f885251e42eb8cd1fbf5d4fbcc01efdbe8b9f4d4379e16664d4d0f3d4a9869` |
| C16 toolchain lock | `eb4b93b16d3343f37273d334bae4af4660af45317308d7a39d948fe2186f2f59` |

## Banked execution results

`probes/pc286/boot-banked-m68k.c` binds 128 KiB over eight scattered banks with
32767/1-byte pairs. The original program fetches a word across a boundary,
jumps from 0x9000 to 0x19020, stores/loads a long across three banks, pushes and
reads a cross-bank stack value, executes LINK/UNLK and returns through that
stack. Native assertions and independent physical RAM inspection check exact
data and return bytes. Logical span tests include the complete 128 KiB range,
last-byte bounds and high access bits without 16-bit narrowing.

Failure cases check COMMIT denial after preflight, final write refusal, LINK's
out-of-range final SP, forbidden A-lines, callback/context aliasing, missing
callbacks, stale generation and an arena gap overlapping native CS/DS.
CPU/RAM snapshots remain unchanged on the required refusals. All three
deliberately broken decoder variants are detected at distinct markers.

| Native286 profile | Payload bytes | Modeled cycles | POST | Outcome |
| --- | ---: | ---: | --- | --- |
| Optimized baseline | 14927 | 3668699 | B8 | pass |
| Unoptimized interpreter only | 20339 | 3873420 | B8 | pass |
| Store before COMMIT guard | 14965 | 3044519 | E1 | defect detected |
| Frame before fallible write | 14937 | 3032546 | E2 | defect detected |
| Missing A-line policy refusal | 14927 | 3551635 | E3 | defect detected |

The unoptimized profile changes the interpreter's optimization only; other
translation units remain size-optimized. Its payload exceeds BootHush CLI's
16 KiB aggregate inspection limit. Two fresh replays compare different
payload ranges and require identical complete reported machine state. Every
replay also checks the complete payload checksum in the original loader before
execution. This is not a simultaneous full-memory snapshot or a cryptographic
loader authenticity check. Other cases need one replay. All loaded payload
bytes match, source media hashes remain unchanged, objects are USE16/80286 and
no default/runtime libraries are linked. Cycles include loader and diagnostic
work; they are neither instruction timing nor physical performance results.

## Flat regression and primitive regression

The bridge harness passes 36 existing hosted sub-suites: startup, six bus and
five guard cases under Release, address/undefined sanitizers and C99. The
independently maintained startup harness is also rerun after the engine edit:
30 host sub-suites, C16 small/compact compile/import audits, four native286
outcomes and five Mac128K outcomes all pass. Its original PEA-value,
UNLK-increment and premature-LINK-write mutants remain detected on both
targets. Compact model has compile/import evidence only.

| Flat native profile | Payload bytes | Modeled cycles | Outcome |
| --- | ---: | ---: | --- |
| Native286 baseline | 10674 | 498902 | B8 |
| Native286 wrong PEA value | 10682 | 270881 | E2 |
| Native286 wrong UNLK increment | 10674 | 381856 | E3 |
| Native286 premature LINK write | 10704 | 428431 | E4 |
| Mac128K O0 baseline | 17440 | 1280440 | B8 |
| Mac128K Os baseline | 10396 | 515836 | B8 |
| Mac128K wrong PEA value | 10400 | 198284 | E2 |
| Mac128K wrong UNLK increment | 10396 | 355484 | E3 |
| Mac128K premature LINK write | 10416 | 413006 | E4 |

The updated primitive's five-case `-e` qualification also passes after adding
numeric validate/span APIs. Optimized/unoptimized payloads are 4911/7534 bytes
and reach B8 in 584089/1032408 modeled cycles. Wrong byte order, missing lifetime
check and premature-write variants reach E1/E2/E3 as expected. This report
supersedes no earlier evidence; it identifies the current provider source.

The coordinator separately built `classick_m68k_startup_tests`,
`classick_bus_tests` and `classick_guard_tests` in
`C:/Repos/.task-temp/mac-keyboard-release-20261117-a`. Its selected CTest run
passed 12/12: six bus, five guard and one startup test. The coordinator then
reran those existing binaries without rebuilding and again passed 12/12 in
0.17 seconds, preserving the exact log as
`C:/Repos/.task-temp/banked-m68k-20261118-d/coordinator-cpu-ctest.log`.
The worker checked that file's twelve named passes and SHA-256
`c4f5e8c37c11c8b563fa4abde6fdc9d0c5e5d6d344722b7da63aad9df3cfdf14`.
This is coordinator-run evidence, not a worker-executed CMake build.

An earlier attempt inspected the shared LastTest.log when it listed the CPU
tests, then copied it after another CTest invocation had overwritten it.
That retained `coordinator-ctest.log`, SHA-256
`d41e0bcf5f66d5455e433b31a265f98fcd653d8fdc9faedacc361210534e955b`,
contains only `storage.mac-boot-header` and is explicitly NOT CPU evidence.
The coordinator detected the mismatched capture; the later immutable log above
corrects it. No source change or rebuild was needed for the repeated CPU tests.

```powershell
& C:/Repos/ClasSICK-0.97/.tools/cmake/cmake-4.4.4-windows-x86_64/bin/cmake.exe --build C:/Repos/.task-temp/mac-keyboard-release-20261117-a --target classick_m68k_startup_tests classick_bus_tests classick_guard_tests
& C:/Repos/ClasSICK-0.97/.tools/cmake/cmake-4.4.4-windows-x86_64/bin/ctest.exe --test-dir C:/Repos/.task-temp/mac-keyboard-release-20261117-a -R '^(bus\.(registers|operands|branches|failures|services|probe)|guard\.(events|atomicity|control|services|aliases)|m68k\.startup-instructions)$' --output-on-failure
```

## Review, retained attempts and reproduction

The CPU author handed over the prior qualified decoder only after its own
exact-input run; the coordinator preserved that snapshot separately. An
independent read-only reviewer inspected the new interface/spec, adapter,
provider additions and engine diff and found no concrete defect under the
documented native ownership/preflight assumptions. Its harness review found
an omitted transitive fixture header (`apps/aline-probe/program.h`), prompting
the final source-identity correction. The running Python binary is now pinned,
and replay result equality is explicitly checked. The handed-over flat startup
harness received the same identity corrections and the new backend header.
Human provenance review and worker/model-specific effort accounting remain
unmeasured; no human or physical validation is implied.

Initial bridge `-a` passed hosted and optimized native checks, then refused the
unoptimized invocation because the CLI inspection request exceeded its limit.
`-b` completed with bounded replays; `-c` added full-range span controls. Final
`-d` includes the independent harness-review corrections. All earlier outputs
are retained and no implementation failure was suppressed. The worker made no
source edits while writing this receipt. The coordinator froze the exact
qualified snapshot before handing mutable CPU/backend files to the next
instruction increment; this receipt does not qualify those later changes.

Reproduce the bridge from the repository with a fresh external output path:

```powershell
& C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe -B scripts/Test-BankedM68k.py --cc .tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain --emulator C:/Repos/.task-temp/boothush-floppy-checksum-20261117-a/a/boothush-active.exe --output C:/Repos/.task-temp/banked-m68k-fresh
```

The other exact invocations are retained in their reports and use
`scripts/Test-M68kStartup.py` with the pinned Windows m68k bin directory, and
`scripts/Test-PC286BankedGuest.py` respectively. Source/spec/tool identities in
the reports remain the authority if future files change. Root owns central
CMake/provenance integration and commits; the worker did not stage or commit.

The next milestones still need explicit service transactions, streamed loading
into guest-owned banks, actual startup entry/initial-memory contracts, broader
instruction coverage and manager/call/session migration. Full System/Finder
execution, persistent writes, physical validation, cumulative stack bounds and
integrated whole-runtime performance remain open. The current provider repeats
bounded geometry checks per operand; it establishes correctness of this route,
not a whole-OS speed result.
