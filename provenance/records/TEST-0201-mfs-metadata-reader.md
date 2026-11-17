<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0201 - Paired native MFS metadata

2026-11-17; SPEC-0112; IMPL-0142; SRC-0613/0071/0105/0109/0612.
Original host Release/sanitizer fixtures each pass 24,300 check evaluations;
routine storage CTest passes 2/2. Native BootHush Mac/286 diagnostics pass seven
expected baseline/defect outcomes using a shared RAM-backed metadata fixture.
Overflow acceptance and an omitted entry bound are detected on both CPUs.
The final verifier snapshots declared source/helper/tool identities before
builds and confirms them unchanged afterward. The
[canonical receipt](../../docs/development/mfs-metadata-20261117.md) owns exact
commands, artifacts, sizes, corrected verifier limitations and remaining gaps.
Actual 286 disk-provider integration is TEST-0202. No complete mount, reference
System Disk boot, general segmented backend or physical compatibility is claimed.
