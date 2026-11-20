<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0145 - Quick arithmetic and scalar clear/test

Implement original 68000 ADDQ and SUBQ on the supported alterable data
destinations at byte, word and long widths. The three-bit immediate denotes
1 through 8, with zero encoding 8. Data arithmetic and XNZVC match ADDI/SUBI
at the chosen width. Word and long operations on an address register update
the whole 32-bit register and preserve all flags. Reject byte-size An.

CLR and TST accept the original processor's alterable data addressing forms.
Both preserve X and clear V and C. CLR writes zero, clears N and sets Z.
TST copies the sign and zero state of the addressed operand to N/Z and leaves
the value untouched. A TST memory operand can reside in read-only storage.
CLR of memory performs a read before the write, as the manual specifies for
MC68000 and MC68008. Exclude later TST extensions, Scc/DBcc encodings
sharing the quick family, and size-3 neighbors of CLR/TST from these handlers.
DBcc retains its pre-existing dispatch and qualification.

Use the existing checked effective-address, guard and scalar transaction
paths. A memory RMW evaluates the EA once and commits the register/flags
update with its store. The project's existing address policies, bounds and
refusal behavior remain in force.

Source: Motorola *M68000 Family Programmer's Reference Manual*, ch. 4,
ADDQ pp. 4-11/12, CLR pp. 4-73/74 (including original 68000 read-before-write
note), SUBQ pp. 4-181/182, TST pp. 4-192/193, and the addressing tables.
SRC-0097 identifies the published ISA reference. [NXP copy](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf)
