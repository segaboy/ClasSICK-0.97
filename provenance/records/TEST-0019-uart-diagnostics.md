# TEST-0019 — UART transport and diagnostic native successor

2026-10-08. SPEC-0013 / ADR-0017 / IMPL-0017. Original register/ACPI/keyboard/
timer/memory fixtures and scene expectations; hosted callbacks only. Privileged
byte-port assembly is reused and inspected solely in unloaded owned images.

Portable startup suite checks an independent literal twenty-access transcript,
SCR restoration and 256 divisors across the full 16-bit range. Error suite corrupts
each of eight checked readbacks and tests all 256 LSR values. Queue suite exercises
512-byte atomic capacity, rejection without mutation, wrapped FIFO order, retained
bytes/guards, loss/sent saturation, THRE gating and final TEMT. Bounds suite covers
arguments/output stability, regression, all unfinished startup deadline positions,
100000-call pending-work rejection, elapsed timeout, corrupted state and UINT64_MAX.

Native gating/output/failure suites check no port or framebuffer access on gates,
exact ASCII records for repeat/releases/two Space presses/Escape, 24/32-bit wrap
60-second scene completion, final drain, retained header/ring and guard bytes.
Unsupported readback/line errors/stalled TX do not alter keyboard completion.
Timer failure, a frozen final-drain call bound, too-fast final-drain tick bound and
twenty-one rapid Space presses overflowing whole records retain honest diagnostics.

Incremental development uses C:\ClasSICK\uart-development-a. Initial failures
were test expectations: the last startup step was already READY (so an idle
timeout was inapplicable), and a reused timer read count made an intended late
fault occur at initialization. Overflow development initially used time progress
that emits only two frames for a one-second duration; rapid Space transitions
provide the correct adversarial producer. Its first drain expectation exceeded
the stated tick budget; a smaller synthetic step now permits completion while
loss remains observed. initial-tests-failed.log, overflow-fixture-failed.log,
overflow-gcc-failed.log and drain-fixture-failed.log retain those failures.
Nine new development Clang/GCC checks and prior keyboard regressions pass.

Final fresh full wrapper evidence is recorded in
[the UART snapshot](../../docs/development/uart-diagnostics-evidence.md).
The first full -a root passed normal Clang x64/i686 tests, then its sanitizer
build hit the native frame warning on a 2216-byte instrumented hosted frame.
CMake was corrected to retain the 2048-byte native/uninstrumented audits while
excluding instrumented host frames from that budget. The -a root/log are retained;
the complete final run uses never-reused uart-verification-20261008-b.
That full Windows PowerShell 5.1 -Sanitizers wrapper passed with exit zero:
Clang x64 100 per Debug twin/Release/ASan+UBSan and i686 76; GCC x64 85 per
Debug twin/Release and i686 61 (792 CTest checks across nine configurations).
Twelve Clang/GCC O0/O2 UART import/frame audits and two endian audits pass.
Four EFI twins pass 59 original symbols, three DIR64 relocations, one eight-byte
writable slot, 256 vectors, 23 mutation rejections and ten omitted-object links.
The snapshot/manifest retain all 59 labels / 53 distinct fingerprints, with 45
prior fingerprints unchanged, four changed and four new. Compiled/test/script
inputs remain unchanged after verification; final additions are evidence only.
No firmware/ports/VM/image was executed; native delivery/faults/B2 and all edition
boots remain open. Remote matching/publication and human provenance review pending.
