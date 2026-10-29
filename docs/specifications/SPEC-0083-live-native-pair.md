# SPEC-0083 - Continuing native Mac paired-capture candidate

Original engineering composition,2026-10-29. Adopted for the owner-authorized
paired release work;implementation/qualification pending. Builds on SPEC-0081.
Recent paired-capture ROMs are finite MAR1 measurement programs;their deliberate
checkpoint must remain separate from a window used to test continued dragging.

Add --window-timer-live-pair as a separate explicit builder mode. Require
--window-drawing,--window-timer-poll-probe and --window-timer-refresh-probe.
Forbid idle/activity/recovery/first-input/defer/phase-idle modes and a nondefault
timer divisor. Existing --window-timer-pair-probe dependencies/defaults remain.
The new CLASSICK_MAC_TIMER_LIVE_PAIR flag defaults0 and only permits composition
of the existing paired-read callback with a continuing foreground loop.

The live profile has no measurement checkpoint. Retain all ordinary startup,
fault,exclusion,inbox and loss behavior. Use the same six reads/order,64NOP
recovery,register IO,decoder and timer settings as the qualified pair route.
No new MMIO,assembly,device programming,sign inversion,loss relaxation,replay or
arbitrary callback. No claim that removing a checkpoint fixes Gray/coherence loss.

Builder/profile/manifest identities distinguish live from diagnostic. Native
proof still requires call-free64NOP pair,IRQ encodings,twins,seals,imports and
RAM/stack bounds;live rejects idle/activity/first-input checkpoint symbols.
Host scripted IO must exercise activity-off service beyond512passes and later
deliver further events. Planned cases are not executed results.

The existing finite diagnostics remain available. Default source behavior is
preserved when the new flag is0. Concurrent original cursor-art changes are
separate and may change whole-image identities. This does not qualify physical
timing,full128K,reference systems,human input,System1 or release readiness.
