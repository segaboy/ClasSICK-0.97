<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0227 — Guest PBRead/PBWrite on disposable MFS media

SPEC-0136 / IMPL-0166, 2026-11-19. `tests/file-io.c` exercised malformed
guest windows, provider failure and mark/count behavior. Native guest
probes called canonical A002/A003, checked the numeric PB and D0 results,
and read the saved bytes after restart. Three write/restart pairs passed
under `scripts/Test-MfsWrite.py`. The [canonical receipt](../../docs/development/file-io-20261119.md)
gives the full 22-run suite, hosted checks and remaining integration limits.
