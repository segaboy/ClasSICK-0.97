# Owned x64 terminal exception state

SPEC-0008 / ADR-0012; independent hardware layer, no portable-core CPU dependency.
state.c serializes bounded GDT/TSS/IDT bytes, tested under hosted compilers at
both widths. exceptions.S alone performs privileged descriptor setup and terminal
fault capture. It is linked into the inspected EFI image and never hosted tests.

The sole 8-byte active-state image-data slot is writable and non-executable;
tables and four emergency stacks reside in the existing owned loader bundle.
No firmware, sample OS code, headers or runtime library adopted. No native
instruction/exception execution or boot result is implied by byte/image tests.

See [contract](../../../docs/specifications/SPEC-0008-x64-exceptions.md) and
[verification](../../../docs/development/x64-exception-evidence.md) for limits.
