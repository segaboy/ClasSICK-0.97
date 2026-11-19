<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0158 — Existing MFS fork overwrite

2026-11-19. SPEC-0128 / TEST-0219. `core/storage/mfs-write.[ch]` and
`block-write.h` implement original bounded operations on an exclusive
mounted volume. The provider receives complete physical sectors. The caller
receives the definite prefix and failing sector on error. Format and lock
inputs are the reviewed SPEC-0112/0115 (SRC-0613/0614) and native provider
contracts SPEC-0125/0126. Codex used project code and independently generated
fixtures. No third-party implementation or protected media contributed.
GPL-3.0-or-later. Human provenance and physical validation remain open.
[Implementation and evidence](../../docs/development/mfs-write-20261119.md)
