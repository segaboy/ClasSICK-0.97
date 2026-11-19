<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Executing a generated two-sector program on Macintosh 128K and installed 286

2026-11-19. On both models, the two-sector loader read the generated disk,
validated its header, published all 1024 bytes, and ran the program acquired
from RAM. Changing the immediate operand on disk changed the 32-bit return
value on both machines. The 286 ran the test after a fresh install and cold
boot, with the installation floppy removed.

[SPEC-0130](../specifications/SPEC-0130-boot-block-image.md),
[IMPL-0160](../../provenance/records/IMPL-0160-boot-block-image.md) and
[TEST-0221](../../provenance/records/TEST-0221-boot-block-image.md) describe
the storage interface and probes. SRC-0617 and SRC-0622 record the published
metadata and startup evidence. This test's entry offset, register state, stack,
bootstrap and return convention are independently specified for the generated
disk. Original 128K startup dispatch and required low-memory state remain open.

The disk contains an original BRA.W at byte 2, a MOVE.L at byte 510, its
four-byte immediate starting at byte 512 and an RTS. The Mac IWM provider
supplies both sectors. The 68000 calls the RAM image and returns to the caller.
The PC BIOS provider reads those same Macintosh sectors from LBA 0 of a
read-only, zero-padded 1.2 MB transport. A native 286 executable loads the
image into guest banks with a bank split inside the immediate operand.
The guarded interpreter takes three steps; the test checks D0, the return
target and the restored stack pointer. Diagnostics live outside the ROM
parity contract. No Apple ROM or owner media was used.

| Generated input | Mac and installed 286 result | Mac cycles | 286 cycles |
| --- | --- | ---: | ---: |
| Healthy | D0 = 1256ABCD hexadecimal | 4,501,246 | 2,979,071 |
| Different immediate | D0 = AAB16798 hexadecimal | 3,830,484 | 2,979,073 |
| Bad signature | Signature refusal, no entry | 4,416,724 | 2,523,959 |
| Invalid counted name | Name refusal, no entry | 4,416,748 | 2,523,995 |
| Failed second read | I/O refusal, no entry | 4,313,786 | 2,454,080 |
| Cold recovery | Healthy D0 and cycle count reproduced | 4,501,246 | 2,979,071 |

All twelve runs checked the complete published RAM image: exact disk bytes on
success and an unchanged zeroed destination on refusal. Hosted optimized and
ASan/UBSan builds separately exercised the preservation of a prior nonzero
image, canaries, truncated capacity at the device and argument rejection.
The installation run explicitly entered INSTALL and stopped at POST AC after
131,711,330 modeled cycles. The complete installed disk matched the prepared
fixed-geometry output. A second fresh run compared the complete linked native
runtime with RAM after boot. The install source, blank disk and both targets'
media inputs retained their original hashes.

| Artifact | SHA-256 |
| --- | --- |
| BootHush with drive selection | `5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e` |
| 64 KiB Mac diagnostic ROM, 2,792 linked bytes | `e9a73b1c3d912b873f8f74c88ec9bd36ac10a3cc438880bbece7b316499d316b` |
| Installed 286 runtime, 15,496 linked bytes | `295fbe6faf968c1e40eb9ca312753b9a750465f60d30d0ef2b03d49e4670e89c` |
| Installer floppy | `e1c1c1b97cce57f917701a4fad77c6228c64f4162e490cb4ac6a1cd683b19a39` |
| Entire installed disk | `ff25677a739c503875d5c1b78cdbf14e8c784e30fe1b5d6b5c0adda0971980ae` |
| Qualification report | `9f4c7e7763fb354e1e8f855b8da22f08ea0faaf27db6f0ed6020241956ccfd14` |

The retained external run directory is `boot-block-entry-20261119-d`. Its
`report.json` contains the build and boot commands, source and artifact hashes,
host outcomes, installer manifest, and per-case numeric results.
Reproduce with [Test-BootBlockImage.py](../../scripts/Test-BootBlockImage.py),
the listed BootHush build, the project's Windows m68k compiler, pinned C16
assembler/linker, host clang and an empty external output location.
The remaining interoperability milestone is to implement and verify the
original startup contract against the authorized disks. These tests establish
disk-loaded execution only for the generated program and modeled devices.
