<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0048: Native rectangle outline on a SPEC-0041 view

Status: Accepted engineering contract2026-10-18; SRC-0280/0282/0284 / ADR-0054 (contract only), baseline public 01e6400d9f83a51ebb93bb7747f21af627829720.
Original native engineering contract. This is not the historical FrameRect and makes no
historical claim. It is not a boot observation, trial or release. Nothing here is implemented.
This outline-contract slice changes no runtime, API header, outline code/fixture, build input, trap, registry or loader. Screenshot infrastructure is recorded separately.

## Scope
This spec defines an original portable freestanding C helper. It paints a black outline of a
rectangle on the existing 128x64 one-bit SPEC-0036 `cs_qd_rect_port`, going through an existing
SPEC-0041 `cs_qd_view`. The same source targets 68000 and x86 native builds.
The following stay byte-for-byte stable: `quickdraw-rects.h`, `quickdraw-view.h`, `cs_qd_rect`,
the `cs_qd_view_*` functions, `sizeof(cs_qd_rect_port)` (1032), `sizeof(cs_qd_view)`, the field
layout and the counter meaning. No struct field is added. There is no guest A8A1/SDK ABI, no pen
state, no pattern, no transfer mode and no region. The current demo, ROM and EFI images are unchanged.

## Proposed API (future separate header/module, not created here)
`int cs_qd_view_outline(cs_qd_view *view, const uint8_t rect[8], int32_t horizontal_width,
int32_t vertical_height);`
It returns 1 on success and 0 on refusal.

## Engineering decisions
OL1 Name and placement. The helper lives in a new separate module, for example
    `personality/system1/quickdraw-outline.{h,c}`. Existing translation units are not edited.
    Calls that already exist must behave identically whether or not the module is linked.
OL2 Pen sizes are explicit integer arguments. `horizontal_width` is the thickness of the left
    and right strips in pixels. `vertical_height` is the thickness of the top and bottom strips.
    Both are signed int32. Only values in [0,32767] are accepted. Values below 0 or above 32767 refuse.
OL3 View validity follows SPEC-0041 exactly: view and port are nonnull, x and y are in
    [-32768,32767], top and bottom are in [0,64], left and right are in [0,128], and an inverted
    clip is valid. A null or corrupt view refuses.
OL4 Preflight. Every check happens before any rect byte is read, any pixel is written or any
    counter changes. The checks are: view validity (OL3), then rect nonnull, then the pen ranges
    (OL2). If any check fails, the helper returns 0 and view, port pixels, paints, erases and the
    rect bytes are all unchanged. After preflight passes, no further refusal is possible.
OL5 Decode. The rect is 8 bytes in big-endian order: signed16 top, left, bottom, right. Each
    value is decoded independently of host endianness and host width. Each edge is then
    sign-extended to int32 and the view offset is added (top+y, left+x, bottom+y, right+x).
    The results lie in [-65536,65534].
OL6 Empty geometry. If physical top >= bottom or left >= right, the rect is empty. An empty
    rect, including an inverted one, is never swapped or normalized. The call succeeds with no
    pixels drawn. It still counts under OL10.
OL7 Zero pen. If either pen size is 0, the call is a valid empty success: no pixels are drawn
    and it counts under OL10.
OL8 Geometry before clipping. Let T,L,B,R be the physical edges, w the horizontal width and
    h the vertical height, with all arithmetic in int32. All intermediate sums/differences stay within [-98303,98301],
    so nothing overflows. The four internal strips are half-open:
    top    [T, L, min(T+h,B), R]
    bottom [max(B-h,T), L, B, R]
    left   [T, L, B, min(L+w,R)]
    right  [T, max(R-w,L), B, R]
    Coverage is the per-pixel set union of the four strips. All strips are computed from the
    unclipped rect before any intersection with the clip. Strokes may overlap, and huge strokes
    may cover the whole rect. Overlapping pixels are painted once, which is idempotent.
