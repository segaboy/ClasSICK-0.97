# Boot media evidence — 2026-10-08

SPEC-0014 / ADR-0018 / IMPL-0018 / TEST-0021. The audited O2 loader payload is now
packaged in an original, reproducible 64-MiB GPT disk image with one FAT32 EFI
System Partition holding only `\EFI\BOOT\BOOTX64.EFI`. This closes the
"formatted media" item of the [readiness audit](boot-readiness-audit.md) as a
generated and independently checked artifact. **It is not a boot result.**

| Item | Value |
| --- | --- |
| Payload | 37,888 bytes, SHA-256 `8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2` |
| Image | 67,108,864 bytes, SHA-256 `370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078` (local GCC/Clang) |
| GPT | Disk GUID 5AD001EF-55C4-45C1-91E6-B46E536C16DD; header CRCs 7A6BB824/F46774E1; entries AE2A952C |
| ESP | LBA 2,048–129,023, FAT32, 78 reserved, 2×985 FAT sectors, data at disk LBA 4,096 |
| Allocation | Clusters 2–4 directories, 5–78 payload, 124,851 free |

Reproduce on Windows after a hosted build:
`classick_boot_media.exe <efi>\payload\EFI\BOOT\BOOTX64.EFI classick-097.img`, then
`scripts\Test-BootMedia.ps1 -Image classick-097.img -Payload <same EFI>`.
The image is generated output and is not committed.

Local container results, black-box corroboration and controls are in
[TEST-0021](../../provenance/records/TEST-0021-boot-media.md). The pinned
Windows wrapper requires every hosted writer build to produce the same image.
Pinned CI: pending at publication of this snapshot.

Still open before any launch: firmware eligibility (no candidate firmware is
cleared), isolated machine/device qualification and B2. Converting the raw
image for a VM or writing it to USB media needs its own recorded procedure.
