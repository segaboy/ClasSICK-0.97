<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Installed 286 startup from separate Macintosh media

On 2026-11-19 BootHush booted the native 286 installer floppy, accepted
`INSTALL` on a blank whole-disk image, and saved the resulting fixed disk.
Two cold boots from that installed disk opened the System and Shell resource
maps named by the supplied System Disk. Each reported 39 resource types, 144
references and warning mask 1 in 92,644,486 modeled cycles. This is the first
installed 286 path here to reach that milestone with a separate data floppy.
The selected disk's own startup instructions and Finder have yet to run.

The builder compiles a 10,455-byte payload with the pinned 16-bit Open Watcom
toolchain. The HDD loader clears 9,810 bytes of BSS beginning at offset 10,456
and supplies the stack itself. The linker accordingly reports no stack
segment. No runtime library appears in the link map. The application uses
the same startup coordinator as the earlier two-target probe. It reads sector
zero of a separate floppy transport containing the original 800 logical sectors
in order. It retains both opened resource sources until this diagnostic stops.

| BootHush test | Result |
| --- | --- |
| Install onto blank disposable HDD | Guest-created image matched the expected image in full |
| Generated fragmented MFS disk, twice | Two maps, six types, eight references |
| Same volume with a PC boot signature added, twice | HDD boot selected, same guest state |
| Generated invalid boot header, twice | Header/signature refusal before opening any map |
| Supplied System Disk, twice | Two maps, 39 types, 144 references |

Every boot checked the full linked payload in RAM, eight status words, and the
execution segment. The two repetitions for each case took identical modeled
cycle counts. The original supplied container, read-only transports, and
installed disk retained their input hashes.

| Identity | SHA-256 |
| --- | --- |
| BootHush cc182c2 | `5fa45b7542409300b7604606a78c81d4ee95e888a57c592a83d0cec43ce1056e` |
| Native installed payload | `f58efbe6c34c2318dfb0fad1c9dfd528c790b6f8048e70c542a5f8c080c89954` |
| Installer floppy | `3cba246ffe8c83ad1e835d91ee616aa23e6a813317ff715ea178f9fdcbc5a73c` |
| Installed fixed disk | `c657ef9c318ef5905d90f7f0ae1d69c4b03d66e2c3c2a3dbebdd47955daee0f7` |
| Build manifest, installed-startup-20261119-b/media.json | `38e580178942d6f946faf7ba161dd21684df0c5d2b510d676bb63265318c2e75` |
| Nine-run report, installed-reader-test-20261119-b/report.json | `f15a16156399bdc669c3aa236d528e555798f399b3d40022215a4dc67c353729` |

Build with `scripts/Build-PC286StartupMedia.py --tool-root TOOLCHAIN --assembler WASM --output FRESH_BUILD`.
Run `scripts/Test-PC286InstalledStartup.py --build FRESH_BUILD --emulator BOOTHUSH --output FRESH_TEST`.
The optional coordinator-only `--disk` argument takes the admitted private DC42
outside Git. Keep that output outside the repository. The default uses generated
media. The generated installer and fixed disk contain no supplied disk data.

As a regression check, the common packager also reproduced the previous
preview's installer floppy, disk and installer stage byte for byte from its
qualified native payload. Its public build command rejected the old session's
source manifest because cursor.c has since changed, as the original provenance
check requires. The comparison above covers packaging only. This work claims
no new release, physical-hardware result or complete ROM service.
