<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0138 — Absolute-short operands for MOVE and MOVEA

SRC-0097, Motorola, *M68000 Family Programmer's Reference Manual*, §2.2.16
(printed p. 2-18), specifies an absolute-short memory operand using mode 7,
register field zero. Fetch one extension word and sign-extend its address to
32 bits. Chapter 4's MOVE (pp. 4-116–118) and MOVEA (pp. 4-119–120) supply
the existing transfer and condition-code rules. This specification selects
that addressing mode for the project's existing original-68000 MOVE.B/W/L
source and destination and MOVEA.W/L source paths.
[Motorola reference, hosted by NXP](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf)

The addressing word is independent of operand width. If both operands use the
mode, consume the source extension before the destination extension. The
existing operand transport applies either the strict map policy to the full
effective address, or DATA24 normalization to the address used for a data
read/write. For example, an extension of 0x8000 yields 0xFFFF8000, and DATA24
accesses 0xFF8000. MOVEA.W independently sign-extends its *data* word to the
destination register. MOVEA preserves CCR. The existing MOVE flags and register
size rules apply unchanged.

Each step retains the existing transaction semantics. Missing instruction
extensions, invalid data spans/permissions or odd word/long operands preserve
the frame, gateway, and mapped data. A tentative postincrement on the source
is discarded if the destination fails. These strict and DATA24 policies are
ClasSICK engineering choices. The manual supplies the ISA facts only.
