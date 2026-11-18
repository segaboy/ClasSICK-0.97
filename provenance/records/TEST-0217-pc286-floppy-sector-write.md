<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0217 - Native single-sector write and fresh-boot persistence

2026-11-18; SPEC-0126/IMPL-0156. Original
`probes/pc286/boot-floppy-write.c` and `scripts/Test-PC286FloppyWrite.py`
pass twelve native boots and eighteen C16 object audits. Three optimized
write/fresh-read pairs, one unoptimized pair, default read-only refusal and
three deliberate mutations have their expected markers. Complete exported
image comparisons, loaded payload identities, source/canary preservation and
unchanged pristine inputs pass. Exact hashes and limitations are in the
[canonical receipt](../../docs/development/pc286-floppy-write-20261118.md).
No multi-sector atomicity, MFS write, owner-media or physical claim.
