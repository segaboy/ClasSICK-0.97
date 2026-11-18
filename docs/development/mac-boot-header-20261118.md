<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Data-only startup configuration - 2026-11-18

SRC-0617 / SPEC-0119 / IMPL-0149 / TEST-0210. The new original
`core/storage/mac-boot-header.[ch]` reader acquires exactly logical blocks 0-1
through the existing block service and publishes only common header metadata.
The output contains copied counted names and raw signature/version/page/count/
heap values. It contains no entry address or executable bytes. The four entry
bytes and all bytes after the common 138-byte header are never decoded.

The first completed report is
`C:/Repos/.task-temp/mac-boot-header-20261118-b/report.json`. Its exact commands,
source/spec/helper/tool hashes, generated input hashes and results are retained
beside the report. `inputs_unchanged: true` covers all declared sources and tools.
No selected owner disk or other protected media was used in this slice.

The follow-on explicit-container qualification below identifies the current
harness separately; the final-b evidence is retained for its exact earlier hash.

## Verification

Windows Clang Release and ASan/UBSan each pass **137,032 assertion evaluations**.
These are not that many independent tests. Coverage includes every 16-bit raw
version, every name-count byte in each of seven slots, all truncated lengths,
odd input alignment, asymmetric endian fields, raw names, ignored bytes,
canaries, invalid controls and first/second-callback unavailable/I/O/unknown
failures. A successful two-block acquisition followed by metadata rejection
preserves output; a provider failure also preserves the full acquisition array.

Both native CPUs execute the same original data-only diagnostic using their
actual current modeled floppy providers. The generated logical media contain
independent numeric patterns and raw counted names, not copied boot code.
Each lane passes six scenarios:

- Complete header acquisition and independent numeric/name-digest oracle.
- Wrong signature rejected with complete output preservation.
- The seventh name's count of sixteen rejected with complete preservation.
- Second-sector failure preserving output and all 1024 acquisition bytes.
- Fresh healthy recovery reproducing the original diagnostic/emulator result.
- Deliberate name-bound bypass rejected by the unchanged expected-result oracle.

The 286 lane verifies its entire loaded payload on every run. Both target links
have no runtime-library dependencies; the Mac link has no unresolved imports.
BootHush executable SHA-256 is
`7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a`.

| Baseline diagnostic | Payload bytes | Other measurement | Modeled cycles |
| --- | --- | --- | --- |
| Native 286 | 1828 | BSS 2792 bytes | 199170 |
| Native 68000 Mac | 3224 | 64 KiB padded diagnostic ROM | 4397792 |

These are complete diagnostic costs, including providers and instrumentation,
not isolated decoder throughput or physical hardware measurements. The native
workspace is a fixed 2048 bytes; no candidate-sized temporary or allocation is
needed. Publication follows complete validation and uses infallible copies.

The first harness attempt used the wrong CHS fault coordinate and correctly
refused to report success because the injection count was zero. The final run
uses the admitted 15-sector PC geometry: logical header block one maps through
the transfer offset to cylinder 2, head 0, sector 2. The Mac fault is track 0,
sector 1. Earlier outputs remain retained, not substituted for final evidence.

## Scope and provenance

The common header fields come from SRC-0617's later published Apple material,
independently restated in SPEC-0119. The conflicting version-dispatch prose is
not implemented. Raw heap/count/version extraction applies no sizing policy,
version acceptance, startup default, entry convention or control transfer.
The 4C4B signature is an explicit reader profile, not newly established original
MFS/128K historical behavior. Caller ownership and native segment confinement
follow the existing block-reader contract; general alias detection is absent.

Codex authored the specification, core, fixtures, diagnostic and harness from
reviewed field facts and original project interfaces. No manual sample code,
Apple/Microsoft implementation, private media, disassembly, protected assets or
third-party emulator source was an implementation input. Own contributions are
GPL-3.0-or-later. Coordinating integration review and independent human provenance
review remain distinct from the executed checks.

This completes bounded startup configuration acquisition. File lookup using
those names, resource startup, historical entry ABI, legacy execution, OS startup,
System Disk boot, persistent writes and physical qualification remain separate.

