# SPEC-0032 - CODE-only launch preflight

Engineering contract v1, 2026-10-14. Profile `classick-code-preflight-v1`.
SRC-0163/0165/0166 / ADR-0036. Contract only, unimplemented. Human provenance
review pending. No TEST/IMPL/build, fixture, guest ABI or execution is introduced.

## Purpose and evidence boundary

Before a loader changes an application heap, validate its immutable CODE metadata
and candidate A5/stack layout. A successful result is data-only preflight metadata,
never a launch transaction, owned executable image, callable entry, reservation,
heap-reset plan or permission to execute. Source selection has one supported
outcome, 68000 CODE; it does not select a native variant or introduce a native SDK.

Reuse SPEC-0024's reader, SPEC-0030's lifecycle constraints, SPEC-0031's source/
resource ownership and the independently checked SRC-0130 LK decisions. The latter
records eligible SRC-0071 II-57/59/60-62, I-89/91/94-95 and III-226, with the
published SRC-0068 SL chapter's differences. Those reviewed facts motivate the
fields; the original engineering requirements are explicit here. Neither edition
proves January1984. All AL-2/LQ-1..15 and prior historical questions remain open.
Raw research drafts, manual samples and protected executables are not inputs.

The 0x3F3C immediate-word push encoding is already present in the independently
authored SPEC-0028 session program and assembler comparison. Its instruction
occupies four bytes and pushes two stack bytes. This is existing ISA evidence,
not a copied manual sample or executed historical jump-table observation.

## Conceptual operations and native protocol

Two original host operations are defined; names are conceptual, not an assigned
numeric/native ABI: INSPECT_CODE_FORK and CHECK_LAUNCH_LAYOUT. The latter uses
a constructor-produced immutable inspection snapshot held by the caller. Both require explicit
ordinary inactive context, no root/callback/patch/other manager operation and
truthful synchronized native storage. Unknown/interrupt context refuses; this
read-only startup restriction is an engineering choice, not historical behavior.

Caller-owned typed inputs, result, report and scratch are aligned/live and
disjoint from source, map descriptors/backing buffers and protected runtime
controls. Native byte buffers need no extra alignment. Check pointer ranges and
aliases before writing diagnostics. Source is outside guest backing buffers and
immutable/live throughout inspection; no pointer from a file is dereferenced.
CHECK_LAUNCH_LAYOUT independently validates the snapshot's native bounds before
iteration. Metadata is owned protocol data, not an authenticated capability.

Failures preserve the source, guest memory, manager/lifecycle/session/CPU state
and canonical result. Safe disjoint diagnostics and bounded scratch may change.
Scratch is caller-provided, with explicit capacity; no host allocation, process
mutable globals, callbacks, guest instructions or persistent resource lease.
Publish the complete result only after all applicable checks pass. Refused input
must not leave a partly valid canonical manifest or layout.

Profile requires a positive uint32 maximum source length, with no default.
Native lengths above UINT32_MAX refuse before narrowing or reader access.
The selected bounded subset supports at most64 jump-table entries and16 distinct
positive segment IDs, including CODE1. These are engineering capacities, not
historical limits or manager/RAM capacity promises. Cache16/ticket32/manager32
limits remain independent; CODE0 is raw metadata, not an extra resident cache.

Results contain copied scalar fields, file offsets/lengths, warnings, declared
segment IDs/ranges and checked candidate logical addresses, not host function
pointers, executable tokens or guest handles. Validity is specific to successful
inspection/layout and immutable inputs. No source-lifetime ownership transfers.
Later consumption must independently retain/revalidate the source, metadata,
context, epoch, map and capacity under the future loader's contract. A numerical
source identity or copied metadata is not authentication or a durable reservation.

## CODE resource inspection

