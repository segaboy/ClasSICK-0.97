<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0617 - Later published Macintosh boot-header metadata

2026-11-18; primary-source scope for SPEC-0119. Apple, Inside Macintosh: Files,
1992, printed 2-57 through 2-59, Boot Blocks:
https://developer.apple.com/library/archive/documentation/mac/Files/Files-101.html.
Official chapter PDF:
https://developer.apple.com/library/archive/documentation/mac/pdf/Files/File_Manager.pdf,
PDF pages53-55. Apple Technical Note DV03, March1987:
https://developer.apple.com/library/archive/technotes/dv/dv_03.html.

These published sources establish the common header's field order and widths,
counted15-byte filenames, configuration counts and heap-size fields. DV03 also
identifies version at offset6, page flags at8 and a later heap field at134.
The two logical startup blocks are acquired using the existing block contract.
The opaque entry field and subsequent machine code are never decoded, inspected,
exported or used to implement this metadata service.

Limitations are material: the 1992 signature description is explicitly HFS;
its two explanations of old-format version13 execution contradict each other.
The 1987 note limits the offset134 heap setting to machines above128K. Neither
source establishes an original128K/MFS entry address, initial registers, stack,
low-memory state or transfer/return convention. The independently authored
metadata reader exposes configuration without resolving those dispatch rules.
An observed selected-disk metadata match cannot promote later prose to a
January1984 compatibility contract.

Search-index text of the published1985 manual III-42 supports only the general
startup-sector sequence; direct access to that mirror failed. It is not admitted
here for unobserved entry details. No confidential drafts or implementation
sources are used. Facts and original validation rules are separated; no manual
sample implementation is copied. System/Finder execution remains a later step.
