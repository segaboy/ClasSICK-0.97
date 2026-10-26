<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0069 - Checked segmented guest storage

2026-10-23; SRC-0399..0401 / ADR-0080 / IMPL-0071 / TEST-0084.

Status: adopted original engineering contract and executable reference model for partial P286-2; production integration is deferred. `core/memory/guest.c` is unchanged. Native Mac/x64 fast paths and the existing flat-view API are unchanged. No per-byte callback is imposed on native builds. This contract adds no raw/far pointer, physical address, memory-model or ABI freeze, allocator, loader or helper body.

## 1. Scope
The existing flat API (`cs_guest_window` and its views) lends native pointers. It cannot represent a logical 128 KiB region on C16, where size_t is 16 bits and the native chunk limit is 32767. A segmented build must therefore not silently keep that API. It also must not reject every valid operation larger than one chunk. Two candidate C16 surfaces are modeled:
- `read`/`write` for widths 1/2/4, performed inside the backend.
- A bounded checked capability that yields chunk descriptors.

## 2. Logical map
- At most 8 regions in the 24-bit space, with the current rules:
  - even base and even size;
  - non-overlapping;
  - READ is mandatory, and WRITE/EXECUTE imply READ;
  - base < 0x1000000 and size <= 0x1000000 - base.
- Flags are validated with full 32-bit masks (for example `flags & 0xFFFFFFF8UL`). The narrow `~7u` idiom is invalid on C16. Any new flag helper must use `(uint32_t)~mask`, as `cs_c16_clear_flags` does.
- A logical operation must fit ONE region. Adjacent distinct regions never jointly satisfy it.
- A region owns an ordered list of backing banks.
  - At most 64 banks in total.
  - Each bank has a canonical identifier in 1..64 that is unique across the whole map.
  - Each bank size is 1..65535, representable as a size_t16 length. This does not prove that an arbitrary native far base can address the whole bank without an offset wrap; backend admission must establish that independently.
  - Bank sizes must sum exactly to the region size: no hole, no excess, order as listed.
- Banks are described by logical bank offsets, never by encoded native pointers.
- Backing keys must equal exactly the set of canonical IDs referenced by the regions (no extras). In the Python model readable banks are exactly `bytes` or `bytearray`; WRITE needs `bytearray`. These are model types, not a native representation choice. Flags and access masks outside uint32 are rejected before any full-width mask.
- Creating a map preflights the candidate first. Invalid MAP/ARG has no registry or lifetime effect and returns no map; exhaustion never wraps.
- A word or long may cross backing banks inside one region. Values are big-endian. Word and long accesses need an even logical offset.
- Canonical identifiers do not prove physical disjointness. Ownership admission and non-aliasing of banks are an external invariant, and the model claims no alias detection. Far-pointer inequality is not alias freedom, and uintptr order is not physical order. Arbitrary far addition does not carry across a 64K bank, so bank offsets are always computed in logical arithmetic.

## 3. Validate-before-effect and transaction rule
Before any output or write effect, the operation validates:
- the whole map and its region coverage;
- bank coverage, order and absence of holes, capacities, and writable backing;
- permissions, arithmetic, and the requested span (including the final byte at the 24-bit limit).

A failed write leaves every bank unchanged. A failed read leaves the output unchanged.

The backend and the map/banks must be stable and exclusively owned across preflight and commit.

**Native backend obligation.** A real target backend must guarantee that a preflighted byte or chunk access cannot fail. If it cannot, whole-operation transaction staging is required. This model cannot waive that. A Python bytearray is NOT qualified 286 segmented access. It demonstrates arithmetic and ordering only. The target backend must do bank-offset to far-pointer conversion itself, using qualified compiler behaviour. That behaviour is not assumed here. There is no evidence of qualified far-memory access or of general C type alignment. A whole-map native preflight plus an infallible, stable commit backend remains a prerequisite. No partial write, the physical-alias owner invariant and the unchanged native fast path are preserved. The lifetime registry is an explicit owner context, not a global or a physical-address proof.

## 4. Error precedence
Precedence follows the existing `guest.c` entry points:
- window: access (0, bits outside 7, outside uint32) is ARGUMENT first. Then MAP, then ADDRESS (zero length, at >= 2^24, span past 2^24, no single covering region), then PERMISSION.
- read: null output or missing READ is ARGUMENT first. Then MAP, WIDTH (not 1/2/4), ADDRESS (misaligned word/long). Then window checks the remaining access bits, the span and permission.
- write: MAP, WIDTH, alignment ADDRESS, then window (span and WRITE).

For read and write, a malformed map wins over a bad width, and a bad width wins over alignment.

New errors are model-only and never mix into the list above:
- STALE: the capability's map generation or lifetime no longer matches.
- CAPABILITY: the capability was released or is otherwise unusable.
- EXHAUSTED: the lifetime or generation counter is exhausted.

`cs_guest_result` is NOT extended yet; production enums and `guest.h` are unchanged.

## 5. Checked capability
A capability is opened only after a full-span preflight, before the first chunk is exposed. It carries:
- the map lifetime and generation;
- the region and bank cursor;
- the logical position and remaining length;
- the stored access rights. Descriptors stay (bank id, offset, length); a native backend must use the capability's rights and never infer permission from a bank ID.

