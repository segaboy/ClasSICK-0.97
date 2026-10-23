# SPEC-0068 - Isolated MC68000 short exclusion

2026-10-23; SRC-0394..0398, ADR-0079, IMPL-0069 / TEST-0082.

<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
## Original callback contract

## Interface

- `uint32_t cs_m68k_foreground_enter(void *cookie)`
- `void cs_m68k_foreground_leave(void *cookie, uint32_t token)`

`cookie` is ignored. The header includes only `stdint.h`. The source has no mutable state, MMIO
access or handler installation; native import checks are required.

## Behaviour claimed

- `enter` reads SR with MOVE from SR into a data register, then in the same
  asm block executes ORI.W #0x0700 to SR. This sets IPL to 7 and leaves S,
  trace and other SR fields as they were. The token is that snapshot word
  zero-extended (upper 16 bits zero).
- `leave` writes the low 16 bits of the token to SR with MOVE to SR from a
  data register. The value written is exactly the saved word, never a
  constant enable value.
- Both asm blocks are `volatile` and clobber `cc` and `memory`. The memory
  clobber only orders compiler-visible memory accesses. It is not a CPU
  barrier and says nothing about faults or physical-board behaviour.

## Not claimed

- That caller condition codes survive. Compiler prologues and epilogues and
  the callback ABI may change CCR. The only claim is that the snapshot word
  is restored at the leave write instruction, plus the architectural mode and
  mask under the preconditions below.
- Level 7 exclusion. Level 7 is not masked by IPL 7.
- That interrupt latency, CPU concurrency or a full 128K fit is shown by
  compilation or disassembly checks.
- That the opcode checks cover every possible instruction.

## Preconditions (caller obligations)

1. Supervisor state with trace off at both calls. The supervisor-only SR write causes a privilege violation in user state;
   original MC68000 MOVE-from-SR itself is unprivileged.
2. One serialized foreground caller. Nested use must pair tokens in LIFO
   order, and each `leave` gets the genuine token from its own `enter`.
3. No fault, re-entry or longjmp between `enter` and `leave`.
4. While the section is held, no ISR, level-7 source, synchronous exception,
   DMA, other CPU or other nonparticipant touches the protected state.
5. An interrupt can be taken just before the mask instruction. It must
   finish under a later ISR contract, which is not defined here.
6. The caller's mask must still allow capture where that matters. Owner
   logic and rendering stay outside the short section; never mask a whole
   render.
7. The planned single non-nesting producer at levels 1..6 must be quiescent
   across lifecycle rebinding. Its exact entry, return and register-save
   rules are still **UNADMITTED**.

## Status

This code is isolated and not bound to foreground, ROM, boot, drivers or ISRs.
No target execution or release is authorized. M1-M7 remain partial/open.

## Required audit

`scripts/Verify-M68kExclusion.py --tool-root R --output NEW` compiles only
`arch/m68k/exclusion.c` at O0, O2 and Os, each twice with a delay (6
objects), with `-mcpu=68000` and no LTO. It checks that the twin objects are
identical, that there are no imports or mutable data, that exactly the two
APIs are defined, and that each frame is static and at most 64 bytes. It also
checks the ordering and operands of the SR instructions in the decoded
objects (register-source check, not a complete token dataflow proof) and the clobbers in the source. It writes `manifest.json` and runs
nothing on the target.

Lead adoption: preserve instruction order across blank lines; reject unexpected
disassembly syntax; strip source comments before clobber checks; bound subprocesses.
Checks and limitations are in [the evidence](../development/cpu-c16-evidence.md).
Native compilation passes at the source-pinned CI revision in that receipt;
no target execution or hardware exclusion proof is claimed.
