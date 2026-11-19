<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0170 — Shared displacement and index decoding for MOVE

SPEC-0140 / TEST-0231 / SRC-0097 / SRC-0118. Original changes to
`core/cpu/m68k-bus.c` let ordinary memory operands reuse the existing control
address decoder. MOVE destination selection admits only alterable addresses.
The source and destination paths retain the existing checked scalar read,
staged write, data-address and guard policies. Codex used the project's CPU
interfaces and published Motorola addressing definitions. No external
implementation, private disk code or protected assets informed this change.
GPL-3.0-or-later; independent review remains open.
