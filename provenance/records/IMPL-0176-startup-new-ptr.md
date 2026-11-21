<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0176 - Paired native startup NewPtr

2026-11-21. SPEC-0146 / TEST-0237. Original Codex-assisted work in
`personality/system1/startup-heap.[ch]` uses SPEC-0023's Memory Manager
results, alignment and zero-size rules and the existing low-memory writer.
Two 32-bit guest cursors avoid borrowing a contiguous native pointer to
segmented 286 memory. The bounded allocator needs eight bytes of persistent
state. Its fixed guest payloads are accessible to the calling 68k program.
Both normal targets initialize their directory and heap after obtaining the
startup sectors, then dispatch through the current slot.

On Macintosh, `platform/mac128k/disk-startup.c` bridges the shared allocator
to physical RAM. `arch/m68k/disk-startup-reset.S` supplies the callable
ROM entry and the real register result and return paths. The 286 shares
the allocation contract through its interpreter and native banked RAM.
The project selected zone geometry, side-state representation and failure
policy independently. The last-error word follows the inference distinguished
in SRC-0629. GPL-3.0-or-later. Human provenance review remains open.
