<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native resource metadata preflight - 2026-11-18

SPEC-0121 / IMPL-0151 / TEST-0212; owner-selected runtime scope SRC-0612/0614.
Both native targets now mount the selected MFS medium and structurally preflight
every nonempty resource fork through their modeled floppy providers. Two cold
runs per target agree on **10 files, 8 nonempty/opened resource forks, 75 type
records and 273 resource reference records**, with aggregate warning mask 7 and
no reported failure. The selected input remains unchanged.

These are structural metadata observations. They do not establish that any
resource works, has been loaded or executed, or supplies a Resource Manager
service. No System/Finder startup, application execution, persistent file write
or physical hardware behavior is claimed. The earlier
[MFS media integration](mfs-media-20261118.md) and
[resource transport qualification](resource-transport-20261118.md) retain their
separate claims.

The [sanitized numeric companion](resource-media-20261118.json) owns the exact
report, source, tool, media and build hashes, diagnostic words and modeled cycle
counts. It excludes commands, private input filenames/paths, transfer images,
memory dumps, selected-media file names, type keys, resource IDs and resource
payload bytes/digests. Public source paths and original-program hashes remain.
Whole-medium hashes identify the admitted runtime input; they are not payload
exports. The original disk and transfer copies remain outside Git.

## Workload and publication

`probes/storage/resource-media.[ch]` composes `cs_mfsv_mount`, the strict directory
iterator, `cs_rfs_mfs_bind` and `cs_rfs_open`. Empty logical resource forks are
skipped, including allocated-empty forks. Binding validates the full allocation
chain. Open checks the admitted resource metadata bounds and ordering, names and
declared payload spans. It performs no resource lookup or payload extraction.
Its permitted metadata cache read-ahead can include adjacent in-file bytes;
those bytes are not interpreted as code or exported by the diagnostic.

The sixteen-word report counts only completed opens in the type/reference totals
and warning OR. Files/nonempty forks seen include a failing entry. The first
failure reports its one-based file ordinal, domain, status, field and position.
The view destination is filled with a canary before open and checked unchanged
on refusal. Aggregate overflow and changed failed-open output have distinctive
outcomes. The probe stops on the first failure.

Both wrappers publish a fixed terminal marker `B8` after the probe returns,
including a classified refusal. Report word 14 distinguishes successful `B8`
from refusal `D1`..`D4`, arithmetic failure `DE` and changed-output failure `DF`.
The harness checks that report, not just the terminal marker. A valid optional
runtime trial that encounters a format refusal records `observed-refusal` and
`resource_preflight_complete: false`; it is not called a resource success.

The 286 wrapper uses the existing native modeled provider with 800 logical
sectors starting at PC floppy sector 60, after the original diagnostic loader
and program. The Mac wrapper uses the existing native IWM provider with logical
raw sectors validated from the DC42 container. This transfer arrangement is an
explicit diagnostic bridge, not the final native product disk/boot design.

## Original fixture and controls

The separate original generator `scripts/resource_media_fixture.py` declares
four files: two fragmented nonempty resource forks, an unallocated empty fork
and an allocated-empty fork. The larger fork is 66,438 bytes; its map starts at
offset 66,042 and crosses a 256-byte cache boundary. The final allocation maps
this high logical position to low physical sectors 26/27. There are 68 used and
324 free 1,024-byte allocation units. Raw names, signed IDs, a reference resource,
an empty type list, large payload metadata and all seven warning bits occur in
the original fixture. No protected bytes informed its construction.

Its declarative oracle expects 4 files, 2 completed forks, 6 type records,
8 references and warning OR 127. Logical-medium SHA-256:
`26b7638fc7d5a11d050c767210cef02fbddd938f8674063c3680f7286092e16c`.
The final six-case run used a 1,200,000,000-cycle/30-second bound per case:

