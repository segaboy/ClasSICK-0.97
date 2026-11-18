<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Opt-in banked GetTrapAddress service - 2026-11-18

Canonical receipt for SPEC-0124 / IMPL-0154 / TEST-0215. This adds one real
native286 service path to the bank-backed interpreter, reusing the admitted
SPEC-0020 trap-address interface and SPEC-0025 collapsed completion rule.
Its six-byte transaction uses the separately developed SPEC-0123 atomic
transfer backend. Only original fixtures and approved own source/specifications
were used. No protected disk/code, external implementation, HQ, excluded research,
held UART bodies, WSL or alternate emulator was inspected or used.

## Behavior and ownership

`personality/system1/banked-services.[ch]` provides `cs_s1_banked_step`, taking
the transfer backend, a current native directory, frame/report, stack limits
and immutable guard. It is backend-neutral rather than a cast from a guest
address to a native service pointer. Only A146 and A046 enter GetTrapAddress.
All other A-lines remain refused, and the existing bus/transfer entry points
continue to refuse every A-line. No shared CPU/provider source was edited by
this service worker.

The wrapper performs a guarded opcode peek to select the explicit service.
Ordinary instructions delegate to the existing engine, with a second guarded
fetch. The routing OPCODE event has before==proposed==input frame; subsequent
execution events use a distinct staged frame. Pure immutable policies can
therefore refuse the first or second fetch without mutable counters. The
contract requires stable ordinary RAM, pure synchronous guards, exclusive
control/backing ownership and no reentry, DMA, mutation or retained pointers.
This extra fetch is not an MMIO-capable route.

Before report publication, the wrapper checks full disjoint native spans and
typed alignment for descriptor, gateway, frame, report, limits, guard and both
retained contexts. The concrete 286 backend excludes entire CS and DS/SS
envelopes from its guest arena; this protects code, sources and native controls.
Native object validity and complete truthful extents remain caller obligations.

The service resolves current directory slot 0x46 and requires its target to
equal get_token. It calls only the read-only `cs_trap_decode`/`cs_trap_get`
operations; it never invokes generic trap dispatch after that check. Remapping
slot 0x46 to Set, unsupported or guest identities cannot select a different
intrinsic. D0's low word selects the queried directory entry. A146 returns its
logical token in A0; A046 preserves A0. Other registers and A7 remain unchanged;
the existing mediated CCR rule applies and proposed PC advances by two.

The entire [A7-6,A7) scratch must be in the admitted stack, at or above 0x400,
and writable. After validation and COMMIT permission, one `write_bytes` call
publishes completed SR/PC in six big-endian bytes. CPU-frame publication follows
only success. A last-byte refusal preserves all six bytes, frame and directory;
two separately fallible scalar writes are explicitly insufficient. Reports may
change on ordinary refusal; unsafe native-envelope refusal preserves them too.
No active manager or heap mutation is admitted by this profile.

## Exact qualification

Final successful report:
`C:/Repos/.task-temp/banked-services-20261118-c/report.json`.
SHA-256: `3958c19be7c43a0900f8c460329152afb7fb0d23e627ac52998f0b63343613f1`.
The report retains exact commands, selected source/spec/helper/tool hashes,
object identities, native observations, media identities and replay results.
All declared inputs were hashed before compilation and checked unchanged after
every test. Generated images, mutants, objects and logs remain outside Git.

| Qualified input | SHA-256 |
| --- | --- |
| `personality/system1/banked-services.c` | `98bae2c94d090baa73d8ab86bd61c424504071bef65b943be23f06d52f9e7f2e` |
| `personality/system1/banked-services.h` | `74166906ffeb38c9a64db461c3fb8c15e8d616b8e0ad148724e10699a98dcc3b` |
| `tests/banked-services.c` | `99e1959b1cafa586bf292eb90f89f7733659ea942114a6b60bc3ef0b83e1115b` |
| `probes/pc286/boot-banked-services.c` | `5b4d3c76d8157cb017de13d18c882fab3479a491e5c4a13de765dac0d69b596d` |
| `scripts/Test-BankedServices.py` | `ed901204dc211f610a1541c9d6519f6ae50e5706549f79bb3ed1781792fdf9f7` |
| `SPEC-0124` | `22d003137d48f2809626fd9e15dafca83038ad74c9aad45a2f5750b3e4752a23` |
| Shared CPU dependency | `6ce3eddbaf9ea5cbf5133db4538a00f01373d3ada1326d187e831bd65efbb016` |
| Native bank provider dependency | `57c7a22c753a37fac4fad5788a02cc67bc8144341ea318cfa452ce98b802a801` |
| Native transfer adapter dependency | `922f7c240354dd282f89c7c32fa9fcae7f48605865343e7f5382c0abbd797263` |
| Pinned BootHush executable | `7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a` |
| Windows Clang executable | `93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4` |
| Python executable | `50f885251e42eb8cd1fbf5d4fbcc01efdbe8b9f4d4379e16664d4d0f3d4a9869` |
| C16 toolchain lock | `eb4b93b16d3343f37273d334bae4af4660af45317308d7a39d948fe2186f2f59` |

The complete report also identifies all compiled shared sources/headers,
SPEC-0020/0025/0117/0120/0123 and SRC-0618. The latter two describe the shared
transfer/CPU dependency, not new historical facts for GetTrapAddress. Native
objects are USE16/80286, linked without default/runtime libraries. No firmware
or third-party implementation was imported into the new service.

