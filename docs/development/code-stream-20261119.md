<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Shell CODE metadata from the selected System Disk

The native 68000 and 286 probes both read the boot header, open the designated
System and Shell file resource forks, and inspect the Shell jump table. With the
offset measured from the byte following each four-byte segment header, both
targets validate all 52 rows and three segment descriptors on the selected
System Disk. Each target produced the same aggregate result on two cold runs.
The literal resource-start interpretation refused at row 49 (zero based),
because its computed target lay inside the header. It had checked all 52 row
forms by that stage. The literal failure occurred at fork position 668.

This discriminates two explicit interpretations for this workload. The
post-header interpretation is an engineering hypothesis, not an established
historical rule. The caller selects one profile for the entire table. Neither
profile authorizes execution. The private probe consumed metadata through
BootHush's modeled floppy providers. It did not copy an executable to guest
RAM, execute Finder, or test physical hardware.

| Selected disk result | Resource start | After header |
|---|---|---|
| Native runs | 2 Mac + 2 PC | 2 Mac + 2 PC |
| Result | routine in header, row 49 | all rows and groups validated |
| A5 sizes above/below | manifest withheld | 448 / 2492 |
| Jump table length / A5 offset | manifest withheld | 416 / 32 |
| Native source read after inspection workspace reuse | not reached | passed |
| Private aggregate run report SHA-256 | `0422af9618d1530fa777f052909d3d68b3b8c2df9f4b155ca15f7ccd0fb48aef` | `b61f30480a38e011ad68e40ade3ab814bed292fba75acdd3a14a1f4edda2247b` |

The coordinator verified the private DC42 input identity
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`.
Only the numeric report informed this result. Private transfers, protected
bytes, and runtime outputs remain outside the repository.

## Reproduction on original fixtures

Run `scripts/Test-CodeStream.py` with the pinned native toolchains, host
compiler, BootHush, an external output directory, and either
`--routine-base literal` or `--routine-base after-header`. Omitting `--disk`
generates a fragmented MFS image from scratch. The qualified BootHush
executable SHA-256 was
`3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4`.
Each report records all source and tool identities and command arguments.

For each profile both native machines validated the 180-row, 20-segment
fixture, reported a late injected I/O error at the expected row-batch
boundary, and recovered on a fresh run. All 12 native outcomes agreed with
the generator's expected reports. Optimized and ASan/UBSan host tests each
passed 1,022 checks, including the 8,192-row/64-segment boundary and invalid
offset classifications. Three deliberate defects (early publication,
four-byte offset shift, skipped group validation) failed the unchanged checks.
Six malformed DC42 wrappers were refused and the generated valid wrapper
round-tripped.

| Generated fixture | Resource start | After header |
|---|---|---|
| Report SHA-256 | `19dfebd84b3e0b60484c66057e0a350b9e0cfc9f5233632e511402a0766349df` | `307ed561da0f6194ecb8f3edbe7c08791d9ab55fe3a31bdc6c96f408b01c6406` |
| Mac 64 KiB ROM SHA-256 | `e5033f0b7772565530595681cda141521d5add2dab0f33cdb74b417e5eb95e54` | `ff6cf64998ba50ea370f5dc4facf602672e7ed6ccc183032a9e73b3c1b9c8fef` |
| Mac linked payload / data and BSS bytes | 13,064 / 19,692 | 13,068 / 19,692 |
| PC payload SHA-256 | `9f57c66c0166dadc221cdd6412333619548d4e6520672135efdc6d4b94c7e791` | `57a067e764aab94693d91f615e6cd67c7e33dbb47dffe8929872085a7f07f8ea` |
| PC payload / BSS bytes | 14,227 / 18,862 | 14,227 / 18,862 |

## Resolving a row for a future loader

SPEC-0132 uses the resulting manifest to resolve one requested jump-table row
without saving the whole table. It reads eight bytes of row metadata,
rechecks the row and segment bounds, and returns the descriptor index and a
resource-relative offset suitable for a later copy plan. It returns no guest
address. Its source owner and medium must remain live and immutable.

On independently generated inputs, 1,103 checks passed in each of the optimized
and ASan/UBSan hosted builds. All 180 rows resolved under both interpretations
after clearing the construction workspace. The provider recorded exactly one
eight-byte read per row. Simulated I/O failure, damaged row forms and
out-of-range requests preserved the destination. The new module also compiled
to native 68000 and 286 objects. The separate native trials above exercise
inspection and do not exercise this row API.

Evidence chain: SPEC-0127 / IMPL-0157 / TEST-0218, and SPEC-0132 /
IMPL-0162 / TEST-0223. The implementation follows the project-owned
SPEC-0032/0116/0122 contracts and their recorded applicability limits.
All implementation, tests and generated fixtures are original project work.
Independent historical, release and physical review remain open.
