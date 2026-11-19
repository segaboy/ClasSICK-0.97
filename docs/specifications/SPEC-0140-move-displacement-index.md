<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0140 — Displacement and brief index addresses in MOVE and MOVEA

Extend the SPEC-0022 and SPEC-0138 scalar operand profile using the effective
address rules already admitted by SPEC-0026. Source authorities SRC-0097 and
SRC-0118 cover the Motorola *M68000 Family Programmer's Reference Manual*,
§§2.2.6–7 and 2.2.11–12 (printed pp. 2-8–9 and 2-13–14), §2.4 and Chapter
4 MOVE/MOVEA. Source and destination may use An plus a signed 16-bit
displacement, or An plus a word/long Dn/An index and signed 8-bit displacement.
Only the source may use the corresponding PC-relative forms. The base PC is
the address of the extension word. Word index values are sign-extended from
their low 16 bits, long index values use all 32 bits. These forms each consume
one extension word irrespective of operand width.
[Motorola manual, hosted by NXP](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf)

Source evaluation, including any tentative address-register update, precedes
destination evaluation. MOVE.B/W/L and MOVEA.W/L keep their existing transfer,
flag and size restrictions. PC-relative operand reads use normal data-read
permissions after the instruction extension has passed its fetch check.

The project accepts canonical unscaled brief extensions with bits 10..8 clear.
Its bounded arithmetic checks base plus index before adding the displacement;
either intermediate or final unsigned 32-bit overflow/underflow refuses the
step. The chosen data-address policy applies after address calculation. Under
DATA24 the resulting data address is narrowed to 24 bits. Strict mode checks
the entire address against the map. These profile restrictions make no claim
about the response of original silicon to reserved bits or wrapped addresses.
Ordinary guard, read, store, register and commit semantics of the bus
interpreter apply throughout.
