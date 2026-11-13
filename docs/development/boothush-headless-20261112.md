# Headless BootHush Mac and 286 work — 2026-11-12

The owner selected headless BootHush for Mac and 286 development, keeping the
Windows desktop available. This work used only original ClasSICK code, ROM,
generated test media and BootHush's documented black-box interfaces. No other
emulator, reference system, Apple or Microsoft implementation, or emulator
implementation source was used. Native x64 PC development remains paused.
Prior evidence is preserved; no physical, human, historical System 1 or release
qualification is implied.

## Inputs

- BootHush checkout revision `1ef5b09` and executable SHA-256
  `5c6447bfca0141c9da2648c574bd37ba42ef160a6433bd44217a27fa584e3a23`.
  The checkout revision does not independently prove binary build provenance.
- Original Mac candidate SHA-256
  `faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`,
  built from `0f8d85df5b6fe21508c22594e0dc20923d4eebe6`.
- User-supplied BootHush README SHA-256
  `f4501c3608a4c8bb17780241a39b8613d58824dde2a5c9f3a87f44050457935c`;
  mouse runbook SHA-256
  `b636a9c4b95936bcb86982b0eddb868ed789bbe5a941aa05ccb085a2714ca15d`.
  Both matched their local checkout documentation and have byte-identical
  intake copies in `C:/ClasSICK/boothush-headless-20261112-a`.
- 286 compiler: the previously pinned Open Watcom 16-bit toolchain. The new
  runners verify the existing toolchain lock before use. All disk images and
  detailed outputs stay outside Git in the workspace above.

## Mac release path: repeatable headless drags

[Test-BootHushMacWindow.py](../../scripts/Test-BootHushMacWindow.py) now checks
four separate original-ROM cold boots using BootHush's invisible owned-window
message path. Every checkpoint reported zero pending steps, hard losses,
omissions and guest faults; the button was released:

| Checkpoint | Pointer | Window | Presses | Pattern |
| --- | --- | --- | --- | --- |
| First drag and release | `(132,80)` | `(112,70)` | 1 | 0 |
| Movement after release | `(148,80)` | `(112,70)` | 1 | 0 |
| Second drag and release | `(148,56)` | `(112,46)` | 2 | 0 |
| Content click | `(150,110)` | `(112,46)` | 3 | 1 |

The final run reported 12 accepted samples and 570 delivered steps. Replaying
its recording matched the final cycle, all 1,024 inspected bytes and the entire
framebuffer. The [guest-only final frame](../evidence/screenshots/2026-11-12/boothush-headless-drag.png)
was converted losslessly from PBM and reviewed. BootHush's own `mac_live` CTest
also passed with the required ROM environment variable set and its explicit
fixture success message, including zero losses and omissions. These results
cover injected window messages, not ordinary Windows mouse delivery or a person.

The earlier [mouse timing probe](../../scripts/Probe-BootHushMacMouse.py) was
rerun with this BootHush executable. Its 8,192- and 4,096-cycle guarded cases
passed; the 2,048-cycle case again ended at `(255,171)` instead of `(288,171)`,
with 29 omitted passes, and the 8,192-cycle no-guard case ended at `(287,171)`
with two omissions. The probe's exit 1 correctly reports these failures. The
Mac input problem is not fixed and no release has been made.

Two older timer-poll experimental ROMs were also compared at 8,192, 4,096 and
2,048 cycles. An initial 65-million-cycle attempt injected movement before
those ROMs reached ready and is invalid as a movement comparison. A later
readiness check reached stage 16 at cycle 77,711,848; reruns scheduled input at
90 million cycles. Both timer profiles remained near the center or reported
losses in the six bounded cases. Their earlier results and identities are in
`timer-profile-comparison.json` and `timer-late-comparison.json` in the private
workspace. These old profiles are not substituted for the current candidate.

## 286 path: actual 16-bit guest execution

[Test-BootHushPC286BankPlan.py](../../scripts/Test-BootHushPC286BankPlan.py)
compiled the existing bank planner and a separate original diagnostic using
the pinned 16-bit `-2 -ms` profile. An original boot sector used BootHush's
documented built-in firmware to load the linked payload from a generated,
read-only 1.2 MB floppy. The emulator's 286 executed the cross-bank plan and
the odd-address refusal; the normal case reached POST `A5`. The entire loaded
payload matched the original bytes, and the floppy hash was unchanged. Two
deliberately wrong expectations independently reached their expected `E3` and
`E4` markers. Three cases passed, with the normal result at 8,727 emulated
cycles. This is an original diagnostic, not a ClasSICK system boot.

