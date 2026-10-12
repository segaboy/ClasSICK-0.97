# TEST-0026 - Macintosh 128K ROM build, hosted checks and trial record

2026-10-11, SPEC-0018 / IMPL-0022 / ADR-0021. Initial status: **hosted/unloaded
verification passed; Snow trial not yet run**. Later trial observations must
be recorded separately without rewriting this checkpoint as an earlier boot.

24 hosted suites pass in Clang x64 O0/O2, Clang i686 O2 and GCC x64 O2.
Each helper suite uses 100,000 arithmetic pairs against 64-bit host oracles,
signed-minimum/power-of-two cases and memcpy guards. Each profile also passes
controller writes/flag acknowledgment, mouse transitions/hold/saturation,
stalled VBL/synchronization/completion, scene pixel/guard and CRC fixtures.
Four separate zero-divisor child processes fault as required; not ordinary returns.

Relocated Ubuntu GCC 13.3.0, binutils 2.42: four full ROM builds (two each Os/O0)
pass 68000 assembler enforcement, ELF32 big-endian/vector/entry/RAM/ROM bounds,
undefined-symbol rejection, loaded-image seal audit and deterministic twins.
Every build rejects seven own-image corruptions and six omitted source objects.
No target runtime archive or startup library enters the link.

Os uses 3,168 ROM bytes; O0 4,588. Both allocate 28 bytes of data/BSS and reserve
8 KiB for stack. Largest reported frame is 100 bytes Os / 84 O0; all listed
frames plus 64-byte margin total 772 / 640 bytes. The sum is conservative for
this reviewed nonrecursive call graph, not a measured stack high-water mark.
GCC reports bounded argument-push frames as dynamic,bounded; unbounded frames
are rejected. Failure history retains the earlier overly strict static-only
audit and its correction, not a compiler/ROM runtime failure.

Image hashes, source-input fingerprints, tool identities and scope are in
[the evidence snapshot](../../docs/development/mac-rom-evidence.md). Native code
has not executed in this initial record. No operator, time calibration, physical
hardware, System 1 compatibility or full edition result follows from these checks.
Original fixtures/code assisted by Codex; human provenance review pending.