## Explicit container route, same day follow-on

The coordinating lead requested an explicit `--disk` runtime route. It validates
an external DC42 wrapper through the already reviewed `Test-MfsVolumeMedia.py`
helper and places transfer copies only in the chosen external output directory.
The original container is hashed before and after the suite. Source, helper,
specification and tool identities remain pinned. Native diagnostics still export
only numeric metadata and a copied-name digest; no entry or code bytes appear.

Default synthetic testing retains its independent oracle, faults and mutation.
The explicit route uses the same numeric-report probe without fixture constants,
requires successful header extraction and equal reports across both native CPUs,
and repeats each healthy run exactly. It applies no mutation or fixed-sector
fault to an explicit input. The route itself confers no new media authorization.

| Report under C:/Repos/.task-temp/ | Actual result |
| --- | --- |
| mac-boot-header-20261118-b/report.json | Earlier harness: 12 native cases, both host profiles |
| mac-boot-header-20261118-c/report.json | Current default: 12 native cases, both host profiles, one accepted original DC42 and six malformed-wrapper refusals |
| mac-boot-header-20261118-d/report.json | Current explicit route on the generated original DC42: four native runs, both host profiles, unchanged original input |

The six wrapper controls independently alter length, counted container name,
encoding, format tag, declared data length and checksum. They are rejected by
the same imported parser used by `--disk`, before native trial. Both host
profiles still pass 137032 assertions in c and d. The d metadata report also
matches c's independent original-media oracle, although the d runtime route does
not use that oracle internally. Both reports have `inputs_unchanged: true` and
identical source identity maps.

Earlier-b harness SHA-256:
`186cfd88117d2e6a546aba2748955dc0bc7877be7ab6ac7d327ccac59fefeeff`.
Current c/d harness SHA-256:
`693587ac46cb7d41019947bb8911743485dab445be9e621b53d33b6f8928cb4c`.
Unchanged decoder SHA-256:
`9ae51679a69f3453a21e7d64e0229c7b61e817a92eafe4ca02c4ec3bce1c914c`.
Generated original DC42 SHA-256:
`68465dc47545bad636079b50181be324874b335fb67e38eb1942f2bd742eb5e7`.

The explicit-route qualification used only this generated original container.
Its report's `private_runtime_input` flag selects the explicit route; it is not
a claim that the fixture was protected media. Owner-media trials remain with the
coordinating lead. No owner media was accessed by this implementer.

## Coordinating lead's selected-container observation

The coordinating lead subsequently ran the authorized selected DC42 input and
reported PASS in
`C:/Repos/.task-temp/mac-boot-header-selected-20261118-a/report.json`.
This implementer reviewed only that report's numeric metadata and identities,
not the selected container, transfer media or guest-output files. The report
uses the identical c/d harness and decoder hashes above and records unchanged
inputs. Its selected-container SHA-256 is
`1e03b75a05bbf441ae4e1808f70e38ffe0a07e3cc3298de151c0d2775552c647`;
logical-media SHA-256 is
`af412a6e3ef312ec51b6d53e99ca65e3246ab047650df835ce4f29f58f9f65c1`.

Both native CPUs acquire exactly two logical blocks and return the same
successful metadata report. Each CPU's fresh repeat reproduces its baseline
report and emulator result. Output guards remain intact.

| Raw field | Observed value |
| --- | --- |
| Signature | 4C4B hexadecimal |
| Version | 18 decimal |
| Page flags | 0 |
| File-control-block count | 10 |
| Event count | 20 |
| 128K heap field | 17152 |
| 256K heap field | 32768 |
| Other heap field | 49152 |
| Copied-name digest | 2471722755 |

The complete diagnostic takes 200038 modeled cycles on native 286 and 4457176
on native Mac68000. These are acquisition observations, not physical timing.
Names remain represented here only by their digest; no entry/code bytes are
exported. The raw version value establishes no dispatch rule. No selected
System-file initialization, legacy execution, boot or write claim follows.
