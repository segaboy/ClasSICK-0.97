<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0107 - 286 whole-disk diagnostic on the BootHush model

2026-11-14. This experiment generates an entire disposable raw disk for the
documented BootHush PC286 built-in firmware geometry: 615 cylinders, four
heads, 17 sectors per track, 512 bytes per sector. This is a model contract,
not an assertion about an IBM PC/AT's physical disk or BIOS. The image has an
original signed boot sector, followed by up to 59 sectors of linked payload;
it is not a partitioned disk, installer, filesystem or production boot medium.

The diagnostic reads drive `80h` at cylinder zero, beginning with head zero,
sector two. It uses bounded reads at the four head boundaries into `1000:0000`,
optionally tests the payload's 16-bit additive sum, clears the declared BSS,
establishes diagnostic segments and stack, and enters an in-bounds offset.
Failure of a BIOS read reports `ee`; an optional checksum mismatch reports
`ed`. The sum detects the tested single-byte corruption, not all corruption
or an adversary. Input payload, entry and BSS bounds are checked before media
creation. The disk is generated locally and must not overwrite existing media.

The geometry source is SRC-0610, BootHush's own documented model in
`C:/Repos/boothush/docs/pc-at.md`; it supplies no physical-hardware evidence.
Unknown real geometry, boot-drive selection, partitioning, installation, BIOS
variation, disk error recovery, integrity/authenticity, review and physical
testing remain open. [Evidence](../development/mac-pc286-shared-parity-20261114.md).

2026-11-15: the fixed-disk diagnostic also offers an alternative CRC-16 over
exactly the linked payload bytes: polynomial `0x1021`, initial value `0xffff`,
no reflected input or output and no final XOR. It is selected instead of, not
alongside, the original additive option. A mismatch reports `ed`. The original
additive profile and old media remain unchanged. CRC detects the tested
balanced-byte corruption that the sum misses, but has collisions and provides
no authenticity, malicious-tamper protection, production boot or installer
guarantee. [Evidence](../development/mac-pc286-shared-parity-20261114.md).