## Outcomes

Each Release, address/undefined-sanitized and C99 hosted build passes **619707
assertions**. The independent host oracle covers both accepted words, every
directory index 0..511 and all 32 CCR inputs, D0 high-word preservation, every
invalid requested low-word index 512..65535, remapped signatures, policy and
last-byte callback refusals, ordinary instruction delegation, old A-line
policy, low-memory/stack/continuation/mode/lifetime failures, native aliases
and an explicitly misaligned descriptor under sanitizers. These are exhaustive
checks of those bounded domains, not all possible trap words or OS behavior.

The actual native286 probe binds 128 KiB over eight scattered banks and runs an
original program containing MOVEQ, GetTrapAddress, MOVE/MOVEA, A046 and NOP
through the wrapper. It verifies returned token 0x401234, A0 preservation, D1/D2,
PC/A7, the unchanged native directory, exact six scratch bytes across three
physical banks and neighboring canaries. It separately checks final-write
failure, COMMIT denial, remapped slot refusal, both pure fetch-denial policies,
old transfer-entry A-line refusal, context aliasing, invalid continuation and
an invalid query. The final successful scratch is independently inspected at
physical 0x38FFC, 0x41000 and 0x42000.

| Native286 case | Payload bytes | Modeled cycles | POST | Outcome |
| --- | ---: | ---: | --- | --- |
| Optimized baseline | 18153 | 4075767 | B8 | pass |
| Unoptimized service wrapper | 19447 | 4194315 | B8 | pass |
| Frame published before failed write | 18162 | 2317725 | E1 | defect detected |
| Six-byte completion split into scalar writes | 18190 | 2555217 | E2 | defect detected |
| Current directory signature check omitted | 18138 | 2879307 | E3 | defect detected |

Only the wrapper is unoptimized in the second case; other units remain
size-optimized. Each payload exceeds the CLI's 16 KiB aggregate inspection
budget, so two fresh replays compare all its bytes and require identical
complete reported machine state. Each replay also verifies the full payload's
checksum in the original loader before execution. This is not one simultaneous
full-memory snapshot or a cryptographic authenticity proof. Every native case
has CS=0x1000, its distinct required marker and unchanged source-image hash.
Loader failure markers are distinct from mutation markers. Cycle counts include
boot, provider setup and diagnostics; physical timing, service-only timing,
whole-runtime performance and cumulative stack bounds were not measured.

The coordinator additionally built the hosted CPU/service targets and passed
14/14 routine CTest cases: six bus, five guard, startup instructions, register
transfers and banked services. Its immutable log is
`C:/Repos/.task-temp/startup-write-integration-20261118-a/cpu-ctest.log`, SHA-256
`f5f82919457d928b9444594ca181129530e7c8f1f2c09adb99b0135516250517`.
The worker read that exact saved file, verified the hash and all fourteen named
passes. This is coordinator-run CMake/CTest evidence for the same shared CPU
hash above, separate from the worker's exact-input harness.

## Review and retained earlier runs

An independent read-only reviewer inspected the interface/spec, full wrapper
and relevant tests, including both pure fetch-denial cases, directory remapping
and final-write preservation. No concrete defect was found under the stated
ownership/purity/envelope assumptions. A second independent reviewer inspected
the native probe and harness source identities, marker separation, whole-payload
comparison, physical scratch checks and replay matching; no concrete false-pass
or identity gap was found in that scope. Neither reviewer reran the suite.
Human provenance review and worker/model-specific effort measurements remain
separate and unmeasured.

Initial `banked-services-20261118-a` passed; `-b` added complete SPEC-0123/source
dependency identities. Afterward, the CPU owner independently found that the
new generic transfer entry could form `&backend->scalar` before rejecting a
misaligned wrapper. That owner fixed the shared entry and added its regression.
This service's own envelope already checked wrapper alignment before accessing
members. The service suite added an explicit unaligned-input preservation case
and reran all profiles/native cases as final `-c` against the corrected shared
source. Earlier reports remain retained as pre-core-fix evidence; their passing
typed calls do not qualify the corrected snapshot. No service implementation
defect was hidden or suppressed.

## Reproduction and open work

From the repository, choose a fresh output directory:

```powershell
& C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe -B scripts/Test-BankedServices.py --cc .tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain --emulator C:/Repos/.task-temp/boothush-floppy-checksum-20261117-a/a/boothush-active.exe --output C:/Repos/.task-temp/banked-services-fresh
```

Root owns central CMake integration and commits. The host test needs
`tests/banked-services.c`, `personality/system1/banked-services.c` and the
existing `classick_memory` target (which publicly supplies CPU/trap services).
The test executable takes no arguments. No shared CPU/provider file was changed
by this increment; the CPU owner supplied the separately qualified transfer API.

This establishes an explicit read-only trap query with atomic scratch/frame
completion. It does not identify the first service required by the selected
System Disk. SetTrapAddress still needs a prepared directory delta; bank-aware
memory traps need complete copy/fill/master-cell/native-metadata transactions,
heap/scratch exclusion and overlap/compaction/CLEAR rules. The current directory
is private native state, not a historical low-memory table, and its inherited
RAM patch range is bounded to 64 KiB. Guest patches, Toolbox/auto-pop, general
exceptions, active heaps, selected-program execution, initial ROM/boot handoff,
persistent filesystem use, Mac-native service integration and physical tests
remain separate. No System/Finder startup or desktop is credited here.
