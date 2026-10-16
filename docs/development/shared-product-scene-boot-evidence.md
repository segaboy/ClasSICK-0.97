# Shared-scene boot captures - 2026-10-16

SRC-0213/0214; SPEC-0036/0037. The owner requested a screenshot of each current build
for a blog post. That fresh direction covered one isolated own-ROM Snow128K boot
and one isolated own-image VirtualBox boot for screenshots. Both sessions are closed.
No new runtime build or input qualification occurred.

The exact65536-byte ROM SHA-256 is
`95d7cdef447768586e2d8e9af18771afbe87f197c273269690650fc18e2fe542`.
Compiled source `a8b6aae8e583b26d16012d5af330b2a2092c6c59`; prior compiled-only
record `20efa50fd1ef4eb636609dfa2e4be738a5420233`. Current native fingerprints,
local/CI ROM bytes and complete compiler manifests still match. The existing
2217 local/2735 CI checks are reused, not re-executed by these screenshot sessions.

Snow v1.5.0-317c188, Early128K, fresh profiles, normal speed, RelativeHw, no
disks/shared directory/PRAM/extension ROM/serial bridge: the captured result shows
the new monitor/C mark, APP/097, hollow rectangle, six phases and0097. Input0000.
The original window PNG and clean crop are retained outside Git with identities
in the [machine-readable record](shared-product-scene-boot-evidence.json).

VirtualBox7.2.16r174877, a fresh isolated x86-64 EFI VM with only our independently
checked image, shows the same scene centered at(256,213) in1024x768. Logical canvas
512x342, six phases and0097, input0000; its existing timed progress is partway
through. The direct framebuffer PNG and full window capture are retained. One
initial capture request arrived before the display existed and returned0x0;
capture succeeded after the same boot exposed its display. No second boot.

The Mac camera-button attempt produced no additional file/dialog; a later window
maximize attempt reported unavailable input geometry. The delivered Mac image
is a crop of the successful full window capture, not an emulator screenshot export.
No scene pixels were regenerated, recolored or composited. The PC crop retains
its original512x342 pixels. Neither session includes a human-input observation.

These are project prototype observations, not historical Macintosh evidence.
The [compiled-only checkpoint](shared-product-scene-evidence.md) stays unchanged.
No desktop/disk-System boot or physical Macintosh/full128K qualification is
inferred. Native-app, source/UART, historical/parity/edition/B2/human gates and
both [System1/ROM requirements](system1-rom-boot-requirements.md) remain open.
Fresh owner go-ahead precedes future emulator/VM/reference trials.
