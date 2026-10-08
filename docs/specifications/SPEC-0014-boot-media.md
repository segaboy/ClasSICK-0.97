# SPEC-0014: Fixed-profile GPT/FAT32 boot media

Version 1, finalized 2026-10-08. Original PC development packaging profile;
SRC-0056 owner decision, SRC-0057 Microsoft FAT32 specification, SRC-0058 UEFI
2.11 media sections, the existing SRC-0038 removable-media path and SRC-0039 ISA.
ADR-0018 / IMPL-0018 / TEST-0021. Generated and checked only; no firmware, VM or
physical machine has read the image. Human provenance and rights review pending.

## Interface facts used

UEFI 2.11 section 5.2.3 (printed 114 / PDF 198) defines the protective MBR:
zero boot code and disk signature, one record with OSType 0xEE, StartingCHS
0x000200, StartingLBA 1 and SizeInLBA equal to disk size minus one, three zero
records and 0x55/0xAA at 510/511. Sections 5.3.1–5.3.3 (115–120 / PDF 199–204)
define the primary header at LBA 1 and backup header at the last LBA, a
16,384-byte minimum entry array, First Usable LBA of at least 34 for 512-byte
blocks, the header and entry-array CRC32 fields, entry layout, the EFI System
Partition type GUID C12A7328-F81F-11D2-BA4B-00A0C93EC93B and attribute bits.
Section 4.2 (PDF 174) names the CCITT32 CRC with polynomial 0x04C11DB7; the
project's reviewed SPEC-0007 convention is the reflected form with all-ones
initial and final values, check value 0xCBF43926. Appendix A (PDF 2054) gives
little-endian TimeLow/TimeMid/TimeHighAndVersion GUID storage. Section 13.3
(462–466 / PDF 546–550) specifies FAT32 for a system partition and FAT12/16 for
removable media (13.3.4.1 also allows FAT32 there), requires firmware to support
all three, asks that the FAT32 data region be aligned and places the
removable/default loader at `\EFI\BOOT\BOOTX64.EFI` (SRC-0038).

Microsoft's FAT32 File System Specification v1.03 supplies the boot sector/BPB
and FAT32 extension tables, FAT type determination by cluster count only
(FAT32 at 65,525 clusters or more, staying 16 clusters clear of the cutover),
the FAT32 sizing table and FAT-size arithmetic, reserved FAT[0]/FAT[1] values,
28-bit entries and EOC, FSInfo signatures/fields, the sector-6 backup boot
record, 32-byte short directory entries, dot/dotdot rules and date format.

## Fixed profile

Logical blocks are 512 bytes and the raw image is exactly 67,108,864 bytes
(131,072 LBAs). LBA 0 is the protective MBR above; EndingCHS is 0xFFFFFF because
this image declares no CHS geometry. Primary header LBA 1, entries LBA 2–33,
First Usable 34, Last Usable 131,038, backup entries 131,039–131,070 and backup
header 131,071. Header revision 0x00010000, size 92, 128 entries of 128 bytes,
unused bytes zero. The disk GUID 5AD001EF-55C4-45C1-91E6-B46E536C16DD and the
partition GUID 2A9F2871-A46A-48FB-844C-5429E306796A are fixed project values
generated once so builds are reproducible. Consequence: two copies of this image
must not be attached to one machine at the same time.

The only entry is the ESP at LBA 2,048–129,023 (126,976 sectors, 1-MiB aligned),
attributes 0, name `ClasSICK ESP` in UCS-2. Both entry arrays are identical.

The FAT32 volume uses 1 sector per cluster (the specification's table for this
size), two FATs, root cluster 2, FSInfo sector 1 and backup boot sector 6. The
FAT size is computed with the published arithmetic for 32 reserved sectors:
985 sectors. Reserved sectors are then raised to 78 so the data region starts at
volume sector 2,048 (disk LBA 4,096, 1-MiB aligned); the FAT stays larger than
the 124,930 entries it must hold. That gives 124,928 clusters, well above the
FAT32 threshold. Media 0xF8, hidden sectors 2,048, total sectors 126,976,
FSVer 0:0, mirrored FATs, drive 0x80, extended signature 0x29, volume ID
0x20261008, label `NO NAME`, type string `FAT32`. Geometry fields are zero.
The jump is EB 58 90 and the boot-code area holds only an original four-byte
stub FA F4 EB FD (CLI, HLT, short jump back to HLT; SRC-0039 encodings) at
offset 90; UEFI does not execute it. Sectors 0/1 and 6/7 hold the boot
sector and FSInfo; FSInfo records the exact free count and next free cluster.

Allocation is fixed: cluster 2 root (`EFI` directory entry only), cluster 3
`\EFI` (dot, dotdot = 0, `BOOT`), cluster 4 `\EFI\BOOT` (dot, dotdot = 3,
`BOOTX64.EFI` with the archive attribute), then the payload in contiguous
clusters from 5 ending in 0x0FFFFFFF. Short 8.3 names only; no long names, no
volume label entry. FAT[0] = 0x0FFFFFF8 and FAT[1] = 0x0FFFFFFF. All dates are
2026-10-08 (0x5D48) with zero time. Every byte not defined here is zero.

## Writer contract

`cs_media_build(payload, size, image, image_size, layout)` accepts a nonempty
payload of at most 63,961,600 bytes (124,925 clusters) and an image buffer of
exactly the profile size that does not overlap it. It validates before writing;
on any rejection the image and layout are untouched. ARGUMENT covers null
pointers, empty payload, wrong size and overlap; CAPACITY covers a payload that
does not fit. Success writes every image byte and reports cluster counts and the
three CRCs. The writer uses no allocation, globals or host byte order.

The host tool `classick_boot_media BOOTX64.EFI output.img` refuses an existing
output path, a payload that does not begin with `MZ` and a payload that does not
fit. It does not inspect the PE image; SPEC-0007's image audit remains the
payload check. The payload selected for evidence is the audited O2 EFI copy.

## Verification and limits

Hosted tests use a separate oracle reader and table CRC. An independent
PowerShell checker re-reads every field of the produced image; corruption
controls (including re-sealed GPT field changes) must be rejected for the
expected reason. All hosted writer builds must produce one identical image.

This is a packaging contract, not a boot result. Firmware FAT/GPT acceptance,
VirtualBox disk conversion/attachment, removable-versus-fixed media handling,
4-KiB-sector devices and physical USB media remain unverified. No long-name
support, other file, signature/Secure Boot material or BIOS boot path is claimed.
Firmware eligibility, machine qualification and B2 are separate open gates.
