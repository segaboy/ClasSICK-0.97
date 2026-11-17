<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0202 - Native 286 floppy provider

2026-11-17; SPEC-0111/0112/0113; IMPL-0143; SRC-0610/0612.
Six original native BootHush diagnostics pass: real-sector reads/bounds,
second-sector failure atomicity, wrong-head and ignored-carry defect controls,
actual-media MFS header/known-entry decoding and header-read failure.
Full loaded-payload equality, exact source/emulator identities and unchanged
input media are verified. The [canonical receipt](../../docs/development/pc286-floppy-provider-20261117.md)
owns exact observations, commands, hashes, review and limitations. These are
modeled native tests, not physical BIOS, complete filesystem or System boot.
