<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0089 - More visible paired cursor

2026-10-31; SRC-0586 / IMPL-0100 / TEST-0133. Original product decision for the current preview scenes. It supersedes only the seven-pixel cursor geometry in SPEC-0056, SPEC-0059, SPEC-0061, SPEC-0070 and SPEC-0071; the rest of those contracts remains in force. It makes no historical cursor claim.

Use the same 15-by-15 monochrome plus with a one-pixel white halo and hotspot (7,7) in the native Mac, native PC and Windows preview. The plus and halo are original project artwork. The logical pointer still ranges over the full 512-by-342 scene. At the edge, the artwork clips to the screen; it is not relocated away from its hotspot. Button, motion, cancellation and drag rules do not change.

On the PC, dirty publication must include old and new cursor rectangles `[x-7,x+8) × [y-7,y+8)`, with checked/clamped arithmetic, in addition to any scene dirty rectangle. Two disjoint cursors cover at most 450 logical pixels before clipping, not the former 98-pixel bound. No new framebuffer reads or cursor storage are required. Tests compare published pixels with the RAM canvas after movement including the corner-jump and recovery sequence, and verify the image and restoration on black and white backgrounds at every corner.

This is a visibility improvement only. It does not establish that the host supplies mouse events, that the Mac signals are decoded correctly, or that mouse dragging works. The [evidence](../development/cursor-visibility-20261031.md) records the actual build and test scopes.
