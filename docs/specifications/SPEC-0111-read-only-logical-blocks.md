<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0111 - Bounded read-only logical blocks

2026-11-17; SRC-0612 / IMPL-0141 / TEST-0200. Original engineering contract
for shared native storage infrastructure. No historical disk format, filesystem,
driver, System boot, Apple implementation or segmented guest backend is implied.

`cs_block_device` declares a positive uint32 block count, an immutable native
callback and its native context. A block contains exactly 512 bytes. The
synchronous callback reads one LBA into a supplied staging block and returns
OK, UNAVAILABLE or ERROR. On OK it fills all 512 bytes; on failure it may have
changed any prefix or subset of those bytes. It cannot retain that pointer,
write outside the block, mutate controls/output, or reenter this operation.
The device is stable throughout a call; cross-block snapshot consistency is a
provider obligation, not something this service establishes for changing media.

Each read accepts uint32 LBA/count plus caller-owned output and staging buffers
with size_t capacities. These describe truthful, live, independently owned
native extents inside single native objects and segments. Each admitted capacity
is at most 32767 bytes; an oversized capacity is a checked numeric refusal.
All control objects, provider backing and the two buffers must be disjoint and
stable during the call. Real pointer validity, extent, overlap, physical far
aliasing and a callback's compliance are preconditions, not checked properties.
Exact output/staging pointer equality is refused without cross-object ordering.
No native pointer is made from an LBA or a guest address. This bounded contract
does not normalize far pointers or join segments.

Refusal order is fixed:

1. Null device/callback/buffer, zero device blocks/count, equal buffer pointers,
   or capacity greater than 32767: ARGUMENT.
2. Count greater than 63, LBA outside the device, or count greater than
   `blocks - lba`: RANGE. Subtraction is guarded by the LBA check.
3. Either capacity less than `count * 512`: CAPACITY. Multiplication follows
   count admission, so the largest native span is 32256 bytes, within C16 limits.

No provider call or byte mutation occurs for these preflight refusals. A device
with UINT32_MAX blocks has maximum valid LBA UINT32_MAX-1. No one-past LBA is
formed by the provider loop; every `lba + block_index` is already admitted.

Call the provider once per block in increasing LBA order into successive staging
blocks. Stop on its first failure. UNAVAILABLE maps to UNAVAILABLE, ERROR to IO,
and any unrecognized provider return to PROVIDER. Partial staging writes and
provider state remain; there is no rollback promise for either. Do not call a
later block or retry implicitly.

Only after every provider call succeeds, copy exactly the requested byte span
to output. This commit has no callbacks, allocations, fallible lookups or return
path other than OK. Under the stated native preconditions every non-OK result
preserves the entire output object, and every result preserves its unused tail.
The initial implementation uses a simple byte copy and allocates no storage.
One block therefore needs 512 output plus 512 staging bytes. Sixty-three blocks
need 64512 combined bytes in separate valid native objects; the API does not
assert that both objects fit one 286 near segment or a complete Macintosh OS.

Original tests exercise generated content/order, all count and capacity bounds,
high LBAs without a huge allocation, exact alias, preflight call suppression,
partial first/late failures, provider statuses, byte preservation and canaries.
Target execution and compilation are recorded separately in the
[receipt](../development/block-read-foundation-20261117.md).
