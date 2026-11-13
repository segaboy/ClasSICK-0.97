<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# BootHush developer report: new Mac witness and x64 mapper

**Date:** 2026-11-13. **Audience:** BootHush developers. This is a developer
handoff and evidence receipt, not a release or physical-hardware qualification.
It updates the [earlier report](boothush-developer-report-20261113.md): the
requested filtered Mac RAM-write witness and host-side x64 image mapping now
exist. ClasSICK uses BootHush headlessly, without other emulators or taking
control of the Windows desktop.

## Tested identities and supplied guides

- BootHush checkout `2f8370a`; tested executable SHA-256
  `c3f07405a2985a7fedc9604ff6fe5208586068e9130de2201dd9d42e703957c4`.
  Checkout and executable are independently identified; the checkout alone
  does not prove binary build provenance.
- The owner's `mac-mouse-witness.md` SHA-256
  `7fe91a8bc458eb8481022bd8b333f75744c763ec552b59255a5bbe23a2e904a5` and
  `classick-x64.md` SHA-256
  `a762a8cb1a4b79bfcb9f0fc08c29396009d9d42bef86833a0c0ae83f89f8aff4`
  match the local published docs. Byte-identical intake copies and detailed
  results remain outside Git at `C:/ClasSICK/boothush-20261113-d`.
- Original Mac ROM SHA-256
  `faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`.
  The independently checked retained build copy has the same bytes. Its own
  link map places `motion` at `0xb0c`, size `0xa8`. A separately compiled
  assertion using the original pinned 68000 compiler and hash-matched
  `motion.h`/`inbox.h`, with the build's fast/short/loss-count flags,
  confirmed `motion_drops` offset `0x74` and total size `0xa8`: the internal
  counter is at `0xb80` for this ROM, not a general Macintosh address.
- Provenance caveat: two files in the retained scratch source directory
  (`window-entry.c`, `fast-collect.inc`) no longer match that build manifest.
  The corresponding current project originals match the recorded hashes;
  the two headers used for the layout proof also match. Do not treat the
  entire scratch source directory as a byte-identical build snapshot.

## New finding: actual Mac omission updates

Using `--mouse-witness-detail 1 --mouse-witness-memory 0xb80:160`, separate
2,048- and 4,096-cycle edge cases traced the internal omission counter and its
published copy together with mouse, SCC, VIA and interrupt events. Traced and
untraced results and full framebuffer bytes matched within each case. Both
witnesses covered cycles 60,000,000–65,000,000 with zero dropped rows.

- At **2,048 cycles**, the guest ended at `(255,171)`. The internal counter's
  low word at `0xb82` was written with values 1 through 29, and the final
  published copy was 29. Its first nonzero internal write was cycle 60,018,518;
  the first nonzero low-word write to the published copy at `0xc1e` was 29 at
  cycle 60,147,638. The latter is a foreground publication, not the first
  omission. The CPU presents longword writes as separate 16-bit transactions;
  these timings are model instruction boundaries, not physical bus timing.
- At **4,096 cycles**, the guest reached `(288,171)` and both counters remained
  zero; no nonzero write to either counter was recorded in the selected window.

The internal field is incremented by ClasSICK's own `drop()` path; the
published trace is copied by the foreground. The sequence shows a guest
decision occurring during the modeled fast input, but does not by itself
distinguish a guest defect from model timing or prove physical behavior. A
prior independent counterexample shows that missing edge direction cannot be
safely guessed from an ambiguous late sample. No guest mouse behavior was
changed and the fast-input blocker remains open.

The current BootHush RAM witness fulfilled the prior request; do not implement
it again. A useful next emulator investigation is an **independently validated
Mac CPU/device timing and interrupt-latency model**, with explicit instruction
and bus timing granularity, SCC latch/acknowledgement behavior, and reproducible
tests for phase changes before acceptance and during service. An opt-in,
bounded interrupt-mask/CPU-state observation, correlated with the existing
witness and with drop/coverage reporting, could help explain the latency. First
check whether the current interface already supplies enough information. Do not
change the model merely to make this ROM pass, and label any physical timing
assumptions and gaps.

## x64 progress and remaining release blocker

