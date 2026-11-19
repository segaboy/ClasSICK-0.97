<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0224 – Bounded original-ROM startup transfer

2026-11-19. The authored return case and the separate screen-pointer read/jump
prove that the same 68k disk program can take an observable control path on the
native Mac and installed 286. Malformed metadata and an I/O fault in the second
sector refuse entry. The private harness requires the precise previously
identified disk and a completed public-controls manifest, runs read-only and
records only numeric execution observations. No private opcode, memory span,
register dump, trace, graphic, or transformed container enters the public suite.

See [bounded startup transfer](../../docs/development/boot-handoff-20261119.md)
for the latest exact executable, outcomes, observation limits and source history.
