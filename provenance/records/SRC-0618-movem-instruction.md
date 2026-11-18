<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0618 - Published MOVEM instruction facts

2026-11-18; input to SPEC-0123 / IMPL-0153 / TEST-0214. Motorola,
*M68000 Family Programmer's Reference Manual* (1992), MOVEM, printed
pages 4-128 through 4-130 (PDF pages 232-234; zero-based indices 231-233).
[Publisher-hosted PDF](https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf#page=232).
Accessed 2026-11-18. Published instruction documentation; no implementation,
sample program, protected owner media or excluded research was consulted.

## Factual restatement

The opcode is `0x4880 | (direction << 10) | (size << 6) | EA6`, followed by
a 16-bit register mask. Direction 0 stores, 1 loads; size 0 means word, 1 long.
Condition codes are unchanged. Normal/postincrement mask bits 0..15 select
D0..D7,A0..A7 in that transfer order; predecrement reverses the correspondence
and order to A7..A0,D7..D0. Each transfer advances or decrements by 2/4 bytes.
Word loads sign-extend into both data and address registers.

For predecrement, a listed base register supplies its original value on
MC68000/MC68010; MC68020/030/040 and CPU32 instead supply original minus operand
size. Postincrement discards a listed base register's loaded value and retains
the final incremented address.

Base-68000 stores allow `(An)`, `-(An)`, displacement/indexed An and absolute
short/long. Loads allow `(An)`, `(An)+`, displacement/indexed An, absolute
short/long and displacement/indexed PC. Register-direct and immediate forms
are excluded; later full-extension/memory-indirect forms are not base-68000.

The cited MOVEM entry does not explicitly define zero-mask side effects.

## Scope and review

Any zero-mask refusal is an independent bounded-profile decision, not a claim
that the architecture defines that encoding as illegal. Instruction fetch,
callback faults, transaction boundaries and cycle/performance policy also need
the separate original contract. This record establishes documented behavior,
not physical validation, complete CPU coverage or System startup.

Codex independently checked publisher text and the three instruction pages.
No manual pages or copied implementation are stored in the repository. Human
provenance review remains separate; this source admission is limited to the
instruction facts above.
