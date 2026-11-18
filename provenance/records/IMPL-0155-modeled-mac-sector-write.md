<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0155 - Modeled Macintosh sector writer

2026-11-18; SPEC-0125 and SRC-0619. Original C GCR/checksum encoder and bounded
68000 assembly sender compose with the unchanged validated reader to preserve
sector tags. Input media requires separate explicit writable-copy admission.
No filesystem mutation or physical claim. Implementation, retained padding
defect, exact identities and boundaries belong to the
[canonical receipt](../../docs/development/mac-floppy-write-20261118.md).
