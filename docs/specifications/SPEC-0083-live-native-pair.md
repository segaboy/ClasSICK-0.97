# SPEC-0083 - Continuing native Mac paired-capture candidate

Original engineering composition,2026-10-29. Adopted for the owner-authorized
paired release work;implementation/qualification pending. Builds on SPEC-0081.
Recent paired-capture ROMs are finite MAR1 measurement programs;their deliberate
checkpoint must remain separate from a window used to test continued dragging.

Add --window-timer-live-pair as a separate explicit builder mode. Require
--window-drawing,--window-timer-poll-probe and --window-timer-refresh-probe.
Forbid idle/activity/recovery/first-input/defer/phase-idle modes. Divisors per
the amendment below. Existing --window-timer-pair-probe dependencies/defaults remain.
The new CLASSICK_MAC_TIMER_LIVE_PAIR flag defaults0 and only permits composition
of the existing paired-read callback with a continuing foreground loop.

The live profile has no measurement checkpoint. Retain all ordinary startup,
fault,exclusion,inbox and loss behavior. Use the same six reads/order,64NOP
recovery,register IO,decoder and default timer settings as the qualified pair route (explicit rate amendment below).
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

## Amendment 2026-10-29 - live pair timer divisor experiment

A first divisor-1 continuing Mac pair trial was paused at 40957 services with 3
deliveries, 332 losses and first reason 6 (two-bit phase jump); it gave no usable
motion or drag. To obtain a source-identified rate comparison, the live profile
may take an explicit --window-timer-divisor of 1, 4 or 16 only (other values
refused), using exactly the existing numeric reload 4096/divisor. Default is 1
and unchanged. Live still forbids checkpoint diagnostics, keeps symbol audits,
seals, IRQ/stack bounds and twin proof, and never halts for observation. Loss and
refusal behavior are unchanged. No MMIO, acquisition, assembly, decoder, inbox
or device-programming change beyond the existing reload parameter. Profiles:
native-window-timer-live-pair-v1 (divisor 1),
native-window-timer-live-pair-divisor4-v1,
native-window-timer-live-pair-divisor16-v1; the manifest records
programmed_reload_divisor and timer_reload_count while retaining live/default
identity. CI builds divisor 4 only, in a separate comparison artifact. This is an
original engineering experiment with no physical or historical qualification and
no claim that a different rate fixes loss or yields motion.
