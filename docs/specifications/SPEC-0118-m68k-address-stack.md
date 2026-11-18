<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0118 - Bounded 68000 address and stack instructions

2026-11-18. Status: original implementation contract. Source: SRC-0616,
the lead-reviewed published Motorola 1992 programmer's manual instruction
encoding and interface facts. Existing SPEC-0022, SPEC-0026 and SPEC-0034 memory,
stack and guarded transaction contracts remain applicable. This extends the
mapped bus interpreter, not the earlier minimal `cs_m68k_step` entry point.

## Implemented instructions

| Instruction | Encoding | Result |
| --- | --- | --- |
| LEA EA,An | `(word & 0xf1c0) == 0x41c0` | Assign the computed 32-bit effective address to An. |
| PEA EA | `(word & 0xffc0) == 0x4840` | Predecrement A7 by four and store the computed 32-bit effective address. |
| LINK.W An,#d16 | `(word & 0xfff8) == 0x4e50` | Push old An, assign that stack slot address to An, then add signed d16 to A7. |
| UNLK An | `(word & 0xfff8) == 0x4e58` | Read the long at An, assign it to An and assign the stack slot address plus four to A7. |

All four preserve the entire SR, including the CCR. LINK.W and UNLK accept
A0 through A6. Their A7 aliases return `CS_M68K_OPCODE`; the reviewed source
intake does not settle that alias ordering for this bounded profile. LINK.L,
MOVEM and DBcc are outside this increment. Unsupported forms do not advance PC.

LEA and PEA reuse the existing SPEC-0026 control-address decoder: `(An)`,
`d16(An)`, canonical unscaled brief `d8(An,Xn)`, absolute word and long,
`d16(PC)` and canonical unscaled brief `d8(PC,Xn)`. Word indexes and absolute
word addresses are sign extended. PC-relative bases are the extension-word
address. Scaled/full extension words and all other modes are refused.
The computed address is a value: it need not identify mapped, aligned or
24-bit memory, and computing it does not generate operand READ or TARGET
events. As in the existing decoder, signed additions reject 32-bit overflow
or underflow instead of wrapping. DATA24 does not normalize these values.
Assigning A7 remains subject to the existing final bounded-stack check.

## Transaction and bounds policy

All opcode and extension reads use the existing mapped execution policy and
guard events. PEA calculates its effective address using the original A7 before
the stack push. LINK fetches its complete displacement before staging a write.
Each push checks underflow, stack interval, even alignment and a complete
four-byte writable mapping. UNLK checks its complete four-byte stack interval
and readable mapping before proposing either register update. A LINK local
allocation changes A7 but does not initialize local bytes; a negative
displacement may reach the stack base and a positive displacement may reach
the one-past-end stack pointer. The resulting A7 must satisfy the existing
bounded even-stack policy. Without explicit stack limits, ordinary mapped
access and checked 32-bit additions still apply.

A successful instruction advances PC past its opcode and any extensions.
It does not mark a branch or subroutine call and does not invoke a service.
The existing continuation check and COMMIT guard run before a staged store or
frame is published. Failures, including a late invalid A7 or rejected COMMIT,
leave the complete frame, gateway and guest backing unchanged. Diagnostic
reports may change. There is still at most one deferred four-byte store for
these instructions; no unbounded store list or allocation is introduced.

For the admitted C99 compiler, native alignment checks use owned member-offset
wrappers with compile-time nonzero power-of-two checks; C11 builds retain their
native alignment query. Guarded entry rejects a guest region whose backing
length exceeds either native `size_t` or `ptrdiff_t` capacity before narrowing.
The native C16 probe uses one near data segment and small live backing arrays;
this is not a general segmented/far-pointer guest-memory backend.

## Evidence and limits

Original host fixtures cover all admitted modes and destination registers,
condition-code preservation, extension boundaries, signed arithmetic,
invalid forms, stack edges, permissions, denied events and final-commit
failure. An original composed prologue/body/epilogue program exercises all
four instructions with existing MOVE instructions. Native closure and probe
results belong to TEST-0208 and IMPL-0148 and must name their exact tool/input
identities. Native compiler stack figures describe individual frames, not a
cumulative stack proof. A modeled target probe is not physical hardware,
historical instruction timing, a full exception model, complete 68000 support
or System Disk startup qualification.