| Target | Case | Probe outcome | Modeled cycles | Result |
|---|---|---|---:|---|
| Native 286 | Baseline | `B8` | 4,445,765 | Complete original oracle |
| Native 286 | Late metadata fault | `D4` | 2,179,792 | Exact IO refusal |
| Native 286 | Fresh recovery | `B8` | 4,445,765 | Full result matches baseline |
| Native Mac128K | Baseline | `B8` | 63,011,404 | Same complete oracle |
| Native Mac128K | Late metadata fault | `D4` | 27,274,338 | Same exact IO refusal |
| Native Mac128K | Fresh recovery | `B8` | 63,011,404 | Full result matches baseline |

The injected sector is logical LBA 27: PC transfer CHS `2:1:13`, Mac track 2,
sector 3. The PC provider reports one injected firmware read failure. The Mac
provider reports the requested persistent synthetic GCR checksum alteration.
The first map cache page succeeds and the second fails. Both targets identify
file 2, resource transport status `IO` (12), field `MAP` (3), position 66,048.
Only the first completed fork contributes aggregates: 1 opened fork, 2 types,
3 references, warning OR 30. Failed-open view publication remains unchanged.
Fresh processes with injection removed reproduce complete baseline results.

Independent fixture serialization/round-trip checks and a separate strict host
composed-probe check also passed baseline, partial-provider failure and recovery
with 29/13/29 sector requests. Independent review found no concrete defect in the
shared probe or wrappers. The native report remains the execution evidence for
the two target/provider combinations; host checks do not replace it.

For the implementer's separate route checks, `--disk` used only generated
original DC42 containers. Four valid cold-run outcomes matched the original
counts while using no original count oracle in that route. Four deliberate
format-refusal outcomes changed only the second fork's type-list offset to zero:
both targets reported file 2, `OVERLAP` (6), `TYPE_LIST` (4), position 66,066,
preserving the first fork's aggregates. These trials prove that a classified
refusal is retained rather than relabeled success. They do not independently
requalify every DC42-container validation rule.

## Selected runtime observation