Use the tolerant SPEC-0024 source reader unchanged. Resolve exact type CODE
(0x434F4445) and signed ID0. Missing, requested duplicate type/ID, malformed source,
bit7 unresolved reference and unsupported system-heap request remain distinct.
Here local-storage eligibility means a real local reader payload (not a bit7
reference) plus the application-only attribute rule below. Source-outside-guest
backing is a separate earlier native/alias check, not this lookup-stage test.
No reference data offset/specification is followed. In this application-only
preflight profile, resSysHeap bit0x40 refuses for every selected CODE resource,
including CODE0; that is a profile exclusion, not a malformed-file judgment.
Other attributes, raw names and reader warnings remain metadata. No unrelated
type/ID is selected, no first-winner ambiguity rule and no native fallback.

CODE0 starts with four big-endian uint32 fields in order: above-A5 size,
below-A5 size, jump-table byte length and A5-relative jump-table offset. This
unsigned interpretation is selected engineering behavior; signedness remains
historically open. Require the complete16-byte header before reading fields.

Check header semantics in this order: all four fields even; nonzero table length
divisible by8; offset at least32 and no greater than0x7FFE; checked offset+length
no greater than above-A5; at most64 entries; checked16+table length within CODE0.
The table begins at resource byte16. Offset32 is canonical in this profile;
larger admitted offsets flag NONCANONICAL_JT_OFFSET, slack above the actual table
end flags WORLD_SLACK and extra CODE0 bytes flag TRAILING_BYTES. Flags grant no
historical equivalence. Above-A5 may exceed offset+length: B is then the upper
world boundary, not the table's exact end. Do not scan ignored trailing payload.
These three flags remain nonfatal when all required checks pass; they are not
promoted into CODE0_FORMAT or substituted for the actual reader warnings.

For each eight-byte initial entry, in table order, read the routine-offset word
at+0, push instruction word at+2, segment-ID word at+4 and trap word at+6.
Require push word0x3F3C and trap0xA9F0. These select the already supported
immediate-word push and reviewed LoadSeg trap shape; no instruction executes.
Loaded JMP entries, auto-pop forms or other encodings return ENTRY_FORM, not an
invented historical error. Segment IDs must be1..32767. The first entry must
name CODE1 (LK-20); do not defer its rejection until guest instructions run.

Collect distinct segment IDs in first-appearance order (therefore CODE1 first).
Reject the17th distinct ID before exceeding scratch/result capacities. For each,
resolve exact CODE/ID through the same reader; do not load, lock or allocate.
Every selected segment has at least a four-byte header plus two bytes available
at each admitted routine start. This profile caps total segment resource length
at32768 bytes, including its header, as an explicit conservative engineering
interpretation of the reviewed32K ceiling. It does not settle the historical
counting convention, larger-resource execution or LQ-11.

The first two big-endian uint16 words are the first-entry table-relative byte
offset and entry count. For this profile the offset is relative to the table's
start, not A5, CODE0's header or a native pointer. This explicit interpretation
is not a new historical observation. Require offset divisible by8, count1..64,
and checked offset+8*count within the table. Every declared covered row must
designate this segment, and every table row naming this segment must lie in its
declared interval. Thus the selected subset uses complete contiguous groups;
no overlapping/missing group is silently repaired. Keep this restriction separate
from the generic resource reader's tolerance of shared source spans.

Routine-offset policy is explicitly RESOURCE_START_LITERAL: unsigned even word,
at least4, with checked routine_offset+2 within that segment's logical length.
The four-byte header is counted in the base (LK-3); it is never a target.
Record this policy and `historical_offset_parity_unverified=true` in every valid
manifest. The alternative payload-start reading remains documented in SRC-0130;
it is not guessed from bytes or admitted as an implicit fallback. Inspection
proves only the selected policy's bounds, never AL-2 or instruction correctness.
No whole-code disassembly, machine-code reconstruction or payload execution.

Shared file spans do not themselves choose a winner or imply shared residency.
Independent selected segment keys still undergo their own header/group checks.
Source warnings survive in per-resource metadata. An ignored unrelated resource
cannot become executable or a native variant merely because it is present.

