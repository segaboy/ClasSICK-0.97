# SPEC-0006 — Bounded UEFI preboot data and exit transaction

Status: finalized v1 for independent preboot contracts, 2026-10-08, lead self-review.
Owner SRC-0033; published interfaces SRC-0034; ADR-0010; IMPL-0010 / TEST-0012.
Profile: x64 PC UEFI, explicit little-endian memory-descriptor bytes. Hosted tests
also exercise 32-bit widths; they do not execute firmware or prove native startup.
This is a platform contract, with no historical Macintosh behavior or new core API.

## Storage and results

All pointers denote truthful live objects/buffers of their declared size. Outputs
are disjoint from inputs, and callers own lifetime, synchronization and immutability
through each call. Physical addresses are uint64_t values, never dereferenced by
these validators. No allocator, firmware callback, mutable global, I/O or heap.
Results: OK=0, ARGUMENT=1, FORMAT=2, LIMIT=3, OVERFLOW=4, OVERLAP=5,
OWNERSHIP=6, STATE=7. Validation errors preserve outputs and all input bytes.
Validation does not attest firmware honesty, actual page permissions, writable
hardware, pointer accessibility or ownership outside these stated preconditions.

## Framebuffer

Input fields: physical base/size, width/height, pixels per scanline and GOP pixel
format. Accept only RGB-reserved (0) and BGR-reserved (1); bitmask/Blt-only reject.
Original profile bounds: dimensions 1..8192, pitch >= width and <=16384 pixels,
size 1..256 MiB, nonzero base and entire physical span below 2^47. Pitch*4*height
must fit declared size. Success emits byte stride and required byte length, without
accessing pixels. These bounds are our supported profile, not UEFI requirements.
Unsupported modes fail before any framebuffer write; no mode-changing callback is
allowed in the final map/exit transaction. Core RGBA8 alpha is not a GOP reserved
byte: the later native presenter must convert original pixels explicitly.

## Memory map

Input is a byte buffer, byte length, returned descriptor stride and version.
Version must be 1; length positive <=256 KiB; stride 40..256, multiple of 8;
length divisible by stride and count <=1024. Read the first 40 bytes using explicit
little-endian offsets: type 0, physical start 8, virtual start 16, pages 24,
attributes 32. Ignore extension/padding bytes. No host struct cast or assumed
array stride. Physical/virtual starts must be 4 KiB aligned; pages positive;
pages*4096 and physical/virtual exclusive ends must fit uint64_t. The top page
ending at 2^64 is outside this bounded profile. Unknown types remain reserved;
unknown attribute bits are preserved without inferring free memory. Pairwise physical overlap rejects;
unsorted/noncontiguous maps are allowed. Success emits descriptor count only.

Worst-case work is bounded O(count^2), with constant scratch storage; no sorting
or rewriting of firmware bytes. A readable, disjoint output is required. Map bytes
must remain immutable across validation and ownership checks.

## Owned allocations

Validate 1..8 explicitly reserved page-aligned spans, with kind code=1 or data=2.
Each has nonzero base, positive page-multiple size, entire span below 2^47, and is
fully contained in one map descriptor of matching EfiLoaderCode (1) or
EfiLoaderData (2), without EFI_MEMORY_RUNTIME. Spans must not overlap each other
or the entire validated framebuffer span. Cross-descriptor allocations are outside
v1. This verifies map consistency; the loader must separately establish actual
AllocatePages/LoadedImage ownership and preserve the allocation ledger.
Conventional memory, boot-service ranges, runtime/ACPI/reserved/unaccepted and
unknown types are not accepted as owned allocations. No automatic reclamation.

## Exit transaction model

State has phase READY=0, SNAPSHOT=1, RETRY=2, EXITED=3, FAILED=4; attempts 0..3;
opaque last key. Init establishes READY/zero attempts. Snapshot records any key,
including zero, only from READY/RETRY with attempts remaining. A caller may record
one exit outcome only in SNAPSHOT: success=0 -> EXITED; stale key=1 -> RETRY while
attempts<3, otherwise FAILED; other failure=2 -> FAILED. Each outcome increments
attempts exactly once. Recording a valid outcome returns OK even if phase FAILED;
this means the model accepted the observation, not that firmware exited.

Allowed calls: unrelated boot service only in READY before any attempt; GetMemoryMap
in READY/RETRY; ExitBootServices only in SNAPSHOT; native loop only in EXITED.
SNAPSHOT forbids logging/protocol/allocation/mode operations before exit. RETRY
permits only re-capturing into the already allocated buffer; no console, protocol,
allocator or file operation. FAILED permits no firmware call or native-loop entry.
This is a deliberately stricter project policy than UEFI's permitted retry services.
Terminal failure uses an independently implemented halt/diagnostic path; returning
to firmware after a first exit attempt is prohibited by our loader contract.

The model calls no firmware and cannot enforce what external code actually calls.
The loader must use it alongside checked final map/owned allocations, copy all
required descriptors before exit, and invalidate boot-service pointers on success.
Three attempts is an original finite policy; failure is observable, never retried
indefinitely. Fake outcomes and metadata checks prove this model only.
An invalid map or failed capture during RETRY requires the caller's terminal halt,
without another firmware call; it is not recorded as an exit outcome that never occurred.

## Acceptance

Original literal/independent fixtures cover unusual pitch, unsupported modes,
bounded arithmetic, unaligned bytes/extension strides, unsorted maps, overlapping
descriptors, reserved ownership, runtime spans, independent outputs and all finite
exit outcomes/transitions. Test with Clang/GCC, actual i686, x64 sanitizers and
object import/data audits. Actual UEFI ABI calls, kernel entry/stack/exceptions,
native devices and B2 boot remain separate implementations and observations.
