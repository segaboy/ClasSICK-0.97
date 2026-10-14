# SPEC-0031 - Hosted Resource Manager residency foundation

Engineering contract v1, 2026-10-14. Profile `classick-hosted-resource-residency-v1`.
SRC-0155..0158 / ADR-0035. Lead self-review; human provenance review pending.
Contract only, unimplemented. No historical Resource Manager ABI is introduced.

## Basis and scope

Use the implemented SPEC-0022 checked map, SPEC-0023 bounded manager,
SPEC-0024 immutable reader and SPEC-0030 lifecycle gate. Reuse the reviewed
resource-format decisions in SRC-0106 and the launch intake's LK-9/11/17
motivation (SRC-0130). Those source records are provenance, not additional
unstated rules; the behavioral requirements are explicit below. No new manual,
source, sample, binary or observation
informs this engineering contract. RF/LQ/AL and other historical questions stay open.

The first service manages copies of local resources from one host-supplied
immutable fork in application-heap handles. It supplies explicit host leases,
resident metadata and deterministic release/eviction/close behavior. It does
not search multiple files, read disks, resolve bit7 references, interpret CODE,
execute content, initialize a launch, install traps or expose an application SDK.
Host operations below are original concepts, not aliases for GetResource,
LoadResource, ReleaseResource, DetachResource, OpenResFile or ResError.

The existing reader returns file spans, not allocations. A later loader needs
stable resident bytes and a clear owner before turning any bytes into a target.
This service grants data residency only. Even a map with EXEC permission does
not make its returned bytes executable or certify arbitrary application entry.

## Owned state, admission and bounds

One zeroed native service binds once to the same live manager/map and initialized
lifecycle controller. Constructor validation assigns CLOSED without changing
guest RAM or reinitializing the manager. Tagged service objects cannot be
reinitialized, including after FAULTED. Borrowed native objects are truthful,
aligned, live, synchronized and disjoint; native forgery is not authenticated.
Typed native objects require alignment; byte buffers have no extra alignment rule.
Service storage, input/request/report/output objects, source bytes, map descriptors,
guest backing buffers and protected controls must not overlap. A source fork is
outside every guest backing buffer in this hosted profile. It is borrowed,
never modified, and remains live until successful CLOSE; faults retain that borrow.

Each operation requires explicit ordinary context and no active root session,
returning callback, patch or other manager operation. Unknown/interrupt contexts
refuse. Optional supplied session controls must independently prove inactivity.
The lifecycle controller must be STABLE and pass its admission check, be bound to
this manager, and have a nonzero heap epoch. OPEN ownership requires the captured
epoch to match; CLOSED OPEN captures the current epoch, including after reset.
Manager/config/map/global
integrity is freshly checked. TheZone must already select the application zone;
the service does not temporarily change it or other low-memory globals.

The caller maintains closed-world ownership: no independent resize, disposal,
unlock, flag change or payload write of service-owned handles, and no untracked
retained resource pointer. Other admitted manager objects remain independent.
Every operation checks resource records/cells against its ownership metadata:
kind HANDLE, application zone, matching logical size/address/extent, live non-NIL
cell, exact flag byte0x80 and current complete mapped extent. Detected differences
are CONFLICT before new effects, without silent repair. This is not universal
raw-address generation detection or authentication of payload contents.

Failure precedence is fixed: mandatory native/output range and alias safety;
service identity/operation/state (including retained FAULT); ordinary inactive
context/lifecycle admission; epoch/registration/zone and manager/cache integrity;
operation arguments and profile/source bounds; source/key or ticket resolution;
required issuance counters and cache/ticket capacities; then actions. Skip rules
irrelevant to that operation. Cache hits require a free ticket, not a free cache
slot. RELEASE/EVICT/CLOSE issue no IDs and do not fail merely because counters
are saturated. Bounds that would be narrowed are checked before conversion or
reader access. Every refusal preserves protected guest/source ownership; only
safe owned diagnostic/transient fields may change as explicitly described.

