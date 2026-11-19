<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0163 – Experimental boot-block entry on native Mac and installed 286

2026-11-19. Original Mac reset/IWM and installed 286 BIOS startup probes
load sectors 0 and 1 with SPEC-0130, initialize the four SPEC-0137 globals
and enter at logical $8002 under the convention specified in SPEC-0133.
The 286 uses four 32 KiB banks and a guarded instruction loop. The Mac uses
native instructions; BootHush supplies the finite observation window.
Neither lane interprets a protected file as an implementation source.

See [bounded startup transfer](../../docs/development/boot-handoff-20261119.md)
for register, RAM, dispatch and machine limitations and the exact evidence.
Project-authored GPL-3.0-or-later code, authored with published interfaces,
existing project services and original fixtures.
