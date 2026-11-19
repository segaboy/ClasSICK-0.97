<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0230 - Native 286 low RAM mirrors

2026-11-19. SPEC-0139 / IMPL-0169. The project-authored
`boot-mac128k-ram.c` probes all 32 mirrors, writes across physical banks
and an interior alias boundary, verifies preservation at the aperture cutoff,
and exercises high mapped/unmapped memory, stale and permission cases. The
Windows-native harness compares loaded bytes and five independent physical
reads under BootHush. [Evidence and reproduction](../../docs/development/mac128k-ram-aperture-20261119.md).
