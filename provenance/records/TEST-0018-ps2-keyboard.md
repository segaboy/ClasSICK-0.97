# TEST-0018 — Polling keyboard and interactive native loop

2026-10-08. SPEC-0012 / ADR-0016 / IMPL-0016. Independently authored ACPI,
controller responses, scan bytes, memory and scene expectations. Hosted callbacks
only; privileged assembly is inspected in unloaded images.

Baseline replay: unmodified public f1e7a09 (compiled source c82abe7) passed the
full Verify-ProgressLoop matrix locally. UTF-16 progress-loop-local-a.log and
downloaded CI 37812572045 log retain the comparison: 49 labels / 43 distinct
artifact fingerprints match. After the replay one Debug directory was reused for
incremental development, so its binaries no longer represent the baseline; the
original log and hash records do. Final milestone verification uses a fresh root.

ps2.startup/errors/stream/overflow check the exact 18-write transcript, resetting
configuration after self-test, two FE retries for each keyboard byte and rejection
of a third, IBF gating, 64/65-byte drains, all 30 phase deadlines and call caps,
auxiliary/error replies, unexpected ACK/BAT/readback, clock regression and sticky
terminal failures. Expected normalized events cover press/repeat/release, orphan
breaks, E0/E1/Pause, auxiliary prefix preservation, transport recovery, every byte
alone, full FIFO state, sequence failure and diagnostic saturation. Every poll is
checked for the one-status/one-operation bound and invalid fixture access.

acpi.keyboard checks revisions 1–6 and all 256 low flag bytes, bit isolation,
absent/old indications, checksum/length/access/argument rejection and output
stability. native-keyboard.gating/interactive/failure use the real core FIFO,
timer extension, arena, band scene and presenter: no device access or framebuffer
write on rejected gates; repeat/release policy; two Space presses then Escape;
24/32-bit wrapping 60-second runs; retained Space offset; exact trace guards,
pixel oracle and scanline padding; startup failure, deadline and timer stall.

Development Clang Debug: ten new checks pass. Development EFI inspection passes
four O0/O2 twins, 53 named symbols, three DIR64 relocations, one eight-byte
writable slot, 23 corruption controls and eight omitted-object links. Its enclosing
ad-hoc launcher exited 1 due to a malformed exit expression after the wrapper
passed; the wrapper's complete results JSON/log are retained. Final full wrapper
uses the normal launcher and independently finished successfully (exit zero).

Final local/CI identities and hashes are recorded in the keyboard evidence snapshot.
The first full fresh root (ps2-keyboard-verification-20261008-a) stopped at GCC's
misleading-indentation warning in two test mains, after the Clang configurations
passed. The test formatting was corrected; GCC's ten new checks then passed.
That root/log is retained as failed development evidence; final verification
uses ps2-keyboard-verification-20261008-b, without overwriting the failed log.
That full matrix passed: Clang x64 91 per Debug twin/Release/ASan+UBSan and
actual i686 71; scoped GCC x64 76 per Debug twin/Release and actual i686 56.
The EFI controls and AArch64 LE/BE driver audits pass. See the snapshot for hashes.
Compiled/test/script source `e0b9d1a5116d033266dd0270d0f88a7c538b212c`.
Pinned [Windows CI 37820711475](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37820711475)
passes on that exact staging revision before main publication, with the same full
matrix and two additional bootstrap builds. All 55 local/remote comparison labels
/ 49 distinct artifact fingerprints match; 33 of the prior 43 are unchanged.
No firmware, native ports, VM, native scene or B2 has been observed. Human review
pending. Tests cannot establish physical device support or provenance certification.
