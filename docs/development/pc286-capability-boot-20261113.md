<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# 286 capability execution in BootHush — 2026-11-13

The owner requested headless BootHush-only progress toward the paired release
and substantial 286 progress. The previously adopted original capability and
borrow model (SRC-0574 / SPEC-0087 / IMPL-0098 / TEST-0122) had passed hosted
tests and 16-bit compilation, but had not been linked and executed as 286
guest code. This slice adds an original bounded diagnostic, not a production
memory provider or operating-system boot.

## Inputs and method

- BootHush checkout `091abac`, Windows executable SHA-256
  `2b67834f4134a83b77b9870e560a0bcf2b5d2fd116dff01a11ec5054ef15f9cf`.
  The checkout identity does not establish how the executable was built.
  Only its documented black-box interface was used.
- Pinned Open Watcom 16-bit toolchain, identity checked against
  [the lock](../../scripts/pc286-toolchain-lock.json). The runner compiles the
  actual bank planner, catalog, binding and capability modules using `-2 -mc`;
  the independently original [fixture](../../probes/pc286/boot-span-cap.c)
  requires four-byte data pointers. A separate `-ms` attempt was rejected at
  that pointer-width assertion. No system runtime library is linked.
- [The runner](../../scripts/Test-BootHushPC286Capability.py) constructs its
  own 1.2 MiB diagnostic floppy using the existing original one-track loader.
  It uses BootHush's built-in PC/AT model and four fixed far-memory windows;
  no Apple, Microsoft, reference or external guest media was used.
- Full logs, images and machine-readable reports remain outside Git at
  `C:/ClasSICK/boothush-pc286-cap-20261113-a`. Two independent final builds
  and runs (`lifetime` and `lifetime-twin`) produced identical reports and
  payload hashes, including the explicit near-data rejection.

## Results

In the normal guest case, a four-byte logical write was borrowed across two
separate six-byte banks. The guest verified bounds and neighboring sentinels,
read-only permission, the unchanged read-only cursor after a refused writable
borrow, an old capability becoming stale on rebinding, writes to the new
binding without changing the old banks, release refusal, stale refusal
after a generation advance or a new owner lifetime, and lifetime exhaustion.
The original `A7` success marker was reached at cycle 87,114. BootHush
inspected the **entire 7,078-byte loaded payload and
all four six-byte windows at the same stop**; they matched expected bytes and
the input floppy hash was unchanged.

Three separately compiled controls deliberately expected incorrect results in
the fixture, and stopped at the corresponding failure markers: `D5` for
permission, `D8` for a stale capability after rebind, and `DD` after release.
Each control's complete loaded payload and four windows were independently
checked at its stop; each input disk remained unchanged. These are
**wrong-expectation controls**, not mutations of the capability implementation
and not proof that every implementation defect would be detected.

The passing baseline payload SHA-256 is
`fd9c872fb8d0c81e104dcc60e1050388fe4c8adab66a712573d4c1924c89a756`;
its generated floppy SHA-256 is
`8afabed7a074180928bd7afea7c324d7284f19636256e769aa530cbace7b3e27`.
The fixture and runner are lead-authored original project material,
GPL-3.0-or-later. No additional worker or emulator implementation was used.

This establishes that these bounded capability operations execute under this
16-bit compiler and emulator configuration. The caller must still supply
truthful disjoint live memory and stable metadata under SPEC-0087. It does not
establish production ownership, hard-disk installation, full ABI or stack
safety, physical hardware behavior, performance, a full 286 system or Mac
application compatibility.

## Paired-release status and BootHush gaps

The same BootHush executable reran the pinned Mac readiness-relative two-drag
and replay checkpoints successfully. The faster directional probe still failed:
at edge 2,048 the rightward pointer ended `(255,171)` with 29 guest-reported
omissions; an unguarded 8,192 case also missed a pixel. Only four of the eight
guarded 2,048/4,096 four-direction cases passed. These are synthetic modeled
inputs, not a Mac fix or physical/human qualification.

BootHush's current integration guide still states that x86-64 CPU execution and
UEFI boot are unimplemented. Its x64 media verifier does not execute the PC
candidate. The emulator therefore cannot yet run the PC half of the paired
release. The earlier request for correlated guest-RAM writes in the Mac mouse
witness is also still open in the currently documented interface. Headless
video capture is newly documented but is not a substitute for the missing x64
execution or the remaining Mac input checks. No other emulator was used.

The paired release is not ready; no release was published. Relevant prior Mac
evidence and the exact x64 developer requirements remain in
[the paired investigation](boothush-paired-20261113.md).
