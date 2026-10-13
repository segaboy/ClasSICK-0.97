# ADR-0033 - Stage typed Toolbox outputs before completion

Accepted 2026-10-13, SRC-0139/0140 / SPEC-0029.

The existing OS register hook must preserve PC/A7 and cannot represent Pascal
parameter cleanup or no-return completion. Add a separate trusted session step
driver with per-routine convention and completion metadata. Decode checked stack
values into owned bounded storage and stage fixed output bytes/result before
committing any writes. This preserves early refusal and prevents a late control
fault after a trusted service effect.

Bounded v1 admits pure original returning builtins and root-only ExitToShell.
Register-based Toolbox exceptions, retained pointers, loader continuations,
Pascal guest patches, direct veneers and manager callbacks remain explicit
refusals pending their own reviewed contracts. Procedure auto-pop uses a checked
caller return long; function and owned-token auto-pop are refused rather than
inventing historical cleanup or weakening owned RTS completion. Existing bus,
call/session APIs and manager bindings retain their behavior.

The dispatcher compiles natively as ClasSICK OS/core conformance; no native
application ABI, SDK, loader, heap reset, boot entry or historical trial. More
general callback/lifetime evidence is collected separately by Claude.
