<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Shared pointer and window semantics on the 286

The Mac/286 preview requires one shared native pointer and window behavior on
both CPUs. Can the existing pointer and window core execute its drag, release,
post-release and content-toggle semantics as real 16-bit 286 code, and reject
invalid 32-bit input flags when `unsigned int` is 16 bits? This is an original
software and emulator observation, not yet a native graphical session or a
release qualification. The owner-approved independent implementation policy is
recorded in [the current policy](independent-implementation-20261114.md).

## Change and verification

`core/ui/window.c` now casts the legal button and flag masks to `uint32_t`
before complementing them. On a 16-bit `unsigned int`, `~7u` and `~255u`
previously zero-extended their upper bits when applied to the 32-bit input,
allowing some invalid high bits. This change preserves the intended SPEC-0053
contract on existing wider hosts. Host directed tests add high-bit cases and
check state/output refusal atomicity.

`probes/pc286/boot-window-core.c` calls the actual shared `cs_pointer_*` and
`cs_window_*` functions, not a reimplementation. The original diagnostic
initializes a window at `(80,70)`, presses its title at `(100,80)`, moves it
right 32 and up 24 on release, verifies post-release movement leaves it at
`(112,46)`, and toggles content once, including a held input. It checks
invalid high bits and leaves a 12-byte result in a known diagnostic RAM window.
The original loader supplies one near data/stack segment. Numeric pointer
range checks in this fixture do not establish general far-pointer alias safety.

The runner verifies every pinned toolchain member and invokes Open Watcom
2026-10-01 16-bit `-2 -ms -zl -s -os -zastd=c99`, linking with no default
libraries. `-s` is a diagnostic choice, not a production stack policy. It
generates its own 1.2 MiB floppy and executes it headlessly in BootHush:

```powershell
$py = 'C:\ClasSICK\bootstrap-clean-source-verified\.tools\llvm-mingw\llvm-mingw-20260908-ucrt-x86_64\python\bin\python.exe'
& $py -B scripts/Test-BootHushPC286WindowCore.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --emulator C:/Repos/boothush/out/windows/boothush.exe `
  --output C:/Repos/.task-temp/NEW-WINDOW-CORE-TEST
```

Two runs are retained at `C:/Repos/.task-temp/pc286-window-core-20261114-b`
and `-c`. BootHush executable SHA-256
`1f306c427d1a8c42b9d547b2b25a424e4b4e6a389a52ae5861d030e0fe43bc25`.
Baseline 3,667-byte payload SHA-256
`85c89b590eec1f52c756f6bea301dd570d6aa82633be140b6753426e38272bb0`;
floppy SHA-256 `005d60e8a14dd39c95ebc7c209c073586b5131750e6bad38e97cf05a59957cac`.
The baseline reached POST `a9` at modeled cycle 34,020 and returned diagnostic
bytes `57323836702e0100966e005a` (`W286`, window x=112/y=46, pattern=1,
pointer x=150/y=110, no held drag). The input floppy hash remained unchanged.

A wrong-expectation control reached `db`. Separately, the runner compiled a
temporary version with the old narrow masks restored; that actual defect
reached `d9` instead of success, proving this fixture detects the high-flag
failure on the tested compiler. Both controls are generated outside Git and
leave their floppies unchanged. The direct Clang Windows host build passed 233
directed checks and the separate reference passed 10,000 authentic transitions
(80,002 checks). The toolchain executable/header hashes, source hashes, full
commands, maps, logs, media and structured reports are retained in the output
directories; generated media is not tracked.

## Limits and next dependency

This connects actual shared semantics to a 286 CPU model, but inputs are
compiled test calls: no keyboard, cursor, display, live interaction, original
Mac comparison, installed system, physical hardware or whole-OS boot is
established. The 286 input/presentation binding, complete model and normalized
pixel parity, budgets, E1–E4 evidence, physical and human gates, review and
separate release publication remain open. Existing Mac and published release
artifacts are unchanged.
