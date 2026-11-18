<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0115: bounded read-only MFS volume, directory and forks

2026-11-18. Original engineering contract building on SPEC-0111/0112 and the
coordinator's scoped SRC-0614 structural observation. It admits a strict read-only
profile, not a complete historical MFS implementation or Macintosh File Manager.
No protected disk, code, manual page, excluded research draft or external
implementation was provided to this implementer. The coordinator reports that
the selected image's consecutive active records/zero tails and high-nibble-first
map produce ten entries and twenty consistent fork descriptions. That observation
does not establish every historical disk's layout or deletion behavior.

## Ownership and failure contract

Each native object/span is truthful, live, stable, disjoint and confined to one
native segment. Caller supplies exclusive ownership and a conforming stable
device/media; no reentry, arbitrary alias detection, far-pointer normalization,
allocation or guest pointer conversion is provided. A mounted volume owns its
copied header/map and borrows its device/context until retired by the caller.
Do not mutate the mount or underlying disk during use. A seal detects accidental
uninitialized use; it is not validation of arbitrary fabricated native objects.

Every error preserves the entire output object and directory cursor. Scratch
and provider effects may survive. END preserves the entry but commits the
exhausted cursor. Successful C structure padding is not a serialized format.

## Mount

Read logical blocks2-3 through the existing all-or-nothing block service using
two caller-owned1024-byte workspace arrays. Decode the header under SPEC-0112.
Map entries begin64bytes into the read buffer. The960-byte map stores640 possible
entries, with bytes AB CD EF describing values ABC and DEF. Entry0 corresponds
to allocation block2. Values0,1,FFF mean free,end,reserved; other values must be
within2..allocation_count+1. Count free entries and require the header's free count.
Unused map capacity is copied but not interpreted.

Each allocation block whose logical-sector interval intersects the directory
must have FFF in the on-media map. Verify this; never manufacture reservations,
repair the map or write the medium. Other reserved entries are retained. Check
all products/sums against the admitted device size using bounded arithmetic.
Only after all checks succeed publish the header,map,device and initialized seal.
This is not global ownership validation: cross-linked files and unreachable used
blocks are not detected by mount.

## Directory and exact lookup

A zero-initialized cursor contains relative directory block, byte offset and
number of returned active entries. Scan each512-byte block from offset0. Decode
active entries under SPEC-0112, advancing by the even-rounded extent. A zero
flags byte requires the entire remaining block tail to be zero before moving to
the next block. A nonzero inactive flag or nonzero tail is FORMAT, not a guessed
deleted-record rule. Empty/full blocks and exact512-byte endings are supported.
An active record beyond the recorded file count refuses before publication.
At the directory end require the returned count to equal the header count.

Directory I/O or malformed records preserve cursor and caller entry, allowing
an explicit retry against stable media. There is no implicit retry or caching.
Lookup uses exact byte-count/name equality and returns the first match; it makes
no historical case-folding, character-validity, uniqueness or C-string claim.
Names0..255bytes and embeddedzero/high bytes are raw metadata. NOT_FOUND preserves
output. Lookup can return a found record before validating later directory bytes.

## Fork validation and reads

Select data or resource fields explicitly. Require logical length<=physical
length and start0 exactly when physical length0. Zero logical length with allocated
physical storage is allowed. A nonempty chain visits allocation numbers2..count+1;
an80-byte visited bitmap rejects cycles, and free/reserved/out-of-range links
refuse. Any block intersecting the directory refuses. The complete chain must
end at value1 and sum to exactly physical bytes using checked accumulation.
Do not infer a chain from a byte length or truncate it to hide bad metadata.

A range request has offset,count and disjoint output/staging spans. Count and
both capacities are at most32767bytes; capacities must cover count. Validate
offset<=logical and count<=logical-offset without overflow. Count0 at EOF is
allowed, with nonnull spans, and still validates the complete chain. Validate
the full chain before any data read. Then map arbitrary unaligned byte ranges
through fragmented allocation blocks and512-byte logical sectors. Read all bytes
into caller staging before publishing output. A partial provider write or failure
on any later sector preserves every output byte and its tail. Provider/scratch
and already-filled staging may retain effects; unused staging tail is preserved.

The source contains no runtime division/multiplication requirement for mapping:
checked bounded shift/add scaling and at most allocation_count subtractions
handle variable allocation sizes. This is a native read service, not an OS trap,
resource loader, executable launch, persistence implementation or boot result.

## Verification requirements

Original generated data must cover both forks, fragmented cross-sector ranges,
EOF and empty requests, maximum map index, invalid/free/reserved/cyclic links,
length mismatches, directory overlap, weird names, zero and malformed tails,
entry-count disagreements, failure preservation and guards. Validate Release and
sanitizer hosts plus native68000/C16 execution separately. Use meaningful mutated
chain/range/publication implementations to show the native oracle notices defects.
Record exact builds, commands, source/tool/media identities and limits in the
coordinator's canonical receipt. No protected reference bytes are test fixtures.
