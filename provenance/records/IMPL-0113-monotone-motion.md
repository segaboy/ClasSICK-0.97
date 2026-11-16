# IMPL-0113 - Optional monotone input coalescing

2026-11-16 follow-up: zero operands bypass general checked addition while
preserving SPEC-0097's sum and admission rules. Original portable code,
self-reviewed by Codex; [measurements and limitations](../../docs/development/input-followup-20261116.md).

2026-11-16: original compiler-assisted checked addition and selective private
helper inlining reduce the native producer cost without changing SPEC-0097.
The portable C addition path remains available to the 286 compiler. Codex
implemented and tested this increment; no external AI service or game code was
used. Tests, comparative builds, rejected experiments and modeled timing
limits are in [the Windows native receipt](../../docs/development/windows-m68k-toolchain-20261114.md#interrupt-mask-and-instruction-cost-investigation-2026-11-16)
and TEST-0189. Physical and release qualification remain open.

2026-11-05; SRC-0598, SPEC-0097, TEST-0160. Original GPL-3.0-or-later code in apps/window-scene/inbox.c/.h; SHORT-only binding in platform/mac128k/motion.c/.h and window-entry.c. Claude proposed the optional operation and API tests from reviewed original public inputs; Codex reviewed/integrated, added real-graph/native binding tests and MCM1 diagnostics. Strict operation and capacity16 remain unchanged; only eligible equal-button monotone unread tails combine. Claimed copies, button edges, reversals and hard-loss cancellation are preserved. [Executed checks and native limitations](../../docs/development/release-input-20261105.md#monotone-queue-fix). No general chronology, callback, physical or release claim.
