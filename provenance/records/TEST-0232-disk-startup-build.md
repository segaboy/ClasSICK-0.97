<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0232 - Install and cold-boot generated disk workloads

`scripts/Test-DiskStartupBuild.py` builds independent twins, checks their
manifests, installs on an empty disk and compares its complete output. The
separate transport supplies a generated program that loops more than 256
times and executes its own code outside the boot blocks. Loader faults,
data variations and the 286 A-line cases have independent observations.
See [the build receipt](../../docs/development/disk-startup-build-20261119.md)
for input provenance, scope and measured results.
