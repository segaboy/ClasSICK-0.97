<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# BootHush development report: ClasSICK Mac, x64 and 286

**For:** BootHush developers. **Status:** actionable requests and observed
results, not a release qualification. **Date:** 2026-11-13.

ClasSICK uses BootHush exclusively and headlessly on Windows 11 so the owner
keeps control of the desktop. The goal is a paired Mac/x64 release, alongside
a separate native 286 port. Do not substitute another emulator when BootHush
lacks a feature. Please verify the current repository first: this report records
checkout `e386511` and executable SHA-256
`df7736c41cd0c9e83c13fe342ee9f52bbeb3eedb25cbb7d47a5bbef055b3bef9`;
checkout revision does not independently establish executable build provenance.
The owner's supplied capture and x64 guides were admitted as SRC-0604: their
SHA-256 hashes are `b5f84b694c88462f682f8f173a4bee4816fa34b2ecb51af79028e5a35c56945d`
and `fcf433edc3541ee5034065bf74ca30427f108d37c992d0826a809e58d972df87`;
each matched the corresponding local guide and byte-identical intake copies are
outside Git in `C:/ClasSICK/boothush-20261113-c`.

## What is working

- BootHush's documented readiness-triggered Mac input, multiple memory ranges,
  mouse record/replay and headless framebuffer capture are useful to ClasSICK.
- A new own-ROM Mac run captured 68 guest-only frames at five frames per second
  to Y4M and converted them to GIF using BootHush's bundled converter. The run
  completed two synthetic window drags and a click, and ended with pointer
  `(150,110)`, window `(112,46)`, three presses, no reported hard losses or
  omissions, and no pending mouse transitions. A separate replay at the first
  drag checkpoint produced the [losslessly converted guest screenshot](../evidence/screenshots/2026-11-13/boothush-first-drag.png).
  A final replay matched the cycle, both guest RAM ranges and entire framebuffer
  from the video run. All 68 GIF frames decoded locally, with 24 distinct RGB
  frames. The GIF SHA-256 is `f579f44f30409c7a4849f11c631fcc58515c69c716cfef989c73fdea045c7556`;
  the Y4M SHA-256 is `ebfd30ce3e467c9c064e95284ab4c7caf344a9b31ad4a8649dc25506f00179f0`.
  Both are outside Git under `C:/ClasSICK/boothush-20261113-c`. The indexed
  screenshot was independently decoded and matched the original PBM pixels.
  These are
  injected-window messages, not physical or ordinary OS mouse delivery. The
  video is sampled in emulated time; it is not a physical-speed measurement.
- The 286 profile executed an original pinned-Watcom planner/binder diagnostic.
  It split a 32,768-byte bank into 32,767 and one-byte spans, converted the
  actual far pointers, wrote across that split and used a second bank with a
  nonzero offset. An original wrong-expectation control stopped as intended.
  This is not a full OS, production RAM provider or physical hardware proof.
  The baseline reached `A8` at cycle 16,666, the control `B5` at 16,669. The
  whole 4,128-byte payload and both backing regions matched at the same stop;
  both source floppies remained unchanged. The baseline payload SHA-256 is
  `2d359fc274b26162a272b9f6f76e1b89f1cc251f22141342e07ff2e874164ab4`.
  Near-data compilation failed at its explicit far-pointer width guard.
- Current BootHush documentation describes synthetic PS/2 auxiliary-device
  and UEFI allocation/map-key models. These are useful foundations, but they
  are not connected to an executing x64 guest. We did not rerun their component
  suites in this slice.

## P0 — x64 UEFI guest execution

The current x64 media verifier reports `executed:false`; BootHush cannot yet
execute the existing x64 PC candidate. This is a hard blocker for testing the
PC half of the paired release using the owner's chosen emulator. A fresh
preflight on this executable accepted the exact disk/EFI pair and left both
unchanged, but did not execute the guest.

Please deliver a bounded, headless x86-64 machine that can load the actual
unmodified `\EFI\BOOT\BOOTX64.EFI` from the supplied 64 MiB GPT/FAT32 disk,
execute the relevant CPU/privileged operations and a documented UEFI profile,
and report loader, firmware-exit, native execution and faults separately. The
UEFI memory-map key must have a real lifecycle, including stale-key rejection;
an isolated host-side model or unconditional success is not an EFI boot.

The guest also needs a mapped GOP framebuffer (1024x768 first; logical
512x342 centered), ACPI PM timer, and its selected 8042 keyboard/PS/2 mouse
profile. The existing optional mouse component must be wired to the machine,
ordered controller bytes and interrupt/input paths. Provide readiness-relative
headless device input, guest-owned multi-range diagnostics, full framebuffer
capture and reproducible input/replay with clearly stated timing limitations.
The guest's 160-byte `CWP1` record is at dynamic `trace_base + 2048`; provide a
documented way to locate it instead of guessing an address. Serial is not needed
for the first window milestone. Do not claim physical, host-GUI or real-firmware
qualification from synthetic guest input.

