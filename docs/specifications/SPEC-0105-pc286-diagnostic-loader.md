<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0105 - Four-track 286 diagnostic loader

2026-11-14. This is a test-media contract, not a production boot or installation
contract. The fixed 1.2 MB test floppy has a boot sector followed by up to 59
sectors of linked payload. The boot sector makes bounded BIOS reads that stay
within each 15-sector track, beginning at cylinder 0, head 0, sector 2 and
continuing through cylinder 1, head 1, sector 15. It loads at 1000:0000,
establishes the documented diagnostic segment/stack state, and jumps to an
entry within the payload. Failed reads report `ee`; BSS clearing and optional
test-only poisoning retain the earlier loader behavior. A too-large payload or
out-of-range entry is rejected before media creation.

An optional diagnostic mode sums exactly the payload bytes modulo 65,536 and
compares them with the value embedded at image creation. A mismatch reports
`ed` before entry. Padding is excluded. This detects the tested single-byte
corruptions but does not authenticate the image or detect all damage; for
example, balanced byte changes can preserve the sum. It is disabled by
default and does not define a production media-integrity contract.

The model is tested against BootHush, including boundary and short-read
controls. Real BIOS geometry, media, failure handling, memory layout, boot
drive selection, hard-disk installation and release qualification remain open.
See [the evidence record](../development/mac-pc286-shared-parity-20261114.md).