The coordinator ran the already admitted selected input. This implementer and
receipt author received only the authorized numeric report, not the owner disk,
transfer files, runtime stdout memory captures, resource bytes or assets. The
selected container SHA-256 is
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`;
its validated logical-sector SHA-256 is
`af412a6e3ef312ec51b6d53e99ca65e3246ab047650df835ce4f29f58f9f65c1`.

The selected run used a 4,000,000,000-cycle/60-second bound per cold start:

| Target | Cold runs | Cycles in each | Files | Opened forks | Type records | References | Warning OR |
|---|---:|---:|---:|---:|---:|---:|---:|
| Native 286 | 2 | 122,735,524 | 10 | 8 | 75 | 273 | 7 |
| Native Mac128K | 2 | 1,462,543,478 | 10 | 8 | 75 | 273 | 7 |

All sixteen report words match across targets. Within each target the full
returned emulator result and aggregate report match across cold runs. The
header file count equals the enumerated count, all eight nonempty forks opened,
and failure fields are zero. Original/container/transfer hashes are unchanged.
There is no selected-media fixed-sector injection or original-fixture count
oracle. Both targets use the same shared format profile; agreement is integrated
native execution evidence, not independent historical implementation proof.

Warning mask 7 is exactly the OR of SPEC-0024's `LAYOUT` (1), `MAP_RESERVED` (2)
and `HANDLE` (4) flags. These respectively identify one of that profile's layout
deviations, a nonzero byte among the map's first 22 bytes, and a nonzero recorded
handle field on a local resource reference. The aggregate cannot attribute flags
to particular files or resources and does not mean every fork had all three.
It establishes no handle validity, resource behavior or functionality. The
counts are type/reference **records**, not unique keys or working resources;
this workload does not call duplicate-key lookup.

## Identities, memory budget and retained limitation

All four final report groups use identical declared source, tool and build
identities: 34 source/spec/helper files and 18 tool/header/license entries are
hashed before and after. The pinned Watcom selection and Windows-native m68k
target identity are checked. Native links use no default/runtime library and
the Mac ELF has no unresolved imports. This is the declared identity set in the
companion, not an additional claim about unrecorded transitive tool components.

Pinned BootHush SHA-256:
`7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a`.
Harness SHA-256:
`7d6ccf6e1e1dc070f6659dd57118ea79ccbfd3ef8a08c80b6b3f548a0a571e40`.
Native 286 payload SHA-256:
`69120e1bd86441860384bdf7a64bb9a0d7b3211ed8d596b76651635b4c4c0830`.
Padded Mac ROM SHA-256:
`d029aa40073d1db0e26427f298ce0acd13280da23ac5b3322bde4b0d75528fec`.

The 286 program is 9,888 bytes with 4,506 BSS bytes; linked storage ends at
segment offset 14,394 below the initial stack top 65,534. The diagnostic loader
poisons/clears BSS and checks the loaded program checksum. Every loaded original
program byte is also compared with the linked payload through bounded RAM
inspection. Disk buffers are outside those inspection ranges.

The Mac payload is 9,788 bytes. Existing linker/reset files are unchanged:
data/BSS occupies `0x0B00..0x1FA8` (5,288 bytes), leaving 88 bytes below `0x2000`.
The reserved stack span remains `0x2000..0x4000`, below framebuffer `0x1A700`.
Compiler local frame reports include 152 bytes for the shared probe and 224 for
resource open. The companion includes all local frames. These are not cumulative
stack proof or whole-system RAM qualification. Repeated full-chain validation
and modeled drive access remain real costs; target cycle counts are neither
physical timing nor a cross-architecture speed comparison.

The original `resource-media-20261118-a` attempt requested inspection of the
loaded Mac ROM. The pinned emulator refused before guest execution: status
`error`, cycles 0, PC 0, "requested memory is outside physical RAM and video
memory". That failed attempt is retained, not a Mac guest result. The final
harness inspects only Mac diagnostics in RAM and pins the ROM artifact before
and after execution. It does not claim loaded Mac ROM byte comparison. No
emulator switch, limit bypass or production reader/provider change was made.

## Evidence and reproduction

The following external reports remain available; their hashes are in the numeric
companion. The first is the canonical original paired execution report:

| Report directory under `C:/Repos/.task-temp/` | Native outcomes | Result |
|---|---:|---|
| `resource-media-20261118-b` | 6 | Original baseline/fault/recovery verified |
| `resource-media-original-valid-route-20261118-a` | 4 | Generated DC42 valid route verified |
| `resource-media-original-refusal-route-20261118-a` | 4 | Generated DC42 refusal route verified |
| `resource-media-selected-20261118-a` | 4 | Authorized selected metadata preflight completed |

Each directory contains `report.json`. All 18 planned outcomes are verified;
six are deliberate refusals, not successful preflights. The original report
SHA-256 is `1a5a775da3548cd1cb2102451c8b9e975ba604e15a59e3d974e329f0483a210f`.
The selected numeric report SHA-256 is
`5494912dd6a9bed40b17d3831db04cadf5728459fbdc72529bcfadfbd29026a5`.

From the repository root, reproduce original-media verification with explicit
admitted Windows tool paths and a fresh external directory:

```powershell
& $py -B scripts/Test-ResourceMedia.py --tool-root $watcomRoot --m68k-bin $m68kBin --emulator $bootHush --output $newExternalOutput
```

`--lane pc286|mac128k|both` selects the targets. `--disk` is reserved for an
explicitly authorized runtime DC42 input outside the checkout; it does not
authorize obtaining, inspecting as implementation source or redistributing
protected content. The script preserves the admitted four-billion-cycle limit.

Codex authored the probe/harness; a separate worker authored the original
fixture and reviewed the probe; the coordinator alone ran the owner-selected
medium. Exact model/effort identifiers, elapsed worker effort and provider usage
were not recorded in these reports. No new external research, private HQ,
excluded research originals, held UART bodies, WSL, UI session or physical
trial was used. Human provenance review and startup, application, persistence
and hardware gates remain open.
