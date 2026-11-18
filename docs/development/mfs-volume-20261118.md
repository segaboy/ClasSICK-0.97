<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native read-only MFS volumes and forks — 2026-11-18

SRC-0613/0614 / SPEC-0115 / IMPL-0145 / TEST-0204. The new shared freestanding C
service mounts a checked header/map snapshot, iterates strict consecutive records,
looks up counted raw names and reads arbitrary byte ranges from fragmented data
or resource forks. Complete chain/range checks precede data I/O; all data stages
before publication, preserving caller output after late or partial provider failure.
There is no allocator, automatic repair, guest-pointer conversion or disk write.

The canonical focused report is
C:/Repos/.task-temp/mfs-volume-20261118-c/report.json.
Independent generated host fixtures pass113202 assertion evaluations in each
Release and ASan/UBSan profile. These are repeated byte/state evaluations, not
113202independent tests. Cases cover640-entry map limits, non-power-of-two1536-byte
allocations, a32767-byte request near UINT32 logical EOF with an independent64-bit
sector oracle, fragmented unaligned reads, both forks, cycles/bad links/lengths,
reserved directory overlap, raw names, malformed tails/counts and failure preservation.

Four native286 outcomes and five native68000 outcomes pass: baseline B6, Mac O0/Os,
and rejected free-count bypass D4, omitted complete-chain validation D5 and early
publication D6. Full loaded286payloads match; Mac diagnostics show expected markers
without reset faults. All declared source/spec/helper/tool/media inputs are checked
before/after, with no runtime library in native links. The routine Release CTest
integration also passes storage.block-read, storage.mfs-read and storage.mfs-volume.

C16 small/compact core text is2340/2755bytes, importing only the existing project
block/header/entry services. The complete baseline286diagnostic payload is5219bytes.
Mac Os diagnostic raw ROM size is5664bytes, SHA-256
fdcaae811e4d8592fcca1266f3d7ad5ed55e5fbd423e7d97b36b3931f3007d60.
Local compiler stack reports and exact object sizes are retained; they are not a
cumulative whole-OS stack/RAM proof. BootHush executable is the pinned7adb8a74 build.

The first native verifier attempt exceeded the4096-byte inspection limit, so it
was corrected to compare contiguous bounded chunks. A separate deliberate mutation
initially produced an unused-variable warning; the mutation was corrected before
the complete final pass. The production core did not require a correctness fix.
A separate read-only review found no concrete bounds/arithmetic/publication defect
under the documented immutable mount and truthful/disjoint native-object assumptions.

This deliberately strict profile rejects nonzero inactive/deleted entries and
ambiguous directory tails. Lookup is exact counted-byte equality, not historical
collation. Mount validates map values/free count and existing directory reservations;
it does not detect ownership shared across files or repair unreachable allocations.
The selected-image observation supports only its stated scope; general historical
MFS and File Manager behavior remain unqualified.

Reproduce with scripts/Test-MfsVolume.py and explicit Windows Clang, native m68k,
pinned Watcom, BootHush and fresh external output paths. Source/generated fixtures
are original; the implementer received a reviewed contract and numeric observations,
not protected disk bytes, private HQ, manuals, excluded research or external code.
Actual floppy-media integration is tracked separately from this RAM-provider suite.
