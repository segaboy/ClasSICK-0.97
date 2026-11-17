<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Original read-only block foundation

2026-11-17; SRC-0612 / SPEC-0111 / IMPL-0141 / TEST-0200. The owner requires
end-to-end use of the selected boot disk, including its 68k System/Finder code,
on the Macintosh replacement ROM and standalone 286 target. This slice supplies
an original storage primitive toward that goal. It neither inspects nor executes
protected disk content and does not establish a disk or System boot.

## Contract and implementation

[`core/storage/block-read.c`](../../core/storage/block-read.c) reads 512-byte
logical blocks through a synchronous native provider. It preflights LBA/count,
capacities and exact buffer equality, stages every block, and publishes output
only after all reads succeed. A failed provider can change staging and its own
state; all caller output bytes remain unchanged. Provider unavailability, I/O
failure and unknown status have distinct results. There is no allocation,
implicit retry, filesystem interpretation or hardware operation in the service.

The initial request cap is 63 blocks, or 32256 bytes. Each admitted native span
is at most 32767 bytes and stays inside one native object/segment. Truthful live
extents, stable disjoint controls/backing/buffers, and a conforming provider are
preconditions. The service does not discover arbitrary overlap, normalize far
pointers, protect against a provider writing beyond its block, or implement the
segmented guest-memory backend. One-block reads need 512 output plus 512 staging
bytes; 63 blocks need 64512 combined bytes in separate valid native objects.
There is no claim that both fit a single near segment or a complete Mac OS.

The algorithm is an original two-phase staged copy with subtraction-based range
checks. It never derives a native pointer or byte offset from an unbounded LBA.
The specification fixes refusal order and partial-effect limits. No historical
MFS packing, boot-block fields, firmware algorithm, third-party source or assets
were used.

New file scope: `core/storage/block-read.[ch]`, `tests/block-read.c`,
`probes/storage/block-read.[ch]`, `probes/pc286/boot-block-read.c`,
`probes/mac128k/block-read.c`, `scripts/Test-BlockRead.py`,
`scripts/Test-MacBlockRead.py`, SPEC-0111, IMPL-0141, TEST-0200 and this receipt.
The Macintosh diagnostic reuses unchanged project-owned `mac-reset.S` and
`mac-rom.ld`; it does not replace a released ROM or the product entry point.
The lead adds an isolated `storage.block-read` CTest target to CMake, preserving
preexisting unrelated CMake working-tree edits.

## Executed checks

The final build/report is
`C:/Repos/.task-temp/block-read-20261117-c/report.json`. The focused runner is
[`scripts/Test-BlockRead.py`](../../scripts/Test-BlockRead.py). Its original
host generator covers every request count 1..63, success
at high LBAs through UINT32_MAX-1, every failed-block position 1..63, a partial
23-byte failing write, three provider results including unknown status, exact
alias refusal, capacity-by-one and request arithmetic bounds, call ordering,
zero callbacks on preflight refusal, unused output/staging tails and canaries.

| Check | Actual result |
| --- | --- |
| Windows Clang 23.1.1, Release `-O3 -DNDEBUG` and strict warnings | Pass; 1032994 assertion/check evaluations |
| Windows Clang, `-O1` with ASan/UBSan and no recovery | Pass; 1032994 assertion/check evaluations |
| Integrated CMake Release `classick_block_read_tests` / CTest `storage.block-read` | Build passes; 1/1 CTest passes with the same 1032994 checks |
| Windows-native m68k GCC 16.2.0, `-mcpu=68000`, freestanding `-O0` / `-Os` | Both compile; no undefined imports; function text 324 / 222 bytes |
| Same m68k compiler frame reports | 28 / 36 bytes, dynamic bounded; function-local reports exclude provider/caller and do not prove cumulative stack safety |
| Pinned Open Watcom 16-bit `-2 -ms` / `-mc -os`, warnings as errors | Both compile; 80286/USE16 metadata, no external imports; core text 290 / 332 bytes and zero static data |
| Original 286 baseline RAM-provider diagnostic | Pass, POST B4; 1028-byte complete linked payload, no runtime-library modules |
| Actual core defect: copy output before all reads finish | Rejected at POST E4, the late-failure preservation check |
| Actual core defect: remove the LBA range check | Rejected at POST E5, the invalid-LBA/no-provider-call check |
| Native Mac128K original ROMs, same shared fixture, `-O0` / `-Os` | Both pass at B4; the same early-copy/range defects are rejected at E4/E5; no undefined imports or own-reset fault |

The large host numbers are repeated byte and state assertions, not a million
independent tests or reference compatibility observations. The test oracle and
implementation were authored in the same AI workstream; mutation controls give
specific fault-sensitivity evidence, not independent-author provenance proof.

The lead's routine-suite integration reused the fresh Release build tree
`C:/Repos/.task-temp/mac-keyboard-release-20261117-a`. Its build and CTest logs
are `C:/Repos/.task-temp/system-disk-boot-20261117/block-ctest-build.json` and
`block-ctest.json`. Only the isolated new target was added for routine coverage;
the existing unrelated working-tree CMake changes were preserved.

The original target fixture is shared as
[`probes/storage/block-read.c`](../../probes/storage/block-read.c), exporting
`cs_block_read_probe()`. It uses two 1026-byte local guarded buffers, generated
data and native callbacks. The 286 wrapper only reports its result to POST.
Its target workload covers two-block success near the uint32 LBA limit, a late
partial provider failure, canaries, range/capacity/exact-alias refusals and call
suppression. The 63-block workload is hosted evidence, not a target memory-fit
or performance claim. Watcom's `-s` suppresses stack helpers for this isolated
diagnostic; it is not a production stack policy. The existing original loader
supplies the real-mode stack; the linker warning about no declared stack
segment is retained in the raw diagnostic map.

