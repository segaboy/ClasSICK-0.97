<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Native outline prospective acceptance matrix (SPEC-0048)

Status: UNEXECUTED. These are proposals only; no row has run and no result is claimed.

Oracle: independent int64 inverse mapping. For each physical pixel (r,c) in 128x64, compute
the logical rect from the decoded int64 edges and offset. The pixel is set if it lies inside
the clip, inside the rect, and within h rows of the top or bottom edge or within w columns of
the left or right edge, with w>0 and h>0. Expected = before OR set. Expected paints =
sat(before+1); erases unchanged. The view is compared bytewise.
Guards: canary bytes before and after the port, view and rect; padding bytes compared through a
memcmp snapshot; rect bytes unchanged.

| # | Case | Expect |
|---|---|---|
| 1 | null view | 0, all bytes unchanged |
| 2 | view->port null | 0 no-effect |
| 3 | corrupt x=32768 / y=-32769 | 0 no-effect |
| 4 | corrupt clip top=65 / right=129 / negative | 0 no-effect |
| 5 | null rect, valid view | 0 no-effect, paints unchanged |
| 6 | w=-1, h=-1, INT32_MIN | 0 no-effect |
| 7 | w=32768, h=INT32_MAX | 0 no-effect |
| 8 | preflight order: corrupt view and null rect together | 0; null rect is never read; do not form invalid native pointers outside caller premises |
| 9 | w=0 or h=0, valid rect | 1, pixels unchanged, paints+1 |
| 10 | inverted rect (top>bottom, left>right) | 1, no pixels, paints+1, no swap |
| 11 | zero-area rect (top==bottom) | 1, no pixels, paints+1 |
| 12 | full-port rect [0,0,64,128], w=h=1 | 1-px border, 380 black |
| 13 | w=3, h=2 asymmetric | strips match the oracle exactly |
| 14 | 2w>=width or 2h>=height | solid fill of the rect |
| 15 | w=h=32767 on a small rect | solid fill, no overflow |
| 16 | signed16 extremes [-32768,-32768,32767,32767], offset 0 | edges offscreen; no pixel painted from the clip boundary |
| 17 | same with offsets -32768 and +32767 | int32 range [-65536,65534] handled; oracle match |
| 18 | outline partly offscreen, left edge clipped | no artificial column at clip left |
| 19 | clip cuts through the middle of the rect | no painted clip boundary artifacts |
| 20 | inverted or empty clip | 1, no pixels, paints+1 |
| 21 | clip independence from offset | same physical result as the oracle |
| 22 | byte boundary edges at c=7,8,15,16,120,127 | exact bit masks |
| 23 | prefilled black/pattern port | OR semantics; no pixel cleared |
| 24 | idempotence: same call twice | pixels same, paints+2 |
| 25 | paints=UINT32_MAX-1, then two calls | stays at UINT32_MAX |
| 26 | paints=UINT32_MAX, call on an empty rect | stays UINT32_MAX, returns 1 |
| 27 | erases arbitrary value | never changed |
| 28 | view struct bytes and padding | unchanged after success and refusal |
| 29 | rect buffer bytes | unchanged |
| 30 | outer canaries around port, view, rect | intact |
| 31 | two ports and two views interleaved | no cross-port effect; counters isolated |
| 32 | existing cs_qd_rect/view_rect regression | byte-for-byte same pixels, counters, hashes |
| 33 | sizeof/offsetof audit, port 1032 | unchanged |
| 34 | 256 seeded random rect/offset/clip/pen (including invalid) | oracle match or no-effect |
| 35 | endian: BE decode on LE x86 and BE 68000/ARM-BE | identical results |
| 36 | native width: Clang/GCC 32/64, int size variance | identical results |
| 37 | sanitizers ASan/UBSan | clean; no signed overflow |
| 38 | freestanding symbol audit | no libc/global/alloc/external calls |
| 39 | Mac 68000 native compile, zero-interpreter closure | compiler-only; not booted |
| 40 | PC x86 and EFI twins compile | compiler-only; existing modes unchanged |
| 41 | SPEC-0047 workload, demo, ROM and EFI hashes | unchanged (not integrated) |
| 42 | future: selected-view integration mode | deferred to a follow-up spec |
| 43 | future: legacy guest outline binding | deferred; out of scope |
| 44 | historical FrameRect equivalence | NOT claimed; H1-H5 open |

Coverage boundaries: rows 1-38 are portable native tests. Rows 39-41 are compiler and
regression audits only. Rows 42-44 are explicit non-coverage. No emulator, VM, hardware,
reference, full128K, physical, B2 or System1 gate is affected or closed.
