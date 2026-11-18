<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Streamed resource access - 2026-11-18

SPEC-0116 / IMPL-0146 / TEST-0206 compose the already reviewed SPEC-0024
resource format and SPEC-0115 MFS service. A resource fork no longer needs a
contiguous native pointer: the new source callback reads checked256-byte chunks
at32-bit offsets. Its MFS adapter validates the resource chain before binding.
Open/find publish complete metadata only after validation; extraction stages up
to32767 bytes before publication. Failure leaves destination bytes unchanged.

The final report is C:/Repos/.task-temp/resource-transport-20261118-a/report.json,
SHA-256 cc0cafb9afeecfe93b0de9704eae5c7798644440b3d1aaf80744f6993da0b0f9.
Release and ASan/UBSan each pass532840 assertion evaluations. These are repeated
checks, not that many independent test cases. Original fixtures compare the
existing contiguous reader's format/report semantics and independent expected
bounds, names, payload spans, duplicate refusal and callback-failure behavior.
All metadata alignments modulo256, high32-bit offsets, native span limits and
unchanged output/canary checks are exercised.

The native fixture uses an original fragmented70000-byte MFS resource fork,
with its map beyond64KiB. Native286 baseline and duplicate-ID/early-publication
defect controls pass. Mac68000 O0/Os baselines and both controls pass. The early
publication control encounters a partial late provider failure; the complete
caller output remains unchanged in the correct implementation. Full loaded286
payload, declared source/spec/helper/tool identities and media preservation are
checked. Native links have no runtime libraries. BootHush remains the exact
7adb8a74cef009b7c532df24458c3a5db6b77725356fdb31e5ac4966d7ed971a build.

C16 small/compact transport text is4931/5551 bytes with zero imports; the adapter
is173/265 bytes, importing only MFS fork read/validation. Native286 diagnostic
payload is10477 bytes; Mac Os/O0 payloads are9368/15264 bytes. These are component
measurements, not whole-system RAM or cumulative-stack qualification. Independent
read-only review found no concrete semantic defect under the stated stable,
disjoint native-object and callback preconditions. Exact elapsed worker effort
and provider usage are not measured.

Reproduce with scripts/Test-ResourceTransport.py and explicit Windows Clang,
native m68k toolchain, pinned Watcom and BootHush paths in a fresh external
output directory. No protected disk, implementation, asset, private HQ or
excluded research material was supplied to the implementers. All fixtures are
original. The full MFS chain is revalidated on each callback; integrated
performance is not yet qualified. This is transport and metadata access,
not Resource Manager service loading, executable startup or System/Finder boot.
