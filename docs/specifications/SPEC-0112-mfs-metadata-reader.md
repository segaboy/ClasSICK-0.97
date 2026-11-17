<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0112 - Bounded MFS metadata reader

2026-11-17; SRC-0613 (review of SRC-0071), SRC-0105/0109/0612;
IMPL-0142 / TEST-0201. This first MFS slice decodes volume metadata and one
caller-selected directory entry. It does not mount a filesystem, enumerate a
directory, interpret the allocation map, extract forks or boot a System Disk.

## Source and scope

The lead rechecked the owner's published 1985 Inside Macintosh Volumes I-III,
SRC-0071, printed II-120 through II-123, Figures 6-8 (PDF pages 688-691 in
the already identified copy). These pages describe disk layout and published
format fields. No Apple implementation, manual sample, in-memory manager layout,
ROM, reference image or excluded research draft is an implementation input.
This is a published 1985 format contract, not established January 1984 behavior.

Logical data blocks contain 512 bytes. Startup occupies blocks 0-1 and the
master-directory region occupies blocks 2-3. Its first 64 bytes contain volume
information. Multibyte fields below are decoded as unsigned big-endian values,
bytewise, without native structure overlays. Names remain counted raw bytes;
no character validity, collation, NUL termination or creation policy is implied.

## Volume fields

Offsets are relative to the beginning of logical block 2. The descriptions are
independently restated format facts from II-121/Figure 7.

| Offset | Width | Meaning |
| --- | --- | --- |
| 0 | 2 | Signature, hexadecimal D2D7 |
| 2 | 4 | Initialization timestamp |
| 6 | 4 | Last-backup timestamp |
| 10 | 2 | Attributes; hardware lock bit 7, software lock bit 15 |
| 12 | 2 | Recorded file count |
| 14 | 2 | First directory logical block |
| 16 | 2 | Directory length in logical blocks |
| 18 | 2 | Allocation-block count |
| 20 | 4 | Bytes per allocation block |
| 24 | 4 | Allocation clump bytes |
| 28 | 2 | First logical block of allocation storage |
| 30 | 4 | Next file number |
| 34 | 2 | Recorded free allocation blocks |
| 36 | 1 | Volume-name byte count |
| 37 | variable | Volume-name bytes within the 64-byte information region |

`cs_mfs_header_decode(block, length, device_blocks, out)` accepts an exact
512-byte input and a truthful device block count. Null input/output or a length
other than 512 returns ARGUMENT. A device with fewer than four blocks then
returns GEOMETRY. Next, a wrong signature returns SIGNATURE.
The following original structural checks return GEOMETRY, with output unchanged:

- Device contains at least four logical blocks.
- Name count is at most 27, the remaining information-region capacity. Zero is
  preserved as anomalous raw metadata, not treated as a valid creation name.
- Directory begins at block 4 or later, has positive length and fits the device.
- Allocation storage begins at block 4 or later; count is 1 through 640; size is
  positive and divisible by 512. The 640 ceiling follows from 960 remaining
  master-region bytes and 12 bits per entry; it admits no nibble-packing order.
- Allocation storage fits the device: after checking its first block, compare
  count against available logical blocks divided by blocks per allocation block.
  An equivalent bounded shift/add check may enforce this inequality without
  native division helpers. Never form an unchecked count-times-size product.
- Recorded free allocation blocks do not exceed allocation-block count.

Directory/allocation overlap is allowed: II-122/123 explicitly describes that
placement. This reader does not verify the corresponding reserved map entries,
allocation ownership, alignment, consistency of the file count, clump semantics,
next-file-number uniqueness, timestamps or historical acceptance of anomalous
metadata. Preserve every attribute bit and the raw clump value. Source prose
describes clump multiples; enforcing that relation is outside this first reader.
Only after every check succeeds, publish copied scalar fields and a counted
name into the caller's `cs_mfs_volume`, zeroing unused name-array bytes.
No borrowed pointers remain.

`cs_mfs_header_read(device, scratch, out)` validates non-null controls/callback,
positive device size and required distinct objects before reading. Its
`cs_mfs_workspace` owns two independent 512-byte arrays. It reads only LBA 2
through SPEC-0111, then calls the same decoder. A device shorter than the
two-block master region refuses before I/O. Provider UNAVAILABLE, IO and unknown
status remain distinguishable. All errors preserve `out`; workspace/provider
state may change. It does not read or validate block 3 or the allocation map.

## Single directory entry

`cs_mfs_entry_decode(block, length, offset, out)` receives one exact 512-byte
logical block and a caller-selected even byte offset. Null arguments, incorrect
length or an odd offset return ARGUMENT. Offset outside the block returns
ENTRY_BOUNDS. If bit 7 of the byte at that offset is clear, return INACTIVE
without reading a name or inferring its length; output stays unchanged. This
also applies at offset 510. No next-entry or end-of-directory meaning follows.

For an active entry, require all 51 fixed bytes and its counted name. Its full
extent is `(51 + name_length + 1) & ~1`, and must fit this same logical block.
All bounded arithmetic fits a 16-bit unsigned value. Failure returns
ENTRY_BOUNDS before publishing anything. Padding is uninterpreted: no zero check
or modification. The input is never changed.

Offsets below are relative to the supplied entry, from II-123/Figure 8:

| Offset | Width | Meaning |
| --- | --- | --- |
| 0 | 1 | Raw flags; active bit 7 and file-lock bit 0 |
| 1 | 1 | Version, distinct from Finder file-type information |
| 2 | 16 | Opaque Finder information |
| 18 | 4 | File number |
| 22 | 2 | Data-fork first allocation block |
| 24 | 4 | Data-fork logical byte length |
| 28 | 4 | Data-fork physical byte length |
| 32 | 2 | Resource-fork first allocation block |
| 34 | 4 | Resource-fork logical byte length |
| 38 | 4 | Resource-fork physical byte length |
| 42 | 4 | Creation timestamp |
| 46 | 4 | Modification timestamp |
| 50 | 1 | File-name byte count |
| 51 | variable | File-name bytes |

Copy all fields, opaque Finder bytes and up to 255 counted name bytes into
`cs_mfs_entry`, zeroing unused name-array bytes, with no borrowed pointers.
This does not add a C-string termination promise. The successful result may also retain
its numeric offset and even-rounded extent as original diagnostic locators.
Preserve zero-length active names,
embedded zeroes, high bytes, unknown flags and the version. Such extraction is
not approval of these as creatable names. Do not reject a zero logical length
with allocated storage, nor silently make an absent fork from that combination.
Fork lengths, start blocks and ownership are raw metadata, not validated chains.

## Native safety and validation

All buffers, controls, provider backing and outputs describe truthful, live,
stable, mutually disjoint native objects, each inside a single native segment.
Real pointer validity, arbitrary overlap and equivalent far aliases are caller
preconditions, not dynamically verified properties. No guest pointer is formed.
The workspace arrays are distinct subobjects. No allocation, global mutable
storage, retained callback/name pointer or fallible publication step is needed.
Every non-OK result preserves the complete output object; successful members
are defined, but C structure padding is not a serialization format.

Original fixtures must exercise endian asymmetry, all decoded fields, geometry
boundaries/overflow, directory-inside-allocation acceptance, independent lock
bits, raw names and versions, exact block-boundary entry fits, inactive tail
slots, active empty names, large lengths, input/output canaries, read failures
and failed-output preservation. Native Mac68000 and 286 execution and compiler
closure are separate from hosted/sanitizer checks. Mutated safety checks must
be rejected by the same native diagnostic when feasible. All results and measured
budgets belong in the implementation receipt, not a filesystem boot claim.
