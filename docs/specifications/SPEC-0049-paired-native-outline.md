<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0049: Paired native outline preview

Status: accepted lead engineering profile under owner continuation2026-10-19. Candidate:
paired-native-preview-1 (draft only; no publication).

## API
`int cs_outline_draw(cs_selected_draw_workspace *, cs_selected_draw_result *)`
in `apps/outline-drawing/workload.h`. Reuses the SPEC-0047 types. The same
portable C builds for 68000 and x86.

- If either argument is NULL, it returns 0 and changes nothing.
- Otherwise it first resets the port pixels, counters, composite and result.
- No allocation, mutable globals, libc, callbacks, guest memory/traps or
  retained pointers. Only const static tables.
- Selection table: globals_begin 0x7000, a5 0x7200, g 0x71FC, count 2,
  IDs 0x7000/0x706C, initial current 0. Views are local to the call. Each
  `plan.view` is matched by comparison against the local views.

## Geometry (physical = logical + offset)

| Port | Offset | Clip | Logical rect | w,h | Black |
|---|---|---|---|---|---|
| 0 | (8,8) | [8,8,56,56] | [0,0,48,48] | 3,2 | 456 ring |
| 1 | (72,8) | [8,72,56,120] | [-8,0,56,48] | 4,3 | 384 rails |

On port 1 the physical rectangle is [0,72,64,120], so the horizontal strokes
fall outside the clip. No stroke is added at the clip edges. The composite
union is 840 black pixels. Each port gets 1 paint and 0 erases.

## Phases
1. Reset.
2. Port 0 set/get/outline done.
3. Port 1 set/get/outline done.
4. Final get returns 0x706C and these checks pass: current, selections=2,
   counters, per-port counts. Then the OR composite, FNV-1a hash and black
   count are recorded.
5. Union count is 840.
6. value 97, success 1.

On failure the phase, current, selections and finished port effects stay as
they are. The composite stays zero, and success is never forced.

## Modes
`CLASSICK_OUTLINE_DRAWING` requires `CLASSICK_SELECTED_DRAWING`. It calls
`cs_outline_draw` explicitly in the Mac native entry and in PC boot prepare,
with no function renaming. In this mode PC boot draw uses progress 0, the
same as the Mac. Step, report and timer diagnostics do not change. Other
modes and `cs_scene_render` are unchanged.

## Tests (to be done during integration)
- NULL refusal leaves no effects.
- Exact per-port and union counts, hash stability, phases, counters.
- Rejection of an injected failure, with the composite left at zero.
- Strict C11 warning set; 32/64-bit, both endians, O0/O2/Os; freestanding
  (no libc import).

## Limitations
- The PC 60-second diagnostic lifecycle is kept. It is a known limitation of
  this release candidate.
- Continuous desktop, USB motion, hardware, actual boots, packaging and
  publication are separate later gates.
- No VM, emulator, hardware or reference run is claimed.

Native truthful live/aligned/disjoint/exclusive caller premises persist. H1-H5 from SPEC-0048 remain open; this is not historical FrameRect. Physical/full128K/B2/human/native-external-app/Linux/complete-edition and both System1 boot gates remain separate.

Subsequent2026-10-19 engineering profile: [SPEC-0050 continuous PC preview](SPEC-0050-continuous-native-preview.md) removes normal deadline/Escape termination only when its explicit flag is enabled. The earlier exact images and diagnostic-mode statements above retain their dated meaning.
