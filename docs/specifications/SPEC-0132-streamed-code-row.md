<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0132 — Resolve one inspected CODE jump-table row

A future loader needs a segment resource and a resource-relative offset for a
given zero-based jump-table index. In ordinary inactive context, accept a
successful SPEC-0127 manifest and read only that eight-byte CODE0 row through
the original source. The caller retains the immutable manifest and its original
live source/provider/media for the full call. No inspection workspace is needed.
Use separate truthful aligned native objects contained in their respective
address segments. Reject overlap between the manifest and destination before
consulting the provider. Hidden aliases and reentrancy remain caller constraints.

Require a published tag, a recognized offset policy, consistent row/table
bounds and a matching segment descriptor for the row. Check the row's push,
segment identifier and trap against its group. Apply the manifest's offset base
once and require an aligned two-byte span entirely in the segment after the
four-byte header. On success publish the segment descriptor index and offset
from the start of that segment's logical resource. On failure preserve the
entire destination. A row resolution does not read the destination instruction
or authorize its execution. This is an original data-addressing API for both
SPEC-0127 engineering profiles.
