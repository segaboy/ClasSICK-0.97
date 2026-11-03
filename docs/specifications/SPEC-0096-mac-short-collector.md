<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0096 - Experimental shorter Mac input collection

2026-11-03. This opt-in experiment builds on SPEC-0072, SPEC-0091 and the admitted interface facts SRC-0363 and SRC-0592. It is not the normal or a qualified physical-hardware profile. It may combine with SPEC-0095 and observation-only loss counts. It requires the fast collector and excludes the other behavior probes and timeline.

The SCC write guard uses eight NOPs and the read guard five, rather than sixteen on each access. For a channel already pending in the entry RR3 snapshot, the collector reuses its first RR0 byte as its second sample before acknowledgment. Nonpending channels retain both reads. This assumes the pending DCD latch remains closed until acknowledgment; other bits in the reused byte are not a second observation and can be stale. The existing strict ambiguity and loss rules remain in force. No unobserved direction is reconstructed.

The latch model can compare this change with the original collector, but does not establish real bus timing or hardware behavior. Timing, SCC reset-command spacing, latch behavior and complete input delivery require target qualification. An emulator drag observation is not a physical or general usability pass. Release criteria remain unchanged.
