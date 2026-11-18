<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0145 — Bounded read-only MFS volume service

2026-11-18; SPEC-0115/SRC-0613/0614/TEST-0204.
core/storage/mfs-volume.[ch] implements owned metadata/map snapshots, strict
directory traversal, counted-byte lookup, complete fork-chain validation and
staged byte-range reads. Algorithms and fixtures are independently authored.
Codex implementer plus independent original-test author and read-only review.
[Canonical receipt](../../docs/development/mfs-volume-20261118.md) owns identities,
tests, measurements and limits. No external implementation/protected bytes used.
GPL-3.0-or-later; physical/historical/human provenance gates remain open.
