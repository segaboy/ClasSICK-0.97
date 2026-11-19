<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0223 — Checked row resolution

SPEC-0132 / IMPL-0162. Original `tests/code-stream-row.c` verifies resolved
offsets and groups for every row in two variants of the generated volume.
It counts bytes and provider calls, tests argument and read failures,
and uses manifests after discarding inspection workspace.
[Builds, checks and limitations](../../docs/development/code-stream-20261119.md).
