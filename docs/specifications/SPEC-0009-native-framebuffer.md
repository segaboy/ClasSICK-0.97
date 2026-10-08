# SPEC-0009 — Native linear framebuffer presentation and original boot scene

Status: finalized v1, 2026-10-08; implementation lead self-review, human provenance
review pending. Owner SRC-0044; UEFI GOP pixel interfaces SRC-0034 (12.9.2);
internal SPEC-0001, SPEC-0003, SPEC-0006–0008. ADR-0013 / IMPL-0013 / TEST-0015.
This is an original native-PC platform contract and test scene. It describes no historical
Macintosh behavior or appearance, and no QuickDraw semantics.

## Scope

Three original layers, all freestanding C with no heap, library calls, mutable globals
or firmware callbacks:

1. `platform/pc/framebuffer.h` validates a native linear 32-bit framebuffer target
   and converts clipped core RGBA8 surface pixels into it with direct byte stores.
2. `apps/boot-scene/scene.h` renders a deterministic original geometric scene,
   one bounded horizontal band at a time, through the SPEC-0001 surface API. It also
   allocates staging from a SPEC-0003 arena and draws complete frames band by band.
3. `platform/uefi/native.h` gates presentation on a successful exit plus ready
   descriptors, using the owned SPEC-0007 handoff metadata. It also records a bounded
   result in owned trace storage. The EFI entry wrapper only supplies identity pointers.

Excluded: firmware Blt, console, input or mode callbacks after exit; GOP mode
changes; reads of framebuffer memory; alpha blending; scaling; fonts; timers;
keyboard; diagnostics output devices; page-table or caching-attribute changes.

## Framebuffer target

`cs_fb_init(out, base, size, width, height, pitch, format)` writes the descriptor
only after it validates the inputs. Here, `pitch` is pixels per scanline, as in the GOP
mode information, and `base` is a `volatile unsigned char *` to `size` live writable
bytes. Formats: `CS_FB_RGB_RESERVED` (0) and `CS_FB_BGR_RESERVED` (1), each four
bytes per pixel. Profile bounds match SPEC-0006: width and height 1..8192;
pitch >= width and <= 16384. The required span is `pitch * 4 * height` bytes,
computed in 64-bit arithmetic before narrowing, and must not exceed SIZE_MAX,
PTRDIFF_MAX or `size`.

Results and precedence: OK 0; ARGUMENT 1 (null output/base, or null source/target
when presenting); FORMAT 2; DIMENSION 3 (width/height/pitch bounds);
OVERFLOW 4; STORAGE 5; SOURCE 6 (source descriptor rejected by SPEC-0001
revalidation, or a non-RGBA8 source). Failures preserve the output descriptor and
write no framebuffer byte. Validation cannot prove that a pointer is live, mapped,
writable, uncached or truthful; those remain caller preconditions.

## Pixel conversion and clipping

`cs_fb_present(target, source, left, top)` revalidates the target and then
revalidates the source through `cs_surface_init` on a local copy. The source must
be `CS_SURFACE_RGBA8`. The source's top-left pixel is placed at signed destination
(left, top). The written region is the intersection of
`[left, left+source.width) × [top, top+source.height)` with the logical
`[0,width) × [0,height)`. It is computed with 64-bit intermediates, so INT32
extremes cannot overflow. An empty intersection succeeds without writing.

For each destination pixel `(x, y)` in the intersection, the source bytes are
`R,G,B,A` at `source.storage + (y-top)*source.stride + (x-left)*4`. The destination
bytes at `base + y*pitch*4 + x*4` become:

| Format | Byte 0 | Byte 1 | Byte 2 | Byte 3 |
| --- | --- | --- | --- | --- |
| RGB reserved (0) | R | G | B | 0 |
| BGR reserved (1) | B | G | R | 0 |

Source alpha is discarded; it is not blended or premultiplied, and it is never copied
into the reserved byte. Pixels outside the intersection, scanline padding beyond
`width` and bytes beyond the span are never written. The presenter never reads
framebuffer memory, never touches source padding, and writes each intersected pixel
exactly once, using four volatile byte stores in ascending address order, rows top to
bottom. Store width and write-combining behavior are deliberately unspecified beyond
that. Rendering speed and tearing are not acceptance criteria in v1.

## Original boot scene

`cs_scene_render(band, width, height, band_top, step)` fills band row `r` with
scene row `band_top + r`, for scene dimensions `width × height`. Preconditions:
width and height 1..8192, `0 <= band_top < height`, and a valid RGBA8 band. Band
pixels beyond the scene's width or height are left unchanged. Results and
precedence: ARGUMENT 1 (null band); DIMENSION 2 (scene size or band top);
SURFACE 4 (SPEC-0001 revalidation rejected the band descriptor); FORMAT 3
(band not RGBA8); otherwise OK 0. Validation happens before any fill.

Integer scene geometry uses `u = max(1, min(width,height)/32)` and truncating
division. Rectangles are half-open and painted in this order, later ones over
earlier ones. Colors are `0xRRGGBBAA`.

1. Background: whole scene, `0x1E3A4CFF`.
2. Corner markers, side `2u`: top-left red `0xFF0000FF`, top-right green
   `0x00FF00FF`, bottom-left blue `0x0000FFFF`, bottom-right white `0xFFFFFFFF`.
   They identify orientation and byte order in a capture.
