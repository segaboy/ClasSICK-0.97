# IMPL-0006 — Portable monotonic time and hosted timed behavior

Date 2026-10-07. Owner SRC-0023, independent SPEC-0005 v1, TEST-0008.
Codex project lead self-review; human provenance review pending.

Permitted inputs: project policy/contracts, owner continuation, C11 interface
semantics SRC-0020, Microsoft interface prose SRC-0024 and pinned tool headers.
Original canonical seconds/nanoseconds operations, uint32 carry/borrow, checked
overflow/backward rejection, monotonic observation and fake advancement. Core
has no allocation, host calls, floating point, 64-bit arithmetic or global data.

Windows provider contains host-only uint64 quotient/remainder conversion, bounded
frequency, raw and normalized monotonicity, explicit unavailable/error handling,
injectable raw transitions and actual QPC reads. No external library introduced.
The native viewer uses a timer only to wake, checks a core deadline on a fresh
sample, and flashes an original 250ms strip on Space. Fake and hosted sources
share the sampling/deadline/render path; repeat/release does not rearm.

Independent oracle uses uint64 scalar nanoseconds in host tests, separate from
core pair arithmetic; literal conversion fixtures include non-divisible ratios
and near-limit values. Tests inject failures/raw regressions and exercise live
QPC and a delivered Windows timer. Whole output/state fields/guards preserved.

AI: Codex GPT-6-family contract/code/test authoring and self-review using permitted
inputs. A visible adjacent Microsoft example is disclosed in SRC-0024; it was not
used as replacement implementation. No third-party/Apple implementation or assets
copied. Project-owned work GPL-3.0-or-later; no staffed/legal certification claim.

VirtualBox 7.2.16r174877 was observed by read-only version query under SRC-0023.
No VM/startup/firmware adoption or boot verification. B1/debugger, native timers,
m68k budgets/boot, exact historical semantics and all edition gates remain open.
