<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Candidate PC/AT BIOS interface reference

2026-11-15. The question is what published interface description could support
a reviewed 286 disk contract, including drive parameters and read failure
handling. The current BootHush-only whole-disk diagnostic is deliberately not
such a contract.

Research lead only: IBM, *Personal System/2 and Personal Computer BIOS
Interface Technical Reference*, first edition, April 1987, publication 84X1514,
[archived scan](https://bitsavers.trailing-edge.com/pdf/ibm/pc/ps2/PS2_and_PC_BIOS_Interface_Technical_Reference_Apr87.pdf).
Its preface names the PC/AT among intended systems, and its fixed-disk interface
discussion is printed pp. 2-58 through 2-64. The retrieved web text was used
only to identify the edition, scope and relevant interface pages; no BIOS ROM
implementation, listing, executable code or protected assets were inspected or
copied. No local PDF identity or source-catalog admission is asserted.

This later publication is a candidate for separate provenance, rights and
applicability review. Its date and broad product scope do not prove behavior of
the original 1984 PC/AT firmware. Do not use it as an implementation source or
physical-hardware proof until admitted under project policy. The current
geometry preflight is an original test of BootHush's documented built-in
model, not an implementation based on this lead. No copying of the publication
into Git is authorized by this note.
