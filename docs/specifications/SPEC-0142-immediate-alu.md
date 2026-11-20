<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0142 - Immediate arithmetic, logic and comparison

For the original 68000 instruction profile, implement ADDI, SUBI, CMPI, ANDI,
ORI and EORI at byte, word and long sizes. Fetch the immediate operand before
any destination extension. The byte operand occupies the low byte of a word.
Accept Dn and alterable memory destinations using the addressing forms already
implemented by SPEC-0022/0138/0140. The compare forms supported here exclude
later processors' PC-relative destinations. Reject address-register direct,
reserved sizes, later address formats and adjacent instruction classes.

Dn writes preserve the portion outside the selected width. Addressed memory
operands use a single effective-address evaluation and a single read, including
the usual address-register updates. ALU operations stage the write to that same
address. Compare reads and updates flags without writing its destination. It
therefore accepts readable storage even if the backing map denies writes.
Source and destination, data-address policy, instruction fetch, guard and final
commit follow the existing checked scalar paths.

N and Z describe the truncated result. Logical operations clear V/C and
preserve X. Add and subtract set V when signed arithmetic exceeds the
selected signed range. Their C and X report unsigned carry or borrow.
Compare computes destination minus immediate, sets NZVC as for subtraction,
and preserves X. The three dedicated byte logical forms targeting CCR operate
on its five implemented flags only. The privileged SR variants remain outside
this implementation.

Source: Motorola *M68000 Family Programmer's Reference Manual*, ch. 4,
ADDI (4-9/10), ANDI (4-18/19), CMPI (4-79/80), EORI (4-102/103), ORI
(4-152/153), SUBI (4-179/180), and accompanying logical-immediate-to-CCR
descriptions. SRC-0097 identifies the published ISA reference. Register
semantics are documented facts; the map and refusal policies are project
designs. [Publisher's reference](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf)
