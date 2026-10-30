<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright 2026 Dean Howell. -->
# SPEC-0086 - 286 checked access experiment

2026-10-30; SRC-0566, IMPL-0096, TEST-0115. Adopted for an isolated original model, not a production memory provider or boot path. The [access contract](../development/pc286-span-access-contract.md) defines the environment, refusal ordering, byte effects and caller obligations. SPEC-0069 and SPEC-0082 remain prerequisites.

The pinned 16-bit compilation requires four-byte data pointers; near-data memory models must fail at the explicit pointer-size guard. A successful native compilation is evidence about the selected compiler profile and object metadata, not proof of target execution, memory ownership, ABI, whole-program stack, real-hardware performance or application compatibility. Pointer reachability, truthful extents, nonaliasing and exclusive map stability remain caller obligations. No 286 boot, 128K Macintosh, PC GUI or release claim follows.
