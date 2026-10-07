# TEST-0006 — Independent arena conformance and viewer ownership

Date: 2026-10-07. Contract SPEC-0003 v1, implementation IMPL-0004.
Fixtures independently authored by Codex from the contract; self-review only.
Exact source/artifact snapshot: [arena evidence](../../docs/development/arena-evidence.md).

## Executed local results

Pinned LLVM-MinGW 20260908 (Clang/LLD 23.1.1), CMake 4.4.4, Ninja 1.13.2,
Windows x64, built-in Windows PowerShell 5.1. Verify-Arenas.ps1 -Sanitizers passed.
Fresh debug-a/debug-b, x64 Release, actual i686 Windows execution, and validated
x64 ASan/UBSan each pass **16/16** CTest checks. Intentional heap overflow and
signed overflow detection controls pass before instrumented conformance.

Added checks: arenas.layout, errors, matrix, ownership, freestanding and
viewer-memory. Existing surface, GDI conversion/bounds/pixels, window lifecycle
and bootstrap checks also pass. Surface/presentation-test executable hashes remain
unchanged; arena-test and updated viewer fresh-twin hashes match.

- Literal offsets 0/4/8 in a 13-byte region exercise alignment holes, exact full
  allocation, exhaustion, reset/reuse and padding/trailing-byte preservation.
- Empty null/non-null pools; null arguments; zero-size/invalid alignment;
  malformed metadata; SIZE_MAX and PTRDIFF_MAX requests; cumulative overflow
  from a small valid arena; explicit error precedence and rejection transactionality.
- Independent matrix: capacities 0..65, every prior cursor, requests 0..67, all
  power-of-two alignments through tested max_align_t alignment 16. **751,740**
  whole-buffer cases per execution. Oracle scans offsets using remainder; it does
  not call a core helper or reproduce the core bit-rounding expression.
- Two disjoint arenas retain independent cursor/bytes across interleaved
  allocations/reset. Typed native record creation uses host-allocated storage.
- Successful viewer mode checks literal color/mono/scratch offsets and the full
  1,342,422-byte budget before actual hidden-window paint/switch/close checks.
  One-byte-short backing fails scratch reservation with used=683,730 and no scratch
  pointer; host-owned storage is released after callbacks/spans retire.
- Optimized uninstrumented x64/i686 core objects reject imports/global data.
  ARM64 little/big-endian arena objects compile and pass the same audit; no runner
  or native ARM64/m68k execution result. Strict warnings are errors.

No implementation/compiler/test failures occurred in this local matrix. A
documentation patch context mismatch was corrected before publication; it did
not modify or rerun implementation. Remote CI is recorded only after completion.

## Limits

Live truthful max-aligned backing, non-aliasing descriptors/outputs/pools and caller
synchronization/lifetime are preconditions. No individual free/rewind, stale-span
detector, secure erasure, hostile isolation, over-aligned/page allocator or general
heap fragmentation test. The tested contract is generic native memory reservation,
not historical Memory Manager semantics or a Macintosh 128K footprint result.

No B1 closure, OS image, historical compatibility, mini vMac/physical Mac boot,
second compiler, edition parity or Linux divergence claim. Static archive
reproducibility remains unproven because COFF timestamps vary. No release binary
is published; hosted runtimes remain separate development dependencies.
