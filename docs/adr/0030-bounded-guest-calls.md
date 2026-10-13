# ADR-0030 - Bounded returning guest call service

Accepted 2026-10-13 for SPEC-0026; engineering profile only. SRC-0116 records
the owner's direction to consider QuickDraw research and proceed to the next
proposed 68k runtime slice. K-1/K-3/K-6/K-7 from SRC-0113 constrain the design.

Use one checked bus interpreter, separate control-address decoding, optional
explicit stack limits and an owned reentrant call runtime. Its first signature
is a returning no-parameter/no-result procedure. Readonly depth-specific logical
tokens plus RTS/stack completion checks prevent accidental continuation into
host plumbing; guest values never select or become native function pointers.
Charge instruction attempts to all ancestors and bound active service depth.

Restore caller controls on success and the entire caller frame on failure;
retain completed guest side effects. This makes failure explicit without an
unbounded RAM snapshot or a misleading promise to undo allocation/IO/dispatch.
Register-preservation diagnostics remain separate from mandatory safety checks.
Defer marshalling, guest patch installation, interrupt/nonreturning entries,
manager implementations and native application interfaces to later contracts.
QuickDraw drafts remain research, with unresolved edition/layout/drawing choices;
no graphics implementation is part of this slice. No VM/emulator run authorized.
