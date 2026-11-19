<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0127 - Bounded streamed CODE metadata inspection

2026-11-18; IMPL-0157 / TEST-0218. Original engineering composition of prepared
SPEC-0122 Shell resource sources, SPEC-0116 transport and SPEC-0032's already
admitted CODE metadata rules. Reuse its source intake SRC-0163/0165/0166 and
SRC-0130 decisions with their published-edition/offset limitations; no new
historical fact, protected implementation, executable body or owner-medium
inspection informs this implementation.

## New protocol and boundaries

Inspect a constructor-produced immutable cs_rfs_view using explicit positive
maximum_source, maximum_entries and maximum_segments. The source length is a
uint32 file length, not a native contiguous extent. maximum_segments is1..64.
The caller explicitly declares ordinary inactive context: no active root,
callback, patch or manager operation. Other/unknown contexts refuse. This
standalone native preparation has no active guest map, lifecycle or manager.
No guest state is touched and no existing lifecycle/ownership gate is bypassed.

All controls, source provider/context/backing and output storage are aligned,
truthful, live, disjoint and individually contained in native segments, used
exclusively without reentry. Validate representable disjoint native extents for
the request, source view, full supplied workspace, manifest and report before
any write. Workspace capacity is at least its fixed structure size and at most
32767 bytes. Provider backing/lifetime and hidden native aliases remain caller
preconditions; the resource callback interface has no declared context extent.

The new manifest is distinct from SPEC-0032's old64-row/16-segment manifest.
It stores the four CODE0 fields, counts, warnings, validated CODE0/resource spans,
and at most64 segment descriptors in first-appearance order. Each segment has
the existing cs_rf_entry plus reader warnings and checked first/count table
range. It does not store an array indexed by every jump row. Total rows are
bounded by explicit maximum_entries and the actual CODE0 payload extent.
Increasing these engineering capacities supplies no historical compatibility
claim and does not relax the unchanged old APIs' capacities.

The manifest owns its scalar/span metadata and copies the input resource view
by value. Its callback/context still borrow the ORIGINAL source owner's lifetime,
not the inspection workspace. That owner, its media and controls must remain
live/immutable for subsequent trusted bounded reads. The manifest can supply
validated spans for a future loader's copy plan; it is not authenticated source
ownership, a reservation, loaded image, guest handle, callable entry or execution
authority. Old code-image/layout/run APIs must not consume its distinct tag/type.

## Inspection rules

Use the existing resource view/find/read profile unchanged. Select exact CODE0
and required positive CODE IDs. Missing/duplicate/unresolved-bit7/system-heap
resources remain distinct; resSysHeap0x40 refuses all selected resources as the
same application-only profile exclusion. Other attributes and resource warnings
remain metadata. No unrelated resource is selected or native variant inferred.

Require CODE0's16-byte header and decode unsigned big-endian above-A5, below-A5,
table length and A5-relative table offset. Preserve SPEC-0032's order: all even;
positive table length divisible by8; offset32..7FFE; checked offset+length within
above-A5; entry count within the caller maximum; complete16+table length within
CODE0. Noncanonical offset, world slack and trailing bytes retain their separate
nonfatal warnings. No trailing CODE0 payload is scanned.

Read the initial table at resource offset16 in batches of at most512 bytes.
Each row retains the admitted routine word,3F3C push word, positive1..32767
segment ID and A9F0 trap shape; the first row names CODE1. Validate all row forms,
IDs and distinct-segment capacity before looking up any segment. Record each
segment's first/last row and number of appearances in bounded scratch.

Then process segment IDs in first-appearance order. Find the resource, require
local/application storage, enforce the unchanged conservative32768-byte whole
segment-resource ceiling, and read only its four-byte header. Its first word is
a table-relative byte offset divisible by8; second is a positive count. Check
offset+8*count within the table. Its declared first/count must exactly equal the
observed first/count, and last-first+1 must equal count. These conditions prove
the same complete contiguous-group rule as SPEC-0032 without retaining every row.
Group-header failures report the segment header; unlike the old array protocol,
they do not promise the earliest mismatching table slot.

Re-read that group's table rows in bounded batches to validate routine offsets:
unsigned even, at least4, with two bytes available within that segment's resource
length. The default base is RESOURCE_START_LITERAL, including the four-byte header.
For a separate metadata experiment the caller can explicitly select AFTER_HEADER.
For this profile add four to the unsigned word using 32-bit arithmetic before
applying the same alignment and extent rules. Relative word zero therefore
names the first two bytes following the header. An unknown selector is an
argument error. Keep the same selector for the entire table. Returning the
selected base and the literal flag lets a consumer distinguish these manifests.
The alternative is an engineering hypothesis about the origin of the offset.
It makes no new historical assertion and does not change SPEC-0032.
Both profiles record historical_offset_parity_unverified=1 and execution_authorized=0. No routine
instruction bytes are requested, interpreted, exported or executed. Transport
metadata cache read-ahead remains its separately stated behavior.

## Publication and diagnostics

All construction occurs in caller workspace. Publish the complete manifest with
one infallible native copy after every applicable check succeeds. Every refusal
preserves every manifest byte. Unsafe native-envelope refusal also preserves
report/workspace; other refusals may change diagnostics, scratch and provider
state. A failed late read must not leave a partly valid output or erase a prior
manifest. Workspace can be destroyed afterward because no pointer to it escapes.

Reports identify a cause, semantic field, reached key/slot/absolute position,
actual resource-transport result and counts of validated rows/seen segments.
For a routine-range refusal, the native probe also reports the zero-based table
row and one reason in precedence order: odd (1), before the end of the header
(2), beyond the resource (3), or fewer than two bytes left (4).
Transport IO positions remain distinct from guessed routine execution locations.
Reports always leave execution_authorized zero. The native composed probe exports
only numeric aggregate/status/layout fields, never keys, IDs, names, jump words,
routine bytes or payload digests. Its optional private DC42 route is executed
only by the coordinator, retains classified refusal and never becomes a fixture.

## Verification

Original fragmented MFS media names a System resource source and a Shell source
with CODE metadata beyond64KiB. Baseline has180 jump rows and20 segments, exceeding
both old64/16 capacities. Positive explicit maxima, exact bounds/one-above,
header/forms/first-ID/group/routine rules, resource eligibility and transport
failures must preserve outputs. After success destroy the construction candidate
and consume its copied source and validated spans. Deliberate early-publication,
wrong-routine-base and skipped-group defects must fail unchanged original checks.

Qualify optimized/sanitized hosts and the actual native286/Mac68000 modeled
providers, including a late CODE0-table read fault and cold recovery. Pin complete
source/helper/tool/input identities and native code/BSS/stack layout. No full
loader, launch layout, guest memory mutation, INIT dispatch, protected execution,
System boot, selected-disk write or physical hardware result follows.
