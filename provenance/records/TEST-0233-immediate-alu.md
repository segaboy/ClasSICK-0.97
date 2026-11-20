<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0233 - Immediate ALU results and condition codes

SPEC-0142 / IMPL-0172. The `operands` suite in `tests/m68k-bus.c` covers
1,764 operation/width/value/initial-X combinations against an independent
64-bit signed and unsigned arithmetic oracle. It checks register high bits,
PC advance, all five flags, exact extension count, big-endian writes, indexed
read-only compare, the A7 byte stride, logical CCR operations, store rollback,
a truncated extension and ten unsupported encodings. Four original mutations
(spurious carry on equality, compare writing back, replacing CCR, wrong word
width) each failed the unchanged operand suite.

On 2026-11-20 the existing `scripts/Test-M68kStartup.py` passed all 30 host
suite invocations (including optimized and ASan/UBSan), four native 286 and
five native Mac regression cases, including the existing fault controls. The
native regressions in that script exercise the combined decoder using the
preceding address/stack fixtures. The immediate ALU oracle runs in its host
operand suite. The small-model 286 decoder object comprised 8,457 text bytes
with unchanged external import set. Decoder source SHA-256
`662a9d4aa39b9e6fcd5d033d36baf76ca10fbb662e98732faeabddb9372e0d16`.
Full native regression report SHA-256
`e6e3a5e02b23b7b1d81d72cdc3efc4dfd086c2acd25addf58b11ec6ca9d81f90`.

The subsequent SPEC-0145 combined decoder retains all these host checks.
Its generated normal-boot disk program produced the eight-byte immediate
ALU result `0000000611200005` on both the native Mac ROM path and the installed
286 interpreter. The installed PC refused the adjacent immediate-to-SR form.
