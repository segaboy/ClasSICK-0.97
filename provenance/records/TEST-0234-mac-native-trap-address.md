<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0234 - Macintosh disk-startup trap returns

2026-11-20. SPEC-0143 / IMPL-0173. The original authored guest
`tests/mac-startup-traps.S` and driver `scripts/Test-MacStartupTraps.py`
qualified direct Get/Set entries, real vector-10 returns, both A0 conventions,
CCR, nested patches, preserved registers and stack on the normal product.
Two negative disks reached the named terminal handler. Three cold boots passed
on the BootHush Macintosh 128K model. GPL-3.0-or-later.
See the [qualification receipt](../../docs/development/mac-native-trap-address-20261120.md).
