<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0156 - Native286 one-sector floppy writer

2026-11-18; SRC-0620/SPEC-0126/TEST-0217. Original
`platform/pc286/boothush-floppy-write.[ch]` validates a bound range and native
bounce, preserves the source and invokes exactly one modeled BIOS AH03/AL01
operation. Existing read-provider arithmetic is reused without editing it.
No retry, multi-sector transaction, filesystem mutation or protected input.
[Canonical receipt](../../docs/development/pc286-floppy-write-20261118.md)
owns identities, native observations, measurements and remaining limits.
GPL-3.0-or-later; physical/historical/provenance gates remain distinct.
