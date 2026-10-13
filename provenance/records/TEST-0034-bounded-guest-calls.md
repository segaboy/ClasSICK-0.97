# TEST-0034 - Bounded returning guest-call conformance

2026-10-13; SPEC-0026 / ADR-0030 / IMPL-0029 / SRC-0116/0118/0119.
Original CPU/core fixtures, not a native application or historical observation.

Six hosted suites plus freestanding import/global audit. Opcode tests cover
all 128 JMP/JSR effective-address fields: 56 legal control cases and 72 refusals,
checking condition codes, exact continuation push bytes and A7-before-push
evaluation. 256 indexed cases use a separate signed 64-bit test oracle over
PC/address bases, D/A selection, every register, word/long and sign choices;
A7 uses its actual stack value. Seven noncanonical brief encodings refuse.

Boundary snapshots check CPU, directory and RAM after odd/tagged/nonexecutable
targets, truncated extensions, short sign extension, signed arithmetic wrap,
unwritable/odd/short stacks and invalid continuations. Explicit stack limits
cover BSR, JSR, RTS, A-line scratch and MOVEA leaving the interval. DATA24 never
normalizes a control target.

The original20-byte assembler/C fixture returns after six instructions through
an internal JSR/RTS and PC-relative JMP, exposes D0=17/D1=42, restores PC/A5/A7
and reuses the runtime. Built-in GetTrapAddress runs inside an ordinary call.
Nonfatal D3/A5 diagnostics are distinct from mandatory return safety.
Fourteen pre-entry refusals leave frame/runtime/stack unchanged; CPU/return/budget
failure cases retain completed guest data or SetTrapAddress changes while
restoring the caller frame. Subsequent good calls succeed.

A trusted original OS hook calls back into the same runtime. Nested calls run
traps and RTS; ancestor budget limits stop execution and restore contexts.
An already exhausted ancestor refuses the child before its return-token write.
Four active service levels refuse a fifth, without leaving owned depth state.
Unknown contexts and signatures refuse; no parameter/result marshalling or
interrupt completion behavior is claimed.

Run Verify-GuestCalls.ps1 for the complete pinned hosted/compiler/sanitizer and
unloaded-image regression chain, ten extra call-object audits and debug twins.
Verify-GuestCallsM68k.py checks the five-object original68000 closure in O0/Os,
two complete relocatable links, frame reports, metadata size and independent
assembler bytes. CI runs the same chain. Quantitative outcomes, hashes and
preparation failures belong in development evidence. No VM/emulator/boot trial.
