# ADR 0018: Package the loader in a fixed-profile GPT/FAT32 image

Accepted scoped design 2026-10-08. SRC-0056/0057/0058; SPEC-0014 / IMPL-0018 /
TEST-0021. Human provenance and rights review pending.

Write our own generator rather than run a formatter. Formatting tools add their
own boot-sector code and their behavior would become an unreviewed dependency;
reading another implementation is outside the project's process. The owner
accepted Microsoft's FAT32 specification terms for this PC boot tooling, which
sits outside the System 0.97 clean-room scope (SRC-0056). ECMA-107/FAT16 was the
considered alternative; UEFI specifies FAT32 for a system partition.

One fixed profile keeps the contract small and testable: a 64-MiB raw disk,
1-MiB-aligned ESP and data region, short names and contiguous allocation. Fixed
GUIDs, serial and timestamps make every build produce the same bytes, at the
cost that two copies cannot be attached at once. A raw image can be converted
for a VM or written to a USB device later; both uses need their own evidence.

Verification uses two readers that share no code with the writer (a C oracle in
the tests and a PowerShell checker) plus re-sealed corruption controls and a
cross-compiler identical-image requirement. Generic sizes, 4-KiB sectors, long
names, more files and signed images would need a revised contract.
