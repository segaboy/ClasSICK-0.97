<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0228 - Startup pointer publication

`tests/startup-globals.c` checks the entire guest buffer, including the vectors
preceding the writable window and the bytes after it. It exercises exact
minimum capacity, a one-byte short window and a null pointer.

The hosted CTest `startup.low-memory-pointers` passed on 2026-11-19.
Both native handoff builds use the same freestanding implementation.
The [disk handoff receipt](../../docs/development/boot-handoff-20261119.md)
records the authored disk controls and supplied-disk observations.
