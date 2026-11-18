<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0620 - Original modeled 286 single-sector write interface

2026-11-18. Source basis is independently authored BootHush `src/pc286.cpp`,
`src/pc286.h`, original `tests/test_pc286_floppy_write.py` and the existing
SPEC-0113 read-provider contract. No external firmware or third-party CPU
implementation body was used. This records an original product interface,
not a claim of historical physical BIOS behavior.

The opt-in built-in firmware accepts INT13 AH03/AL01, drive0, fixed80x2x15
512-byte geometry, with the same CHS and whole-buffer validation as reads.
All validation precedes a single private-image copy. Success returns CF clear,
AH0; disabled writes return CF set/AH3. Other validation errors set CF and
their existing status. The caller supplies a complete stable conventional-RAM
sector buffer within a physical64KiB boundary. `--floppy-out` explicitly enables
writes to the emulator's copied image; original input media stays unchanged.
Persistence/export admission remains the emulator CLI's separate contract.

The coordinating agent admitted Windows twin executable SHA-256
`3d48d7515d3492ac796d3899ed2c47603d3330e3f0f64352fd29ed6a78284ff4`.
Its own twenty original BIOS-write boots and ten existing286 tests passed
before this provider implementation. SPEC-0126 is the new native provider
contract; IMPL-0156/TEST-0217 record its actual qualification. No protected
media, physical target, FDC-port write or filesystem mutation is admitted here.
