<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0095 - Mac baseline pending-latch check

2026-11-03; SRC-0363 / SRC-0592 / SPEC-0072 / SPEC-0091. An independent review identified a race in the original capture policy: an enabled latch can close after the entry RR3 read but before all three equal samples, leaving its bit pending even though the accepted baseline already contains the new endpoint. In the existing source-informed latch model, the next PRIMING service then rejects that pending/unchanged pair. This model case is not proof that the same race caused the Snow observations.

This opt-in experiment adds one complete guarded RR3 read in WAIT only, after three stable samples and before publishing a baseline. Unsupported bits take the existing pending fault path. If any supported pending bit is set, leave the inbox in WAIT, clear remembered phase and return EMPTY; do not acknowledge the newly observed latch or infer movement. A later ordinary service may acknowledge it and attempt a fresh baseline. If no bit is set, retain the existing baseline behavior. PRIMING, READY, loss, cancel and normal builds retain their existing decision rules.

The late read consumes time and may delay baseline establishment during continuous movement. It does not prove atomicity, capture all physical edges, guarantee a rate, verify polarity or settle hidden even transitions. It may combine with fast collection and observation-only loss counters, but not other behavior probes. Host fixtures must inject a latch after the entry RR3 read, verify the original false loss, then check retry, unsupported late bits, a later coherent edge and both collection modes. Native, emulator, human and physical tests remain separate.
