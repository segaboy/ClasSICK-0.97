<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0151 - Paired native resource metadata preflight

2026-11-18; SPEC-0121 composes existing SPEC-0115/0116 and the admitted native
block providers. Original `probes/storage/resource-media.[ch]` and target wrappers
enumerate an MFS directory and open each nonempty resource fork, publishing only
aggregate metadata and first-failure diagnostics. Completed-open counters and
failed-open view preservation are explicit. `scripts/Test-ResourceMedia.py`
qualifies both targets using the independently generated
`scripts/resource_media_fixture.py` fixture; its optional runtime route retains
classified refusal rather than calling it success.

[Canonical implementation/evidence receipt](../../docs/development/resource-media-20261118.md)
owns source/build identities, memory costs, selected numeric observations and
limits. Original GPL-3.0-or-later code; no protected implementation source or
owner media was supplied to the implementers. The coordinator's separately
authorized selected-media trial supplies numeric runtime evidence only. No
resource functionality, executable loading or System startup is claimed.
