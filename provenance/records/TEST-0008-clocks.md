# TEST-0008 — Portable time, deterministic clock and Windows provider

Date 2026-10-07. SPEC-0005 v1 / IMPL-0006. Original Codex fixtures, self-review.
Exact source/artifact/remote snapshot: [clock evidence](../../docs/development/clock-evidence.md).

Pinned LLVM-MinGW 20260908 (Clang/LLD 23.1.1), CMake 4.4.4, Ninja 1.13.2,
Windows PowerShell 5.1. Verify-Clocks.ps1 -Sanitizers passes **34/34** checks in
fresh x64 Debug twins, Release, actual i686 and validated x64 ASan/UBSan. Existing
graphics, arenas/input tests still pass. Intentional sanitizer detection controls
precede instrumented tests. ARM64 LE/BE objects compile/import-audit only.

- Literal carry/borrow, immediately before/equal/after deadline, max canonical
  value, invalid units, nulls, backward readings, overflow and rejection precedence.
  Fake zero/positive advancement, equal samples, reset epochs and independent clocks.
- Independent wide-integer scalar oracle: **5,184 boundary pairs** check addition,
  elapsed subtraction and deadline comparison, including UINT32_MAX seconds.
  Output sentinels and guards check rejections and neighbor fields. Core never
  calls this oracle or uses its uint64 arithmetic.
- Thirteen independent literal host tick/frequency fixtures, plus bad frequency,
  arithmetic limits, unavailable/negative/backward samples, equal samples and
  successful retry. Raw regressions reject even if rounding yields equal time.
  Live QPC init and 1,000 nondecreasing reads per execution.
- Hidden fake-clock viewer checks rendered strip pixels at 249,999,999ns and
  exactly 250,000,000ns, repeat non-rearming, native-message Space input, delayed
  wakeup and timer destruction. Another hidden viewer waits for a real delivered
  Windows timer/QPC sample, paints and closes; 15s CTest bound, no exact-delay promise.
- Optimized core has no undefined symbols/global data on x64/i686 and compile-only
  ARM64 LE/BE. Strict conversion warnings are errors. No extra tools/runtime.
- Fresh clock-test, Windows-clock-test and updated viewer executable hashes match.
  Five other test executable hashes retain their preceding values.

The full local matrix passed on the first run, without compiler/conformance failures.
Remote success is recorded only after observing completion. Windows nanosecond
units do not establish physical accuracy or scheduler guarantees; positive
frequency > UINT32_MAX is explicitly unsupported by this provider contract.

No interactive debugger acceptance, B1 closure, event timestamps, historical
TickCount, calendar/scheduler, m68k/ARM64 runtime, mini vMac/physical Mac or OS
edition result. Installed VirtualBox availability is a future PC test option.
