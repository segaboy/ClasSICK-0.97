<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0216 - Macintosh write/export/restart qualification

2026-11-18; SPEC-0125 / IMPL-0155. Release and ASan/UBSan each pass13,710
assertions. Eight original native runs verify raw/taggedDC42 write/read,
fresh read-only restarts, default protection and checksum-refusal/no-export.
Full exported images match independent expected byte edits; original inputs
remain unchanged. Routine CTest integration passes. Earlier missing padding
and incomplete roundtrip-review coverage remain recorded failures/limitations.
The [canonical receipt](../../docs/development/mac-floppy-write-20261118.md)
and its numeric companion own exact hashes, commands, modeled outcomes and
remaining historical/physical/filesystem/System-startup boundaries.
