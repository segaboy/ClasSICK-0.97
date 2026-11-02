<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0092 - Experimental Mac loss classification

2026-11-01; SRC-0592 / SPEC-0072 / SPEC-0091. The existing first-loss record retains only one reason. This opt-in engineering diagnostic counts accepted loss transitions by the existing reasons NO_PHASE, UNSTABLE, PENDING and OVERFLOW, in that order. Each 32-bit count saturates; a fresh prepare clears them. Already-LOSS services and hard faults that fail the inbox transition do not add a count. No result, hardware read or write, decision, input record, recovery or first-loss behavior changes. It adds interrupt work and may alter timing; no measurement proves physical behavior.

When the interrupt-window profile enables `--window-loss-counts`, the foreground copies those counters under its existing exclusion into the separate 20-byte MLC1 record: big-endian tag `4D4C4331`, then four counts in reason order. MWP1 remains unchanged. The record is absent from default builds. The option may combine with fast collection but not the other diagnostic or timer profiles. The builder records and bounds-checks its own RAM address. A partial or stale foreground snapshot is not a chronological event log or an assertion that UI setup caused an observed loss.

Tests must cover reset, all four reason classes, saturation, repeated recovery, already-LOSS and fault behavior with both normal and fast collectors. Target tests and physical behavior are separate; this diagnostic does not fix or qualify mouse input.
