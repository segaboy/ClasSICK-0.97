<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Paired native MFS metadata reader

2026-11-17; SRC-0613/0071/0105/0109/0612; SPEC-0112; IMPL-0142; TEST-0201.
ClasSICK now decodes bounded MFS volume metadata and a caller-selected directory
entry on native 68000 and 286. Header acquisition uses the shared logical-block
interface. Copied raw names/fields survive workspace reuse; failures preserve
the caller's entire output. This is the next storage foundation, not a complete
mount, File Manager or System Disk boot.

## Design and source admission

The lead rechecked eligible published 1985 disk-layout fields and boundaries
in SRC-0071, II-120 through II-123/Figures6-8, and independently restated them
in SPEC-0112. The [source record](../../provenance/records/SRC-0613-mfs-metadata-fields.md)
pins the edition/pages/hash and exclusions. No protected implementation, disk,
manual sample or untracked research draft was used.

`core/storage/mfs-read.[ch]` implements exact-block decoding, guarded geometry,
raw attribute/version/Finder/name preservation, bounded copied names and
block-service error mapping. The allocation-area extent check uses bounded
shift/add arithmetic to avoid wide-division helpers. All refusals precede the
direct publication section; no entry-sized temporary or second copy is needed.
Header acquisition needs a caller-owned 1024-byte workspace. Native objects
remain disjoint, stable and confined to single segments; this is not a general
segmented guest-memory backend.

Empty/high-byte/embedded-zero names and inconsistent fork lengths can be
represented as raw metadata. Their extraction does not establish historical
acceptance or permission to create such entries. Directory/allocation overlap
is admitted without claiming that reserved map entries have been checked.
The reader does not infer traversal rules from inactive entries.

## Verification

The final focused report is
`C:/Repos/.task-temp/mfs-read-20261117-d/report.json`, with full commands,
source/helper/tool hashes, object measurements and per-run artifacts beside it.
It records `inputs_unchanged: true`: all declared inputs are captured before
building and checked again after execution, including the actual inline-loader
helper, native headers/specification, selected C16 members and emulator.

Windows Clang Release (`-O3 -DNDEBUG`) and ASan/UBSan each pass **24,300 check
evaluations**, not 24,300 independent tests. Coverage includes all decoded
fields, odd native input alignment, separate lock bits, raw names and versions,
every file-name length 0-255 at an exact-fit boundary, inactive tails, failed
output preservation and an independent 64-bit geometry oracle over allocation
counts 1-640 and selected unit sizes/boundaries. Routine CTest passes
`storage.mfs-read` and the existing `storage.block-read`, 2/2.

Current BootHush executable SHA-256:
`7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a`,
source commit `5d613614dd4f17d605de75ecc3f38c9129079488`.

| Native case | Expected marker | Modeled cycles | Payload bytes |
| --- | --- | ---: | ---: |
| 286 baseline | B5 | 182854 | 2582 |
| 286 wrongly accepts overflow | C5 | 72667 | 2582 |
| 286 omits entry extent guard | C7 | 108090 | 2576 |
| Mac O0 baseline | B5 | 819330 | 4532 |
| Mac Os baseline | B5 | 356746 | 2960 |
| Mac wrongly accepts overflow | C5 | 118108 | 2960 |
| Mac omits entry extent guard | C7 | 187984 | 2948 |

All seven expected outcomes pass. Defect markers mean the original diagnostic
detected deliberately broken code. These runs use the same original RAM-backed
metadata fixture; actual 286 media integration is recorded separately in the
[floppy-provider receipt](pc286-floppy-provider-20261117.md). Each 286 run checks
the complete loaded payload and unchanged generated media. Mac diagnostics use
the unchanged original reset/linker binding, inspect the 28-byte diagnostic at
0xb00, require the expected marker with fault fields clear, and preserve the
64 KiB padded ROM input.

The optimized Mac ROM SHA-256 is
`4bdc64721c61e9adcb5c1e8d2be6caf9fb0294e0e29fc32be4fddb5072229018`.
The baseline 286 floppy SHA-256 is
`6a060c778425e360f0c8add4664f9bbee17c757ddb61bab203c71d94db5492e9`.
These are original diagnostics, not updated released products.

## Native measurements and review

C16 small/compact object text is 1029/1269 bytes, with no other segment bytes.
Only the project `cs_block_read` is imported, resolved in the target link; no
runtime-library modules are introduced. Compact is a compilation check, while
the actual 286 diagnostic uses the small model. Mac Os header-decode/header-read/
entry-decode bodies are 458/152/312 bytes and their local reported frames are
52/40/28 bytes. The shared diagnostic itself has a reported 1980-byte Os frame.
These local measurements exclude cumulative callers/interrupts and do not
establish a complete 128K/286 OS budget or physical performance.

A bounded Codex implementer consumed only the reviewed contract and original
public project material. The lead reviewed the core and requested direct
publication to avoid redundant stack/copy cost. A separate reviewer found no
core bounds/arithmetic defect, but identified end-only input identity capture
in the verifier. That was corrected and the complete focused suite rerun; the
prior `-c` report remains intermediate evidence. Other retained harness fixes
cover same-module OMF import accounting, a warning-clean negative control and
a malformed-record fixture that now leaves its guard bytes pristine. No core
test failure was hidden. The independent reviewer did not execute tests.

Reproduce with the declared native tools and a fresh external output directory:

```powershell
python -B scripts/Test-MfsRead.py --cc CLANG_EXE --m68k-bin M68K_BIN --tool-root WATCOM_ROOT --emulator BOOTHUSH_EXE --output NEW_DIRECTORY
```

Use exact paths and tool identities from the report. Root's routine CTest logs
are in `C:/Repos/.task-temp/mfs-integration-20261117-root/`. No WSL, other emulator,
protected media, held UART body, private HQ input or game-derived code entered
this implementation. Provider resource usage was not measured. Independent
human provenance review and physical validation remain open.

## Next dependencies

Directory enumeration, allocation-map packing/validation, fork traversal,
Mac floppy binding, File Manager services and the 286 legacy-code/service
bridge remain necessary. The reviewed published pages do not resolve 12-bit
byte packing. A separate documentation lead cites Apple sample code without
claim-specific provenance, so it did not close that source gate; no sample or
emulator implementation was opened or adopted. January1984 behavior remains
unverified. The exact owner-selected image is still unidentified and untested.
See the [full-use milestone](system-disk-boot-20261117.md).
