<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0172 - Immediate data operands in the checked 68000 interpreter

SPEC-0142 / TEST-0233. The shared interpreter in `core/cpu/m68k-bus.c`
decodes the six immediate data ALU operations and three logical CCR forms.
It reuses existing extension fetch, operand addressing, guard, flag and
transactional write paths. Arithmetic uses unsigned values of the selected
operand width. The 286 builds this implementation as native C16. No host
pointer is formed from an emulated address.

Codex authored the implementation and tests from the published Motorola
interface and public project sources. No private disk bytes or external
implementation informed this slice. GPL-3.0-or-later. Independent review
and instruction timing qualification remain open.
