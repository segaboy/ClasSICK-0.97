# TEST-0007 — Independent keyboard FIFO and hosted input

Date: 2026-10-07. SPEC-0004 v1 / IMPL-0005. Codex-authored independent fixtures;
self-review only. [Pinned snapshot](../../docs/development/input-evidence.md).

Verify-Input.ps1 -Sanitizers uses the existing pinned LLVM-MinGW 20260908
(Clang/LLD 23.1.1), CMake 4.4.4, Ninja 1.13.2 and Windows PowerShell 5.1.
Fresh x64 Debug twins, Release, actual i686 and validated x64 ASan/UBSan run
24/24 CTest checks each. Intentional sanitizer defect controls are checked first.
ARM64 LE/BE compile/import checks are supplemental, with no runtime claim.

- Literal two-slot FIFO/wrap/full/pop/retry; sequence UINT32_MAX acceptance and
  exhaustion without wrap, reset/reuse, source and repeat preservation.
- Zero-capacity/null storage, null arguments, malformed metadata, size arithmetic
  limit, invalid source/key/action/repeat, error precedence and unchanged rejected
  output/descriptor/backing. Two queues remain independent across reset.
- Independent shifting-list oracle: capacities 0..4, every eight-operation trace
  over push Space, push Escape release, pop and reset: **327,680 traces** and
  **2,621,440 operations** per execution. Every pending record checked immediately,
  whole-array failure snapshots, guard/trailing records and final draining.
- Independent Windows table: 120 combinations of message/key/flags, including
  unsupported/system/text messages and high-bit flags; ignored output preserved.
- Hidden real Win32 window: synthetic press/release/repeat and dispatched native
  key messages feed one queue/consumer. Source/sequence/order/rendering verified.
  Sixteen pending repeats survive surfaced overflow and drain before recovery;
  synthetic Escape closes. These are delivered messages, not physical keyboard
  device injection or owner manual acceptance.
- Viewer pool: 1,342,744 bytes, record start 1,342,424, sixteen 20-byte records;
  one-byte-short input reservation fails without losing existing buffers. Prior
  scratch-boundary failure, surfaces, arena and presentation checks still pass.
- Optimized core objects have no imports/global data on executed x64/i686 builds
  and compile-only ARM64 LE/BE. Strict conversion warnings are errors.

The first local 23-check matrix passed. Test-only strengthening then inspected
all pending records each step and added the one-byte-short queue-storage check;
the complete fresh 24-check matrix passed. No compiler or conformance failure.
Artifact hashes and remote results are recorded in the snapshot after completion.

[Windows CI 37719495759 passed](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37719495759)
at e237cbb: source audit/setup, bootstrap twins, complete five-configuration matrix,
sanitizer detection controls and ARM64 LE/BE import checks. Six executable hashes
match local. No code/tests/scripts/build inputs changed after c99ec65.

Limits: Space/Escape only, no timestamp/clock, pointer/text/modifier support,
physical driver, m68k execution, second compiler, historical Event Manager,
B1/M0.1 closure or OS edition certification. No Apple inputs or release binaries.
