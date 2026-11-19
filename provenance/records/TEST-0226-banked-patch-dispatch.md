<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0226 - Patched A-line execution and return

2026-11-19. SPEC-0135 / IMPL-0165. The project-authored fixture executes
A047, A146, A017, A117 and a saved guest address. It checks the current
handler, direct chained call, A0 conventions, preserved registers, SP, PC,
condition codes and caller continuation. Hosted cases add nested handlers,
depth limits, malformed and overlapping state, bad return addresses and
failure preservation. The [receipt](../../docs/development/banked-patch-dispatch-20261119.md)
identifies the native BootHush environment and fault controls.
