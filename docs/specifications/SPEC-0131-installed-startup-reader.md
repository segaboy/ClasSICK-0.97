<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0131 - Installed native 286 startup reader

Use the SPEC-0110 formatter, installer and fixed-disk bootstrap to deploy a
small-model 286 payload built from the SPEC-0122 startup-file coordinator and
the SPEC-0113 native disk provider. The installed payload binds drive 0, LBAs
0 through 799. The transport is a read-only 1.2 MB PC image containing the
unchanged 400 KiB logical Macintosh volume beginning at byte zero. The fixed
disk holds only project-authored bootstrap and program data.

On cold boot, prefer the installed fixed disk even if the floppy also contains
a PC boot signature. After mounting the volume, select the two names specified
in its boot header and open their resource maps. Retain both in native storage.
Publish eight 32-bit little-endian status words (result, stage, domain, status,
opened, types, references, warnings). Stop at POST B8 on success, D1 on a
classified startup refusal, or D0 on provider-binding failure.

Qualification uses a disposable blank fixed disk, the guest INSTALL command,
whole-image comparison, independent cold starts, a deliberately invalid
header, a dual-signature generated image, the linked native payload checked
in RAM and aggregate outcomes from the selected unchanged disk. This diagnostic
ends before executing disk-based system software. The physical PC drive/media
interchange and ROM-equivalence gates remain open.
