<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0231 — Scalar MOVE displacement/index addressing

SPEC-0140 / IMPL-0170. In `tests/m68k-bus.c`, a four-instruction original
program moves word, long and byte operands through the displacement and index
forms and sign-extends a MOVEA.W result. It checks the two-operand extension
order, PC base, signed word/long indexes, data/address register selection,
memory guards, flags and destination bytes after each relevant instruction.
A further index uses the source postincrement from the same instruction.

Additional cases anchor a negative PC-indexed read, strict versus DATA24
address normalization, invalid PC-relative writes, reserved brief extension
bits, odd data addresses, missing extension words and intermediate arithmetic
overflow. Denials of both extensions, source read, destination write and final
commit preserve guest state. The previous unsupported-mode regression now
uses reserved source mode 7/register 5.

On 2026-11-19 all six bus suites and all five guard suites passed in optimized
and ASan/UBSan hosted builds. Separate original mutations moved the PC base
past its extension, zero-extended a word index, or admitted PC-relative writes.
The operand suite rejected each. Workstation decoder SHA-256
`25a01af5553119fc7a58c0e120daf92a38086c45cdd098454c59379eab8d375d`;
optimized/sanitized bus executables
`b8820d53d98b34f6ecbb08691a68eb36b6d938dd1432e2515265236145b8a632`
and `0f13dda528d2f088019dffe3b566cf20ade6177d6efc7fef9370c36bd48259c5`.
These hosted checks establish scoped interpreter behavior.
