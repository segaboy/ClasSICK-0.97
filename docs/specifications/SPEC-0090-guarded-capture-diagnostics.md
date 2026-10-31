<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0090 - Guarded capture diagnostics

Two opt-in, host-only helper modes investigate the owned isolated VM window. They are not guest OS code. Both accept only the existing one-row zero-motion no-button plan, and default to dry-run. Existing process identity, executable path/hash, unique owned window, Default desktop, button mapping and held-button checks remain in force.

`--place-capture-origin` accepts only the VBox target. It moves the bound window to (-100,-100) without resizing, activation, input or changing system preferences. It reports actual client coordinates and whether the one previously observed screen point (2,2) currently belongs to that client. The point check is a read-only observation, not a prediction that later capture will place the cursor there. Ownership and desktop are rechecked after placement and geometry reading.

`--inspect-capture` reads the existing owned-window capture indicator after normal preflight and rechecks the base guards. It changes no capture state and sends no input. Neither mode authorizes an event: all later events still require foreground, process identity, Default desktop and physical owned-client containment immediately before each event. Guarded release behavior remains unchanged.

These tools cannot establish guest acceptance or physical-hardware behavior. The [test evidence](../development/host-input-20261031.md) distinguishes actual guest observations from API receipts.
