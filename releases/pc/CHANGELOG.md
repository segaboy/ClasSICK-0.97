# ClasSICK native PC preview release history

## 0.2.0-alpha.1 - 2026-11-17

- Paired native Mac68000/286 interactive-window preview, faster byte-row cursor
  composition, original click/drag behavior and exact shared pixel checks.
- 286 whole-disk boot/installer with occupied-target refusal, CRC, retries and
  boot-sector-last installation. Mac uses qualified 4096-cycle input profile.
- [Notes](0.2.0-alpha.1.md); human response and physical/compatibility limitations remain explicit.

## 0.1.0-alpha.1 - 2026-10-19

- First published experimental native x86-64 UEFI preview,paired with ROM0.3.0-alpha.1. Shared portable C draws the monochrome512x342 branded ring/rails scene,centered in the tested1024x768 framebuffer.
- Mouse-button and Space events update the counter;continuous preview ignores the normal60-second deadline and Escape. End through the host. Prior timed diagnostics remain unchanged.
- Headless injected input accepts38 mouse+5 Space through guest185;scene preservation and same-counter Mac parity pass. Ordinary human capture/focus remains unqualified after intermittent clicks,and one GUI preboot startup failed. These are disclosed preview limitations,not passing human/GUI tests.
- [Notes](0.1.0-alpha.1.md),[setup](TRYING-0.1.0-alpha.1.md),[frozen candidate](0.1.0-alpha.1.json),[publication receipt](0.1.0-alpha.1-publication.json). EFI,demo/raw-disk ZIP,complete source and checksums verified after publication.
- System1 booting,physical PCs,B2 and complete edition/native external-app qualifications remain open. Preview publication does not close them.

## Prior planning

The first candidate was tracked in [paired-native-preview-1](../paired-native-preview-1.md). ROM and PC component versions can differ while matching native capabilities;the PC OS path is native x86,with the68k interpreter reserved for legacy compatibility.
