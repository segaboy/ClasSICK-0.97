<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Mac and 286 shared-window experiment

2026-11-14. Can the existing shared window input owner and compositor run in
16-bit 286 code and produce the same seven visible states as the existing Mac
ROM? This is a software test and BootHush observation, not a physical-hardware
observation or release qualification. The [owner direction](mac-286-paired-direction-20261114.md)
and [independent implementation policy](independent-implementation-20261114.md)
govern the current work; older blanket emulator-source exclusions in the former
are superseded by the latter.

## Implementation and scope

On base commit `1dcc86c`, the original C16 fixture links the existing surface,
cursor, pointer, mouse, window, product scene and window scene/input sources.
C99-compatible static assertions and field initialization permit the existing
code to compile with Open Watcom's 16-bit compiler. The fixture uses BIOS key
events (arrows move 16 logical pixels, space toggles held state), initializes
the pointer to `(100,80)`, composes a 512x342 canvas, and presents it centered
in 640x350 EGA. The independent test-only loader now reads at most 22,528 bytes.
It is not a production loader or completed PC system.

The first dirty-presentation attempt left a stale cursor after release: the
shared scene's dirty band can be empty when only the cursor moves. The current
adapter also publishes the old and new cursor footprints. Two deliberate
single-pixel mutations, one inside the canvas and one in the margin, are
rejected by the image comparator. The loader regression covers sizes 6, 7168,
7169, 14848, 14849, 22016, 22017 and 22528, invalid sizes and an injected
third-read failure; local receipt `C:/Repos/.task-temp/pc286-three-track-20261114-b`.

The linked test executable uses four arithmetic modules from the locally
extracted Open Watcom `clibs.lib`: `i4d`, `i4m`, `i8d086`, `i8m086`. The
archive SHA-256 is `3d2d2b56c890e4ac6661cee7b85c9aa33369677c0a8ff1ed70f1a711942094f0`;
the library SHA-256 is `045469f2f05f62f8d190b0e1965abc0a1db774b6ca125672f6a1db341b870e03`.
The archive's Sybase Open Watcom Public License and exact linked modules require
source, compatibility and redistribution review before any release use; the
experimental runtime and binary are not admitted to Git or cleared for release.
The test does not measure stack high-water or prove the complete memory budget.

## Model results and reproduction

Pinned WCC `2026-10-01-Build`; host Clang SHA-256
`93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4`;
BootHush commit `9628c4c`, isolated executable SHA-256
`5d4f610e98cbb8b03476f28929d6e5c0f64b941aafac3c7d5b6ef12c76c7eeb3`.
The 22,518-byte 286 payload SHA-256 is
`5303c8611edc3c48d215ff98854eb62d82a82706ad334dd75b6e36ec02ad161f`,
and test floppy SHA-256 is
`630e6d0dce14e2743b78cf8b4525f5ced1c11fbd83f7c94424b656f3b7edec0e`.
Seven separate cold runs compare state, every canvas pixel and all 48,896
white-margin pixels with a same-source hosted compositor. The disk hash is
unchanged. Source hashes, build/link commands, maps, key schedules, frame
hashes, complete guest results and mutation checks are recorded in
`C:/Repos/.task-temp/pc286-shared-session-20261114-e`.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$bh = 'C:/Repos/.task-temp/boothush-replay-series-20261114-a/boothush.exe'
& $py -B scripts/Test-BootHushPC286SharedSession.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --runtime-library C:/Repos/.task-temp/pc286-compositor-link-20261114-a/clibs.lib `
  --host-cc C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe `
  --emulator $bh --output C:/Repos/.task-temp/NEW-286-SESSION
& $py -B scripts/Test-BootHushMac286Parity.py --emulator $bh `
  --mac-rom C:/ClasSICK/mac-candidate-anchored-20261112-a/classick-live.rom `
  --pc-results C:/Repos/.task-temp/NEW-286-SESSION --output C:/Repos/.task-temp/NEW-PAIRED-CHECK
```

The Mac ROM, built earlier from `0f8d85df`, has SHA-256
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`.
With injected Mac window messages and scheduled 286 keyboard input, the
following observed states match, including all 175,104 normalized pixels:

| Checkpoint | Pointer | Window origin | Pattern | Held | Presses |
|---|---|---|---:|---:|---:|
| initial | 100,80 | 80,70 | 0 | 0 | 0 |
| pressed | 100,80 | 80,70 | 0 | 1 | 1 |
| moved | 132,80 | 112,70 | 0 | 1 | 1 |
| released | 132,64 | 112,54 | 0 | 0 | 1 |
| post-release | 148,64 | 112,54 | 0 | 0 | 1 |
| content | 148,112 | 112,54 | 1 | 1 | 2 |
| content-release | 148,112 | 112,54 | 1 | 0 | 2 |

The Mac reported no loss/omission/fault, 12 mouse samples, 540 modeled steps
and no pending work. Its recorded trace SHA-256 is
`01e3b01a4ead7a8ce2a9b167f1990309be463416b5ecb9156ff2500b68b8cbc9`;
replay reproduced all seven frame bytes, sampled memory and CPU boundaries.
The complete commands, reports, hashes, trace and captures are in
`C:/Repos/.task-temp/mac-pc286-parity-20261114-c`. BootHush's independently
tested replay-capture facility is recorded in
`C:/Repos/boothush/docs/mouse-replay-series-20261114.md`. The port independently
used it for title input, see `C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md`;
game source is not an implementation input here.

The original Mac image and experimental 286 payload are different builds, not
a source-matched candidate pair. The hosted reference shares implementation
and is not by itself independent correctness evidence. The sampled sequence does
not cover all bounds, cancellation, rollover or motion speeds. The separate
host CTest run passed 9/9 scene, cursor and window tests in
`C:/Repos/.task-temp/classick-host-20261114-c`.

Production 286 boot/presentation, runtime admission, full budgets, input and
performance qualification, matching candidate builds, E1-E4 dispositions,
human/physical trials, provenance review, packaging and separate publication
approval remain open. Do not treat this experiment as a completed milestone.
