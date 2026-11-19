<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0169 - Optional native 286 post-overlay RAM map

2026-11-19. SPEC-0139 / TEST-0230. Original Codex-assisted scalar adapter
in `platform/pc286/mac128k-ram.c` uses the validated bank provider, numeric
address folding and whole-transfer admission. Two stable aligned RAM words
can represent a four-byte access across an interior 128 KiB boundary.
Higher addresses retain the existing map. Sources are the published 1985
hardware interface and project-owned provider. No third-party code or
assets added. Human review remains open. [Evidence](../../docs/development/mac128k-ram-aperture-20261119.md).
