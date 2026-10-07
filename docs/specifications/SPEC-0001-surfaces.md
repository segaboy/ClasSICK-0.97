# SPEC-0001 — Bounded portable graphics surfaces

Status: **finalized v1 independent core contract**, 2026-10-07; self-reviewed by
Codex under the owner's explicit implementation assignment. Human review pending.
Classification: design decision; **not historical QuickDraw behavior**. Inputs:
SRC-0001, SRC-0016, ADR-0001 and ADR-0002. No historical source or reference pixels.

## Scope and interface

Caller-owned ordinary byte storage, construction, solid rectangle fill and clear.
The public native C interface is `core/graphics/surface.h`; it is not a guest ABI.
No allocation, platform drawing, volatile framebuffer/MMIO, font, blit, alpha
compositing, guest memory resolution or synchronization is supplied. Presentation
and hardware mapping need separate adapter contracts. No mutable global state.

`cs_surface_init(out, storage, storage_size, width, height, stride, format)` writes
the descriptor only after validation and never initializes pixel bytes.
`cs_surface_fill(surface, rect, value)` changes only the clipped pixels.
`cs_surface_clear(surface, value)` fills the complete logical surface, preserving
padding. All three return `cs_surface_result`; zero means success.

## Ownership and lifetime

Storage is a live writable byte array of at least `storage_size` bytes. The caller
owns it and the descriptor throughout every operation; no pointer is retained
outside that descriptor. The descriptor must not overlap storage, and the caller
must serialize overlapping writes. No alignment beyond byte alignment is needed.
The caller must not free/move storage during use; discarding the descriptor needs
no destroy call. Reinitialization on error preserves the previous descriptor.
Different descriptors with disjoint buffers are independent. Sharing storage is
permitted with caller-managed lifetime/synchronization, not isolation.

Descriptors are native values, initialized by this API and otherwise kept intact.
Operations revalidate their fields to reject malformed size/format metadata, but
cannot prove that a non-null pointer denotes a live allocation or detect lying
lengths/overlap. Those are caller preconditions. Guest addresses require a bounded
mapping service; they must never be cast to native pointers for this API.

## Geometry, layout and values

Width and height are `int32_t` in `[1, INT32_MAX]`. Coordinates use the same type.
Origin is the top left; x increases right, y increases down. Row zero begins at
storage; row y begins at `y * stride`. Stride and storage length use native `size_t`.

| Format (stable value) | Logical pixel and fill value | Minimum row bytes |
| --- | --- | --- |
| `CS_SURFACE_MONO1_MSB` (1) | Pixel x uses bit `7 - (x % 8)` of byte `x / 8`; value must be 0 or 1, with no implied black/white color | `width / 8 + (width % 8 != 0)` |
| `CS_SURFACE_RGBA8` (2) | Four consecutive bytes R, G, B, A at `4*x`; numeric value `0xRRGGBBAA` supplies those bytes, independent of host endian/alignment | `4 * width` |

RGBA8 copies all channels exactly, including A; no premultiplication, blending or
color-space operation occurs. Stride may be minimal or any larger byte count,
including an odd count. Full rows are required, including the final row's padding:
the minimum storage span is `stride * height`. Reject strides below row bytes.
Additional storage bytes, row padding bytes and unused low bits in a final mono
byte are preserved by every operation. Clear changes logical pixels only.

Before multiplication or pointer arithmetic, check row size and complete span
against `SIZE_MAX`; also reject a span above `PTRDIFF_MAX` as a conservative native
object bound. These limits are architecture-dependent, not a guest memory limit.
No pointer arithmetic or storage access occurs on rejected construction.

## Rectangles and clipping

`cs_rect { left, top, right, bottom }` denotes `[left,right) × [top,bottom)`.
Reject `left > right` or `top > bottom`, even if the rectangle is outside the
surface. Equal edges are empty. Validate the descriptor, reversed edges and color
before accepting an empty/outside rectangle. Clip by comparisons to `[0,width)`
and `[0,height)` before computing any offset; do not subtract/negate unbounded
coordinates. `INT32_MIN`/`INT32_MAX` endpoints are valid. Empty or disjoint
intersections succeed without writing; pixels outside the intersection survive.

## Stable results and precedence

| Code | Result | Meaning |
| --- | --- | --- |
| 0 | `CS_SURFACE_OK` | Success, including an empty intersection |
| 1 | `CS_SURFACE_ERR_ARGUMENT` | Null descriptor/output or storage |
| 2 | `CS_SURFACE_ERR_FORMAT` | Unsupported format |
| 3 | `CS_SURFACE_ERR_DIMENSION` | Nonpositive width or height |
| 4 | `CS_SURFACE_ERR_STRIDE` | Stride below minimum row size |
| 5 | `CS_SURFACE_ERR_OVERFLOW` | Row/span unrepresentable or span above `PTRDIFF_MAX` |
| 6 | `CS_SURFACE_ERR_STORAGE` | Storage shorter than the required full-row span |
| 7 | `CS_SURFACE_ERR_RECT` | Reversed rectangle edges |
| 8 | `CS_SURFACE_ERR_COLOR` | Mono value other than 0 or 1 |

Validation precedence is null pointers, format, dimensions, row overflow, stride,
span overflow/object bound, storage length, rectangle order, then color. Clear
has no rectangle error. A failed operation changes neither descriptor nor bytes.
Errors have explicit numbers; future additions must preserve existing numbers.

## Acceptance and applicability

TEST-0004 uses independent literal byte vectors and a per-bit/per-channel oracle
derived from this contract, not implementation helpers. It covers odd widths,
minimal/extra stride, exact storage spans, unaligned storage, guards, padding,
clipping, signed extremes, empty/outside/reversed rectangles, error precedence,
constructor failures/overflow, malformed descriptors and independent surfaces.
Tests must fail through process exit status even with `NDEBUG`.

Required verification: pinned native Windows CMake/CTest, strict warnings,
freestanding compilation and undefined-symbol/global-data audit. Validate each
sanitizer with an intentionally failing independent probe before claiming coverage.
Other architecture compile checks supplement execution and are labeled accordingly.
Results: [TEST-0004](../../provenance/records/TEST-0004-surfaces.md) and
[IMPL-0002](../../provenance/records/IMPL-0002-surfaces.md).

Surfaces alone do not satisfy B1/M0.1. The next adapter may present an original
geometric scene at several sizes; arenas, normalized input/events, a clock and
debug workflow remain. Original-hardware, native-PC and Linux-PC editions each
still require their own boot, behavioral and provenance gates. This v1 contract
is a shared candidate foundation, not proof of historical or cross-edition parity.
