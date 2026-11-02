<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0093 - Mac pending-only idle experiment

2026-11-01; SRC-0592 / SPEC-0072 / SPEC-0091 / SPEC-0092. This opt-in engineering experiment addresses the observed pending-only, unchanged-endpoint rejection. After the existing collection, validation and triple-stability checks, a READY, unquiesced producer with a valid READY inbox, remembered phase, nonzero supported pending bits, unchanged A and B levels, and unchanged button returns EMPTY. The existing acknowledgment has already occurred. No pointer/button record is queued, no phase or button state changes, no loss is reported, and no extra hardware operation occurs. Other cases retain the existing policy. Default builds exclude this behavior.

This is deliberately weaker than the normal uncertainty rule: even unobserved transitions can leave an unchanged endpoint. The experiment may silently discard real motion; it is not an inferred property of the Mac or emulator, a correctness proof, a physical-hardware profile, or a release candidate. Do not use it to waive a failed input gate. `--window-pending-idle-probe` is limited to the interrupt window profile and may combine with fast collection and the loss-count diagnostic, but not timer or other probes.

Tests must establish the exact admitted condition and retain loss/fault behavior for changed levels, buttons, unstable samples, invalid inbox state, quiescence and non-READY phases, plus normal direction and button delivery. Native builds and target observations are separate from the model.
