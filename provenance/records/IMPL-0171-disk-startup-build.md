<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0171 - Native disk-startup developer products

`scripts/Build-DiskStartup.py` links the two independent entries under
`platform/mac128k` and `platform/pc286` with the source lists in SPEC-0141.
The Mac uses its own reset and failure vectors. The 286 reuses the
SPEC-0110 media packager and SPEC-0135 guest handler dispatch. Tool and
source identities, native layout and install image hashes accompany the
two artifacts. See the [integration receipt](../../docs/development/disk-startup-build-20261119.md).
