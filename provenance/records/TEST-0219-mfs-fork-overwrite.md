<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0219 — Persistent native MFS fork overwrites

2026-11-19. SPEC-0128 / IMPL-0158. `tests/mfs-write.c` passed 60
assertions in Release and ASan/UBSan, including refuse-before-I/O lock
checks. `scripts/Test-MfsWrite.py` passed 12 native Mac and 10 native 286
BootHush runs. Seven original-fixture exports matched independent full
image oracles and were read in new protected guests. Eight mutations
produced the expected refusal markers. [The canonical receipt](../../docs/development/mfs-write-20261119.md)
records budgets, hashes, coverage and the outstanding File Manager integration.