[Test-BootHushPC286SpanAccess.py](../../scripts/Test-BootHushPC286SpanAccess.py)
separately compiled the existing bank planner, catalog, binding and access
modules with the pinned `-2 -mc` compact model. Its original
[diagnostic](../../probes/pc286/boot-span-access.c) used fixed far windows at
`2000:0000` and `3000:0000`. It wrote and read `AABBCCDD` across the two banks,
checked neighbors, rejected stale and odd accesses, and verified that a missing
late binding caused no partial write. The normal case reached POST `A6` at
48,849 emulated cycles. Three controls with deliberately wrong span, epoch and
binding expectations reached `E6`, `E8` and `EC`, respectively.

For each of the four cases, three deterministic cold boots separately inspected
the code prefix and each six-byte bank; cycles and program counters matched
within that case and input floppy hashes stayed unchanged. The normal 4,683-byte
payload exceeded BootHush's 4,096-byte inspection limit, so only its first
4,096 loaded bytes were externally compared. The guest diagnostic itself
checked both banks within the same boot. These are fixed emulator test windows,
not a production segmented provider, a whole-ABI proof, a full 286 OS, physical
hardware result or performance measurement. Reports: `mac-verified/report.json`,
`pc286-verified/report.json` and `pc286-span-final/report.json` under the private
workspace above. Wrong-ROM and existing-output refusal checks passed.

## BootHush developer feedback

The previous request for a usable `--help`, a longer live-session limit and
headless logical primary-button input has been addressed in the supplied
documentation/build. `--help` and `--version` were observed, and the new
headless input path was actually exercised. The documented extended maximum was
not tested for an hour. The README now explicitly says its default edge spacing
and VBL guard are empirical delivery controls, not physical mouse measurements.

### One inspection range per run — verified limitation

BootHush currently rejects two `--inspect` options with exit 2 and JSON error
`unknown or repeated option`. After running the 286 span diagnostic above, this
reproduces the limitation with its generated original floppy:

```powershell
& 'C:\Repos\boothush\out\windows\boothush.exe' --machine pc286 `
  --floppy 'C:\ClasSICK\boothush-headless-20261112-a\pc286-span-final\baseline.img' `
  --cycles 8000000 --timeout-ms 20000 --until-post 0xa6 `
  --inspect 0x20000:6 --inspect 0x30000:6
```

For segmented memory and larger payloads, add an optional bounded list of
labelled, noncontiguous memory ranges captured at **the same stop**, with
addresses and bytes returned in a JSON array. Preserve the existing single
`memory` response for existing callers. Document and enforce per-range and
total limits, out-of-range refusal, overlaps and side-effect-free access. The
current three-reboot workaround is reproducible but cannot prove that separate
reads came from one atomic snapshot. This is a missing capability, not a claim
that documented single-range behavior is buggy.

### Other capabilities to consider

- A bounded, opt-in 286 execution/device witness with cycle-stamped instruction
  locations, selected physical memory accesses, I/O and IRQ events, filters,
  explicit dropped-row counts and no guest-visible side effects. The documented
  Mac mouse witness is not a 286 trace. This would help diagnose a failed far
  write or boot without changing the guest merely to emit more POST codes.
- Optional Macintosh 400 KiB floppy writes using private copy-on-write media
  and explicit save-to-new-image, with original media unchanged, format and
  checksum handling, write-failure tests and original generated fixtures.
  The README says Mac floppy writes remain open; they are not required for the
  mouse checks above, but will matter for a writable system-disk workflow.
- Optional EGA text-mode rendering using an original or explicitly supplied
  rights-cleared font. The documented text-mode capture currently renders black;
  this would make boot diagnostics visible without importing firmware assets.

No new confirmed BootHush correctness bug was established. The live mouse
timing failure is observed in our guest under this model and has not been
attributed to either implementation or physical hardware. The developer should
keep direct scheduled input, injected-window messages, real host input and
physical observations distinctly labelled.
