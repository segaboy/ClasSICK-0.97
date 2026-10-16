# ROM 0.1.0-alpha.1 publication verification - 2026-10-16

SRC-0200/0201; [release notes](../../releases/rom/0.1.0-alpha.1.md),
[publication receipt](../../releases/rom/0.1.0-alpha.1-publication.json).
The owner said "OK, move on." after the proposed experimental alpha publication
and final source packaging, Snow instructions and provenance/package review.
This extends the earlier groundwork scope only for this component/version.

## Candidate and provenance boundaries

The released bytes are the existing original 65,536-byte ROM, SHA-256
`43c0296f808b85366fdbe55c9bcfd42e2daf9ef5fae77dfecfb85e5f80db0942`.
Compiled source `2ea01129377d248ef48dd81d6f8873e948456ce6`; observation revision
`031212db9de4711d157b8e997aa91777f999a47b`. The tag target identifies release
preparation metadata with matching native inputs, not a new compiled image.
[Prior observation/compiler results](loaded-app-redraw-evidence.md) are retained.
No new native construction, CTest matrix, emulator/VM/reference run or worker call.

Lead review follows SPEC-0035 / IMPL-0039 / TEST-0043, the original-source
manifest and existing source/role records. The original runtime and linker
closure use the explicitly named project inputs with `-nostdlib`; no compiler
runtime, Snow implementation or Apple binary/asset is added to this distribution.
This is an engineering provenance/package review, not human clean-room or legal
certification. Historical, source/UART, human provenance, input reliability,
native-app, physical/full128K/parity/edition/B2 and both System1/ROM gates persist.

## Local package checks

- Four intended assets: demo ZIP, standalone ROM, matching source ZIP and outer
  SHA256SUMS.txt. Exact sizes and hashes are in the publication receipt.
- Demo ZIP has ten named files: original ROM, setup/rebuild README, release
  notes, release metadata, build/qualification manifests, LICENSE, COPYRIGHT.md,
  source/archive identity and an internal checksum inventory. Two independently
  written ZIPs using fixed member metadata are byte-identical; extracted members
  and CRCs pass. It contains no emulator, compiler, System disk or private inputs.
- Source ZIP is a Git archive of the exact compiled revision: 616 tracked files,
  including build/linker/setup scripts, tests, pinned toolchain definitions and
  license/notices. Its file membership equals that commit's tracked tree.
  All 50 native manifest fingerprints match archive bytes and current source.
  UART contents are hashed/archived mechanically only, without body inspection.
- Source archive members are bounded text/source files; no Git history, local
  tools, untracked originals, HQ payloads, private manuals or binaries are included.
- Existing GPL-3.0-or-later grant, author notices and complete matching original
  source are delivered alongside the binary. External tool licenses remain
  separate; no third-party executable is shipped in this release.
- Snow instructions use the already admitted publisher build and recorded
  Early128K/relative-mouse/no-disk conditions. No new platform qualification.
- Updated metadata validation covers both draft and prerelease catalog states;
  local review preparation always remains a marked unpublished draft.
- Public indexed source/documentation guard passes. All 24 untracked originals
  and their existing copies, runtime/test files, qualified inputs and old source/
  build/specification/edition/target evidence remain preserved.

## Publication protocol

After local review, create the annotated component tag at the reviewed preparation
commit. Upload the four named assets to an unpublished GitHub draft prerelease,
download them into a fresh private directory and compare their exact bytes with
the local allowlist before publishing. Keep `prerelease=true` and avoid marking
the alpha as the stable latest release. Re-download the public assets afterward
and record actual tag target, release ID/date, asset IDs and digests in the receipt.

The receipt's status distinguishes prepared from actually published. No published
result is inferred from local files or an upload attempt. System stays unreleased;
ordinary Git/HQ/wiki records only, no OneNote or automated future publication.

## Recorded publication - passed

[The alpha](https://github.com/segaboy/ClasSICK-0.97/releases/tag/rom-v0.1.0-alpha.1)
was published 2026-10-16 with `draft=false`, `prerelease=true`, `latest=false`.
Its annotated tag points to `06dd25ef7e139f947d50710db2acc8b841cb68ad`.
All four draft downloads and all four public downloads are byte-identical to
the local allowlist; GitHub's reported SHA-256 digests and sizes also match.
The publication receipt contains the release ID, actual publication timestamp,
tag object/target and every asset's ID, URL, size and checksum. All downloads
total 1,641,857 bytes; the matching source accounts for 1,519,556 of those bytes.
System remains unreleased. This closes distribution verification only.
