<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Not Super Tilt Bro intake: original Mac keyboard probe

The owner requested a review of `C:/Repos/Not-Super-Tilt-Bro` and use of
eligible evidence to advance ClasSICK. The separate project is developing a
standalone native Macintosh game ROM; it is not the ClasSICK operating system
and does not yet provide a playable complete port. Its source-derived game
logic, NES ROM, graphics and other game assets were not imported. The
separately supplied NES beta has an unverified source revision and is not used
as a ClasSICK compatibility reference. Existing clean-room and publication
gates remain unchanged. The other checkout was changing during intake; the
selected inputs are identified by hashes rather than its tracked HEAD alone.
During intake its status changed from a dirty checkout at `107a0e8` to a clean
checkout at `2fecd31`, apparently through independent work. ClasSICK did not
edit that repository; the intake copies are not asserted to equal either
commit wholesale.

## Admitted evidence and independent observation

The selected three-position Mac controls probe is represented by its local
provenance as original GPL-3.0-or-later input with a reset sequence adapted from
ClasSICK; its source-game-derived running slice is separate and excluded here.
Byte-identical intake copies and a file/hash manifest are outside Git at
`C:/ClasSICK/tilt-intake-20261113-a/intake`. The independent original probe
ROM and the unchanged generated output both have 65,536 bytes and SHA-256
`31b2666be66ac7a02d576998cb9b46aec3a91245d16679877d593748fd581fc6`.
The BootHush executable used for the observations has SHA-256
`7b4c32e24d63432b2103f38c9c1844480a64b1b2bdda4342896e24a19d7c98c7`.
Both are model observations, not physical-Mac measurements.

The game project's existing control test was rerun read-only, with bytecode
writes disabled. All eight cases passed, including two positions' press and
release, bounded movement and exact repeated output. A separate headless
keyboard-witness run on that exact original probe scheduled four transitions;
all four were accepted and the modeled keyboard delivered `01 81 03 83` in
order. Its witness reported no rejected or dropped events.

For comparison, the current ClasSICK experimental ROM SHA-256
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c` was
run in a separate neutral control and with a modeled position-0 press and
release at cycles 60,000,000 and 61,000,000. Both scheduled inputs were
accepted. The keyboard witness showed the two replies queued, **no
`key_delivered` event**, and the queue at two after the second transition.
The final cycle, inspected guest-memory range and complete framebuffer matched
the neutral run. `keyboard_witness.pending` counts scheduled key events, not
unconsumed device queue entries; its zero must not be read as guest acceptance.
This demonstrates a missing keyboard path in this bounded candidate/model
configuration, not a universal claim about every ClasSICK build.
The [guest-only keyed frame](../evidence/screenshots/2026-11-13/boothush-queued-key.png)
was losslessly converted from PBM, decoded and indexed; the neutral frame had
identical pixels. The queue conclusion comes from the witness, not the image.

## Original ClasSICK parser

The lead wrote [SPEC-0099](../specifications/SPEC-0099-m0110-reply.md) and an
independent original pure inquiry-reply parser (IMPL-0119). The narrow profile
accepts modeled positions 0..`0x39`, recognizes `0x7b` as no transition, and
reports malformed bytes without changing the output. It is **command-context
dependent**: replies for a model query or self-test must not be passed to it.
The four observed transitions are checked explicitly; the remaining valid
positions are the documented emulator profile, not individually observed
physical keys. No game implementation or asset was copied into the parser.

The standalone conformance suite checks all 256 byte values, explicit four
observed replies and unchanged output on refusals. It passed CMake/CTest with
Windows Clang, the same build with ASan/UBSan, and a separate Windows GCC
build. The freestanding Clang object had no undefined symbols. The parser also
compiled with the pinned 68000 cross-compiler. A pinned Open Watcom 16-bit
boot diagnostic ran it in BootHush: baseline `A3`, deliberately wrong release
and invalid-byte expectations `E3`/`E4`, full loaded payload equality and
unchanged original floppy. Two builds/runs produced identical reports. These
are wrong-expectation controls, not mutated parser implementations.

The parser is not linked into a Mac or PC production boot path. A reviewed VIA
and keyboard ownership/command contract, timeouts, loss handling, real event
delivery and target-specific integration are still needed. It does not fix
the Mac mouse, establish M0110 physical timing/rollover, or close a release
gate. A supported 68000 cross-compiler was usable for this compile under this
session's permitted WSL access; the other project's previously reported access
denial may reflect its execution environment and is not proven resolved there.

## Other BootHush observations

The newer BootHush experimental x64 software probe ran 22 instructions from
the original project EFI image and stopped on unsupported opcode `0x4d` at
`0x10001b6e`. Its entry uses artificial zeroed firmware arguments; it reported
`firmware:false` and `valid_uefi_context:false`. The original disk hash was
unchanged. This is an instruction probe, not a UEFI boot or application result.
The existing Mac fast-motion issue, x64 UEFI machine gap, physical/human
input gates and paired-release qualification remain open. No other emulator
or reference system was used.
