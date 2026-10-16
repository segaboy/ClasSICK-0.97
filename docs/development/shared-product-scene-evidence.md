# Shared product scene - 2026-10-16

SPEC-0037 / ADR-0042 / IMPL-0041 / TEST-0045; SRC-0209..0212.

One portable monochrome product scene now serves the drawing ROM, native-PC bootstrap and hosted Windows viewer. Both boot paths use the same original 68k drawing fixture, with real result/phase/pixel state. The logical canvas is512x342; PC640x480 centers it at(64,69), with exact pixel parity and white margins. The owner-supplied monitor/C mark is approximated in original black/white geometry.

[Standing display target](display-target.md). The Mac keeps its native surface;
PC uses the existing reported GOP mode. No512x342 hardware mode is required and
640x480 is a checked presentation case, not a mandatory mode. The hosted viewer
now also has a512x342 logical surface; both of its pixel formats show monochrome.

## Verification

All 2,217 checks in nine Clang/GCC debug/optimized32/64/sanitizer profiles
pass, with matching compiler twins for the shared-scene tests and Windows viewer.
The new parity check runs the real original probe separately for Mac and PC,
compares actual port pixels and every logical pixel after RGB/BGR presentation,
and checks margins, pitch padding and tail canaries. An independent point oracle
checks branding, original lettering, panel, six phases, progress and decimal
fields. Complete/banded and tiny/large surfaces, invalid descriptors/states,
odd sizes and exact counter-only preservation are covered. Input1234 is checked
independently; UINT32_MAX counter updates agree with a complete redraw.

All50 import/global/width/endian object audits pass. Core/renderer allocation,
host APIs and mutable globals are absent. The PC wrapper allocates the original
probe workspace in its existing128KiB arena, not on the native stack. At640x480,
the64-bit hosted layout uses112392 bytes:64000 RGBA staging plus48392 aligned
workspace. This is PC memory, not a Macintosh128KiB accounting claim.

Original native-PC EFI O0/O2 twins match within each profile. The optimized
file is148992 bytes, SHA-256 `3db614d71f8df9494bf3cf189f0d2be3a9adf401654ff92b17a9bcb01b468738`;27 corruption and
14 omitted-object controls reject. No foreign imports/runtime; existing native
handoff, exceptions, PS/2, serial and B2 trace semantics retained. The visible
PC counter now counts accepted nonrepeat Space presses. No PC mouse driver added.

Original68000 ROM twins match,65536 bytes, SHA-256 `95d7cdef447768586e2d8e9af18771afbe87f197c273269690650fc18e2fe542`.
Used ROM46164; native data/BSS2168; workspace
45592; conservative stack bound
10844 within24320 reserved bytes, largest frame
604. The original40-byte program still executes13
steps, returns97, paints/erases once, retires one handle and retains2304 black
image pixels. No CPU instruction, guest dispatch, loader or native-app expansion.

Earlier tests assumed the old colored layout and staging-only PC arena; those
expectations now check the declared monochrome geometry and bounded probe
workspace. Windows memory tests retain literal pixel/input offsets plus the
native workspace size. O0 aggregate initialization was changed to explicit
field assignment to avoid an implicit memset dependency. Initial test/build
iterations, final raw logs, image/preview originals and manifests stay private.

## Debugger integration correction

The first CI attempt passed its68000 job and fresh hosted tests, then found an
obsolete LLDB viewer-memory expectation (1342744 bytes). The new64-bit viewer
uses1473768 bytes including the original probe workspace. The wrapper now checks
that value; all six local LLDB launch/source/state/step/resume runs and three
argument rejections pass. No renderer/native input changed for this correction;
the complete matrix and native artifacts remain valid by exact input identity.

## Scope and provenance

The supplied49x56 PNG is retained outside Git with its byte identity. Renderer
geometry is an original monitor/C approximation; no PNG, historical icon, copied
font, manual sample or external implementation was added. Existing independently
authored letterforms and graphics/fixture contracts are reused. No external
worker, new chat, agent, automation or OneNote operation. GPL-3.0-or-later.

Compiler/hosted verification is separate from an actual boot. CI on the exact
committed implementation remains pending until recorded below. No new Snow,
VirtualBox or reference-system trial has been run; each needs fresh explicit
owner go-ahead. Earlier drawing-ROM observations describe their earlier image,
not this new candidate. Published alpha assets/tag and all24 untracked research
originals/existing copies remain unchanged; no UART body was inspected/edited.
Historical R-001/AL-2/QQ-1..13, native-app/source/UART/physical/full128K/parity/
edition/B2/human/input and both mandatory System1/ROM gates remain open.