Bounds are16 cache entries and32 simultaneous lease tickets, selected original
engineering limits. They do not increase the manager's32 total live-object slots.
Caller profile supplies positive maximum source length and maximum logical
resource length, both uint32, with no historical/default numeric value. Outer
native lengths above UINT32_MAX refuse before narrowing. Resources at/above
0x80000000 refuse the manager size domain; further physical capacity is subject
to the existing manager. Checked rounding includes size0's two-byte reservation.
No sum of logical lengths establishes RAM or placement capacity.

Service counters `last_open_generation` and `last_ticket_id` start0 and never
reset on close. Before issuing a new identifier, UINT32_MAX refuses before effects;
otherwise last+1 may be published, including UINT32_MAX as the final valid value.
Issuance occurs only at successful publication. No wrapping or implicit restart.
Cache reuse is qualified by current service/open generation; ticket identities
are never reused in that service lifetime. Native owner identity distinguishes
instances; these are owned host protocol values, not guest capabilities or pointers
that may be dereferenced as functions.

## States and host operations

States are CLOSED, OPEN, BUSY and FAULTED. OPEN/CLOSED describe ownership and
inactivity, not a certificate that caller-mutated state is healthy. Every new
operation revalidates. Calls are synchronous, nonreentrant and have no callbacks,
guest instructions, host heap allocation or mutable process globals.
BUSY admits only synchronous subordinate manager calls owned by that operation;
external/reentrant coordinated entry refuses. It is not a second public entry gate.

| Operation | Admission and successful result |
| --- | --- |
| OPEN(source bytes, native length) | CLOSED, no live cache/ticket. Accept fresh source arguments on each call; construct one internal SPEC-0024 view and publish a fresh open generation and current heap epoch. No guest allocation. Opening an already OPEN/BUSY/FAULTED instance refuses; no implicit close or use of a prior closed source pointer. |
| ACQUIRE(type, signed ID) | OPEN and valid source/key. A resident cache hit returns the same owned guest handle through a fresh ticket. A miss loads a local entry as below. Select the lowest free cache/ticket slot; never evict implicitly. ID must be in signed16 range. |
| DESCRIBE(ticket) | OPEN. Validate native owner, open generation, heap epoch, live ticket ID and matching cache key/handle. Return raw source metadata, handle, logical length and fresh checked data span. No allocation, refcount change or execution authority. |
| RELEASE(ticket) | OPEN. Validate exactly that live ticket. Retire it once and decrement its cache's lease count; retain the resident handle/cell/payload. Releasing a copied/replayed/foreign/stale ticket refuses without affecting another ticket. |
| EVICT(type, signed ID) | OPEN, resident key with zero live leases and valid ownership. Dispose only that owned handle, then remove the cache entry. Active leases refuse. A nonresident key returns NOT_RESIDENT; it does not search/allocate. |
| CLOSE | OPEN, zero live tickets across the entire cache. Prevalidate every owned resource before the first disposal; dispose cached handles in ascending cache-slot order, then clear source/view/cache ownership and publish CLOSED. No system-object disposal or heap reset. CLOSED close is a no-effect success after valid identity/context admission. |

Ticket descriptors contain owner identity, open generation, captured heap epoch,
ticket ID, cache slot, type, signed ID, handle and logical length. All fields are
matched to current owned records, not trusted merely because a handle is non-NIL.
Ticket outputs must be disjoint from source/control/guest/protected input storage.
Copies of a live ticket are not extra leases; one RELEASE invalidates all copies.
DESCRIBE's native read-only span is borrowed only while that ticket remains live
and ownership/context/map invariants hold. Release/eviction/close ends the relevant
borrow; native clients cannot retain unchecked pointers afterward. Logical size0
returns an explicitly empty logical span without dereferencing payload bytes;
its live handle and two-byte physical reservation are still validated.
Its data result is tagged EMPTY with native byte pointer NULL and logical length0;
do not construct a zero-length SPEC-0022 window. Validate the positive two-byte
physical reservation separately. A valid logical payload address may be reported
as metadata, without exposing the reservation bytes as logical resource content.

