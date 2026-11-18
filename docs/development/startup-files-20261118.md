<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Startup file resource-source preparation, 2026-11-18

SPEC-0122 / IMPL-0152 / TEST-0213. The new freestanding coordinator acquires
boot-header metadata, mounts the same read-only MFS device, selects the first
exact counted System and Shell names, and validates both resource maps before
publishing a usable two-source session. Original host and native Mac68000/286
qualification passed. This is a callable startup prerequisite; the product
entrypoints are unchanged and System/Finder execution is not claimed.

## Contract and source boundary

`core/storage/startup-files.[ch]` composes SPEC-0119, SPEC-0115 and SPEC-0116.
It adds an original engineering publication/lifetime contract, not historical
ROM dispatch behavior. No heap values, version bits, entry field, INIT records
or executable payload are interpreted. Names are retained only as lookup controls;
diagnostics contain no names, resource keys/IDs, payload bytes or payload digests.

The destination owns header, mounted-volume, directory-entry, adapter and resource
view metadata. It borrows the caller workspace's disk scratch and the immutable
provider/media. It stays at its original address. Native objects are live,
truthful, disjoint, individually confined to native segments, and used exclusively.
The sole destination copy occurs after both maps open. Adapter volume/entry and
view source-context pointers are then rebased without any fallible operation.
Failure leaves the entire prior destination unchanged; argument refusal also
preserves workspace/report. The prior session remains usable after a later
failed preparation with the same workspace and unchanged medium.

The 1985 published Inside Macintosh Volume I, I-114, InitResources, provides
background that startup opens the System resource file and reads its resource
map. Review used indexed text of the published edition at
https://vintageapple.org/inside_o/pdf/Inside_Macintosh_Volume_I_1985.pdf;
the edition describes ROM105/System file dated May2,1984, not January1984 proof.
This contextual fact supplies no INIT execution order or native startup ABI and
is not used to introduce new executable behavior. Common header field provenance
remains SRC-0617's later published metadata with its explicit limitations.
No owner disk, ROM/executable bytes, confidential draft, third-party emulator
implementation or private HQ content was inspected by the implementation worker.

## Original verification

`scripts/startup_files_fixture.py` adds independently chosen boot-header names
and numeric fields to the original SPEC-0121 generator. Both names contain binary
bytes; the second fragmented resource fork has66438 logical bytes and a map
starting at66042. No reference media is an input to either generator.

`tests/startup-files.c` passes2634 checks in each optimized and ASan/UBSan build.
Checks include both published sources and original payload extraction after
construction metadata destruction; failure after every block callback, each
with partial write and IO/unavailable/unknown return; exact binary/equal names
and first-match selection; missing/case-mismatched names; malformed seventh
header name, mount, second fork chain and resource map; empty resource fork;
argument preservation; prior-session survival and recovery. Three deliberately
broken coordinators are rejected by the unchanged checks: missing source-context
rebasing, early publication and selecting System twice instead of Shell.

The shared native probe destroys the temporary candidate after successful return
and reopens both published resource sources. A late failure at logicalLBA27
checks every destination byte and both guards. The native probe reads only
resource metadata. Its report words are:

| Word | Meaning |
|---|---|
| 0 | Magic53544631 |
| 1 | Coordinator result |
| 2–3 | Stage and file role;2 means no file |
| 4–7 | Domain, status, resource field and absolute position |
| 8–11 | Completed opens, types, references and warning OR |
| 12 | Published sources successfully reopened |
| 13 | Complete destination preserved on refusal |
| 14 | Probe outcome:B8 complete,D1 classified refusal,DF invariant failure |
| 15 | Both destination guards retained |

The canonical final runs use identical source/tool hashes:

| External report directory | Scope | Observed result |
|---|---|---|
| startup-files-20261118-b | Original baseline, late second-map failure, recovery on both targets | Six expected outcomes |
| startup-files-20261118-c | Explicit `--disk` using generated valid original DC42; two cold runs per target | Four completions |
| startup-files-20261118-d | Explicit `--disk` using original invalid boot signature inside a valid DC42; two cold runs per target | Four classified refusals |
| startup-files-controls-20261118-a | Full CLI with truncated/checksum-invalid original containers | Two refusals before output directory creation |

