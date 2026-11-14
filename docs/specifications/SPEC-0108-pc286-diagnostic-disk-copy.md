<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0108 - Disposable 286 diagnostic disk copy

2026-11-14. An original test guest may write the sector-aligned prefix of the
SPEC-0107 image to a freshly generated blank BootHush fixed-disk image, read it
back into a separate RAM region and compare every staged byte. Its test host
must retain the original generated input media, save a new snapshot, compare
the entire output identity with the intended disk and boot that snapshot in a
separate bounded run. The source and destination are generated local test
images; the workflow never accepts or modifies an existing user disk.

Within the modeled built-in BIOS, a write failure reports `e1`, a verification
read failure reports `e2`, and a byte mismatch reports `e3`; successful
comparison reports `ac`. A failed verification read can follow a successful
write and does not roll it back. A test of an injected write failure requires
the generated disk to remain blank for that atomic modeled write. The host
must reject an image with changed staged data even if the guest's readback
matches the changed input. The model accepts the single contiguous transfer
across track boundaries; that is not a physical BIOS guarantee.

This experiment has no partition table, filesystem, disk selection, consent
UI, recovery, power-loss handling or real-media protection. It is not a
production installer or authorization to install on physical hardware.
[Evidence](../development/mac-pc286-shared-parity-20261114.md).
