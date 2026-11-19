<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0225 — Load and invoke original disk-supplied code

SPEC-0134 / IMPL-0164. `tests/code-stream-load.c` exercises multi-chunk
reads, capacity limits, guard preservation and failures at each provider call.
`scripts/Test-CodeStream.py --load-segment` boots both native guests on
original fixtures and verifies data-dependent returns. A separate private
build returns numeric copy results without entering the loaded code.
[Exact results and scope](../../docs/development/streamed-segment-load-20261119.md).
