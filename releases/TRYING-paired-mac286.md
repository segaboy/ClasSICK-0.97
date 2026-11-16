# Trying the Mac / 286 interactive preview

This is the prepared review package for ClasSICK ROM 0.4.0-alpha.1 and
ClasSICK PC Preview 0.2.0-alpha.1. These versions are proposed, unpublished.
Read `review.json` and `REVIEW-PENDING.txt` before treating it as a release.
It shows one original window, cursor, pattern and counter. No Apple ROM,
System software or Microsoft executable is needed. System remains unreleased.

The Mac image is a 64 KiB standalone ROM for the modeled 128K/68000 profile.
The 286 image is an entire 21,411,840-byte disk for the modeled PC/AT 5170,
6 MHz, 640 KiB, EGA and 615/4/17 geometry. It contains no filesystem or
partition table. The floppy is a 1.2 MB installer for that exact disk profile.
Physical machines and general BIOS/storage compatibility are unqualified.

## Bounded headless runs

Use the already identified local BootHush active-profile executable. Its
SHA-256 is `8860227d991a6c6d252815019c2458a1b0af3cae49306ce43e7fdaff5dd04e3b`.
No emulator/compiler/FFmpeg binary is bundled. These commands need no WSL,
desktop window or other emulator. In PowerShell, set `$preview` to the
unpacked directory and `$bh` to that executable. Use new output filenames.

```powershell
python -B "$preview/verify-package.py" --package $preview
& $bh --machine mac128k --rom "$preview/mac/classick.rom" --cycles 100000000 --timeout-ms 20000 --until 0xcac:4:16 --frame "$preview/mac-ready.pbm"
& $bh --machine pc286 --hard-disk "$preview/pc286/hard-disk.img" --cycles 10000000 --timeout-ms 20000 --until-post 0xb0 --frame "$preview/pc-ready.ppm"
```

Run package verification before creating extra files inside the unpacked
directory: the verifier deliberately checks exact package membership.
The prerecorded `media/mac-window.mp4` is continuous guest-time sampling.
`media/paired-checkpoints.gif` compares eight completed states at one second
each. Platform clocks and input schedules differ; it is not a timing race.

Mac input uses mouse motion and its primary button: press the title bar to
drag, release to stop; press content to toggle its pattern. The tested mouse
bridge uses DCD invert mask 3, VBL guard 4,096 and edge spacing 2,048 or 4,096.
These polarity/timing choices are model observations, not hardware settings.
The 286 keyboard adapter moves 16 pixels per arrow make; holding Space holds
the primary button, releasing Space releases it. Repeated Space makes do
not create additional clicks. Escape is not an exit command. Bounded command
runs end automatically. Actual host GUI/human input remains unqualified.

## Installer and failure codes

All qualification installs used newly generated zero-filled image files.
The installer refuses any nonzero target byte, even beyond its own payload.
It checks its staged CRC, writes/verifies the payload and commits the boot
sector last. Type `INSTALL` then Enter; another input cancels with no writes.
The prepared hard disk is exactly the successfully installed image, so it
can be used directly for the bounded boot above. Do not treat this fixed-
profile whole-disk preview as a partition-aware installer for an existing OS.

To reproduce installation and its failure tests against disposable images,
run `scripts/Test-PC286PreviewMedia.py` from the included source. The exact
command, keyboard scan schedule and generated image identities are retained
in `evidence/pc286-qualification.json`. The script requires the source-matched
build directories below; it does not accept or open physical host disks.

| Path | Code | Meaning |
| --- | --- | --- |
| Boot | E1 / E2 / E3 | Profile / exhausted read / payload CRC failure |
| Install | A0 / AC / AD | Prompt / verified success / cancellation |
| Install | E1 / E2 / E3 | Profile / staged CRC / preflight read failure |
| Install | E4 / E5 / E6 | Occupied target / exhausted write / verification failure |

CRC detects accidental damage; the SHA-256 inventory detects changed package
files. Neither is a publisher signature or an independent provenance review.

## Rebuilding

Unpack `source.zip`. `review.json` identifies its exact Git revision and the
source-to-build comparison; unrelated workstation edits are excluded. Build
manifests and qualification reports retain tool versions, hashes and commands.
Use the pinned Windows-native m68k tools and Open Watcom/assembler from those
manifests, plus the existing Windows Clang for hosted checks.

```text
python -B scripts/Build-LoadedAppROM.py --windows-native --tool-root M68K_BIN --window-drawing --window-fast-collect --window-short-collect --window-buffered-collect --window-loss-counts --dcd-invert3 --output NEW_MAC_BUILD
python -B scripts/Test-BootHushPC286SharedSession.py --tool-root WATCOM_ROOT --assembler WASM --host-cc CLANG --emulator BOOTHUSH --output NEW_PC_PAYLOAD --natural-start --raw-keyboard --verify-crc16 --verify-geometry --retry-reads --hard-disk
python -B scripts/Build-PC286PreviewMedia.py --session NEW_PC_PAYLOAD --assembler WASM --linker WLINK --output NEW_PC_MEDIA
python -B font/make_diagnostic_font.py --output NEW_FONT
python -B scripts/Test-PC286PreviewMedia.py --emulator BOOTHUSH --media NEW_PC_MEDIA --session NEW_PC_PAYLOAD --font NEW_FONT/diagnostic-8x14.bin --output NEW_QUALIFICATION
```

The font generator lives in the package's `font` directory, not the unpacked
ClasSICK source tree. Continue with the two parity commands, edge harness and
capture command recorded in the included reports. Their source checks refuse
stale builds; regenerate prerequisite reports after editing inputs.

Project code, documentation and original art are GPL-3.0-or-later; see COPYING
and COPYRIGHT.md. Original font source and its own notice are included.
Toolchain/emulator licenses remain their own. The optional `port-media` folder
shows a separate, incomplete game prototype with original placeholder art.
It is not a ClasSICK implementation input or a delivered game ROM.
