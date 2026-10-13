# Checked guest memory execution checkpoint - 2026-10-12

SPEC-0022 / ADR-0025 / IMPL-0026 / TEST-0030 / SRC-0096-0098 / SRC-0100.
Compiled source `61c6c8de0963ee90748d284dab26b19727563e26`. Lead self-review; human provenance review pending.

The portable core can now read/write checked guest data and execute selected
MOVE/MOVEA, all byte/word branch conditions, BSR and RTS. The map borrows up to
eight caller-owned regions with explicit read/write/execute permissions. Guest
addresses remain distinct from native pointers; alignment/bounds/permissions are
checked before each instruction commits. Failed operations leave registers,
gateway and mapped data intact. The prior SPEC-0021 API and services stay intact.
This engineering profile supplies no allocator or historical Macintosh heap.

An original 36-byte program completes twelve steps and one subroutine call,
with sixteen passing checks, status0/signature4d454d31. It moves/overwrites data,
tests an untaken conditional branch, pushes/pops return addresses and finishes
with data11227f5a, D1=11223344/D2=7f5a/D3=11227f5a/A0=1800/SR=2710.
The pinned 68000 assembler independently agrees with its exact byte sequence.
This is our program in borrowed test buffers, not a legacy application.

## Verified scope

1,110 CTest checks pass in nine fresh Windows configurations:
136/136/136/109/136/121/121/121/94. Includes Clang/GCC, both native pointer
widths and sanitizer coverage; every condition/flag combination, data-width/
register/auto-address behavior, malformed maps, odd/cross-region/permission
accesses, transaction failure, stack targets and original fixtures are checked.
Thirty added standalone audits cover Clang/GCC O0/O2 32/64-bit and ARM64
little/big-endian objects, retaining prior twenty CPU/twenty service audits.
New test-executable debug twins match for both compilers.

GNU 68000 GCC13.3 O0/Os passes ten source-object import/global audits and two
complete relocatable links with no unresolved runtime symbols. Function frames
are bounded; no measured native stack high-water or final linked-ROM budget is
claimed. [Implementation CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38292360762) passes both jobs. Both combined 68000 objects,
the program encoding, every memory build/source fingerprint and function frame
value match local. Stack-record iteration order differed, so comparison sorted
that list by function/bytes; raw manifest byte identity is not claimed.

All six pre-existing EFI profile fingerprints, both own ROM profiles and every
ROM input fingerprint match local/CI and the prior admitted A-line checkpoint.
The normal raw media hash remains d178b720...c66ba40. This change does not link
the new probe into a boot entry. No new VirtualBox/Snow launch or operator input
was performed; earlier native CPU/keyboard/mouse/fault evidence retains its exact
images and scope. [Sanitized manifest](guest-memory-evidence.json) retains full
checksums and bounded build facts, with no private paths, VM IDs or binaries.

## Next dependency and limits

The owner returned Claude's Memory Manager research. The two original drafts
remain untouched and untracked; [lead review](../research/memory-manager-lead-review.md)
records proposed engineering decisions, not approved historical facts. Verify
published interface citations, finalize pointer/handle/zone contracts, and add
explicit 24-bit data-address normalization before flagged handle dereferences.
The current map deliberately refuses high address bits. Allocation metadata,
master cells, guest globals and SYS/CLEAR trap routing are not implemented here.

The owner's separate SRC-0099 handoff keeps x86 application building/loading/
execution gated until his explicit confirmation of a complete 68k experience.
[68000 runtime plan](m68k-runtime-plan.md) folds in the allowed interface registry,
JSR/JMP/nested guest-call service, typed built-in/guest dispatch and CODE-only
launch selection. No future VM/emulator run without his go-ahead. Own native
core/tools/hosted conformance are unaffected; no native application is built here.

Most CPU instructions/addressing modes, user stacks, callbacks, executable
intrinsic stubs/arbitrary patches, full historical gateway and complete OS
remain open. Later published manuals do not prove January1984 behavior. Physical
hardware, measured full budgets, parity and all three full-edition gates remain
unchanged; formal PC B2 is partial solely on criterion6's firmware image hash.
No Apple code/assets, emulator implementation or firmware internals were used.
UART source/test bodies and Claude's research originals were not edited.

## Retained verification history

The first focused command omitted the three standalone object build targets;
nine behavior suites passed and those three audits refused missing files. Building
the targets made all twelve pass with no source change. A prose locator was
corrected before publication. Earlier/final local manifests are retained. The
first direct CI-manifest equality check refused a function-frame list-order
difference; normalized comparison proves identical values and compiled objects.
The first push encountered a concurrent documentation-only asset-research commit
aa0b028; it was preserved through a clean rebase. No tested implementation bytes
changed. That research was not an input to this memory implementation.
