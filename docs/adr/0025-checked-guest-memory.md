# ADR-0025 - Checked memory before allocation services

Accepted 2026-10-12; SPEC-0022, SRC-0096-0098.

Original programs need data reads/writes and subroutine control before allocation
services can be exercised. Add a small caller-owned map and a separate CPU entry
point, preserving the previously verified SPEC-0021 interface. Keep logical guest
addresses distinct from native pointers. Validate all effects before committing
each instruction, so an unsupported operation cannot leave half a MOVE or stack
push behind. This deliberate failure behavior does not emulate a physical bus
error's partially completed hardware cycles.

Use independent RAM/code/stack buffers and a new original program; no allocator
or historical heap layout is inferred. Claude is independently researching the
Memory Manager's published interfaces. Integration follows review of that draft.
The map's eight-region cap and single-region access rule keep the first contract
bounded; broader mapping, MMIO, addressing modes and CPU behavior remain later
contracts. No external emulator implementation or new runtime is incorporated.
