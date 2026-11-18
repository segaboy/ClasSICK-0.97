<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0616 - Published 68000 address and stack instructions

2026-11-18. Lead-reviewed primary source: Motorola, M68000 Family Programmer's
Reference Manual, copyright 1992, hosted by NXP:
https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf.
Printed pages 4-110 through 4-112 (LEA, LINK), 4-159 (PEA), 4-194 (UNLK),
and 4-90 through 4-91 (DBcc). The PDF explicitly identifies base-family versus
later-CPU forms. This is published processor interface documentation, not an
Apple implementation or an emulator source. Existing instruction/source records
remain scoped to their earlier claims.

Admission is restricted to instruction encodings, addressing modes, register and
condition-code effects for the base 68000. LEA/PEA use computed control addresses;
LINK.W/UNLK manipulate stack frames; DBcc tests a condition and conditionally
decrements the low word before a relative branch. Later LINK.L and extended
addressing forms are excluded. Ambiguous alias cases require explicit handling
or refusal, not extrapolation from protected programs.

The implementer receives an independent factual restatement, not manual samples
or protected executable bytes. Original tests, guarded transactional publication,
bounded-stack policy and unsupported-mode refusals remain engineering decisions.
SPEC-0118 records the implemented subset. This intake is not a complete CPU,
exception model, instruction-timing or System startup qualification.