## Candidate launch layout

CHECK_LAUNCH_LAYOUT takes a successful caller-held inspection snapshot, an initialized
stable SPEC-0030 controller/manager/map, and explicit uint32 B, S and H_min.
Use the existing read-only lifecycle admission/integrity gate; refuse PREPARED/
FAULTED/active state before layout. No low-memory/global write or heap operation.
Optional supplied session controls independently prove inactivity and map identity.

B is the exclusive upper boundary of the candidate application world. S is a
positive even stack allotment; H_min is an even gross application-heap minimum,
possibly0. The documented8K default is motivation only: there is no implicit
S, B/BufPtr or H_min value. This does not choose a physical128K memory map.

Checked arithmetic derives, in order:

- A5 = B - above_A5.
- globals_begin = A5 - below_A5.
- stack_top = globals_begin; stack_base = stack_top - S.
- table_begin = A5 + table_offset; table_end = table_begin + table_length.
- candidate_ApplLimit = min(configured application end, stack_base).

Reject every underflow/overflow before a derived value or span is used. Require
0 < B <=0x01000000, even boundaries, nonempty stack wholly in strict24 logical
space, stack_base>=0x400, table_end<=B and table_begin>=A5+32. The32-byte parameter
area, complete above/below spans (skip a zero below span), table and stack must
have checked READ/WRITE data windows. No zero-length guest window is requested.
This does not grant execute permission or publish a native data view as code.

Let Z be configured application.begin. The original manager has32 cells times4
bytes, a128-byte band in each zone. Require checked Z+128<=candidate_ApplLimit
and checked Z+H_min<=candidate_ApplLimit (equivalently Z+max(128,H_min)). The
system band is [system.begin, system.begin+128), also checked. H_min=0 remains
admitted: the whole candidate heap including cells is still nonempty; a zero
allocatable-payload remainder after the cells gets no zero-length window and
does not prove a segment allocation can succeed. Validate the
candidate heap, stack and world against the system zone and both master-cell
bands; preserve low-memory/system ranges. A5-world and stack lie outside the
candidate heap below its limit. Within-world parameter/table overlap rules above
are explicit; source/native controls and backing ranges remain disjoint.
Check native backing-window aliases as well as logical ranges for the candidate
heap, system zone, stack and world. Distinct logical addresses with overlapping
backing bytes do not establish safe independent destinations. Intentional
parameter/table containment within the checked above-A5 world is not a conflict.

Critically, current SPEC-0030 requires its stack span disjoint from the complete
configured system AND application zones, not only from the proposed reduced
application limit. Enforce that same restriction here. A traditional layout
whose stack is inside the configured application zone is LAYOUT_UNSUPPORTED,
even if the arithmetic would lower ApplLimit below it. Do not silently reinit a
manager, alter zone bounds or claim the present lifecycle supports that layout.
An admitted already-disjoint map or a separately reviewed lifecycle extension
is required before a later loader. This is an original profile constraint.

Return checked scalar boundaries, current heap epoch tagged as observed, actual
table end, declared requirements and warnings. Small/zero below-A5 does not prove
the future InitGraf/QuickDraw span exists. Gross H_min and format validity do not
prove room for handles, segment payloads, scratch, native metadata or stack use.
No allocation has been reserved. Any later loader must revalidate before commit.
Inspection is a caller-held scalar snapshot, not a lease. CHECK_LAUNCH_LAYOUT has
no source lookup and reports `source_revalidated_for_layout=false`; its arithmetic
applies to that snapshot, not to an authenticated current fork. Source applicability
ends when those bytes change or their lifetime ends. A layout result is also
specific to its map, epoch and context; their changes invalidate that applicability.
Retaining mathematical metadata never retains source storage or launch authority.

## Failure order and diagnostics

