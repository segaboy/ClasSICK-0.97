<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0116: bounded resource-fork transport and MFS binding

2026-11-18. Original adapter for the already admitted SPEC-0024 format profile
and SPEC-0115 read-only MFS fork service. No new historical-format assertion,
protected input, third-party implementation, executable loading or OS trap.

## Source and native storage

A source supplies a uint32 logical length, context and synchronous read callback.
Every callback reads 1..256 bytes at a checked absolute file offset. Success fills
exactly the requested extent; failure may change that destination extent and its
own state, but never writes beyond it or retains the pointer. Source/media,
constructor-produced view and returned entry stay immutable and live. All native
controls, callback backing, scratch and outputs are disjoint live objects confined
to their native segments. Arbitrary pointers/aliases are caller preconditions.

The stream reader never derives a native pointer from a file offset. A caller
workspace holds one 256-byte metadata cache. Each open/find invalidates that cache
before use. Cache fills begin at an aligned 256-byte position and end no later
than source.length; they may read adjacent in-file bytes not individually needed
by a metadata field. Provider failures during this permitted read-ahead are real
operation failures, with the cache-fill position recorded. No failure is masked
by partially filled cache bytes. A fork above 64 KiB needs no contiguous native view.

## Format behavior

Open/find implement SPEC-0024's exact bounded profile: bytewise big-endian fields,
checked header regions/overlap, type and reference table ordering, name lengths,
payload lengths, FFFF empty counts, signed 16-bit IDs, bit 7 external-reference metadata,
exact requested-key duplicate refusal and existing warning flags. Full structural
reference-list validation precedes individual record validation. Names precede
payloads. Matching the existing contiguous reader is a regression oracle, not an
independent historical proof. No resource payload is interpreted or executed.

The new view owns source metadata and validated numeric regions, not disk bytes.
Entry remains cs_rf_entry and all name/data spans remain file offsets. Format
status values 0..10 correspond to the existing reader's statuses. Additional statuses
are UNAVAILABLE 11, IO 12, PROVIDER 13 and CAPACITY 14. Reports retain field, absolute
position, warnings and type/reference work counters. Unknown callback results map
to PROVIDER. Argument refusal leaves every output/report unchanged. Other failures
may replace report/cache/provider state, preserving complete view/entry outputs.

## Data and name extraction

Read selects either a returned local payload or a present raw resource name.
External-reference payload returns REFERENCE and never resolves or reads a target;
an absent name returns NOT_FOUND. Verify the selected span against the immutable
view's data/map bounds. Count and both native capacities are at most 32767 bytes,
capacities cover count, and offset/count must fit the selected span exactly.
Nonnull disjoint output/staging are required even for count 0, which may be at EOF.

Fill caller staging in consecutive chunks of at most 256 bytes. Publish output only
after every read succeeds. Any partial callback failure or late unavailable sector
preserves the whole output and its unused tail; scratch/provider/filled staging may
retain effects. Unused staging tail is unchanged. A successful zero-byte request
performs no callback. Lengths are not truncated to 16 bits before admission.

## MFS adapter

Binding receives a stable cs_mfsv_volume, directory entry, MFS workspace, adapter
and source. Validate the complete resource-fork chain through SPEC-0115 first;
on refusal leave adapter/source unchanged and return the precise cs_mfsv_result.
On success the source length is resource_logical. The adapter borrows all controls
and owns 256 bytes of transfer staging. Each callback delegates an exact bounded
range to cs_mfsv_fork_read, preserving that service's complete-chain validation and
atomic read behavior. Map unavailable and I/O distinctly; unexpected lower-layer
errors become PROVIDER. It does not cache mutable disk contents, write media or
claim a physical BIOS/floppy provider.

## Verification

Use original sparse resource/MFS fixtures, including a fork larger than 64 KiB,
fragmentation, cross-sector metadata and range reads. Compare admitted format
cases with the existing contiguous reader where native memory permits, plus
independent expected statuses, offsets, bytes and mutation controls. Exercise
short/overflow/overlap/duplicate/name/payload errors; external references; exact
error publication; callback partial failures; maximum native range and EOF.
Qualify real C16 object/link closure and native 286 execution separately from
hosted tests and Mac 68000 builds. Record compiler frames/code/data costs and all
actual results; no full Resource Manager, System startup or hardware claim.