3. Panel: `pw=width/2`, `ph=height/3`, `px=(width-pw)/2`, `py=(height-ph)/2`.
   An outer frame `0xF2F2F2FF` covers the panel, then an inner area `0x2B2B2BFF`
   inset by `u` on each side (empty when the inset consumes it).
4. Eight bars across the upper half of the inner area: let `ix,iy,iw,ih` be the
   inner rectangle, then bar `i` spans x `[ix+iw*i/8, ix+iw*(i+1)/8)` and y
   `[iy, iy+ih/2)`, with colors white `0xFFFFFFFF`, yellow `0xFFFF00FF`, cyan
   `0x00FFFFFF`, green `0x00FF00FF`, magenta `0xFF00FFFF`, red `0xFF0000FF`,
   blue `0x0000FFFF` and black `0x000000FF`.
5. Progress track `0x505050FF`: x `[ix+u, ix+iw-u)`, y `[iy+ih/2+u, iy+ih-u)`,
   with `tx,tw` as its x start and width. Sixteen segments: segment `i` spans x
   `[tx+tw*i/16, tx+tw*(i+1)/16 - u/2)` over the track's y range. It is
   `0x7FD35FFF` when `i < step % 17`, otherwise `0x3A3A3AFF`.

Bars and the track (steps 4–5) are painted only when `iw > 0` and `ih > 0`;
segments only when `tw > 0`. A rectangle whose computed right/bottom edge falls
below its left/top edge is skipped (treated as empty). Every rectangle is translated by `-band_top` and
clipped by SPEC-0001. The rendered bytes for a scene row therefore do not depend on
how the frame is partitioned into bands. This scene is an original test pattern,
not a Macintosh screen, icon or historical startup image.

## Staging and frame drawing

`cs_boot_scene_prepare(arena, target, out)` validates the target and allocates one
RGBA8 staging span from a caller-initialized SPEC-0003 arena, aligned to the
smaller of 16 and the arena maximum:
`band_rows = min(height, 65536 / (width*4))` rows of `width*4` bytes, which is at
most 64 KiB and never less than one row within the 8192-pixel profile. The
128-KiB native arena is never asked for a full-screen staging image. Arena
exhaustion returns ARENA and leaves the arena unchanged, per SPEC-0003. The
prepared scene copies the target descriptor and retains only the staging
pointer, which stays valid until the caller resets or discards the arena.

`cs_boot_scene_draw(scene, step, report)` presents the whole frame from top to bottom.
For each band it initializes a staging surface whose height is the remaining
`min(band_rows, height-top)` rows, renders that band and presents it at
`(0, top)`. The report records the result, bands, band rows, presented rows and
step. Results: OK 0; ARGUMENT 1; TARGET 2; ARENA 3; SCENE 4; PRESENT 5. Before
writing, it revalidates the prepared descriptor and its staging span. A later band
failure stops at that band, and the report states the rows already presented.
With valid inputs no later failure is reachable.

## Native gating and trace record

`cs_native_present(handoff, descriptors_ready, framebuffer, arena, trace)` is the
only post-handoff device-touching operation in v1. It writes a 32-byte
little-endian record at `trace`, which is owned bundle storage, at offsets:
magic `0x31525043` at 0, version 1 at 4, result at 8, handoff stage at 12, bands
at 16, band rows at 20, rows at 24 and step at 28. It writes no other trace bytes.
Results:

| Code | Meaning | Framebuffer touched |
| --- | --- | --- |
| 0 | Presented scene step 0 | Yes |
| 1 | Null argument | No |
| 2 | Handoff not in EXITED phase or firmware status nonzero | No |
| 3 | SPEC-0008 descriptor-ready marker absent | No |
| 4 | Handoff framebuffer rejected by SPEC-0006 or SPEC-0009 | No |
| 5 | Arena initialization or staging allocation failed | No |
| 6 | Scene/presentation failed after preparation | Possibly partial |

A null trace returns 1 and writes nothing. The function checks the exit and
ready gates before validating or writing the framebuffer. It initializes the core
arena over the handoff's 128-KiB arena span and retains no firmware pointer. The native stop path sets `native_entered`, builds
and installs SPEC-0008 state, and calls this function with the ready marker read
from owned CPU state. It then halts, as before. After a failed exit attempt it still
reaches the halt without touching any device. Identity mapping of the framebuffer
physical address and a writable mapping are profile preconditions, not checks.

## Acceptance and limitations

Independent hosted tests use guarded synthetic buffers with unusual pitch,
unaligned source storage and odd strides. They cover both formats with a per-byte
oracle written from this table, all clipping edges including INT32 extremes,
padding, guard and reserved-byte preservation, and every error and its precedence.
They check scene pixels against a separate point-classifier oracle and confirm
partition independence across band heights. They cover staging budgets at width
extremes, arena exhaustion, the native gates and trace record bytes, and that no
framebuffer is written when a gate fails. Clang and GCC, actual i686 where portable,
ASan/UBSan and freestanding import audits apply. The own EFI image links the
presenter, scene, arena and surface objects without new imports or writable image
data, and an omitted-presenter link must reject.

Hosted conversion is not a boot. Real ExitBootServices, framebuffer mapping and
caching, visible output, timing, keyboard, diagnostics, firmware eligibility,
formatted media and B2 remain unverified. The Mac/mini vMac and physical-edition
gates are untouched.
