<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0148 - Guarded 68000 address and frame instructions

2026-11-18; SRC-0616/SPEC-0118/TEST-0208. Original LEA, PEA,
LINK.W A0-A6 and UNLK A0-A6 handlers extend the shared mapped bus decoder.
All retain staged writes and the existing guard/commit policy. Native C99
alignment queries and checked native-span narrowing enable the bounded C16
probe. No external implementation or protected executable was used.
[Canonical receipt](../../docs/development/m68k-address-stack-20261118.md)
owns exact identities, independent lead review, target checks and limitations.
GPL-3.0-or-later; no complete CPU, System startup or physical claim.
