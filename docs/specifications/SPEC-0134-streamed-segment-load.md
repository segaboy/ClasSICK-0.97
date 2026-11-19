<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0134 — Materializing the segment named by one Shell row

Use SPEC-0132 to resolve one jump-table row. In ordinary, inactive context,
copy the entire body of its referenced segment (resource bytes after its
four-byte header) into an exclusively owned, unpublished destination. Return
the copied size and a body-relative entry offset after all reads succeed.
The caller supplies a contiguous native buffer of at most 32,767 bytes.
The manifest, its original provider/media, destination and output must remain
live and disjoint for the full call. The buffer is writable until publication.

Refuse an invalid row, inadequate capacity or overlapping control objects before
writing the destination. Read in pieces of at most 256 bytes. A provider
failure after transfer begins clears the full attempted body and preserves
the result descriptor. Bytes beyond the attempted body remain unchanged.
There is no executable mapping, entry permission, A5 world, jump-table
patching or implicit invocation in this API.

The generated fixture may exercise the resulting entry on a native Mac or
with the 286 legacy execution interface. A separate copy-only build for an
explicitly pinned private input exports only status, copied extent and
entry-offset metadata. Its code contains no call into the copied body.
