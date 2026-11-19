<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0229 — Absolute-short MOVE and MOVEA operands

SPEC-0138 / IMPL-0168. Original `tests/m68k-bus.c` operand cases exercise
byte/word/long transfers with two absolute-short operands, differing source
and destination extension words and guard bytes. MOVEA.W and MOVEA.L verify
the distinction between sign-extending the address and sign-extending the
data, preserving CCR. Negative-address source and destination operations
are checked first under strict addressing, then with DATA24 enabled and a
mapped decoy at the unsigned address. Missing destination extension,
misalignment and write permission refusals check full transaction rollback.
Byte MOVEA remains illegal.

On 2026-11-19 all six existing bus suites (operands, registers, failures,
branches, services and probe) passed in optimized and ASan/UBSan host builds.
Generated single-defect variants that zero-extend the address or read two
extension words both failed the unchanged operand suite. Optimized executable
SHA-256 `98210d421aa39e375ca11ada05837e8881fc1863e450a014fb735c0e285fcc12`;
sanitized executable `616b435eec2e6802e397597f9bc25c8443441ab403040c442e56c2ce6112dc66`.
These tests exercise the hosted interpreter and do not independently establish
native target timing, hardware accuracy or a successful System Disk boot.