Cache identities are `(open generation, type, signed ID)`. Different local keys
that share a file span receive independent copies/handles. Reacquiring an evicted
key can reuse a raw cell/address but requires a fresh ticket and validated cache.
Same-key repeated acquisitions consume tickets, not additional manager objects.
No promised guest GetResource identity or historical reference counting follows.

## Reader results and attributes

OPEN uses the tolerant reader without strengthening its format policy. Requested
key ambiguity refuses without a winner; NOT_FOUND is distinct from bad source.
CS_RF_REFERENCE returns UNRESOLVED_REFERENCE with safe raw metadata and no local
allocation; its ignored offset/specification is never followed as a pointer.
Resources requesting the documented resSysHeap bit0x40 return SYSTEM_HEAP_UNSUPPORTED.
This profile owns application copies only; it never allocates persistent system
resources. The lifecycle's existing FUTURE/REFUSE and system-storage-scrub
boundaries remain; it is not a resolver for file-format system references.

Other raw attributes/names/warnings are metadata. The reviewed later-edition
attribute table is reused from SRC-0106/application-loading; neither edition
proves January1984 behavior. This profile does not implement preload, purge,
protected/dirty writeback or historical file-attribute semantics. It does not
invent a mapping from those file flags to manager flags. Every loaded copy is
locked/nonpurgeable/nonresource (cell flag0x80), regardless of those metadata bits,
as an explicit conservative host choice. No source buffer or on-disk handle field
is changed. A future Resource Manager ABI must separately review those differences.

The reader's acceptance above32K does not establish segment execution support.
This data service accepts admitted lengths under its explicit profile/manager
limits. A later CODE loader still owns its separate32K segment rule and unverified
routine-offset policy. A raw name is a file span, not a native or guest string pointer.

## Miss loading, irreversible effects and diagnostics

Before a miss's first guest effect, validate outputs/aliases, context/controller/
epoch/source/key, slot/ticket/counter/profile limits and all resident ownership.
Perform lookup before allocation. All source bounds and nonoverlap with potential
guest destinations are already established; copy work is bounded by the declared
resource maximum and the reader's finite metadata work. No disk/preload callback.

Request one ordinary application NEW_HANDLE through the admitted manager boundary.
The current original allocator may compact other unlocked application objects
on success. That is permitted, reported through the manager's actual movement
count, and is not described as a no-effect resource operation. Ordinary allocator
failure retains the SPEC-0023 unchanged-state guarantee, returns a distinct
capacity/manager result, and publishes no cache/ticket or new identifiers.
Here ordinary failure means successful manager dispatch with memFullErr. An
unexpected native manager contract/result failure is retained FAULT, even if
it occurs before allocation: report guest effects as uncommitted, reply fields
as unavailable when not written, and no fabricated handle/movement. Fault does
not require a destructive guest boundary to have been crossed. The selected
application allocation compacts within that zone; it does not move system objects.

Once allocation succeeds, guest effects are committed even before a ticket exists.
Freshly validate the actual returned handle/record/cell/complete physical extent,
lock it with the existing manager, and copy exactly the logical file payload.
No further allocation or fallible external work is admitted. New checked-span/
lock failures at this point are unexpected invariant faults, not precommit
refusals or pretend allocation failures. No automatic disposal/rollback attempts
to erase compaction or partial copy effects. Zero size performs no payload access.
Only after ownership, flag and complete copied bytes are valid, publish the
resident cache entry and ticket/identifier accounting. Publication counts are
semantic units, not CPU atomicity or a new concurrency guarantee.

EVICT/CLOSE first validate all requested disposals. A successful disposal is an
irreversible effect. Unexpected failure after that boundary stops immediately,
retains actual completed disposals and diagnostics, and enters FAULTED. It must
not claim all entries remain live, all are closed, or that a retry is harmless.
Predictable close refusal with live tickets changes no guest/source ownership.

