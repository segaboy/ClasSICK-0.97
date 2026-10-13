# TEST-0035 - Typed bounded register patch conformance

2026-10-13; SPEC-0027 / ADR-0031 / IMPL-0030 / SRC-0121/0122/0123.
Original interpreted core fixtures, not historical or native-application tests.

Five hosted suites plus a freestanding import/global audit. Routes exercise
eleven original-memory chains and all four allocation flag combinations,
comparing separate original manager/API outcomes. The ten-byte assembly fixture
performs JSR to a saved NewPtr identity, changes D1 and returns; the trap restores
D1 while exposing D0/A0. Direct JSR/veneer/RTS paths consume four instructions;
tail JMP/veneer completion consumes two.

Conventions test both A0 trap-bit states, zero/positive/negative low-word D0,
A1/D1/D2 restoration, high SR preservation, X and returned condition codes,
A5/A7/PC and scratch completion. Legacy atomic dispatch refuses the same patch.
Early snapshots cover odd/unmapped/nonexecutable entries, identity mismatch,
flags/Toolbox/context, A5 and short stack refusals. Initialization refuses heap
overlap, malformed map count, invalid manager and altered veneers atomically.

Late failure allocates through a saved original, publishes the address into
guest data and updates another directory entry before an unsupported opcode.
The full caller frame restores, allocated pointer still reports its size and
the directory change survives. A forged direct return faults before allocation.
Nested patches charge ancestors, refuse exhausted budgets and a fifth service
level, expose a nested depth cause and unwind to reusable depth zero. A tight
guest loop terminates at its budget. Independent runtimes and Get/Set restoration
check directory identities. Generic CPU/owned-return coverage remains TEST-0034.

Verify-GuestPatches.ps1 runs the complete prior regression chain, nine hosted
compiler/width/optimization/sanitizer profiles, ten extra patch object audits
and matching debug twins. Verify-GuestPatchesM68k.py audits the nine-module
original68000 closure in O0/Os, two complete relocatable links, frame reports,
metadata footprint and independent ten-byte assembler encoding. Quantitative
accepted results and retained preparation failures belong in development
evidence after exact CI comparison. No VM/emulator/boot or reference observation.

Accepted quantitative results, exact revisions and retained failures:
[checkpoint evidence](../../docs/development/guest-patches-evidence.md).
