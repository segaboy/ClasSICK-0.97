<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0157 — Streamed Shell jump-table inspection

SPEC-0127 / TEST-0218. Original freestanding implementation in
`personality/system1/code-stream.[ch]` and corresponding native probes.
Uses a prepared Shell resource fork with bounded row batches and one declared
offset policy. GPL-3.0-or-later. Codex implemented and tested from the
project's specifications, with generated fixtures and numeric coordinator
observations. No external implementation or reference executable was used as
a source. Independent review and historical offset equivalence remain open.
[Evidence and limits](../../docs/development/code-stream-20261119.md).
