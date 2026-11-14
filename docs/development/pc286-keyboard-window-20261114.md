<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# 286 keyboard-driven window experiment

The paired milestone needs real shared window semantics, keyboard input and a
512x342 presentation on the native 286. This original test image links the
actual `core/input/pointer.c` and `core/ui/window.c` with a small PC BIOS
keyboard adapter and a deliberately simple geometric EGA renderer. It boots
in BootHush as a bounded floppy experiment. The renderer is **not** the shared
ClasSICK compositor: this is not yet the paired preview, a production OS boot,
matched Mac pixels, an installed system, a physical test or release evidence.

The fixture initially moves the shared pointer to `(100,80)` in the title of
the `(80,70,224,144)` test window. Thereafter BIOS arrow keys move it in 16-pixel
steps; each space **make** toggles an experimental held-button state. This
supports keyboard-driven drag/release and content click in the fixture; it does
not model a held physical mouse button or establish the final input mapping.
The EGA canvas is centered at `(64,4)` in 640x350. The full viewport, including
all margins, is checked against an independently constructed RGB oracle.

The prior full-redraw version required about 3.3 million modeled cycles after
most inputs. The updated fixture redraws a union of old/new cursor and changed
window rectangles, then reconstructs their contents. Seven cold runs with
scheduled set-2 makes/breaks compare all 224,000 RGB pixels and guest-owned state:
initial, title press, rightward drag, release after upward movement, post-release
cursor move, content toggle and its release. The post-release move stopped at
cycle 31,127,193 after input at 31,000,000 (about 127,193 modeled cycles),
versus 34,314,854 in the earlier full-redraw run under the same schedule.
The release window remains `(112,54)` after subsequent cursor movement; the
content pattern changes once to 1. These are guest/model timings, not measured
physical latency. Initial full paint still costs about 3.3 million cycles and
drag/content redraw remains materially more expensive than cursor-only motion.

## Reproduction and identities

Pinned Open Watcom 2026-10-01 16-bit compiler/header hashes are checked by the
runner. It uses `-2 -ms -zl -s -os -zastd=c99` and no default libraries; `-s`
is a diagnostic choice, not a production stack policy. BootHush executable
SHA-256 `1f306c427d1a8c42b9d547b2b25a424e4b4e6a389a52ae5861d030e0fe43bc25`.
The updated 286 payload SHA-256
`f93d4dd3ad22c7d25cbabcc510d8c831fa5c708f571dd3f78d3cacd0d560d2ec`; floppy
SHA-256 `e762f76f863212d1aab72ea56a1c465ba6412bd0fc6b091eb2f631c1382bb075`.
The exact commands, frame hashes, maps, compiler logs and unchanged input
images are in ignored `C:/Repos/.task-temp/pc286-window-keyboard-20261114-c`;
the previous full-redraw comparison is in the sibling `-b` directory.

```powershell
$py = 'C:\ClasSICK\bootstrap-clean-source-verified\.tools\llvm-mingw\llvm-mingw-20260908-ucrt-x86_64\python\bin\python.exe'
& $py -B scripts/Test-BootHushPC286WindowKeyboard.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --emulator C:/Repos/boothush/out/windows/boothush.exe `
  --output C:/Repos/.task-temp/NEW-WINDOW-KEYBOARD-TEST
```

## Larger diagnostic payloads

The original reusable `scripts/pc286_loader.py` extends the previous test-only
two-track floppy loader from 14,848 to 22,016 payload bytes by adding a bounded
third BIOS track read. `Test-BootHushPC286ThreeTrack.py` checks payload sizes 6,
7,168, 7,169, 14,848, 14,849 and 22,016 bytes, every loaded payload byte, four
invalid input refusals, unchanged input disks and an injected failure of the
third read. In the failure case the first 14,848 bytes remain loaded and the
rest remains zero; POST `ee` is reached. More than one cold run is used where
the emulator inspection limit prevents capturing all bytes at one stop. Output
is `C:/Repos/.task-temp/pc286-three-track-20261114-a`. This is test media, not
a production bootloader, hard-disk installation or hardware observation.

```powershell
& $py -B scripts/Test-BootHushPC286ThreeTrack.py `
  --emulator C:/Repos/boothush/out/windows/boothush.exe `
  --output C:/Repos/.task-temp/NEW-THREE-TRACK-TEST
```

The next critical step is a genuine C16-compatible shared compositor and
presentation binding, followed by same-sequence Mac/286 state and pixel
comparisons. BIOS and EGA behavior here are BootHush-model observations;
E1–E4, physical and human testing, budgets, review and release gates remain.

Subsequent work extended the diagnostic loader limit to 22,528 bytes and
exercised the actual shared compositor on the modeled 286, including seven
Mac/286 comparisons. See [shared-window evidence](mac-pc286-shared-parity-20261114.md)
for the current results, runtime licensing hold and remaining release gates.
