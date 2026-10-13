# Guest Memory Manager trap checkpoint - 2026-10-13

SPEC-0025 / ADR-0029 / IMPL-0028 / TEST-0033 / SRC-0110-0112.
Compiled source `7004ade44f2a5bd7a0bc7f5402a71deaadf418fe`; implementation bf71ed5; lead self-review,
human provenance review pending. [CI](https://github.com/segaboy/ClasSICK-0.97/actions/runs/38398755670) passes both jobs.

An original 82-byte 68000 program now reaches all eleven memory routines through
actual interpreted A-line instructions: 35 steps, 15 traps, signature47545231.
It allocates cleared handles/pointers, reads and writes through locked flagged
cells, preserves contents across resize/movement, copies, shrinks and disposes.
Typed logical identities preserve Get/Set visibility and saved-original calls;
incompatible signatures refuse and arbitrary guest code remains unexecuted.
CPU preflight protects exception scratch from low-memory/heap/BlockMove overlap.
DATA24 is explicit opt-in for data operands; control/implicit stacks and memory
trap arguments stay strict, and registers retain flag-bearing32-bit values.

The local nine-profile matrix passes1299 checks. After an extra unoptimized
object check found compiler padding initialization, all54 affected new CTests
were repeated on the final source; unaffected1245 checks were retained. CI runs
the entire1299-check matrix on the exact current revision. Ten extra binding
object audits pass Clang/GCC32/64/O0/O2 and both endian targets; hosted twins match.
GNU68000 passes14 module audits and two complete relocatable links, matching the
independent assembly bytes. Its complete manifest matches local exactly.
Binding state costs20 bytes on68000, in addition to the prior672 manager and256
guest cell bytes. Whole-closure compiler frame maxima remain608/604 bytes; these
are not stack high-water or final linked-ROM/RAM proof.

Six existing EFI profile hashes and both own ROM hashes/input manifests remain
unchanged. The concurrent first-application research merge adds documentation
only and is not implementation input. Claude's callback and two resource-fork
drafts remain untracked. No new boot binding, VM/emulator run, guest callback,
native application/SDK, source exposure, UART-body work or OneNote operation.
Old native observations retain their exact images. Full historical/OS/physical/
budget/parity/three editions and B2criterion6 gates remain open.

Retained preparation issues: a partial build omitted one audit-only object;
the object was built and its check passed. Earlier in-progress verification
folders were superseded after changes. GNU68000 exposed a report-copy helper;
an unoptimized64-bit compiler exposed padding initialization. Explicit field
copies/initialization remove both. The final audits above pass without target
runtime helpers; initial failed/partial outputs remain private, not acceptance.

[Machine-readable evidence](guest-memory-traps-evidence.json). Next: remaining
JSR/JMP and bounded guest-call execution, then separately reviewed callback
signatures and guest patches. Research source claims need review before adoption.