Rules:
- `chunk` emits a descriptor (bank id, bank offset, length) with length <= min(32767, bank remaining, span remaining). The cursor advances only on success, so there is no skip, replay or overrun.
- A chunk never crosses a bank boundary. A logical span of 32767/32768/65535/65536/131072 bytes is a sequence of chunks, each <= 32767. The 32767 limit is the pinned C16 PTRDIFF_MAX bound, and `cs_c16_chunk_size` is the narrowing helper.
- A new map lifetime invalidates every older capability. Opening on an obsolete map lifetime returns STALE; no fresh-looking stale capability is created. An owner must successfully advance the generation before changing a map or backing descriptor, under exclusive ownership; that makes existing capabilities STALE. If advancement refuses, no map/backing change is permitted. Ordinary byte-content writes do not change the map generation.
- Lifetime and generation counters never wrap or reuse. At the limit they return EXHAUSTED.
- Release is explicit. There is no automatic refresh, and a released capability returns CAPABILITY.
- No capability is retained indefinitely: it is bounded by length and lifetime.

## 6. Retained-view audit
Audit of call-into-guest retained views is future integration work and is NOT solved here. Existing callers that hold `cs_guest_view` across a callout must be inventoried, then moved to capabilities or per-operation access.

2026-10-25,SRC-0442/0443:the [first five-file inventory](../development/retained-guest-views.md)
distinguishes validation-only/consumed-before-execution spans from Toolbox
views used during or after a trusted native handler. The handler forbids
mutation/reentry/retention;no existing flat-code defect is established.
The full audit remains open. Next proposal:logical range preflight and an
infallible prevalidated4-byte call-stack commit. No C16 API/backend is adopted;
resource/lifecycle/patch/drawing/bus/session internals remain unreviewed here.

## 7. Deferred production API
- No flat-pointer API is proposed for a cross-bank logical span. Capabilities are the separate candidate surface; production integration remains deferred.
- The model is not a security or physical-alias proof. Callers keep opaque capability fields, map and backing truthful and stable; forging Python objects is out of scope.

The alignment query is only numeric offset divisibility (`cs_c16_offset_multiple`). General C alignment is unproven.

## 8. Conformance
`probes/pc286/segmented_model.py` is a deterministic, standard-library-only model with assertions. Its coverage:
- a 128 KiB region over 6 banks, including odd sizes;
- reads and writes around every bank boundary, and big-endian long crossing;
- an invalid later bank with no partial effect;
- final-byte 24-bit limits, adjacent-region refusal, high flag bits;
- stale epoch and exhaustion;
- permission and alignment failures, failed-output snapshots;
- chunk lengths 32767/32768/65535/65536/131072;
- malformed canonical IDs and coverage, extra backing keys, bank types, and full-span preflight before the first chunk;
- uint32 flag/access bounds;
- invalid new maps leaving the registry and older capabilities untouched;
- stale-lifetime open refusal and stored capability rights.

The worker executed no tests. Lead execution and independent checks are recorded in [the evidence](../development/segmented-storage-evidence.md).

## Model preconditions and limits

The caller supplies typed integer addresses/sizes/values, well-shaped region
records, a one-element mutable read-output slot, and an owner-created registry
whose counters start at zero and whose limits are in 0..0xFFFFFFFF. Capabilities
are opaque to consumers; no caller changes their fields or registry counters.
Read access outside uint32 is a model argument refusal with no equivalent C
uint32 input. Other Python type/shape exceptions are outside this typed model.
Direct scalar APIs use the explicitly supplied map, matching guest.c; lifetime
revocation here applies to capabilities, not arbitrary retained Python Map
references. Production caller retention and callout inventories remain open.
Full-span preflight cannot make a later backend failure atomic: native support
still needs an infallible stable commit or whole-operation staging. There is no
native byte/chunk commit API or qualified far-pointer implementation here.


2026-10-25,SRC-0444..0446/IMPL-0076/TEST-0089:adopt the separate [logical geometry/descriptive4-byte plan](../development/pc286-bank-plan-contract.md) prototype. Three host profiles/176 frozen assertions/three compiled defects and six final C16 objects verified. This is no backing admission,capability,commit execution,far backend or production ABI;the native infallible-commit obligation remains open. [Evidence](../development/pc286-bank-plan-evidence.md).


2026-10-25,SRC-0448/IMPL-0077/TEST-0090:adopt the separate [numeric backing-catalog prototype](../development/pc286-bank-catalog-contract.md). It bounds caller-supplied canonical intervals/native offsets,not actual physical ownership/representation or infallible access.32 frozen checks/later reproduced regression/three host profiles/six C16 closure witnesses verified;no native pointer,far conversion,commit,link/target run or production ABI.

## 9. Host-memory store4 experiment (2026-10-25)

SRC-0466/0467 adds an original unbound checked-write experiment with borrowed
<=32767-byte native object spans. Whole map/catalog/span validation and all four
destinations precede the first store. Binding stability,true extents,liveness
and nonaliasing remain caller obligations;the epoch is a numeric staleness
label,not a capability. The [model contract](../development/pc286-store4-model.md)
selects its exact refusal ordering,canonical spans and no-wrap epoch preflight.
It does not adopt a production C16 guest-view ABI,far-memory provider or target
link/boot claim. Existing models and flat production paths remain unchanged.
