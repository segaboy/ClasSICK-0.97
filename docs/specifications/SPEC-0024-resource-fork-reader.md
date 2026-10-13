# SPEC-0024 - Bounded read-only resource-fork reader

Finalized engineering contract v1, 2026-10-13; not implemented or executed.
Profile `classick-resource-reader-v1`; SRC-0106 / ADR-0028 / TEST-0032.
Lead self-review; human provenance review pending. January 1984 conformance is
unconfirmed. This specifies an OS/core format service, not an application SDK.

## Evidence and scope

SRC-0071 I-108/110-112/114/125/128-131 and SRC-0068 Resource Manager chapter
11/28/84, RM-13/14/30/34-37/39/41 provide published interfaces and format tables.
The 112-byte area and bit-7 reference meaning differ by edition. Byte order,
unsigned offsets, empty counts and tolerant reading below are explicit profile
choices informed by those interfaces, not observed January 1984 behavior.

Read one caller-supplied immutable resource-fork byte buffer. Validate structural
regions and find a resource by exact four-byte type and signed 16-bit ID. Return
file offsets/lengths and raw metadata. No host I/O, heap allocation, callbacks,
mutable globals, guest addresses, Resource Manager search chain, resource loading,
name lookup, content decoding, writing or application execution.

## Representation and ownership

File offsets and lengths are uint32 values. An outer adapter must reject a native
length above UINT32_MAX rather than truncate it. Decode all fields bytewise in
most-significant-byte-first order; no alignment requirement or native struct cast.
IDs have two's-complement 16-bit meaning. Offset fields are unsigned; FFFF means
no name only in the two-byte name-offset field. Every addition and count-derived
extent is checked before use. An unrepresentable end yields OVERFLOW.

The caller supplies a truthful contiguous input buffer, view, entry and report
objects with valid lifetime and disjoint storage. The input must remain immutable
and the constructor-produced view unchanged throughout lookup. Never treat a
file value as a native pointer. Arbitrary native pointers are outside this caller
contract; guest-facing code must supply checked storage through a later adapter.

`cs_rf_open` creates a borrowing view only after full validation. `cs_rf_find`
takes a numeric uint32 type key (the four file bytes packed most significant
first) and an int32 ID constrained to -32768..32767. Both operate under
`formats/resourcefork/`; implementation headers and ABI details are future work.
The view records validated file regions/counts and available raw diagnostic
spans. An entry records attributes, reference position, optional raw-byte name
span, and either a local data span or the raw four-byte reference specification.

Argument failures preserve every output, including the report. Other failures
preserve the view/entry and input; they may replace the bounded report. Successful
open/find publish complete outputs once. NOT_FOUND and duplicate results preserve
the entry. REFERENCE publishes metadata with no local data; it does not resolve
the target or claim that the target's name/data has been validated.

## Structural rules and deterministic validation

Regions are half-open. Empty regions do not overlap, but their positions must
still lie within the supplied buffer. Error order is:

1. Arguments and length representation; then the complete 16-byte header.
2. Header data end, then map end: checked sum before buffer-bound comparison.
   An end beyond the input is TRUNCATED.
3. Pairwise overlap of nonempty header [0,16), data and map regions: OVERLAP.
4. Map length at least 30 (28 fixed bytes and a type-count word): OUT_OF_RANGE.
5. Type-list position at least 28, with its count word inside the map; name-list
   position at least 28 and at most map length. The latter may denote an empty
   list at map end. Fixed-part overlap is OVERLAP; other map bounds failures
   are OUT_OF_RANGE.
6. Count decoding: FFFF means zero types/resources in this engineering profile;
   otherwise decode stored count plus one. The type list's 2 + 8N bytes must
   fit the map and finish no later than the name-list start.
7. Reference lists in type order: offsets are relative to the type-count word,
   not the first type entry. Each list's 12M bytes must lie after the type table,
   after the preceding list, and no later than the name list. Ordered gaps are
   accepted. Invalid list regions yield OVERLAP or OUT_OF_RANGE as above.
8. Entries in list order: validate their own name first. A non-FFFF name offset
   points to a length byte relative to the name-list start; its complete raw
   character span must fit the map. NAME denotes a name-bound failure.
