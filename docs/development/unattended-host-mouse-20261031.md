<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Unattended host mouse checks

2026-10-31; SRC-0584 / TEST-0131. These two isolated original-candidate checks were performed without a human operator. Neither is a passing host-mouse drag test or paired-release qualification.

## Mac

One Snow Early128K session used the original interrupt-based ROM SHA-256 `673a2cca57d1ec9ffd842f4c858fa4aafbcb042ec6f86445ae5717d4816c4965`. After an automated click in the scene, the process-bound guarded helper requested eight relative `(-1,0)` movements. Its console output reported eight Windows-accepted events, verified foreground and client containment; it did not establish emulator capture or guest acceptance. The helper's full console output was not separately captured to a file.

The later paused own MWP1 v2 trace at `0xB70` showed 9,206 services, ten baselines, ten losses, zero deliveries, pointer `(256,171)`, and window X `80`. There was no pre-input trace, and setup, click and pause actions can contribute to these counters; the losses cannot be attributed to the eight helper movements. The pointer remained at its initial position. No drag or release was qualified. The owned Snow process was externally closed after 131.72 seconds under its 420-second watchdog; the ROM identity was unchanged. Binding and launcher receipts remain outside Git in `C:\ClasSICK\input-qualification-20261027-b\snow-unattended-20261031-b` and `C:\ClasSICK\mac-host-unattended-20261031-b`.

## PC

One isolated VirtualBox 7.2.16 session used original EFI SHA-256 `e634d1a4831dfd9970d839ffb6b6c596bea4315e4fc65db013b4b7ea3a00b955` and disk SHA-256 `fa627d5798f80c8e0b639aa6f4585bca73f29e375e3534ea39fdfb0ef0ee85d0`. After an automated click, the process-bound guarded helper reported a captured indicator, but the physical cursor position `(2,2)` failed its client-containment guard. It refused before sending any of the eight requested movements: zero attempted and zero delivered. The original pre-input and post-click guest framebuffers were byte-identical, SHA-256 `7bd4dc9c78b1729f5d88da5f57ee1813666714caf17fce88913d97cee4abc5a7`.

As a separate E2 control, a virtual-device API requested `(-12,-12)` with no buttons. The guest framebuffer then differed from the preceding one in bounding box `(497,369)-(516,388)`, consistent with pointer movement. This is not evidence of host GUI movement or a drag. The automated runner's immediate poweroff verification failed; a subsequent independent query confirmed the exact VM was powered off, after which it was unregistered. Its disk roundtrip and the prior VM inventory and running-state lists were verified unchanged. No semantic guest shutdown is claimed. Original framebuffer exports and receipts remain outside Git at `C:\ClasSICK\pc-host-unattended-20261031-b`; the helper binding is at `C:\ClasSICK\input-qualification-20261027-b\pc-unattended-20261031-b`.

Neither test identifies the root cause. The earlier E2 Mac drag and PC device movement remain separate from the unpassed host and human input gates. No release or physical-hardware claim follows.
