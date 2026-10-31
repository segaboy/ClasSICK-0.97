<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native input timeline observations

2026-10-31; SRC-0585, IMPL-0099, TEST-0132. The opt-in diagnostics and their limits are specified in [SPEC-0088](../specifications/SPEC-0088-input-timelines.md). They record original ClasSICK state; they do not fix or qualify host mouse operation.

## Mac

An isolated Snow Early128K session used the original timeline ROM SHA-256 `dbab7e3a60f5bb6f0432a17256db4a9ee7e327250452562812b70ea309d2b0b1`. The workspace path and ROM identity were checked before launch and the MTL1 marker was found in the paused guest. Eight small host-relative movements were requested; host capture and the association of individual actions with samples were not proven. The [manually transcribed timeline](../evidence/traces/2026-10-31/mac-motion-timeline.json) contains two accepted button-related samples and four separately recorded unstable triples, with additional records lost to overflow. UI setup and pause can contribute events; this recorder changes interrupt timing.

The four unstable triples each contain changes in both X-related signal values between observations. Ordered intermediate phases and direction are not established. A fresh scoped Claude review proposed two different intermediate histories consistent with the same observed endpoint values; it executed no tests and proposed no fix. The lead retained the existing loss policy. A hosted regression reproduces the first captured triple and checks that it is reported as loss without enqueuing motion.

The first attempted session was invalid: its copied workspace still referred to an older ROM despite the staged new ROM hash being checked. It was stopped and is not counted. The second session used a parsed and checked workspace. The new preflight checks the intended workspace and image, not what an emulator actually loads; the MTL1 marker separately confirms the diagnostic was running. Receipts remain outside Git under `C:\ClasSICK\mac-timeline-trial-20261031-a` and `-b`.

## PC

Two fresh isolated VirtualBox trials used the original timeline EFI SHA-256 `db3f7317389406ad15d2f80171bfbe14417656433a5efdb269bb1c7a0573a233` on the original disk SHA-256 `6cf0454cb8bc2e6ffe4021f7aeb268e47e82ac7bb75305f99e78f0313c054033`. The guest's own halt address and marker were checked before reading its registers. Both owned VMs were powered off and removed; existing VMs were preserved.

The first [record](../evidence/traces/2026-10-31/pc-timeline-a.json) contains no qualifying AUX bytes between READY and a virtual-keyboard stop marker following a host-window click. No deliberate host movement was issued during this interval, so the empty record does not identify the host movement failure.

The second [record](../evidence/traces/2026-10-31/pc-timeline-b.json) contains `38 FB FD`, with decoder counts `0 → 1 → 2 → 0` and a completed delivery. A separately identified virtual-device movement of `(-5,+3)` preceded the marker; the guest pointer changed from `(256,171)` to `(251,174)`. This demonstrates the instrumented guest handling that particular virtual-device event. It does not establish GUI capture, physical mouse movement, stream alignment generally, or dragging. Receipts remain under `C:\ClasSICK\pc-timeline-trial-20261031-a` and `-b`.

## Build and review

The final Mac diagnostic built as matching 68000 twins (23,768 ROM bytes; 964 workspace bytes). A fresh default Mac ROM built from the current source matches a clean build of the same committed baseline byte for byte, SHA-256 `9cb884200e14d830ba7addee1d3423719e9477a80d9b55e18c5c893e7fe54931`. PC timeline image verification passed four compiler/optimization twins, 27 corruption rejections and 28 omitted-object rejections. The changed default PC entry and window object files match fresh builds of the same committed baseline byte for byte; this is not a whole-image equality claim.

The hosted GCC suite passed the normal Mac motion model, the Mac and PC timeline tests, the PC sequence probe, the timeline reader and the workspace preflight. An earlier Clang AddressSanitizer/UndefinedBehaviorSanitizer run passed the four new timeline and reader/preflight checks; after adding the captured-triple regression, a repeat timed out on both native timeline executables after 15 seconds each, although the two Python checks passed. Its cause was not established, so the revised regression has a GCC pass only. Tests use original fixtures. No PC or Mac host-input drag test passed in these trials, and no paired release is qualified by them.

Claude `claude-opus-5-5` Low supplied the PC recorder proposal from a scoped original/public packet; it did not run tools or tests. The lead corrected its pointer access, READY checks and saturation handling, added fault observation, integrated and tested the result. A separate scoped Mac review received the captured values and original project sources, did not use tools or run tests, and advised against inferring direction. The two calls reported provider list-basis estimates of $0.3606322 and $0.2048002 respectively; these are not subscription charges or measured Codex savings. Remaining provider allowance was unavailable. Full bounded inputs and transcripts remain outside Git under `C:\ClasSICK\release2-completion-20261023-a\pc-timeline-20261031` and `mac-observed-timeline-20261031`.
