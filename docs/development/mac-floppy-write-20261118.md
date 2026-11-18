<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Modeled Macintosh sector writes - 2026-11-18

SPEC-0125 / SRC-0619 / IMPL-0155 / TEST-0216. The original Mac writer now
updates explicitly writable private copies through BootHush's modeled IWM.
Original raw and taggedDC42 fixtures pass same-guest readback, full exported
image comparison and a fresh read-only boot from the saved image. No selected
owner image is written. File creation, filesystem metadata updates, System/Finder
startup, physical floppy operation and whole-filesystem transactions remain open.

The [numeric companion](mac-floppy-write-20261118.json) preserves exact tested
source/tool/ROM identities and numeric outcomes. Canonical external report:
`C:/Repos/.task-temp/mac-floppy-write-20261118-c/report.json`, SHA-256
`7991e0fca21f063c3179994393487de17b650c208d6c90f20fbe6c5c1fbbd996`.
Pinned BootHush executable SHA-256:
`3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4`.
Its own `docs/floppy-writes-20261118.md` owns the model/export qualification.

## Implementation and failure boundary

`iwm-write.c` independently encodes the existing GCR/checksum profile.
`iwm-send.S` preserves the native ABI, polls readiness with a finite bound,
submits exactly one709-byte record, waits for acknowledgement and closes the
write gate on every exit. Existing validated reads locate the sector and
preserve its12tag bytes. The original reader is unchanged.

The API borrows an exclusive bound reader plus separate512-byte input and
scratch. Interrupts remain masked. Argument/range refusals precede controller
activity; an initial read must succeed before transmission. It distinguishes
read failure, timeout and rejected write. A post-transmission failure may have
an uncertain commit; the caller must re-read. There is no multi-sector rollback.
The model separately requires `--floppy-out NEW_PATH`, refuses existing outputs,
keeps its source immutable and suppresses export after an invalid record.

## Executed checks

Host Release and ASan/UBSan each pass13,710 assertions, including all256 pattern
seeds at12sector positions, guard bytes, invalid arguments and an independent
final-group vector. These assertions are not13,710 independent scenarios.
Eight native runs use bounded800-million-cycle/30-second budgets:

| Original medium | Write/read | Fresh saved-image read | Protected refusal | Bad-checksum refusal |
| --- | ---: | ---: | ---: | ---: |
| Raw | 48,654,314 cycles | 22,410,696 | 46,331,442 | 4,132,678 |
| TaggedDC42 | 48,698,786 cycles | 22,455,162 | 47,505,068 | 4,201,890 |

Each healthy write run changes11sectors across all five zones, all four zone
boundaries, the final sector and a backward seek. It reads every replacement
immediately, retains input/guard bytes and validates original tag values.
The complete export equals a separately constructed expected image: all other
sectors, tags and DC42 metadata remain unchanged except the required data
checksum. A fresh guest reads all11replacements from the export without enabling
writes. The original input and every ROM remain unchanged.

Protected-mode runs refuse all11write attempts and re-read the original bytes.
Each bad-checksum run acknowledges709accepted bytes, rejects the record, commits
zero sectors, reads the unchanged original sector, and produces no output file.
CLI exit2 and its explicit export-refusal status are expected in those two
cases; they are not successful saves. Normal cases reach the originalB8 marker
without guest faults. Native payloads are3,140..3,208bytes, link without runtime
imports and use the existing reset/linker RAM layout. ROM artifacts are hashed;
the CLI does not compare loaded Mac ROM bytes through its RAM-only inspector.

Routine CMake integration also passes `storage.mac-floppy-write`. The paired
startup-source test passes alongside it; those are distinct component tests.

## Retained defect and review

The initial `-a` native run rejected the first record after709accepted bytes.
The encoder omitted the implicit third byte's high bits in the final two-byte
group. The existing reader ignores those redundant bits, so encoder/reader
roundtrip checks and the first read-only code review did not detect it.
The writable model correctly rejected the noncanonical record. Root reviewed
the original model contract/code, fixed the encoder and added an independently
derived vector: final data80/80 requires group symbolD9 and checksum symbolDF.

The independent reviewer checked that correction and its arithmetic, plus the
assembly's20-byte register save/24-byte argument offset and final acknowledgement.
No further defect was found under the documented exclusive/disjoint assumptions.
The review is source review, not a human provenance or physical verification.
The `-b` run passed six native outcomes; final `-c` adds the two executed checksum
refusal cases. An earlier mistyped executable identity was refused before tests.
All attempts remain outside Git; no failure is relabeled as a passing save.

Reproduce with explicit Windows tool paths and a fresh external directory:

```powershell
& $py -B scripts/Test-MacFloppyWrite.py --cc $clang --m68k-bin $m68kBin --emulator $bootHush --emulator-sha256 3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4 --output $newOutput
```

No third-party implementation, owner disk, visible UI, WSL, new reference-media
admission or physical machine was used. Provider/model effort accounting and
whole-OS performance remain unmeasured. These modeled cycle counts are not
physical timings or an unrestricted hardware-compatibility result.
