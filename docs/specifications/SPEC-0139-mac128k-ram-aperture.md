<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0139 - Post-overlay 128 KiB RAM address mirrors on the 286

2026-11-19. IMPL-0169 / TEST-0230. The published 1985 *Inside Macintosh*,
volume III, III-17 and III-45, describes the low four-megabyte RAM area and
wrapping on a 128K machine. In this explicit 286 profile, the 32 logical
windows from 0 through 0x3FFFFF use the same 0x20000 bytes. All instruction,
operand and span permissions are those of the underlying physical RAM.
The initial ROM overlay is already released.

The provider has one canonical region at zero of exactly 0x20000 bytes,
potentially split across physical banks. Any other region starts at or above
0x400000 and retains strict mapping. A whole scalar beginning in RAM and
extending past 0x3FFFFF refuses before output or memory publication. Aligned
four-byte scalars that cross an interior 128 KiB seam wrap their last word
to physical zero. The exclusive stable ordinary-RAM contract permits both
word writes after preflighting both word spans; no callback to external
code, mapping changes or concurrent access can intervene. Physical bank
seams within a stripe still use the native provider's complete scalar
preflight. A multi-byte permission query spanning stripes checks the one
canonical region, retaining logical 24-bit bounds. Odd word/longword accesses
and unsupported widths refuse as in the generic scalar provider.

This optional factory leaves generic address translation and multi-register
transfer backends unmodified. It implements no device mirrors, initial ROM
overlay changes or cycle-accurate bus transactions.
