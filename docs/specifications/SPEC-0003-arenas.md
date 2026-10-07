# SPEC-0003 — Bounded native memory arenas

Status: finalized v1, 2026-10-07, project lead self-review under SRC-0019.
Classification: independent core design contract. No historical Macintosh Memory
Manager, relocatable handles, guest heap, RAM budget or boot claim. Implementation
IMPL-0004; independent fixtures TEST-0006. Source semantics: SRC-0020.

## Ownership and lifetime

A caller supplies one live contiguous writable region and a separate native
descriptor (`storage`, `capacity`, `used`). Capacity is a truthful byte count,
at most PTRDIFF_MAX; used is in [0, capacity]. Storage must be aligned to
`_Alignof(max_align_t)` whenever non-null. Alignment is a caller precondition,
not inferred by casting a pointer to an integer. Empty arenas permit null storage
only with zero capacity. Non-null zero-capacity storage is also permitted.

The descriptor, output span and backing region must not overlap. Backing regions
of independently live arenas must not overlap; copying a live descriptor to make
a second allocator is prohibited. Callers synchronize access and keep storage
alive until every span is retired. Metadata validation cannot prove an actual
object's length, alignment, ownership or lifetime.

Allocation returns a native byte span (`unsigned char *data`, `size_t size`),
never a guest address. It reserves bytes without reading, writing or initializing
the backing region. Typed use additionally requires suitable C effective-type and
object-lifetime conditions; an aligned declared character array is tested as
bytes, not assumed to become arbitrary typed objects. Native object storage from
the host heap is supplied by the Windows adapter, never requested by the core.

There is no individual free, compaction, realloc, mark/rewind or hidden metadata
inside the pool. Reset invalidates **all** outstanding spans and restores used
to zero without erasing bytes. Reinitialization/reuse has the same invalidation
responsibility. No function detects stale span use or provides hostile isolation.

## Operations and ordering

`cs_arena_init(out, storage, capacity)` validates in order: non-null output,
capacity <= PTRDIFF_MAX, non-null storage if capacity > 0. Success records the
region with used zero. It does not touch storage. Rejection preserves output.

`cs_arena_alloc(arena, size, alignment, out)` validates in this order:

1. Non-null descriptor and output.
2. Descriptor state: capacity <= PTRDIFF_MAX, used <= capacity, and non-null
   storage if capacity > 0. A failed metadata check returns STATE.
3. Nonzero size. Zero-size requests return ARGUMENT, never a phantom allocation.
4. Alignment is a nonzero power of two, no greater than `_Alignof(max_align_t)`.
   Larger/invalid requests return ALIGNMENT; page/over-aligned allocation is outside v1.
5. The aligned start and end must each be <= PTRDIFF_MAX. Requests whose result
   exceeds that arithmetic boundary return OVERFLOW before checking capacity.
6. The aligned end must be <= capacity, otherwise EXHAUSTED.

Success reserves the earliest byte offset >= old used divisible by alignment,
then size bytes. Used becomes the exclusive end, including alignment padding.
The output span points into the supplied region and has exactly the requested
size. A positive successful span never points at the one-past end. Rejection
preserves all descriptor fields, output fields and every backing byte. Integer
checks precede pointer construction; no wrapped pointer arithmetic is permitted.

`cs_arena_reset(arena)` validates the pointer and descriptor using the same
rules, then sets used to zero. Rejection preserves state. Every operation is
bounded constant work; no backing scan, heap, libc call or mutable global exists.

| Result | Value | Meaning |
| --- | --- | --- |
| CS_ARENA_OK | 0 | Requested state transition succeeded |
| CS_ARENA_ERR_ARGUMENT | 1 | Null required argument, zero allocation size or null nonempty init storage |
| CS_ARENA_ERR_STATE | 2 | Invalid descriptor metadata |
| CS_ARENA_ERR_ALIGNMENT | 3 | Invalid/unsupported requested alignment |
| CS_ARENA_ERR_OVERFLOW | 4 | Constructor/allocation exceeds PTRDIFF_MAX arithmetic boundary |
| CS_ARENA_ERR_EXHAUSTED | 5 | Valid request does not fit remaining pool |

## Acceptance and applicability

Independent literal layouts cover padding, exact exhaustion, retry after failure,
reset/reuse and independent arenas. A separate oracle scans candidate offsets
one byte at a time instead of reproducing the implementation's rounding formula.
Vary capacities 0..65, all supported alignments, repeated requests, prior cursor
positions, guard bytes, and SIZE_MAX/PTRDIFF_MAX requests. Compare whole backing
buffers and rejection state; keep explicit checks active in Release builds.

The Windows viewer reserves its color, mono and presentation scratch buffers
from one caller-owned arena. Verify its normal rendering/hidden-window lifecycle
and a deliberately undersized-pool rejection path. The host owns allocation and
release of the region; arena ownership does not normalize Windows input or time.

Use the established strict-warning x64 Debug twins, Release, actual i686 and
validated ASan/UBSan matrix, optimized import/global-data audit, and supplemental
ARM64 little/big-endian compile checks. No m68k runtime, second compiler, native
boot or complete B1 claim follows. Macintosh budgets and replacement firmware
remain separate measurable gates; mini vMac testing must use our own firmware.
