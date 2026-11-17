<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0142 - Bounded MFS metadata reader

2026-11-17; SRC-0613/0071/0105/0109/0612; SPEC-0112; TEST-0201.
Original native C volume-header and single-entry reader in
`core/storage/mfs-read.[ch]`, with SPEC-0111 acquisition, bytewise fields,
copied raw metadata, checked geometry and no fallible publication step.
The bounded shift/add geometry check avoids native wide-division imports.

Original host fixtures, shared native probe, Mac/286 wrappers and focused
verification script implement the contract's checks. CMake includes the
routine `storage.mfs-read` test. The lead admitted published field facts,
reviewed the implementation and requested removal of unnecessary temporary
copies; bounded Codex workers authored implementation/tests and independently
reviewed native safety. No manual, private HQ, protected media, held UART or
untracked research original was supplied to workers. No Apple implementation,
game-derived code or external runtime is incorporated. GPL-3.0-or-later.

The [canonical receipt](../../docs/development/mfs-metadata-20261117.md) owns
actual results, identities, native budgets and unresolved scope. This is not
directory enumeration, a chain decoder, File Manager or System boot.
Independent human provenance and physical validation remain open.
