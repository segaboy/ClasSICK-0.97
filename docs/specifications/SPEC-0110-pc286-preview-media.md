<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0110 - Fixed-profile 286 preview boot and installation

2026-11-16. Original engineering contract under SRC-0608; project-owned
BootHush firmware model SRC-0610 supplies the test interface. This is not a
historical BIOS or physical-disk compatibility claim.

The preview owns an entire generated disk: 615 cylinders, four heads,
17 sectors per track, 512 bytes per sector. It has no partition table or
filesystem. The boot sector requires drive 80h, at least 640 KiB reported
memory and the exact geometry. It reads the payload from sector two onward,
at most three attempts per sector with resets between failed attempts,
verifies CRC-16/CCITT, clears linked BSS and enters the native near segment.
No payload execution follows a profile, read or integrity failure.

The floppy installer requires case-insensitive `INSTALL` followed by Enter.
Any different input cancels without writes. It validates its staged disk
prefix and scans every target byte for zero before writing. Occupied targets
are refused, including data beyond the installed prefix. It writes and
reads back each payload sector, then commits and verifies the boot sector
last. Errors have bounded retries and visible diagnostic codes. This ordering
limits ordinary failed-write outcomes; it is not power-loss atomicity or
protection against a controller that lies about writes.

All build/run tools create new files. Qualification uses disposable generated
images, never host block devices. The builder checks source/tool/payload
identities, linked entry/BSS and staging/scratch/near-segment bounds. CRC is
an accidental-corruption check, not authentication. BootHush's modeled BIOS
and the original diagnostic font are explicit dependencies of observation.
The native payload embeds no external runtime-library module.

Qualification: cancellation, full-disk occupied refusal, staged/payload
corruption, permanent/transient reads and writes, boot-sector-last ordering,
whole installed-image equality, installed boot, and same native shared-window
state/pixels. All reports distinguish emulator evidence from physical,
human, independent provenance review and publication disposition.
