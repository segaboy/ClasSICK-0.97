<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0209 - Native MFS reads through modeled floppy media

2026-11-18; SRC-0612/0613/0614/0615; SPEC-0111..0115; IMPL-0144/0145.
Original `probes/storage/mfs-media.[ch]`, native Mac/286 wrappers and
`scripts/Test-MfsVolumeMedia.py` integrate the existing MFS service with actual
modeled block providers. Final-e passes six original-media baseline/fault/fresh-
recovery cases. The coordinator's final-b selected-private-disk trial passes two
cold runs per native target: ten files, twenty forks, 284392 logical bytes and
matching digest1229986664. All source/tool/media identities remain unchanged.

[The canonical receipt](../../docs/development/mfs-media-20261118.md) and its
sanitized numeric companion own exact identities, cycles, retained verifier
failures, original DC42 controls and scope. Private commands/transfer images and
protected bytes are excluded. Only the coordinator ran the owner-selected disk;
the harness/receipt implementer used original fixtures and a counts-only report.

The late-file fault proves preservation of one failed256-byte request; TEST-0204
separately covers multi-sector publication. The286transfer is diagnostic, not the
final product bridge. No System/Finder startup, application execution, writes,
persistence, historical-version identity or physical-hardware qualification.