BootHush's `--map-x64-media` now reads and relocates the actual disk payload in
host memory. Our relocated-base run at `0x20000000` returned the documented
mapped SHA-256 `1511747e91c9533bd48d365d9e9a32fcf02ad1ea2d90c9177b0b1810874cf0e7`,
221,184 bytes and three relocation entries. A preferred-base mapping also
succeeded; an unaligned base was rejected. The original disk hash stayed
unchanged. Both successes explicitly reported `executed:false`; no guest
CPU, firmware or page protection was exercised.

The remaining P0 is a real, bounded headless x86-64 execution environment:
guest CPU and memory, an eligible UEFI implementation, loading the EFI from
the actual supplied disk, its needed EFI ABI/services and valid memory-map
key/`ExitBootServices` lifecycle, GOP framebuffer, ACPI PM timer and wired
8042/PS/2 keyboard and mouse. Connect the existing PS/2 and map-key components;
do not describe isolated component tests as guest boot. Add readiness-triggered
device input, guest-owned state/`CWP1` inspection, framebuffer capture, clear
faults and replay with honestly measured determinism. The dynamically
allocated `CWP1` record is at `trace_base + 2048`; do not guess its address.

Original disk and EFI inputs:

| Input | Local path | SHA-256 |
| --- | --- | --- |
| 64 MiB disk | `C:/ClasSICK/pc-fast-drag-20261112-a/window-drawing.img` | `80c279a69264cb07f2042bf14503c0687fa32e71057ee1196fc8174afbeeae4f` |
| EFI payload | `C:/ClasSICK/pc-fast-drag-20261112-a/BOOTX64.EFI` | `df284097539ed6b7c90700cb27589900b20d2e4c7574b7637ba5f387abba9fe9` |

The first real milestone should run synthetic CPU/firmware cases and negative
controls, then boot this exact disk, observe actual native stages, and test
neutral/released mouse state, drag, release and later movement. Preserve disk
inputs and keep the desktop free. The full x64 handoff is
`C:/ClasSICK/boothush-x64-enablement-2026-11-12.md`.

## 286 progress and next emulator needs

The original 16-bit [span diagnostic](../../scripts/Test-BootHushPC286SpanBuild.py)
now takes the actual planner output through the target far-pointer binder and
the checked access layer in **one** boot. A four-byte big-endian write/read
crosses both the 32,767-byte span split and the boundary into a second bank.
The guest checks preflight refusal with no partial write, stale epoch with an
unchanged output, and odd-address refusal. Two independently built runs were
identical. Baseline `A8` occurred at cycle 57,121; deliberately wrong fixture
expectations produced `B5`, `B7` and `B8`. Complete 5,883-byte payloads and
both backing regions matched at each stop; input disks were unchanged. The
near-data compiler profile was rejected by its explicit pointer-size guard.
Those controls alter fixture expectations, not the implementation.

This is real execution of an original 16-bit diagnostic in the 286 emulator,
not a production memory provider, system installer, OS boot, physical 286 or
Mac application support. Continue maintaining the existing BIOS, fixed-disk
private-copy/`--disk-out`, keyboard, EGA and 286 witness interfaces. Their
documented installation fixture is not a ClasSICK installation. Future
requirements should be driven by our own installer/graphics diagnostics;
please report exact unsupported BIOS/device cases rather than guessing.

## Capture and response requested

The new build recorded another 68-frame headless Mac Y4M and converted it with
the bundled converter to a GIF SHA-256
`f579f44f30409c7a4849f11c631fcc58515c69c716cfef989c73fdea045c7556`.
Replay matched the same cycle, both inspected RAM ranges and whole framebuffer.
The final PNG pixels match the [previous indexed screenshot](../evidence/screenshots/2026-11-12/boothush-headless-drag.png),
so no duplicate screenshot was added. Video timing is emulated, input is
synthetic, and the animation remains outside Git. Keep capture and conversion
headless, with clear frame/truncation and source/rights metadata.

Please report each implemented milestone with commit, executable and firmware
identities, dependency licenses, exact commands, actual passed/failed/skipped
tests, negative controls, trace drop counts and remaining gaps. Do not treat
host-side mapping as execution, emulator tests as physical qualification, or
a successful paced synthetic drag as proof that the fast Mac mouse is fixed.
Do not require other emulators or import Apple/Microsoft implementation,
reference media or emulator implementation into ClasSICK.
