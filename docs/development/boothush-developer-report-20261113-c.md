<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# BootHush developer report: paired release and 286 memory chain

**Date:** 2026-11-13. This is an evidence receipt and developer handoff,
not a release or physical-hardware qualification. It updates the
[prior report](boothush-developer-report-20261113-b.md). The owner directs
headless BootHush-only testing, leaving the Windows desktop available.

## Tested configuration

BootHush checkout `d736148` has executable SHA-256
`c3f07405a2985a7fedc9604ff6fe5208586068e9130de2201dd9d42e703957c4`.
The same binary was used in the prior report; a checkout revision alone does
not prove how an executable was built. Original generated floppy media, full
logs, guest captures and reports are outside Git in
`C:/ClasSICK/boothush-20261113-e`. No other emulator or reference system was
used. Previous uncommitted work and the earlier research originals are retained.

## 286: planning through capability in one target run

The new original [diagnostic](../../probes/pc286/boot-memory-chain.c) runs the
actual numeric bank planner, far-span builder, binding validator and capability
borrower together as pinned Open Watcom 16-bit `-2 -mc` code on the BootHush
286. The planner splits a 32,768-byte bank at the 32,767-byte span limit and
constructs a second bank with a nonzero far-pointer offset. A writable capability
crosses the first span, its one-byte tail and the second bank in three borrows;
the guest checks a subsequent read-only borrow and staleness after an owner
generation advance.

Two independently compiled and executed runs matched exactly. The 7,216-byte
baseline reached POST `A9` at cycle 62,997. The two deliberately wrong fixture
expectations stopped at `C6` (permissions) and `C9` (staleness). Each stop's
entire loaded payload and both physical backing windows matched expected bytes
in one multi-range inspection; each input image remained unchanged. A near-data
build was rejected at the far-pointer guard. These are wrong-expectation
controls, not mutations of the underlying implementation.

The original diagnostic loader now reads **two floppy tracks** when needed,
up to 29 sectors (14,848 bytes). The
[independent loader check](../../scripts/Test-BootHushPC286Loader.py) booted
original generated payloads of 6, 7,168, 7,169 and 14,848 bytes; every loaded
byte matched, all images remained unchanged, and four invalid input cases were
rejected. The existing smaller bank-plan, span-access and default span-builder
diagnostics also passed. The default span-builder report matched the prior
same-binary result exactly. These are BootHush diagnostic boots, not a
production 286 OS, a hard-disk installer, real-memory ownership or physical
hardware proof.

To reproduce the integrated capability test with a fresh output directory:

```powershell
python -B C:/Repos/ClasSICK-0.97/scripts/Test-BootHushPC286SpanBuild.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --emulator C:/Repos/boothush/out/windows/boothush.exe `
  --capability-chain --output C:/ClasSICK/NEW-CHAIN-TEST
```

### 286 emulator requests

BootHush already documents floppy boot, bounded snapshots, 286 witness, EGA,
keyboard input and private hard-disk copies. Please preserve these interfaces.
An opt-in, deterministic **selected disk-read failure** would help verify both
failure branches of our two-track loader: choose the operation/CHS or ordinal,
return a documented BIOS-visible error, preserve input media, and expose the
injection and outcome in the result. Test first-track and second-track failures,
normal reads with injection disabled, and no-clobber behavior. This is a
requested testing facility, not a claim that ordinary BootHush reads are broken.
The current loader's second-track error branch has not been independently
forced by this work. Future hard-disk installation and EGA/keyboard needs should
be driven by original ClasSICK guest diagnostics, not inferred from the
emulator's existing installation fixture.

## Mac: internal omissions in all four directions

The prior report established, using the matching ROM/link map and a
cross-compiler layout assertion, that this exact ROM's internal omission
counter is at `0xb80`. This is not a general Macintosh address. Fresh headless
2,048-cycle tests traced that address during separate +X, -X, +Y and -Y modeled
movements. In every case the internal low-word writes counted 1 through 29,
the published count was 29, and the final pointer moved one pixel opposite
the requested direction. All four traces covered the requested interval with
zero dropped rows; for each, witness-enabled and witness-disabled guest result
and full framebuffer matched. A previous same-ROM, same-executable 4,096-cycle
comparison reached the requested position with zero omissions.

The input is synthetic, and timing, SCC latch and CPU boundaries are BootHush
model observations. This is not a diagnosis of which component is at fault or
evidence of physical Mac timing. ClasSICK's existing `drop()` deliberately
omits ambiguous movement; inferring unobserved edges would be unsafe. The Mac
fast-input blocker remains open.

### Mac emulator requests

The filtered RAM-write witness is now implemented and working; do not repeat
that request. Please prioritize independently validating the modeled
interrupt/phase timing and SCC latch/acknowledgement behavior at the relevant
boundaries. An optional, bounded witness of interrupt mask/CPU state alongside
the existing phase, register access and RAM events may help explain latency;
first determine whether current outputs already answer the question. Retain
event ordering, coverage/drop reporting and noninterference comparisons. State
clearly what remains unverified against physical hardware. Do not change the
model simply to make the guest pass.

The current headless Mac drag run again passed its paced synthetic sequence.
BootHush produced a 68-frame Y4M recording, a converted GIF and a lossless final
guest screenshot; replay matched the final cycle, two inspected RAM ranges and
the whole framebuffer. The animation is outside Git at
`C:/ClasSICK/boothush-20261113-e/mac-drag.gif` and has SHA-256
`f579f44f30409c7a4849f11c631fcc58515c69c716cfef989c73fdea045c7556`.
The final screenshot repeats the already indexed [guest image](../evidence/screenshots/2026-11-12/boothush-headless-drag.png).
This is synthetic input, not human or physical qualification.

## x64: paired-release blocker

BootHush still cannot execute the x64 CPU or boot UEFI. Its read-only media
verification and host-side EFI mapping are useful, but explicitly report
`executed:false`; the separate PS/2 and memory-map components are not wired to
an executing x64 guest. The same binary previously mapped the exact disk at
preferred and relocated bases and rejected an unaligned base; those existing
results are reused, not presented as a new boot.

Please deliver a bounded headless x86-64 machine that boots the actual
`\EFI\BOOT\BOOTX64.EFI` from the original GPT/FAT32 disk, with the UEFI
services and memory-map/`ExitBootServices` behavior exercised by ClasSICK,
GOP framebuffer, ACPI timer, 8042/PS/2 keyboard and mouse, scheduled headless
input, guest-owned diagnostic inspection and honest replay/fault reporting.
Connect and test the existing components in a real guest; do not label an
isolated model or host-side mapping a boot. The exact candidate identities,
negative cases and staged acceptance are in the
[prior report](boothush-developer-report-20261113-b.md) and
`C:/ClasSICK/boothush-x64-enablement-2026-11-12.md`.

## Handoff requested

For each emulator milestone, report commit and executable/firmware identities,
exact commands, actual passed/failed/skipped tests, negative controls,
trace coverage/drop counts, and any source/license decisions. Use original
fixtures and inputs, keep media immutable by default, and do not require other
emulators or desktop interaction. Do not import emulator implementation,
Apple/Microsoft code or reference media into ClasSICK. No paired release was
published or qualified by this work.
