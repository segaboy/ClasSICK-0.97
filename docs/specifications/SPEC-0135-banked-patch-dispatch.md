<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0135 - Executing a patched OS trap in guest RAM

2026-11-19. IMPL-0165 / TEST-0226. Adopt the published register-based
calling boundary reviewed in SPEC-0017 (Inside Macintosh I-87, I-94-95
and II-383-384) and the independent directory in SPEC-0020 and SPEC-0129.
This is an explicit register-based execution profile for OS words without
routine-specific flags. Individual routine signatures and January 1984
coverage require separate qualification.

An execution owner zero-initializes one patch state per CPU and keeps that
state with the same private CPU, gateway, backing and limits across steps.
The state and controls are separate aligned native objects. The prerequisites
for the SPEC-0129 guarded, stable ordinary RAM backend apply. For a patched
OS entry targeting admitted executable guest RAM, push PC+2 on the guest
stack, record A0/A1/D1/D2 and the return slot privately, and start at the
target. Support four nested patch calls. Keep at least one word of stack
below the return slot for the bounded delegated-instruction profile. Refuse
entry when a push overlaps the first word of the target or continuation.

An ordinary JSR/RTS pair inside the handler follows the 68000 engine.
When RTS uses the patch's saved slot, require the actual stack return
address to equal the recorded continuation, preflight the target, restore
A1/D1/D2, restore A0 for a clear bit 8, and derive NZVC from low D0 as in
SPEC-0020. The routine owns preservation of its other callee-saved
registers. A saved prior RAM routine can be called directly. Each entry
or epilogue performs guard and backend checks before publishing effects.
After entry can commit, four guest stack bytes are written atomically;
the private state and CPU frame follow. Return uses no memory write.

The ordinary Get/Set profile handles the unpatched native Get and Set slots.
Remapped RAM targets in those slots execute through the same guest route.
Toolbox, OS flag variants, direct native-token calls, unimplemented ROM
entries, interrupts, arbitrary exception frames and the guest-visible
low-memory trap table remain outside this profile.
