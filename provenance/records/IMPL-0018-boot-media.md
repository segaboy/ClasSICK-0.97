# IMPL-0018 — Fixed-profile GPT/FAT32 boot media

2026-10-08. SRC-0056/0057/0058, reused SRC-0038/0039 and the SPEC-0007 CRC
convention; SPEC-0014 / ADR-0018 / TEST-0021. Claude (Anthropic; session
configured as `claude-opus-5-5`) wrote the original C, tests, CMake, PowerShell
and documentation. Self-review only; human provenance and rights review pending.
GPL-3.0-or-later. No independently staffed or legally certified clean-room claim.

The two published specifications were reviewed before any code; the SPEC-0014
profile was chosen from them and written down alongside the implementation. No
formatter, partitioning tool, firmware FAT/GPT driver, operating-system file
system, library or code sample was read or used as input. Third-party tools ran
only afterwards as black-box checks of finished images (TEST-0021). No Apple
material is involved. Microsoft's code fragments were read as
interface description and not copied. The owner accepted the Microsoft terms
(SRC-0056); ECMA-107/FAT16 remains the documented fallback.

Paths: `platform/pc/media.h/.c` (writer and CRC), `tools/boot-media/main.c` (host
tool), `tests/boot-media.c` (oracle reader, table CRC and six suites),
`scripts/Test-BootMedia.ps1` (independent checker), `scripts/Verify-BootMedia.ps1`
(full-chain wrapper, cross-build images, refusals and corruption controls), CMake
and the workflow step. The writer zeroes the whole image, then stores every field
byte by byte, so output does not depend on host byte order or allocation.

Design choices beyond the specifications are recorded in SPEC-0014: one 64-MiB
profile, fixed GUIDs/serial/dates, EndingCHS 0xFFFFFF and zero geometry, raised
reserved count for 1-MiB data alignment, contiguous allocation, short names, a
four-byte original legacy stub and an `MZ` check in the tool. GPT CRCs use the
existing project CRC convention, implemented again for the host writer.

The evidence payload is the audited O2 `BOOTX64.EFI`; compiled loader/kernel
source is unchanged. No image was attached to a VM, written to a device, read by
firmware or booted. Firmware eligibility, machine qualification and B2 remain
open; Macintosh/mini vMac and physical-hardware edition goals are unchanged.
