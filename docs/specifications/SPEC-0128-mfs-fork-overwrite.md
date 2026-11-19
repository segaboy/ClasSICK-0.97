<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0128: bounded overwrite of an existing MFS fork

2026-11-18. Original engineering contract built on SPEC-0111/0112/0115 and
the explicitly admitted single-sector providers SPEC-0125/0126. No new
historical format fact is introduced. This is not a historical File Manager
trap, allocation/create API, filesystem transaction or startup claim.

## Admission and API

`cs_block_writer` declares a logical block count, synchronous one-sector
callback and context. A successful callback commits exactly the supplied
512 bytes. It never retains/modifies the source. A failed callback may have
changed any part or all of that sector; every other sector is preserved.
Provider/context effects survive. Invalid callback values are provider errors.
Concrete writable-media admission and later durable snapshot/flush are separate.

`cs_mfsw_fork_overwrite` accepts a mounted volume, workspace, decoded entry,
data/resource fork, writer, byte offset/count, input/capacity and progress object.
The writer block count must exactly equal the mounted reader block count, and
the caller guarantees that both name the same logical medium. There is no
pointer-based proof of device identity. A valid mount and metadata remain
stable under exclusive access; target allocations have no other live owner.
Global cross-link detection is not added to SPEC-0115. All native objects and
spans are truthful/live, disjoint and confined to their native segment.
Deliberately shared read/write provider contexts are allowed, but their backing
cannot alias input, workspace or result. No reentry/concurrent operations,
arbitrary overlap detection, far-pointer normalization or guest addresses.

This contract permits only its own data-sector changes during exclusive use
of the mount. A later read under SPEC-0115 may observe those completed changes
with the metadata still unchanged and the medium stable throughout that read.
It does not permit external media mutation or stale metadata/map reuse.

Count and native input capacity are at most32767. Require nonnull objects even
for count0 and capacity>=count. Require offset<=logical length and
count<=logical-offset using overflow-safe checks. The entire physical chain
must validate under SPEC-0115 before the first write, including cycles, reserved
and out-of-range links, directory overlap and exact physical length. Equal
writer/reader geometry makes all validated allocation sectors writable-range
eligible. Count0 at EOF still validates the whole chain but performs no I/O.
All preflight refusals preserve the complete progress object and issue no writes.
For a nonempty request, refuse a hardware-locked or software-locked volume
(attribute bits 7 or 15) and a locked file (entry flag bit 0) before any
data-sector I/O, returning MFSV_UNAVAILABLE and preserving progress. Source
of the field meanings: SRC-0613 and SPEC-0112, published 1985 Inside Macintosh
II-121 through II-123. The return value and zero-length policy are original
native service decisions. A zero-length operation still validates the chain
and may succeed on locked media. Provider-side write protection can also
change after mounting and remains a separate possible I/O failure.

## Execution and effects

Walk the existing copied allocation map in fork order. Full sectors may write
directly from the input. Partial sectors read the existing512 bytes through the
mounted block reader, replace only the requested portion, then invoke one write.
Each successful callback commits independently. The callback source remains
stable during the call; scratch and provider state may change. No input bytes,
mount, entry, map, directory, header, logical/physical length or timestamps change.
The concrete Mac provider preserves the sector's existing12 tags.

On success publish progress `{completed:count,lba:0,phase:DONE,uncertain:0}`.
On a read failure publish the exact count of requested bytes in prior successful
sector writes, the current LBA, READ phase and uncertain0: this call did not
attempt a write to that failing sector. On any write callback failure publish
the prior successful prefix, failing LBA, WRITE phase and uncertain1. The whole
failed sector may have changed, including bytes outside the requested range.
Do not count any bytes from that sector as definitely completed even if the
provider committed before reporting failure. Return MFSV_UNAVAILABLE, MFSV_IO
or MFSV_PROVIDER as appropriate. There is no retry, rollback or automatic
verification. A caller must decide recovery by an explicitly bounded reread.

## Required evidence

Original host fixtures cover both forks, fragmented and unaligned writes,
partial first-sector prefix accounting, full sectors without reads, zero/EOF,
range/capacity/argument refusal, whole-chain defects outside the requested
region, equal writer geometry, errors before/after writes and a failed callback
that mutates part/all of its sector. Check complete media and input/control
preservation, including metadata and unrequested tails. Release and sanitizer
hosts are separate from native68000/C16 trials.

Native paired fixtures use only original disposable media. Compare the entire
export against an independent expected image, preserve source hashes, and read
the changed forks in a fresh protected run. Mac raw and tagged DC42 retain
metadata/tags; the286 transfer image is an explicit test adapter. Meaningful
mutations must expose skipped preflight validation, wrong fragment mapping or
incorrect completed-prefix reporting. Record exact source/tool/binary identities,
budgets and actual checks under TEST-0219 and the canonical receipt.
