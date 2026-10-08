# TEST-0021 — Fixed-profile boot media generation and independent checks

2026-10-08. SPEC-0014 / ADR-0018 / IMPL-0018; SRC-0056/0057/0058. Packaging and
inspection only: no image was attached to a VM, written to a device, read by
firmware or booted.

## Hosted suites

`classick_boot_media_tests` runs six CTest suites per configuration:
`media.crc` (check value 0xCBF43926 and agreement with a separate table CRC),
`media.layout` (Microsoft FAT32 sizing gives 985 FAT sectors; raising reserved
sectors from 32 to 78 aligns data to disk LBA 4,096; 124,928 clusters; GPT
bounds), `media.structure` (37,888-byte payload), `media.payload` (eight sizes
from 1 to 65,537 bytes plus the full 63,961,600-byte capacity, and overflow
rejection with storage untouched), `media.arguments` (nine rejections, then an
adjacent non-overlapping payload accepted) and `media.determinism` (identical
rebuild from dirty storage). Each built image is re-read by an oracle that
shares no code with the writer: protective MBR, both GPT headers and arrays with
CRCs, ESP entry, BPB, FSInfo and backups, mirrored FAT, the
`\EFI\BOOT\BOOTX64.EFI` walk, payload bytes and slack, allocation/free counts,
and a scan proving every other sector is zero.

## Independent image checker and controls

`Test-BootMedia.ps1` re-derives all fields in PowerShell with its own CRC and
compares extracted file bytes with the expected payload by SHA-256.
`Verify-BootMedia.ps1` runs the whole prior UART wrapper, compares twin hashes
of the tool and test executables, generates the image from the audited O2 payload
with every hosted writer build (Clang x64 debug twins, release, i686 and
sanitized; GCC x64 debug twins, release and i686) and requires one identical
image hash. It then requires three tool refusals (existing output preserved,
non-`MZ` payload, oversize payload) and 30 corruption rejections, each for its
expected reason: MBR, protective record, GPT signature/header/entry CRCs for both
copies, re-sealed ESP type, alignment and attribute changes, BPB fields, FAT type,
data alignment, type string, signatures, backup sectors, FSInfo counts, FAT
mirror/reserved/chain/free region, directory name, file size, payload byte,
stray nonzero bytes and truncation.

## Local results (container, not pinned toolchain)

Ubuntu GCC 13.3.0 with ASan/UBSan at O2 and Clang 18.1.3 at O0 and O2 pass all six
suites; the full GCC CTest project adds six passing media tests. Three existing
`*.freestanding` checks fail identically with and without this change in the
Linux container (an ELF `_GLOBAL_OFFSET_TABLE_` reference and a CMake 3.28
policy difference); they are Windows-toolchain checks. PowerShell 7.4.6 runs the checker and a harness of
the corruption section: 30 rejections and three refusals pass. With the audited
O2 payload (SHA-256 `8e25d6947677b538d84bc17f98ff44d056dcd3d7769d1389676072286ad831f2`)
the GCC and Clang tools both produce image SHA-256
`370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078`; GPT header
CRCs 7A6BB824/F46774E1, entry array AE2A952C; 74 payload clusters, 124,851 free.

Non-normative black-box corroboration, run only on finished images and not
adopted: GPT fdisk 1.0.10 `sgdisk -v` reports no problems and shows the ESP at
2,048–129,023; dosfstools 4.2 `fsck.fat -n` reports a clean volume with 77/124,928
clusters used; GNU mtools 4.0.43 lists `\EFI\BOOT` and extracts a byte-identical
payload; util-linux 2.39.3 `blkid` identifies GPT and FAT32. No source of these
tools was read. Their acceptance says nothing about firmware behavior.

## Pinned Windows CI

Run [37845246261](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37845246261)
on `a6bb92f` failed and is retained. All nine hosted configurations passed CTest
including the six media checks (Clang x64 106, i686 82; GCC x64 91, i686 67),
the EFI O2 payload kept SHA-256 `8e25d694…ad831f2`, and the eight non-sanitized
writer builds generated identical images (each is compared as it is made). The sanitized writer then exited nonzero
with no output: the wrapper had restored the original PATH after the UART chain,
so the ASan runtime DLL from the pinned toolchain was not found. The wrapper now
runs the writer builds with the pinned toolchain PATH; no product code changed.

Run [37846593238](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37846593238)
on `78b00e51c401d1ee967618d7d2df316912cf917d` passed every step. CTest passed in
all nine configurations (Clang x64 106, i686 82; GCC x64 91, i686 67). Twin
hashes: Clang tool `17bcfdfd…5413d1a` and tests `059fb662…bf9b6b3`; GCC tool
`7534dee9…5c94f2` and tests `ad74b18e…f8e47`. All nine writer builds produced
image SHA-256 `370b7d4e7fc4200b77c367e09afa0c4944f534c2623d7f5f68db159c1da46078`,
the same bytes the Linux GCC and Clang tools produced locally. The independent
checker, 30 corruption rejections and three tool refusals passed; the EFI O2
payload stayed `8e25d694…ad831f2`. No image was attached or booted.
