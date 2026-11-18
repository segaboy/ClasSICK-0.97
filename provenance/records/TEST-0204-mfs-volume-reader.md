<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0204 — Native MFS volume and fork reads

2026-11-18; SPEC-0115/IMPL-0145. tests/mfs-volume.c,
probes/storage/mfs-volume.[ch], native wrappers and scripts/Test-MfsVolume.py.
Final-c report passes113202host evaluations each Release/sanitized, C16 small/
compact checks, four286and fiveMac baseline/mutation outcomes. Three routine
storage CTests also pass. Exact identities and failures are retained in
[the canonical receipt](../../docs/development/mfs-volume-20261118.md).
Original RAM-provider fixtures only; no File Manager/System boot/physical claim.