9. For an entry with bit 7 clear, the three-byte offset points to the four-byte
   resource-length field relative to the data-region start. Validate that prefix
   and the entire declared payload within the data region. Region-bound failure
   is OUT_OF_RANGE; arithmetic carry remains OVERFLOW. For bit 7 set, expose the
   raw four-byte specification and no local payload. Its data offset is not used.

These checks accept safely bounded gaps, map-before-data, trailing bytes and
files without the whole 240-byte prefix. Such layouts receive a layout warning.
The type/reference/name regions still follow the checked order above. Regions
inside the map must not alias its structural tables; local payloads/names from
different entries may share or overlap if each individual span is valid.

Do not cap the whole data region at 16 MiB. The 24-bit field limits the position
of a resource's length prefix; its uint32 payload length may extend farther.
No 32 KiB read limit follows from the documented writer limit. Later heap/loader
contracts decide whether any returned content fits a target's memory budget.

## Lookup, warnings and work bounds

Find scans the complete type table for the requested exact type. Zero matches
returns NOT_FOUND, more than one DUPLICATE_TYPE. For one type, scan its complete
list for the exact ID; return NOT_FOUND or DUPLICATE_ID on ambiguity. Unrelated
duplicates do not prevent other lookups. No first/last winner is invented.

Results: OK, REFERENCE, NOT_FOUND, ARGUMENT, TRUNCATED, OUT_OF_RANGE, OVERLAP,
OVERFLOW, NAME, DUPLICATE_TYPE and DUPLICATE_ID. Reports identify the result,
failing field/file position and bounded warning flags. Nonfatal open warnings
cover noncanonical layout, nonzero map-reserved bytes, nonzero ordinary handle
fields, resChanged/bit 0, bit 7, empty per-type counts and large local resources.
A bit-7 specification is not mislabeled an ordinary handle required to be zero.

The reader performs a constant amount of metadata work per type/reference,
does not scan payload bytes, and uses constant native state. Disjoint structural
tables bound total entries by file/map length. Caller storage and compiled
frame/import costs must be measured before any 128K or native claim.

A complete strict-layout/duplicate/shared-span audit is deferred to a separate
contract with explicit caller-owned scratch capacity. It is not included in v1,
and the reader's warnings do not claim whole-file uniqueness or sharing analysis.

## Adopted L-decisions

| Decision | Selected behavior |
| --- | --- |
| L-1 | Tolerant bounded reader; exact layout belongs to a later audit. Global data/map order is tolerated; internal structural list order is checked. |
| L-2 | FFFF means zero resources, with a warning; historical encoding remains open. |
| L-3 | Ambiguous requested type/ID yields a duplicate result; no winner. Unrelated keys remain usable. |
| L-4 | Bit 7 yields REFERENCE metadata and no local data. This conservatively accommodates the earlier system-reference format without claiming the later reserved bit has identical semantics. |
| L-5 | Accept bounded nonzero documented-zero fields and resChanged; warn. Ordinary handles remain opaque bytes, never pointers. |
| L-6 | Accept individually valid shared/overlapping payload or name spans. Full sharing detection is deferred with the optional audit. |
| L-7 | Accept in-bounds resources above 32768 bytes; report the writer-limit warning, not a read-format error. |
| L-8 | Unsigned positions/lengths; signed IDs and the name sentinel are explicit exceptions. |
| L-9 | Checked uint32 file arithmetic, irrespective of native pointer width. |
| L-10 | Fixed order above; the first failing field determines the result. |
| L-11 | cs_rf_open / cs_rf_find in formats/resourcefork; numeric type key and bounded report. No audit implementation or native application headers. |
| L-12 | Raw [16,128) and [128,256) diagnostic spans only when each complete span fits; otherwise mark that span unavailable. They may overlap other regions in tolerated layouts and carry no inferred semantic meaning. |

RF-1 through RF-9 remain historical/documentation questions; choosing an
engineering result does not close them. TEST-0032 is a plan, not a passing run.
No MFS decoder, boot entry, emulator/VM, x86 application or reference observation
is authorized by this specification. Existing owner/platform/edition gates persist.
