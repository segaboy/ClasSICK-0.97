# TEST-0017 — Cooperative timed progress loop

Date 2026-10-08. SPEC-0011 / ADR-0015 / IMPL-0015; original synthetic counters and
ACPI fixtures only. No firmware, VM, port access or visible native output.

Suites `native-loop.gating/progress/failure` (x64) cover all argument and gate
results, with no port read and an untouched framebuffer. Six progress cases cover
24/32-bit counters, wrapping starts, 1,000-tick to one-second steps and 1/7/60-second
durations. They check exact frame counts (2, 8, 17), elapsed time, read count,
maximum delta, final step 16 and the final frame against the scene oracle. Late
samples above half the period are counted; a stall ends after exactly 20,000,000
unchanged reads; out-of-width counter values are rejected.

Compiled/test/script source `c82abe79a31da20e1b325bb17e0f4fbd999122ba`. Pinned Windows CI
[37812572045](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37812572045)
(workflow dispatch on `claude/ci-staging` before `main` advanced) passes: 231-file
guard; Clang x64 81 checks per Debug twin/Release/ASan+UBSan, actual i686 65; scoped
GCC x64 66 per Debug twin/Release, actual i686 50. EFI twins with 43 named symbols,
21 image rejects and four omitted-object links pass. Of 43 named fingerprints, 35
are unchanged. Six change as expected (native-gate and native-timer tests, which
now link the loop, and both EFI images) and two are new. See
[the snapshot](../../docs/development/progress-loop-evidence.md).

Unverified: real timer reads, the visible bar, 60-second agreement with an
independent clock, VM pause behavior, keyboard and B2. Human review pending.
