<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0613 - Published MFS metadata fields

2026-11-17; scoped admission of field facts from the already eligible SRC-0071
published 1985 Inside Macintosh Volumes I-III, Apple Computer/Addison-Wesley.
The owner-provided PDF is 19,245,372 bytes, SHA-256
`b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`.
It remains outside Git. This is the published edition, not an excluded draft.

The lead reread printed II-120/121/122/123, Figures 6/7/8, using exact selected
PDF pages 688/689/690/691 in that identified copy. Earlier SRC-0105/0109 reviews
also visually checked the relevant diagrams/tables. This admission carries the
numeric disk-layout fields, 512-byte block and entry-boundary facts, two-block
master region, attribute/entry bits and counted raw names into SPEC-0112.
The specification independently restates those facts; no manual text, figure,
sample, protected implementation or asset is copied into source or fixtures.

The source does not establish January 1984 compatibility, inactive-record
traversal or the byte/nibble order of the 12-bit allocation map. Those gates
remain open. Capacity/range checks, failure behavior, API, buffer ownership and
the native diagnostic transport are original project engineering decisions.
Directory placement inside allocation storage is permitted by the source;
metadata decoding does not establish its reservation/ownership correctness.

The implementer receives the reviewed SPEC-0112 table and public project
records, not the owner's PDF, protected media, private HQ or untracked research
originals. Original code and generated fixtures remain GPL-3.0-or-later.
Lead self-review is recorded; independent human provenance review remains open.