## BootHush identities and observations

The initial complete run used executable SHA-256
`1f306c427d1a8c42b9d547b2b25a424e4b4e6a389a52ae5861d030e0fe43bc25`.
The same three built images were subsequently rerun, without rebuilding or
repeating host checks, on the current executable SHA-256
`ec4f037699b1e116ca74775f5b257e5deafd3dcfdfc973342ae6bfaa16832730`.
That separate retained report is
`C:/Repos/.task-temp/block-read-current-target-20261117-a/report.json`.
Both reports retain exact commands and identities; earlier results are preserved.

| 286 case | Marker | Modeled cycles, both executables | Payload bytes |
| --- | --- | --- | --- |
| Baseline | B4 | 263368 | 1028 |
| Early-copy defect | E4 | 196288 | 1038 |
| Omitted-range defect | E5 | 272541 | 990 |

Every run independently matched the complete loaded payload at 0x10000 and
verified its generated input floppy hash unchanged. The baseline payload
SHA-256 is `f7ce8d3403255def52ec0d0b3b4b48202703a4edb408c93776212f14394fd9e0`;
its generated floppy SHA-256 is
`2cbb7930cff7d49f3b87834b3c0034b5e05718989ebc738b367467e6adcf2598`.
This checks the common service through a RAM provider inside the target, not
BIOS/floppy reading by the new service or physical hardware behavior.

The lead additionally built and ran the same original shared fixture natively
on the Mac128K model with the current `ec4f0376...` executable. Its distinct
report is
`C:/Repos/.task-temp/system-disk-boot-20261117/mac-block-read-b/report.json`.
`probes/mac128k/block-read.c` binds it to the existing original reset/ROM map;
`scripts/Test-MacBlockRead.py` builds two optimization profiles and two actual
core mutations. Every run checked the 28-byte original diagnostic at 0xB00,
including zero in the reset fault field, and unchanged input ROM hash. Each
ROM is padded to 65536 bytes; the raw payload sizes below include vectors and
startup as well as the core/fixture. They are not total RAM or stack budgets.

| Mac case | Marker | Modeled cycles | Raw payload bytes | ROM SHA-256 |
| --- | --- | --- | --- | --- |
| O0 baseline | B4 | 1231272 | 1948 | `93ea2bed822fbee11062e2217c508e9df82c56b93ebcb23b1c7b00197f01338e` |
| Os baseline | B4 | 539552 | 1384 | `a76e45677575dbf16c2247ccf855c8a43f5e030753c570bb894c5a19e873a71e` |
| Early-copy defect | E4 | 398952 | 1384 | `7642abab3ade4959e9741febcaebf4e3d4c8341b09053ce4eacaa648256487bc` |
| Omitted-range defect | E5 | 540404 | 1372 | `db98e7e6dccc28ee341c7432a490eb7fedff9e43078354cbee902696ee0158e3` |

The Mac and 286 build reports record the identical core source SHA-256
`a8f9ce972e05d5278a93e94e6316bd4f008101cacd0cc9d93c047735a3e66baa`
and shared-fixture SHA-256
`9172e91fadfe5a5d266791e65e97e05420d28022140d03fc337527bddc715d69`.
Both modeled native CPUs therefore execute the same bounded RAM-provider
workload and detect the two specific defects. These cycle counts are retained
observations, not physical performance comparisons or disk throughput.

## Reproduction and provenance

On Windows PowerShell, run the focused script with the existing tool paths:

```powershell
& 'C:/Repos/ClasSICK-0.97/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/python/bin/python.exe' -B scripts/Test-BlockRead.py `
  --cc C:/Repos/ClasSICK-0.97/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe `
  --m68k-bin C:/Repos/.task-temp/m68k-windows-native-20261114-a/x-tools-v3-win/m68k-unknown-elf/bin `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --emulator C:/Repos/.task-temp/boothush-floppy-state-20261117-a/a/boothush-active.exe `
  --output C:/Repos/.task-temp/NEW-block-read-check

& 'C:/Repos/ClasSICK-0.97/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/python/bin/python.exe' -B scripts/Test-MacBlockRead.py `
  --m68k-bin C:/Repos/.task-temp/m68k-windows-native-20261114-a/x-tools-v3-win/m68k-unknown-elf/bin `
  --emulator C:/Repos/.task-temp/boothush-floppy-state-20261117-a/a/boothush-active.exe `
  --output C:/Repos/.task-temp/NEW-mac-block-read-check
```

The runner rejects an existing output directory and checks pinned Watcom
members, Windows m68k frontend identities, object metadata/imports, actual
executed payload bytes, and unchanged original media. Exact source, compiler,
emulator and artifact hashes are in the reports. An intermediate size-report
run rejected a valid Windows `nm` line because of CRLF; accepting trailing
whitespace fixed the report parser, and a new full run passed. No core change
was needed for that tooling issue. The separately authored Mac verifier had
the same Windows-line-ending parser issue on its first attempt; newline
normalization fixed it before the four passing target runs.

Codex authored these files from the owner-approved engineering contract,
project-owned memory/test/loader patterns and original generated fixtures.
The lead reviewed the contract and core and authored/executed the Mac binding
and verifier. No Apple/Microsoft implementation,
protected media/resources, UART bodies, HQ material or external implementation
source was used. Existing compiler tools were used, with no runtime modules
imported into the freestanding core/286 diagnostic. Project contributions are
GPL-3.0-or-later; independent human provenance/toolchain review remains open.

This is not MFS, a physical storage driver, a boot-block parser, System/Finder
execution, physical 128K/286 qualification, whole-OS budget proof, a segmented
guest-memory integration or a released image change. The next storage layer
must provide separately reviewed format and hardware/transfer contracts.
