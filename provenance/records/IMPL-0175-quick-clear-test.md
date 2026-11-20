<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0175 - Quick arithmetic, clear and test

SPEC-0145 / TEST-0236. In `core/cpu/m68k-bus.c`, Codex added original
ADDQ/SUBQ/CLR/TST decoding and reused the SPEC-0142 immediate family's scalar
read, write and checked ALU paths. Word/long address-register quick arithmetic
is implemented separately from data arithmetic. Register partial writes,
memory guard checks and RMW address-register side effects use the same native
interpreter on the installed 286. Source inputs were published ISA facts and
project-owned code. GPL-3.0-or-later; independent review remains open.
