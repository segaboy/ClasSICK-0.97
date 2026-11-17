<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0141 - Bounded read-only logical blocks

2026-11-17; SRC-0612 / SPEC-0111 / TEST-0200. Original native C storage service
in `core/storage/block-read.[ch]`: preflight, bounded caller staging, native
provider reads and output publication only after complete success. Native spans
are single-object/single-segment; no general segmented backend or disk format.
Original host and shared target fixtures, native Mac/286 wrappers and focused
verifiers provide the scoped checks. New Mac paths are
`probes/mac128k/block-read.c` and `scripts/Test-MacBlockRead.py`; they reuse the
unchanged original Mac reset/linker contract. Codex authored; lead contract/core
review and Mac binding/verifier contribution; no external
implementation/assets, protected inputs, copied runtime or historical algorithm.
GPL-3.0-or-later. Human provenance/toolchain and physical review remain open.
See the [canonical receipt](../../docs/development/block-read-foundation-20261117.md)
for file scope, tool identities, sizes, actual results and limitations.
