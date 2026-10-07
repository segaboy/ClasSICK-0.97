# SPEC-0002 — Windows surface presentation v1

Date: 2026-10-07. Status: finalized independent adapter contract; Codex self-review
under the owner's instruction to proceed with the next visible checkpoint. Human
review pending. Inputs: SPEC-0001 v1, SRC-0017 owner authorization, SRC-0018 public
Microsoft Win32 interface prose. No historical Macintosh behavior is specified.

## Ownership and boundary

The Windows adapter reads a valid native SPEC-0001 surface and converts its logical
pixels into caller-owned scratch storage. The source descriptor, source bytes and
scratch must remain live, truthful in length, non-overlapping and exclusively
accessible during the call. No pointer is a guest address. The adapter allocates
no memory; the hosted demo owns its Windows handles and heap buffers. Windows
headers/imports stay under platform/windows, apps and hosted tests, outside core.

The adapter accepts widths/heights 1–4096. This is a bounded development policy,
not a core or historical display limit. Revalidate all SPEC-0001 metadata before
reading pixels. Source padding and alpha bytes are not presentation colors.

## Conversion

`cs_win_pixels` produces top-down rows of four bytes per logical pixel: B,G,R,0.
RGBA8 supplies R,G,B directly; alpha is ignored, including alpha zero. There is no
blending, premultiplication or implied transparency. MONO1_MSB maps bit zero to
white and bit one to black, an explicit adapter choice absent from SPEC-0001.
Source row/bit padding is ignored. Destination stride must be at least width*4;
the full stride*height span must fit capacity, SIZE_MAX and PTRDIFF_MAX. Destination
row padding, trailing bytes and source bytes are preserved. Unaligned byte buffers
are supported. Validation errors change neither source nor destination bytes.

Stable results: 0 OK, 1 argument, 2 invalid source surface, 3 adapter dimensions,
4 destination stride, 5 destination overflow, 6 destination storage, 7 host failure.
Conversion precedence: null source/destination; SPEC-0001 validation; adapter
dimension cap; destination stride; full-span overflow; capacity. A source error is
mapped to result 2 rather than inventing a second set of surface error semantics.

## Viewport and painting

`cs_win_viewport` returns a half-open destination rectangle. Source dimensions
obey the adapter cap; client dimensions are nonnegative int32 values. A zero client
dimension gives an empty rectangle. Otherwise scale is the largest positive integer
fitting both dimensions, with minimum one. If the client is smaller than the frame,
center and crop at 1x. Offsets are (client-drawn)/2, C integer division toward zero.
No fractional scaling or interpolation. Errors preserve the output descriptor.

`cs_win_present` requires a live HDC in ordinary client pixel coordinates (MM_TEXT,
identity origins/transform) and tight width*height*4 scratch. Reject null
arguments and negative client sizes before conversion; then convert and compute
the viewport. An empty client succeeds without touching the HDC. For a nonempty
client, save its state, fill the client with RGB(24,28,36), set COLORONCOLOR, draw
a 32bpp BI_RGB top-down DIB using StretchDIBits/SRCCOPY, and restore the HDC state.
Painting respects the caller's clip region. RGB values are tested on ordinary
memory DCs; an empty or letterbox-only clip succeeds without image scanlines.
Display color management/physical monitor appearance is not certified.
Host failure returns 7; scratch may already be converted and pixels partly drawn.

## Original viewer and acceptance

The native Windows demo starts in a 513x321 original RGBA geometric scene; Space
switches to a separately stored original monochrome scene, Escape or Close exits.
Title-bar instructions use Windows UI text. Repaint and resize use the adapter.
The viewer sets per-monitor-v2 DPI awareness before creating UI, sizes clients
using their actual DPI and accepts WM_DPICHANGED's suggested window rectangle.
Frame scaling remains integer physical pixels. The hosted target is current
Windows 11/Windows Server CI; support for older Windows is not established.
Minimization preserves buffers. Windows key handling here is demo scaffolding,
not the future normalized core event queue. The viewer owns no copied artwork.

Independent tests must check exact conversion bytes, alpha policy, odd widths,
padding/guards, source preservation, rejected metadata/capacity/overflow and signed
client bounds. Render into a real Win32 DIB/DC and inspect all RGB pixels against
independent expectations at integer scales and cropped sizes. Test saved DC state.
The demo's automated mode creates a real hidden window, resizes, repaints through
WM_PRINTCLIENT, switches formats and closes with bounded completion. This proves
the hosted message/render path, not a human screen review or physical display.

Passing this contract adds Windows presentation to M0.1. Arenas, normalized input,
clock/debug workflow and the remaining B1 gate are separate. No native OS boot,
historical compatibility, edition completion or shared edition parity follows.
