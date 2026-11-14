<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0102 - Experimental 286 keyboard pointer mapping

2026-11-14; original interaction choice under the
[Mac/286 owner direction](../development/mac-286-paired-direction-20261114.md)
and SPEC-0051. Status: modeled diagnostic input, not a reviewed production
keyboard driver or human/physical qualification.

The caller initializes an owned state with button released. For each delivered
BIOS-style 16-bit key word, high-byte scan codes `4b`, `4d`, `48`, `50` map to
left, right, up, down respectively in 16-pixel increments. Otherwise an ASCII
low byte `20` toggles the logical held button. Arrow scan codes take precedence
over ASCII. An accepted key emits one shared pointer sample with the current
button state; unknown keys, null pointers and an invalid held state are refused
without changing state or output. State and output are valid nonoverlapping
objects. The caller owns input delivery, retries, and any recovery when a
downstream consumer fails after the state has changed.

This is a latch: a separate Space event releases a held logical button. BIOS
key words do not establish physical release edges, key-repeat behavior, exact
keyboard layout or rollover. Repeated Space events toggle repeatedly. The
milestone's once-per-physical-press and human-input criteria remain open;
this mapping does not waive them. Source, tests, images and limitations are
in [the shared-window evidence](../development/mac-pc286-shared-parity-20261114.md).