Every report separates cause, operation/phase, reader status/field/position/warnings,
manager native result/reply code/movement count, committed flag, actual completed
semantic writes/disposals, open generation/heap epoch and handle/cache/ticket
publication validity. Record an actually allocated/published handle with its
own validity tag even if no complete cache/ticket was published. Empty/not-reached
values have explicit tags; no fabricated success, D0/CCR, ResError global or
historical system-error number. Unsafe mandatory output storage leaves output
unchanged too. Ordinary refusals compare protected state at call entry, not a
rollback to a prior acquisition or to the original file bytes.
Separate persistent native publication from guest effects committed: a cache-hit
ticket or OPEN publication can commit native ownership without any guest write.
Allocator success/disposal sets the guest-effects flag even if later publication
fails. Terminal reports must not conflate those two effect classes.

Causes distinguish ARGUMENT, STATE, CONTEXT, SOURCE, NOT_FOUND, AMBIGUOUS,
UNRESOLVED_REFERENCE, SYSTEM_HEAP_UNSUPPORTED, PROFILE_LIMIT, CACHE_FULL,
TICKETS_FULL, MANAGER_CAPACITY, COUNTER_EXHAUSTED, INVALID_TICKET, NOT_RESIDENT,
IN_USE, CONFLICT and retained FAULT. Numeric ABI values remain implementation
work. OPEN reader failure is SOURCE: no generation or borrow is published,
CLOSED remains, and manager/RAM are unchanged. All reader diagnostics retain
their validity tags. Safe key refusal metadata may include type, signed ID,
reference position, raw attributes, name-present/name span and raw numeric
reference specification; data/handle/cache/ticket validity remains false for an
unresolved reference. Report no invented target, payload address or pointer.

Semantic units are allocator success1 (with its actual separate movement count),
lock1, each copied logical byte1, cache publication/retirement1, ticket
publication/retirement1, each successful disposal1, OPEN publication1 and final
CLOSE ownership publication1. These count completed protocol effects, not every
manager-internal store. RELEASE retires one ticket; native lease-count accounting
belongs to that same unit. First-failure tags distinguish allocation, cache and
ticket publication. A fault after ticket publication retains its issued ID and
actual descriptor even though ACQUIRE did not return success. No later ID reuse.

For EVICT/CLOSE retain uint16 disposed-mask and retired-mask over preflight cache
slots. Each successful disposal records its bit as part of that semantic unit,
then native cache retirement follows and records its separate bit. Stop at the
first failure. An entry with disposed-bit1 is a tombstone in diagnostics even if
its old fields still exist; it cannot be called resident/owned live. Masks refer
to this operation's sealed slot identities, not whatever later shares a raw
handle number. Neither mask nor any fault report authorizes a recovery disposal.

FAULTED retains the first report, source borrow, live control/metadata and actual
manager/RAM effects. No retry, automatic CLOSE, reinitialization, session entry
or recovery is admitted. There is no recovery operation in v1. Reports may be
inspected under their existing storage ownership; a separately contracted future
recovery must decide reconstruction/disposal and source-borrow release.
DESCRIBE/RELEASE/EVICT/CLOSE/OPEN/ACQUIRE all refuse in FAULTED; retained tickets
and prior native spans are quarantined diagnostic values, not usable borrows.
Inspection of an existing saved report creates no new data span or lease.
Source borrowing ends only when CLOSE returns successful CLOSED. A failed close
retains enough prior source identity/view metadata for that borrow even if some
cache/control fields have already been retired; the client must keep the bytes
live. Invariant verification precedes successful source-release notification,
which is the operation's return, not a callback.

## Composition with heap lifecycle and execution