Fixed classes, with irrelevant checks skipped: mandatory native/output safety;
known operation/context and profile/capacity envelope; actual source-reader/open/
CODE0 lookup and local-storage eligibility; CODE0 header semantics/source span;
initial-entry forms/IDs and distinct-segment capacity in table order; selected
segment lookup/header/group/target checks in first-appearance order; then layout
gate/integrity/parameters/arithmetic/map/profile checks for CHECK_LAUNCH_LAYOUT.
The layout operation has no new source lookup: its snapshot is constructor-produced
and immutable while used. Validate its64/16 native bounds before iteration and recheck all
scalar/count/index relationships, never treating arbitrary native forgery as
authenticated input. No failure mutates guest/protected state or publishes a
partly valid canonical result. Report only actually reached/tagged fields.

Within CHECK_LAUNCH_LAYOUT the order is explicit: safe native/count envelope and
context; controller identity/state/read-only integrity (STATE on refusal, with
its actual tagged lifecycle diagnostic); scalar parameter relationships and
checked derivations, including cell/H_min sums (LAYOUT); strict24/even/span-end
scalar bounds (LAYOUT); full configured-zone stack disjointness
(LAYOUT_UNSUPPORTED); candidate-limit/cell/H_min inequalities, other logical
overlap and native-backing alias checks (LAYOUT); then actual map windows/access
(LAYOUT). Never use a derived span before its arithmetic/scalar bounds pass.

Conceptual causes distinguish ARGUMENT, CONTEXT, STATE, PROFILE_LIMIT, SOURCE,
NOT_FOUND, AMBIGUOUS, UNRESOLVED_REFERENCE, UNSUPPORTED_STORAGE, CODE0_FORMAT,
ENTRY_FORM, SEGMENT_HEADER, ROUTINE_RANGE, LAYOUT and LAYOUT_UNSUPPORTED. Native
numeric ABI remains future work. Retain actual reader diagnostics/warnings,
resource key, table slot/header field and checked position where meaningful.
No invented target/handle, Mac system-error15/25/26/28, ResError/MemError or CCR.
Success reports `execution_authorized=false` and no reset/resource publication.

## Prerequisites for actual launch and unexecuted obligations

SPEC-0028 sessions, SPEC-0029 ordinary Toolbox dispatch, SPEC-0030 reset and
SPEC-0031 resource residency already exist, but do not compose into a launcher
automatically. SPEC-0031's external gate accepts CLOSED only; its loaded-resource
owner must not be omitted while starting a root session or resetting. Dynamic
LoadSeg continuation is still a typed refusal. A later contract must define
executable-image ownership, guest mutation/manager filtering, transitive returns,
main-segment lifetime, population commit/fault/teardown and future continuation.
This contract changes none of those gates and grants no entry permission.

Future original verification: native/source/output aliases and unchanged refusal;
CODE0 widths/truncation/count/evenness/overflow/slack/trailing warnings;64/16 exact
capacity and one-above; missing/duplicate/reference/system-heap key handling;
entry instruction/ID/first-CODE1 forms; segment group bounds/mismatch and2-byte
routine-start span at both limits; literal versus alternative offset discriminator
as policy only; exact32768/one-above segment restriction; layout underflow/strict24/
zero-below/exact table end and slack; explicit H_min/cell band/zone/map overlap;
current full-zone stack refusal versus admitted disjoint layout; stale native
metadata/context/epoch and success with execution still unauthorized. These are
unexecuted obligations: no fixture, code, TEST/IMPL/build or reference probe now.

The required two System1/ROM boot directions (SRC-0164) remain separate, untested
acceptance gates. Application preflight, replacement ROM scenes and hosted builds
prove neither direction. All24 originals/copies, prior build/edition/runtime
evidence, source/UART/native/physical/full128K/parity/B2 and human-review gates
remain. No MFS/Finder, guest Launch/Chain/LoadSeg/UnloadSeg/GetAppParms binding,
native application/SDK, boot change, emulator/VM/reference trial or OneNote.
