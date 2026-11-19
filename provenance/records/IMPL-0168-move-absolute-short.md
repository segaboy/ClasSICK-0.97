<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0168 — MOVE absolute-short effective address

SPEC-0138 / TEST-0229 / SRC-0097. The shared MOVE operand selector in
`core/cpu/m68k-bus.c` fetches one word through the existing instruction
extension reader and reuses the existing sign-extension logic. The normal
source and destination helpers apply the configured data-address policy,
operand checks, flags and commit rules. Original Codex implementation from
the published ISA and project-owned CPU contracts; GPL-3.0-or-later.
Independent review remains open.