An OPEN/BUSY/FAULTED service is an unsupported retained-reference owner in the
SPEC-0030 inventory: register it as FUTURE/REFUSE even if currently empty. It
cannot masquerade as an opaque preserved NATIVE span while owning resource state.
Reset/teardown therefore refuses before effects. RELEASE is insufficient: it
leaves cached handles. Successful CLOSE removes resource ownership; only the
closed reference-free controls may be admitted as ordinary immutable native
storage, with old numeric diagnostics explicitly non-live.

Registration ordering is mandatory coordinator protocol. Before OPEN can publish
ownership, the caller must provide a truthful acknowledgment that this service
owner is FUTURE/REFUSE in its authoritative admission inventory. It stays so
through BUSY/OPEN and any FAULT. Only after CLOSE returns successful CLOSED may
the coordinator downgrade the reference-free controls to inactive NATIVE
preservation. Before a subsequent OPEN it must restore FUTURE/REFUSE first.
Every lifecycle request uses a fresh snapshot of that authoritative classification,
not a retained closed-state inventory. While lifecycle is PREPARED, the existing
STABLE gate also blocks reopening/mutating those preserved controls. No new global
registry or automatic interception is supplied by this contract; acknowledgments
and fresh inventory remain truthful host protocol inputs. Failed OPEN in CLOSED
can leave conservative refusal classification until the coordinator validates
that no borrow/ownership was published. It cannot silently admit a live owner.

The trusted coordinator enforces the composite gate on every manager/lifecycle/
session entry. Independent APIs are not intercepted. In FAULTED it must keep the
unsupported registration and refuse execution/reset even if the lifecycle
controller independently remains STABLE. Silent omission of that owner is a
caller-precondition violation, not permission to reset through stale resources.
An unexpected heap-epoch change while OPEN is CONFLICT; no stale-handle cleanup
against the new heap is attempted. CLOSED may OPEN again after a valid reset,
capturing the new epoch without resetting its issuance counters.

This is deliberately not a new clear/rebind extension to SPEC-0030. A future
loader may inspect immutable CODE0 via the reader before reset, then open this
service and acquire resident data afterward. That is an ordering requirement,
not an implemented launch transaction. Source lifetime, layout population and
post-reset load failures remain the loader's responsibility.

No operation in v1 is callable while a root is active. Dynamic LoadSeg, code
pinning across guest instructions and returning callbacks need a later admitted
continuation/ownership contract, including filtering guest resize/unlock/dispose
of live executable resources and checking every transitive return path. A locked
cell alone is not a proof that arbitrary guest code cannot alter it. No generic
stack scan, raw-address authenticity or complete re-entry closure is promised.

## Future verification obligations and deferred work

These are unexecuted requirements, not fixtures or TEST/IMPL/build records:

- Initialization/open/close/reopen identities, maximum/zero/wrapped native lengths,
  source/report/control/guest backing aliases and argument-output preservation.
- Reader warnings, missing/ambiguous keys, shared source spans, names, zero/large
  payloads, unresolved references and system-heap admission refusals.
- Same-key cache hits, independent keys,16-cache/32-ticket/32-manager-slot limits,
  zero-size reservation, exact length/physical capacity and counter saturation.
- Byte preservation and successful allocator compaction versus no-effect OOM;
  retained faults after allocation/locking/copy/publication and each close disposal.
- Foreign/replayed/copied tickets, changed owner/map/epoch/record/cell/flags, raw
  handle reuse and revalidation of returned spans without invented authentication.
- Release retains residency; eviction/close refuses live leases; saved CPU/control
  and system bytes remain; composite gate refuses reset/session during open/fault.
- Future original implementation needs a test-only bounded internal fault seam,
  freestanding/import/width/endian checks and exact native storage/frame costs.
  Source buffer, cache/tickets, loaded extents and existing heap/lifecycle/session
  storage all count; no final128K/ROM/cumulative-stack claim follows from this plan.

No MFS, search chain/current resource file, system references/resources, guest
Resource Manager traps, purge/reload/detach/update/writeback, grow-zone callback,
CODE parser/loader/execution, native application, boot change or reference trial.
Existing source/UART/native/physical/128K/edition/B2/human-review gates remain.
