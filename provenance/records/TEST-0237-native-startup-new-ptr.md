<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0237 - Call NewPtr from separately supplied startup disks

The project-authored callers in `tests/startup-new-ptr.S` run unchanged on
the Macintosh developer ROM and installed 286. `scripts/Test-DiskStartupBuild.py`
assembles them separately from either product, installs the fixed disk on
a fresh machine, and observes the guest's returned pointers, writes, flags,
zone choices, failed requests, patch and restoration. The build manifest
identifies the actual native service entry points on each target.
See the [generated boot receipt](../../docs/development/newptr-20261121.md).

`scripts/Test-MacStartupTraps.py` also booted four authored disks on the
qualified ROM. The new caller in `tests/mac-startup-traps.S` obtained the
plain NewPtr ROM address and used JSR through it for both success and
memFullErr. The caller verified the original plain payload bytes, D0's
high word, A0, D1/D2/A1, the restored stack and MemErr. The observed SR
was $2714 on success with X set, and $2708 on failure with X clear. Direct
execution reached its success witness after 4,245,482 modeled cycles.
The other three disks rechecked Get/Set and the terminal controls.
Report `C:/Repos/.task-temp/mac-startup-traps-20261121-b/report.json`,
SHA-256 `dfc46fb853c860def268b2259e54794b6f97a249a4996b08e22cefcda77ddc8c`.