OL9 Clip. The painted set is the coverage (OL8) intersected with the view clip. The clip is
    physical and already clamped to [0,64]x[0,128]; an inverted clip gives an empty result.
    The clip boundary must never appear as an outline edge: clipping removes pixels and never
    adds any. Pixels outside 128x64 are never addressed.
OL10 Raster op and counters. Covered pixels are set to the same black polarity and bit mapping
    that `cs_qd_rect` uses for A8A2 Paint (stride 16). This is an OR/set operation; pixels that
    are not covered are never cleared. After preflight succeeds, `port->paints` increases by
    exactly 1 per successful call, saturating at UINT32_MAX. This holds even when the result is
    empty, clipped, inverted or zero-pen. The helper never adds 4 (one per strip) and never
    delegates to `cs_qd_rect`, because each such call would add a paint. `erases` is never touched.
    `view` is never modified.
OL11 Effect order. The order is preflight, then decode, then compute, then write pixels, then
    the single counter update, then return 1. No intermediate state is observable to other code
    under the OL13 synchronization premise. No full raster temporary is needed. The algorithm
    (row spans, masks or similar) is not mandated; only the resulting pixels and counters are.
OL12 Freestanding. No libc, allocation, mutable global, static scratch, callback, I/O, trap
    or guest memory access. The helper calls no external function. Stack use is bounded and
    constant.
OL13 Premises the helper does not check. These are caller obligations:
    - Native view, port and rect are truthful, live and appropriately aligned for the whole call; view/rect are readable and the complete port is readable/writable.
    - The view, complete port (pixels and counters) and rect's 8 bytes are pairwise disjoint.
    - The caller has exclusive, synchronized access with no concurrent mutation.
    Invalid raw native addresses that are nonnull are a premise violation; the helper cannot
    check them safely. There is no native/guest aliasing loophole: guest addresses, guest RAM
    and A5-relative records are never valid arguments.
OL14 Integration later. A follow-up feeds a resolved selected native view (SPEC-0047 style)
    into the helper behind a new explicit build mode. Existing modes, the workload, hashes,
    phases, scenes, ROM and EFI remain unchanged.
OL15 Legacy guest binding (an outline trap, pen state, GrafPort pnSize) is deferred. It needs
    a documented ABI, an ownership model, and a separate spec, ADR and owner acceptance first.

## Follow-up prerequisites (must hold before implementation is accepted)
P1 Lead finalizes this engineering contract, source IDs and ADR. A later authorized implementation slice assigns IMPL/TEST IDs; this contract does not authorize runtime code.
P2 The exact `cs_qd_rect` black polarity and bit order are confirmed from the existing reviewed
   source and written into the new module's header comment.
P3 A size/offset audit proves existing port/view layouts unchanged. This contract slice preserves all existing runtime, binding and build-input bytes; later object/ROM identity checks use named build profiles, never an unspecified all-hashes claim.
P4 Build wiring for the separate module is approved for Mac 68000 native, PC x86 and the EFI
   twins, with direct-native symbol and zero-interpreter audits.
P5 The test plan in docs/development/native-outline-test-plan.md is executed by Codex, with
   outputs recorded.

## Not run
No test, build, emulator, VM, hardware, reference or CI run backs this draft. The matrix rows
are prospective only.

## Historical questions (open, not asserted)
H1 Does historical FrameRect use pnSize.h for the left/right strips and pnSize.v for the
   top/bottom strips, with the same inside-the-rect placement?
H2 What does a zero or negative pnSize do historically, and what about pen sizes larger than
   the rect?
H3 How do pen pattern and mode affect framing, and do pen-hidden states suppress drawing?
H4 How do clipRgn, visRgn and portRect interact with outlines on early System versions?
H5 What is the trap word, calling convention and register/stack ABI for a frame call in the
   targeted System 1 era, and do any picture or polygon recording side effects exist?
Answers require documented, reviewed public sources. Until then, OL decisions are original
engineering choices only.
