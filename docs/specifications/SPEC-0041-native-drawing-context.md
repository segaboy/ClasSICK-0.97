<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0041: Native drawing view/context

Version1 accepted2026-10-17; SRC-0240..0243 / ADR-0047 / IMPL-0045 / TEST-0050.
Original native engineering contract; no new historical fact or guest QuickDraw ABI.
This is not a boot observation, release or target trial.

## Scope
This is original portable C for native Mac/PC builds. It adds a drawing-port origin
and a rectangular clip on top of the unchanged SPEC-0036 `cs_qd_rect_port` and
`cs_qd_rect`. It is not a guest GrafPort, and it adds no InitGraf/GetPort/SetPort
ABI, regions, traps, CPU, SDK or loader changes. It makes no historical claim.
All existing gates, preserved originals, records and the frozen wiki stay unchanged.

## API (`personality/system1/quickdraw-view.h`)
`cs_qd_view {port; int32 x,y,top,left,bottom,right}`. Functions: `init`, `offset`,
`clip` and `rect`. Each returns 1 on success and 0 on refusal. The view borrows its
caller-owned port from init until reinit or end. It never owns, frees, serializes
or exposes that port. There is no allocation, libc, mutable global or callback.
The only external function the module calls is `cs_qd_rect`.

## Validity
A view is valid when view and port are nonnull, x and y are in [-32768,32767],
top and bottom are in [0,64], and left and right are in [0,128]. An inverted clip
still counts as valid. A corrupt view returns 0 before any effect. Init may
reinitialize a corrupt view.

## Semantics
- init: null view or port refuses with no change. Otherwise it binds the port
  and sets the offset to 0,0 and the clip to 0,0,64,128. Port pixels and
  counters are never touched.
- offset: needs a valid view and new x/y in range. It assigns x/y only.
  physical = logical + offset. This is a native translation, not SetOrigin.
- clip: needs a valid view and a nonnull rect. The rect is BE signed16
  top,left,bottom,right. Each edge is clamped on its own (vertical to [0,64],
  horizontal to [0,128]) and all four are replaced together. There is no swap or
  normalization. The clip is in physical coordinates and does not depend on the
  offset.
- rect: needs a valid view, a nonnull rect and the word A8A2 or A8A3, all
  checked before parsing. Otherwise it returns 0 with view and port unchanged.
  Edges are decoded to int32 and translated, giving the range [-65536,65534].
  The result is intersected with the clip, which is already inside the physical
  bounds. An empty result is packed as 0,0,0,0. The function calls `cs_qd_rect`
  exactly once, even for empty or clipped results, and returns its result. Any
  helper effects are kept. The view is never modified.
- offset and clip never change pixels or counters.

## Native workload
`apps/native-drawing/workload.c` keeps its reset order and phases. At phase 1 it
inits a local view, applies offset(8,8) and clip [8,8,56,120], and paints the
logical rect [-32768,-32768,32767,32767], which lands on physical [8,8,56,120].
It then erases logical [8,8,40,104], which lands on physical [16,16,48,112].
That is exactly two underlying calls. The pixels, the 2304 black count,
1 paint/1 erase, the FNV hash, value 97 and phase 6 match the old output byte
for byte. If view setup fails, it returns 0 at phase 1.

## Acceptance (independent)
An int64 per-pixel inverse-mapping oracle checks exact image and counts,
including saturation. Cases cover origin extremes, negative, offscreen, inverted
and empty rects and clips, byte boundaries, offset/clip independence, isolation
between two views and ports, null/corrupt/unknown-word no-effect, and reinit that
preserves the port. Add pixel, row and outer guards, 256 seeded random cases and
directed edge cases. Run strict Clang/GCC 32/64, sanitizers, ARM in both
endians, native68000 zero-interpreter closure, the unchanged workload and legacy
conformance, EFI twins, corruption and omission checks, and required CI.
Integrator wires the build, ROM and EFI objects.
