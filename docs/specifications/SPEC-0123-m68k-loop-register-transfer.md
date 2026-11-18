<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0123 - Bounded DBcc and MOVEM transactions

2026-11-18. SRC-0616 supplies published base-68000 DBcc facts; SRC-0618 supplies
MOVEM facts from the published NXP-hosted Motorola programmer's manual. This
extends the shared bus interpreter, not the minimal cs_m68k_step entry point.
SPEC-0022/0026/0034 and SPEC-0120 ownership, guard and stack rules still apply.
No protected program, instruction timing or exception-bus sequencing is claimed.

## Instruction profile

DBcc matches `(word & 0xf0f8) == 0x50c8` and fetches a signed word displacement.
All sixteen conditions use the existing CCR predicates, with condition 1 false
for DBF (distinct from the existing BSR encoding). If true, preserve Dn and
continue after the extension. If false, decrement only Dn's low word; branch
unless the resulting word is FFFF. The signed destination base is opcode PC+2.
Preserve the complete SR. A taken target must pass the existing executable,
alignment, address and target-guard checks before publishing the decrement.

MOVEM matches `(word & 0xfb80) == 0x4880`, with a word register mask before EA
extensions. It supports word/long transfers and the base-68000 control addressing
modes already decoded for LEA, plus store predecrement and load postincrement.
Stores reject PC-relative modes; loads reject predecrement. Register-direct,
immediate and later scaled/full-extension modes are refused. SRC-0618 defines
mask order, word-load sign extension and the base-register alias rules. All SR
bits remain unchanged. The zero mask is refused after its fetch, without operand
access: this is an explicit bounded-profile restriction, not architectural
illegality inferred from the manual's silence.

## Transfer bounds and publication

A nonzero mask selects at most 16 registers and 64 bytes. Decode addresses using
the existing checked signed additions. Predecrement subtracts the entire size
without wrap; postincrement checks the full resulting address. Original register
values supply stores, including a listed predecrement base register. Loads stage
all register updates, then postincrement overwrites a listed base register with
the final address. A7-based An modes check the complete transfer against explicit
stack limits when present. Any loaded A7 also passes the final bounded-stack
check. Existing DATA24 options normalize only the operand address, not register
values or transfer updates; a normalized span cannot cross the 24-bit boundary.

One whole-transfer READ/WRITE guard event, even alignment and a complete mapped
span precede operand access. Loads additionally emit existing scalar READ events
for each selected register, permitting exact late-read refusal. Stores stage a
maximum 64-byte ascending-address buffer; there are no individual writes during
decode. The existing continuation/state checks and COMMIT guard precede final
publication. Any late read, guard, bounds or write refusal preserves all CPU
registers and guest memory. Reports may change. This whole-instruction atomicity
is engineering policy, not a claim about physical fault restart behavior.

The flat path retains its prevalidated direct-pointer commit. The old scalar
backend can execute DBcc/MOVEM loads; nonzero MOVEM stores refuse OPCODE because
it has no multi-byte atomic publication contract. No repeated scalar writes or
rollback approximation is permitted.

## Additive native backend

`cs_m68k_transfer_backend` wraps the unchanged scalar descriptor and adds a
trusted synchronous `write_bytes` callback. `cs_m68k_bus_step_transfer` applies
the native-envelope checks to the entire wrapper. The callback accepts 1..64
bytes, validates their entire extent and commits all or none. No callbacks retain
staging pointers, reenter, mutate controls or change lifetime/geometry mid-step.
Existing scalar instructions continue using the scalar callback and semantics.

The native 286 adapter uses the same stable admitted banked-memory context. Its
new bounded `cs_pc286_guest_write_bytes` checks live generation, full coverage,
permissions and near source extent before any write. It resolves the complete
destination across declared banks, then publishes without another fallible
operation. Byte spans may be unaligned at this service layer; CPU MOVEM enforces
even addresses. A source span cannot wrap its native segment and guest banks
exclude CS/DS/SS backing under the existing service/adapter contracts. There is
no malloc, flat guest pointer, runtime library or hidden unbounded store list.

## Verification

Use independent original instruction fixtures for all conditions/registers,
mask ordering, aliases, sign extension, legal/illegal EAs, zero-mask refusal,
whole-transfer and final-stack boundaries, denied guards and partial/late-read
or final-write refusals. Qualify maximum 64-byte transfers and instruction fetch
across native banks, including code/data beyond 64 KiB. Deliberate mutants must
show that order, base-register semantics and atomic publication are observed.
Rerun the existing flat and banked instruction suites after engine changes.
Record exact tools, sources, program bytes, no-runtime links, stack/code costs,
native outcomes and limits in IMPL-0153/TEST-0214. No System startup claim follows.
