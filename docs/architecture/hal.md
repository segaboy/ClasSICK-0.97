# Hardware and architecture abstraction

Status: proposed contracts; concrete C interfaces are designed and reviewed before
their first implementation. No ABI is frozen by this document.

## Architecture layer

Own CPU entry/reset, exception/interrupt frames, context handoff, interrupt masking,
barriers where required, and architecture-specific address/stack constraints.
Separate architecture properties from board/device properties. An x86-64 core
build must not require UEFI headers; an m68k core build must not require Macintosh
memory-mapped register definitions.

## Platform contracts

| Service | Contract to specify | Initial validation |
| --- | --- | --- |
| Boot context | Versioned memory ranges, reserved areas, framebuffer descriptor, platform capabilities; explicit ownership transfer | Reject invalid overlap/overflow and unsupported pixel formats |
| Memory | SPEC-0003 caller-supplied monotonic arenas; alignment, ownership and reset | Tiny arena, alignment waste, exhaustion, integer-overflow and lifetime tests; individual free/fragmentation outside v1 |
| Graphics | Width/height, byte stride, pixel format, length, access mode; present invalidated regions | Unusual stride, 1bpp MSB-first and native-color conversion, clipped writes |
| Input | SPEC-0004 Space/Escape key records, source and bounded epoch sequence; pointer/text/timestamps later | Independent list traces; overflow/repeat/reset and Windows mapping/viewer tests |
| Clock/timer | Monotonic duration units and wrap/availability contract; deterministic fake clock | No dependency on wall-clock/timezone or exact host scheduler delays |
| Block device | Logical block size/count, bounded reads/writes, flush, media/read-only/errors | RAM/file-backed device; short/failing I/O; non-floppy storage |
| Logging | Bounded diagnostic sink; optional capability | Operates without stdio or console firmware callbacks |
| Halt/reset | Explicit terminal states and best-effort platform operation | Hosted shutdown and native diagnostic failure state |

Validate all sizes before dereferencing. No core-side dependency on a 512x342
screen, a floppy drive, keyboard scan codes, or a particular physical address.
QuickDraw operates on surfaces; presentation/conversion belongs to the adapter.
MFS operates on a generic block device; the Resource Manager uses logical forks.

## Hosted versus native ownership

Windows supplies process memory, a Win32 window, input, and file-backed storage.
The adapter turns those into the same contracts used by a native build. Win32
types and calls stay in `platform/hosted/windows/`.

UEFI supplies an initial memory map and framebuffer information. At native handoff
the loader stops using boot services and the kernel owns allocation, logging,
input drivers, and progress. Firmware text/input callbacks cannot be the post-boot
device abstraction. See [boot](boot.md) and SRC-0005.

Macintosh startup and hardware drivers must initialize without Apple ROM code.
Replacement ROM packaging, overlay transitions, interrupt wiring, video-memory
timing, and input/storage protocols require eligible hardware specifications and
black-box device tests. They are research tasks rather than assumed APIs.

## Portability acceptance

Run identical contract tests against fake, hosted, and native adapters. Compile the
core freestanding with platform headers inaccessible. Audit undefined/link imports.
Test 32/64-bit size differences, big/little endian byte streams, unaligned inputs,
tiny arenas, varying graphics dimensions, and different block sizes. Native m68k
and ARM64 execution evidence is required later; cross-compilation alone does not
prove runtime correctness. Add a second independently supported compiler before
freezing core compiler-dependent assumptions.