Each native completion reports two maps,6 types,8 references, warning OR127 and
two successfully reopened published sources. Late IO refusal reports stage5,
Shell role1, resource domain3/status12, field3/position66048 and only the completed
first map's2 types,3 references and warning OR30. Invalid signature reports
header stage1/boot domain1/status3, zero completed opens and complete destination
preservation. Runtime-route classification is kept separate from harness validity:
the final invalid-header run is `observed-refusal`, never a startup success.

Seven direct container controls also pass: a valid wrapper round-trip plus
truncated, trailing, overlong-name, encoding, format and checksum refusals.
All explicit runtime inputs in this worker's runs are independently generated.
`private_runtime_input=true` records the explicit route, not owner-media origin.

| Target | Baseline/recovery cycles | Late-fault cycles | Invalid-header cycles |
|---|---:|---:|---:|
| Native286 | 7470249 | 2628038 | 556286 |
| Native68000 Mac | 100616420 | 33852464 | 3898358 |

Cold repeat emulator state dictionaries and reports match exactly. Native reports
match across CPUs. The harness pins input/source/helper/tool hashes before use,
then compares them at completion. It inspects every loaded286 payload byte and
retains exact provider-specific injected-failure evidence. Mac ROM artifact hash
is pinned; the admitted emulator does not inspect loaded Mac ROM addresses.

## Memory and identity

The286 program is10860 bytes with10082 BSS bytes; linked storage ends at20942
within its64KiB near segment, with stack top65534. No runtime library is linked.
The Mac program is10628 bytes and data/BSS occupies10900 bytes at0x0B00..0x3594.
This isolated probe generates reset/linker copies reserving0x0B00..0x4000 for
data/BSS and0x4000..0x6000 for stack. Link and verification enforce those bounds;
the framebuffer begins0x1A700. Existing product reset/linker files are unchanged.
Compiler local frames include116 bytes for the coordinator,160 for the probe,
348 for MFS find and224 for resource open. These are local measurements, not a
cumulative stack proof or physical performance claim.

Current coordinator SHA256:
`468c243378e2e9c59fe98d9888083c3a13073c520c6706c863bdf46f36fbf1af`.
Current harness SHA256:
`1aa35a4d9925466190bc69f3ec9b4dab044ede9f4be0c86935a5a7be28e63a39`.
The [numeric companion](startup-files-20261118.json) owns complete source/tool,
build/report/input identities, report arrays, counts and modeled cycle values.
Full generated outputs/logs remain under `C:/Repos/.task-temp/` in the named
directories. Initial `startup-files-20261118-a` also passed; it predates only the
source-hashing-order tightening and removal of an unnecessary core comment.
It is retained separately and is not substituted for the final b/c/d identities.

AI-assisted implementation: Codex storage_review worker under coordinator scope.
The mfs_volume/mfs_tests worker independently reviewed the final core/probe
contracts read-only and found no concrete defect under the declared native-object
and lifetime preconditions. No extra tests were represented as that reviewer's
work. Provider usage accounting and total active/wait time were not available.
All authored code/docs are original GPL-3.0-or-later, copyright2026 Dean Howell.

No full Resource Manager, executable loader, INIT dispatch, launch ABI, native
product integration, persistent write, physical hardware or complete System boot
claim is made by this receipt.

## Coordinator-selected numeric observation

The coordinating agent separately ran the authorized selected input twice per
native target in `startup-files-selected-20261118-a`. This worker read only its
numeric `report.json`, SHA256
`39abc240af4b375dd68d7d4d83c4ba0d73c2b450e638b8d1defdd93348dd8637`,
and did not read the disk, transfer copies or stdout captures. All four reports
complete with two resource sources ready,39 type records,144 references, warning
OR1, two successful post-return reopens and intact guards. Each286 run takes
180779640 modeled cycles; each Mac run takes1644747872 modeled cycles. The repeated
reads are a measured cost of the current unoptimized transport, not a speed claim.

Source hashes exactly match the final original b/c/d runs. The selected run uses
the coordinator's separately pinned Python executable; all other tool hashes
match, and all input/source/tool hashes are unchanged within the run. The numeric
companion retains that tool identity distinctly. Selected container SHA256 is
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`;
logical-medium SHA256 is
`af412a6e3ef312ec51b6d53e99ca65e3246ab047650df835ce4f29f58f9f65c1`.
This confirms selection and structural resource-source preparation for that
exact input. It neither exposes the names/content nor establishes executable
loading, INIT behavior, launch readiness or System boot.