Original candidate identities:

| Input | Path | SHA-256 |
| --- | --- | --- |
| 64 MiB raw disk | `C:/ClasSICK/pc-fast-drag-20261112-a/window-drawing.img` | `80c279a69264cb07f2042bf14503c0687fa32e71057ee1196fc8174afbeeae4f` |
| 207,360-byte EFI | `C:/ClasSICK/pc-fast-drag-20261112-a/BOOTX64.EFI` | `df284097539ed6b7c90700cb27589900b20d2e4c7574b7637ba5f387abba9fe9` |

Acceptance: independently generated CPU/firmware tests and negative controls;
then boot the exact disk, observe guest milestones, record firmware/model and
input identities, capture pixels and `CWP1`, and run neutral-release,
movement, press/drag/release and post-release movement. Report tests actually
run. Detailed interfaces and staged acceptance are in the local handoff
`C:/ClasSICK/boothush-x64-enablement-2026-11-12.md` and project specs
SPEC-0006/0007/0010/0012/0040/0062. The earlier WHPX capability query returned
`0x80070005` in one execution environment; it does not establish host-wide
unavailability. Do not change Windows settings or silently invoke an emulator.

## P0 — correlate Mac mouse omissions with device events

The existing `BHMACMOUSE2` witness shows SCC pending/acknowledgement, phase and
VIA/SCC reads. In a prior identical-ROM test at 2,048 edge cycles, a modeled
phase changed while SCC level 2 was pending; the guest later reported 29
omissions. At 4,096 cycles the same four directions passed. Neither observation
attributes fault to BootHush or ClasSICK, and CPU instruction-boundary cycle
stamps are not physical bus timing.

Please add an opt-in, bounded, address-filtered guest-RAM **write** witness
correlated in the same ordered stream with the existing mouse events. Include
address, access width and bytes/value, byte-order convention, cycle, sequence,
available instruction context, capacity and dropped/coverage counts. Define
overlapping and partial-access semantics, reject invalid filters, avoid MMIO
side effects and guest changes, and show identical guest/frame state with and
without the witness. Existing format/CLI behavior must remain when disabled.

For the exact original ROM
`C:/ClasSICK/mac-candidate-anchored-20261112-a/classick-live.rom`, SHA-256
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`,
the guest's reported omission word is at `0xc1c`. This can be a later published
copy rather than the original decision; distinguish those cases. Compare
2,048 and 4,096 cycle traces; do not infer unobserved direction or claim a fix
from correlation alone. A separately validated timing model is a later need.

## P1 — 286 development support

Preserve the currently useful 6 MHz 286 model, original floppy boot,
multi-range snapshots and bounded memory/I/O witness. The new diagnostic is
`scripts/Test-BootHushPC286SpanBuild.py` in ClasSICK; it checks `A8` success,
`B5` wrong-expectation failure and identical original input media. The two
matching final runs are in `C:/ClasSICK/boothush-20261113-c`.

For the next stage, document exact supported BIOS disk calls, CHS geometry,
memory map, EGA modes, keyboard scheduling, persistent disk-copy semantics and
witness limitations needed to develop a 286 disk-installed system. Provide
actionable failures and deterministic output; add only missing features shown
by concrete ClasSICK diagnostics. Future system installation needs writable
private hard-disk images without modifying originals, booting the resulting
copy in a separate session, and observable error paths. Do not represent
BootHush's existing installation fixture as ClasSICK installation.

## P2 — capture and other open features

Current headless Y4M recording and bundled conversion worked for the Mac run.
Retain explicit emulated-time sampling, frame count/truncation and exact-stop
still capture. Preserve original input media and output-create-new behavior.
Externally distributing a bundled FFmpeg remains subject to its separate
license/source-obligation review; ClasSICK's test does not approve distribution.
Writable Mac floppy support remains a future system-disk requirement. Neither
it nor EGA text rendering is a prerequisite for the mouse experiment here.

## Handoff requested

For each milestone provide commit, executable and firmware/model identities,
dependency origin/license, commands, configuration, actual passed and failed
tests, negative controls, witness truncation/drop counts and remaining gaps.
Keep proposed cases separate from executed tests. Use original fixtures and
original ClasSICK media, keep inputs immutable, and keep the desktop available.
No Apple/Microsoft implementation, reference media or emulator implementation
is to be imported into ClasSICK. Emulator observations do not close historical,
physical, human-input or release gates.
