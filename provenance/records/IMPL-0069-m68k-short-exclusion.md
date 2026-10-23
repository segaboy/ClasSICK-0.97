# IMPL-0069 - MC68000 short-exclusion callbacks

SPEC-0068 / ADR-0079; SRC-0394..0398. Fresh Claude supplies original isolated
`arch/m68k/exclusion.c/.h` and `scripts/Verify-M68kExclusion.py`. Lead adopts
the callbacks, tightens audit parsing/clobber checking and retains own-object
disassembly for review. Two supervisor callbacks, no mutable state or MMIO.
No ISR, SCC/VIA, vector or boot binding. [Evidence](../../docs/development/cpu-c16-evidence.md).
