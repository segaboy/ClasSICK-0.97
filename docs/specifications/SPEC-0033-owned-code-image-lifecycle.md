# SPEC-0033 - Owned CODE image loading and retirement

Engineering contract v1, 2026-10-15. Profile `classick-owned-code-image-v1`.
SRC-0171..0174 / ADR-0037. Contract only, unimplemented. No TEST/IMPL/build,
fixture, guest binding, application execution or new trial. Human review pending.

## Scope and basis

Define an original host-only eager image owner after SPEC-0032's implemented
read-only preflight. All selected CODE segments are copied into exclusively
owned locked application handles, then the reserved A5 world is populated.
No segment can be evicted individually. Source identity, preparation lifetime,
partial effects and explicit retirement are part of the contract.

Use reviewed own SPEC-0022/0023/0024/0028/0029/0030/0031/0032 and SRC-0130's
independently checked LK decisions. Existing original CPU control decoding and
guest-patch fixtures supply the already supported absolute-long JMP encoding.
No new manual/sample/raw draft, executable, resource, asset or emulator input.
This is engineering policy, not January1984 or AL-2/LQ historical confirmation.

The existing resource cache is a separate owner. This image owner reads the
immutable fork through the existing reader/preflight, without opening that cache,
adopting its tickets, transferring its handles or duplicating resident cache
copies. Every associated SPEC-0031 service must pass its existing CLOSED-only
external gate. Exclusive image handles avoid claiming that shared data leases
are executable ownership; SPEC-0031's limits and behavior remain unchanged.

LOADED means populated owned data, not executable admission. No operation here
starts a session, grants execute permission, updates a map, exposes a callable
token or binds Launch/Chain/LoadSeg/UnloadSeg/GetAppParms/ExitToShell. All results
report execution_authorized=false. Dynamic loading, individual unloading and
purging remain unsupported. A later execution contract must close the fetch,
guest-write, manager, callback and return-lifetime requirements below.

## Native ownership, operations and admission

Conceptual operations are PREPARE, LOAD, CANCEL, DESCRIBE and RELEASE; native
numeric ABI/storage layout is implementation work. One zeroed owner binds once
to an initialized lifecycle/manager/map and explicit positive uint32 maxima for
source length, staged bytes and world bytes, plus a nonzero coordinator owner ID.
Initialization publishes EMPTY without modifying guest/manager state. Tagged
instances cannot be reinitialized, including after FAULTED.
No defaults, heap allocation, mutable process globals, callbacks or concurrency.
At most64 entries/16 distinct segments, including first CODE1, reuse SPEC-0032's
header-inclusive32768 limit, literal offsets, warning and group rules unchanged.

Inputs, owner, workspace, native staging, snapshots, diagnostics, outputs, optional
session/gateway controls, inventories and all borrowed/protected buffers are
truthful/live/aligned/synchronized and disjoint from one another and every guest
backing. Byte buffers have no additional alignment. Bounded native ranges, counts,
aliases and output safety precede any diagnostic, staging or owner write. Check
original binding as well as supplied descriptors/backings on a changed binding.
Native objects are constructor-produced immutable protocol state, not authenticated
untrusted records. No checksum or numerical source identity proves provenance.

Every operation, including CANCEL, requires ordinary inactive context: no root,
callback, patch, other manager operation or pending exit; optional session
independently matches the map and is inactive. Except native-only CANCEL's
integrity escape below, lifecycle must be STABLE and pass actual read-only
integrity. TheZone already selects the application;
current ApplLimit must equal the candidate limit from fresh SPEC-0032 layout.
Refuse a mismatch; an independently completed SPEC-0030 reset may precede PREPARE.
No operation here invokes reset, silently edits limits, changes zone bounds or
weakens the complete-configured-zone stack restriction. No physical128K map is chosen.
All regions intersecting the configured application zone or reserved world/stack
must lack EXEC permission. Also reject physical backing aliases between those
destinations and every executable region, even at unrelated logical addresses.
Bindings/permissions remain sealed through ownership. No existing EXEC alias may
turn copied bytes into reachable instructions. This is a conservative data-only
profile; a future execution bridge needs an explicit reviewed extension.

Before PREPARE may publish retained staging/world/segment ownership, the coordinator
classifies this owner FUTURE/REFUSE in its authoritative inventory and supplies a
truthful matching acknowledgment. It remains so until CANCEL or RELEASE returns
successful EMPTY. Existing associated resource services must be CLOSED and reference
free and bound to this same manager/controller/map; inventory every associated
resource owner rather than substituting another closed service. Fresh inventory
and composite gates apply before every external manager,
reset/teardown or session entry; no low-level API is automatically intercepted.
Only this synchronous owner's defined internal manager calls are admitted while
it is busy. Hiding a live image under NATIVE/PRESERVE or omitting it is invalid.
No live guest/native client retains segment/world pointers in v1.
Other owners and retained references must have an explicitly admitted action;
unknown/unsupported ownership refuses. Existing movable application objects may
retain stable manager handles, but untracked raw payload pointers that allocation
could invalidate refuse. This is closed-world host protocol, not a RAM pointer scan.

Generation starts at1 on first successful PREPARE and increases on each new
PREPARE, never on LOAD retries. At UINT32_MAX another preparation refuses; no
wrap, reuse or reinitialization. Capture the nonzero heap epoch separately; these
are native ownership identities, not universal stale-address detection.
Failed preparation consumes no generation; successful CANCEL does not rewind it.

## Preparation, staging and source lifetime

PREPARE accepts a fresh host-supplied immutable resource fork, B/S/H_min, explicitly
reserved world/stack ranges, workspace and caller-provided staging capacity.
Do not consume a prior unauthenticated preflight report as source authority.
Run actual SPEC-0032 inspection and layout against the current source/map/epoch.
Respect their failure precedence, source diagnostics and selected engineering
offset policy. Validate every destination and logical/native alias before copying.

Stage only CODE0's16 header bytes plus its declared eight-byte entries, followed
by all selected segment logical payloads, including their four-byte headers, in
first-appearance order. Do not stage ignored CODE0 trailing bytes or unrelated
resources. Checked required staging is16+8*entry_count+sum(segment_lengths),
at most524816 bytes by this profile; the caller's actual maximum/capacity may be
smaller. Refuse insufficient staging before any retained ownership publication.
The bound is not a RAM budget. Counts/offsets/lengths remain explicit and checked.

Seal the copied manifest/layout, staged-span relationships, map/config/descriptor/
backing identities, six manager globals,32 records and both128-byte cell bands,
current epoch and all admission/profile inputs. Staged headers/entries must agree
with the copied scalars. Caller holds all sealed storage immutable until consumed
or cancelled. Native staging/workspace may change during failed preparation;
canonical outputs, owner ownership, guest RAM and runtime state remain unchanged.
Publish PREPARED only after all native checks/copies succeed.

The original fork is borrowed only during the synchronous PREPARE call. A
successful PREPARE has copied every selected byte needed by this eager profile
and retains no dereferenceable fork/view/name pointer. Its return ends that source
borrow. A failed PREPARE publishes no retained source/staging lease. Numeric file
offsets/warnings may remain diagnostics. PREPARED owns a distinct immutable native
staging borrow until successful CANCEL, LOAD or PARTIAL RELEASE return. Ordinary
PARTIAL and LOAD fault outcomes retain it.
No claim detects arbitrary caller mutation of opaque staged payload bytes.

Capacity checks count existing system/application/scrap records against the shared
32-slot manager limit. Require free slots >=segment_count and sufficient remaining
application payload bytes below current ApplLimit for the sum of even physical
extents. Count only [application.begin+128,ApplLimit), subtracting all occupied
physical extents in that interval, with checked arithmetic. These are necessary
checks only: locked/immovable fragmentation can still prevent actual placement.
Neither H_min nor a sum of free bytes reserves placement or proves allocation.
Do not reconstruct private Apple heap structures or promise atomic multi-allocation.

## Loading and the irreversible boundary

LOAD requires PREPARED and the matching owner/generation/epoch. Revalidate all
sealed map/config/global/record/cell/input values before any guest effect, and
recheck native64/16 bounds, staging relationships, source-policy scalar invariants,
current layout, all reserved destination spans and composite admission. Changed
inputs/epoch refuse without guest effects and retain PREPARED for native CANCEL;
no refusal certifies the changed manager healthy. Staging immutability is a host
protocol precondition, not a byte-authentication claim. No source lookup occurs.
In particular, layout uses cs_code_check_layout on the sealed copied manifest,
not cs_code_inspect on a freed fork. Recheck the staged CODE0/segment headers and
entries against the seal; metadata validation does not reauthenticate payloads.

For each segment, in first-appearance order, call existing NEW_HANDLE with no
SYS/CLEAR flag through the lifecycle-controlled internal manager path. Validate
the actual returned handle/record/cell/full extent, call existing CS_MM_HLOCK
through cs_life_manager_call, then freshly revalidate
the resulting exact flag byte0x80, application zone, logical size and address.
Copy exactly the staged logical segment bytes; padding is not payload. Before
every later allocation and publication, revalidate all earlier owned records/
cells/locked addresses. Locking prevents later admitted allocation from moving
those segments; other preexisting unlocked application objects may compact.
Do not retain native guest-window pointers across a manager call.

An ordinary memFullErr from the first allocator, with no prior guest effect,
returns CAPACITY and PREPARED, preserving manager/RAM. An unexpected native/
lifecycle/lock/invariant failure is FAULT even if no guest effect is established.
The first successful allocation is the irreversible guest boundary; record its
actual handle/slot/address/extent immediately, before locking/copy/publication.
Ordinary memFullErr on a later allocation returns CAPACITY and PARTIAL, with
all actual earlier handles/effects captured and world_written=0. No world copy
has started at that point. PARTIAL permits DESCRIBE and explicit RELEASE only,
never LOAD continuation/retry or CANCEL. Other unexpected failures enter FAULTED. No automatic
disposal, rollback or claim that compaction never happened. This differs from
SPEC-0030's separately exact-planned no-fallible-allocation commit.
The -108 value here is cs_mm_reply.code in native output, with its own validity
tag; this internal path calls no guest ABI and writes no MemError/CCR/global.
A lock/copy failure after allocation always retains guest_committed=true and its
allocated bit, even when locking/publication did not complete.

After every segment is copied/locked and all records/cells validate, recheck the
world windows, then clear exactly the admitted above/below A5 bytes (skip zero
below). Populate each eight-byte table row as BE16 segment ID, BE16 JMP absolute-
long word0x4EF9 and BE32 target. The target is the owned resource-start address
plus the literal routine offset, even/strict24, with two logical bytes available.
It is an original encoding of the reviewed loaded-entry format and own ISA
support, never copied Apple instructions. It remains data in this profile.
Rows may share an admitted target; no deduplication changes ownership.

The32-byte parameter area and slack stay zero, including QuickDraw's+0 and the
Finder-information-handle slot+16. This profile has no Finder-information object
or application-parameter service. It writes no AppParmHandle, CurrentA5,
CurStackBase, CurJTOffset, filesystem/manager global or CPU register. Zero below-A5
is an explicit engineering choice; it does not prove InitGraf space. No stack
bytes/sentinel/frame are populated and no execute flag is changed.

Revalidate complete copied bytes, owned handles/cells, loaded rows and zeroed
world ranges before LOADED publication. Successful LOAD return ends the staging
borrow and clears dereferenceable staging pointers; loaded scalar metadata keeps
only what ownership/retirement needs. A fault before successful return retains
staging even if LOADED publication was reached internally. No report is a lease
for code execution. Source may already have been freed after PREPARE.

## Loaded ownership, cancellation and release

CODE1 and every other loaded segment remain locked/nonpurgeable for the entire
image lifetime. No individual unload, guest resize/unlock/disposal, direct cell
or payload mutation, independent world write, resource-cache adoption or new
external manager operation is admitted while ownership is nonempty. DESCRIBE
returns copied, generation/epoch-qualified scalar metadata only, never a callable
entry, writable/native guest span or new resource ticket.

CANCEL is native-only and requires a genuine PREPARED owner with no allocated/
disposed/live guest ownership and no world write. Under safe native envelope,
ordinary inactive context and truthful inventory acknowledgment, it can retire
that preparation even when the manager/map/epoch no longer passes LOAD admission.
It performs no guest/manager call or stale-handle lookup, consumes staging/plan
ownership and returns EMPTY. Generation does not rewind. Its success does not
clear an independent lifecycle/resource fault or authorize external execution.

RELEASE accepts LOADED or ordinary-capacity PARTIAL, with ordinary inactive
context, current matching epoch/binding, actual lifecycle integrity and closed
associated resources. Preflight every actually owned handle/record/cell/full
extent and external reference/inventory before any write. For LOADED also verify
the loaded table targets and zeroed world fields; PARTIAL requires world_written=0
and does not expect or clear a populated world. Expected loaded bytes/world are immutable
by host protocol; no arbitrary payload-mutation authentication is claimed after
staging has been released. A detected mismatch returns CONFLICT and preserves
the entry state/ownership/RAM; it does not repair or dispose a reused object.

For LOADED clear exactly the owned world first; for PARTIAL skip all world writes.
Then dispose all and only actually owned image handles
in reverse first-appearance order through existing DISPOS_HANDLE. No guest code
can run between units. This is explicit retirement, not rollback or a heap reset.
System objects/cells, unrelated application objects, scrap descriptors, CPU stop
snapshots and stack bytes remain. Earlier allowed compaction effects remain.
PARTIAL release is cleanup of acquired objects, not restoration of the pre-LOAD
heap layout. Its success releases the still-borrowed staging; source was already
released at PREPARE. No automatic disposal is hidden in a CAPACITY return.
After all disposals and native retirements validate, publish EMPTY and release
the world/stack reservation only on successful return. A failure after any write,
including after final native publication but before success, retains FAULTED,
actual masks and reservation; never report all objects live or all freed by guess.

## States, reports and failure order

| State/event | Outcome |
| --- | --- |
| EMPTY / failed PREPARE | EMPTY, no retained ownership; safe workspace/staging/report may change |
| EMPTY / successful PREPARE | PREPARED; fresh generation, immutable staging/world reservation |
| PREPARED / revalidation refusal or first allocator OOM | PREPARED; no new guest effect; CANCEL remains native-only |
| PREPARED / successful CANCEL | EMPTY, no guest effect or generation rewind |
| PREPARED / successful first allocation | POPULATING, irreversible allocation captured |
| POPULATING / ordinary later allocation OOM | PARTIAL, actual allocations retained, no world write; explicit RELEASE can clean them |
| POPULATING / successful LOAD return | LOADED, staging lease ends, all segments pinned |
| LOADED or PARTIAL / preflight RELEASE refusal | Entry state unchanged; no new guest effect |
| LOADED or PARTIAL / RELEASE first clear/disposal | RETIRING, actual progress recorded |
| RETIRING / successful RELEASE return | EMPTY, image reservations end |
| Unexpected internal fault or other post-boundary failure | FAULTED with first-failure snapshot/effects; ordinary allocation OOM takes the explicit PARTIAL exception |
| FAULTED / any operation | Fresh no-effect FAULT refusal; retained snapshot/borrows stay; no recovery/reinit/retry |

A native-safe DESCRIBE in EMPTY/PREPARED/PARTIAL/LOADED reports only tagged actual current
state/scalars; it never reopens source/staging access. In FAULTED even DESCRIBE
refuses; the already owned first-fault snapshot may be inspected without new leases.
POPULATING/RETIRING are synchronous internal states, never callable/reentrant.

Failure order: mandatory native/output/borrowed protection; known operation and
owned identity/state; ordinary context and acknowledgment; applicable lifecycle/
resource/binding/epoch gates; profile/native bounds; actual source inspection then
layout (PREPARE) or sealed-state/staging/layout revalidation (LOAD); remaining
capacity checks; exact operation work. CANCEL skips manager/resource integrity
calls as specified. Mandatory unsafe-output failure preserves report as well.

Reports distinguish ARGUMENT, STATE, CONTEXT, CONFLICT/CHANGED, inherited source/
layout causes, PROFILE_LIMIT, CAPACITY, COUNTER_EXHAUSTED, IN_USE and FAULT.
Retain actual reader/preflight/lifecycle/native-manager result and reply validity,
operation/phase, generation and captured/observed epoch, attempted/completed units,
first-fault versus fresh-refusal flag and separate guest/native effects. Never
invent D0/CCR, ResError/MemError or historical system errors15/25/26/28.

Each successful allocation/lock/disposal, logical copied/world-written byte,
and native ownership publication/retirement is a completed semantic unit, not
a CPU-atomic or concurrency guarantee. Per-segment allocated/locked/copied/live/
disposed/retired uint16 masks refer to sealed first-appearance slots. Disposed
bits are tombstones even before native retirement; no mask authorizes recovery.
Disposed handles/payload addresses are scalar-only diagnostics, never retained
dereferenceable borrows or reusable leases. Surviving staging/control/world
borrows are distinct from tombstones; raw handle reuse grants no cleanup right.
Track exact world-clear and table-byte progress and actual allocator movement
separately. Zero/not-reached data always has a validity tag. A first-fault snapshot
and all surviving borrows stay live; no FAULT recovery operation exists in v1.
Unexpected FAULT is deliberately terminal for this image/coordinator's manager,
reset and session admission, even if lifecycle independently remains STABLE.
Do not bypass it by dropping the owner, reopening raw APIs or claiming reset is
recovery. Future audited recovery is separate; ordinary capacity PARTIAL is not
that invariant-fault state and has the explicit RELEASE path above.

## Execution prerequisites and unexecuted obligations

This owner's composite external gate accepts EMPTY only. PREPARED/PARTIAL/LOADED/FAULTED
cannot be omitted from lifecycle inventory to start/reset a root. A future
separately reviewed execution bridge must instead prove at least:

- An explicit extension of this profile's non-EXEC destination restriction, then
  authorized ownership for every fetch byte, including extensions, jump entries,
  targets, exception paths and trusted exits; existing map EXEC bits alone do not
  establish it. Exclude segment headers/padding and unowned heap/storage.
- Guest and service write filtering for code, cells, jump table, native controls
  and launch fields; include direct instructions, BlockMove, trap output writes,
  aliases and retained/transitive manager operations. Check physical aliases too.
- No mutation/retirement of an owned handle through any manager/patch/callback
  path; continuous low-memory/global validity before each admitted instruction.
- Entire root and all nested callbacks/patches/completions stopped with depth0,
  no pending return/exit/external reference before retirement. Pinning all segments
  until quiescence avoids guessing return safety from a raw stack scan.
- Explicit entry/frame/stack-token construction, bounded budget/depth, returning
  versus ExitToShell outcomes and retained fault snapshots. No automatic Finder,
  heap teardown, LoadSeg continuation or UnloadSeg support follows.

Future original tests must cover source disappearance after PREPARE, staging
capacity/alias/lifetime/count limits and changed seals; full-zone/current-limit
refusals;32 shared slots and locked-fragmentation first versus later OOM/explicit
PARTIAL release, including unchanged world and retained compaction; EXEC and
physical executable-alias refusal; faults before/
after each allocation/lock/copy/world/table/publication and release unit; actual
masks/tombstones and unchanged refusals; exact loaded bytes/targets/zeroing,
same-target rows, epoch/raw-handle reuse, native-only cancel with changed manager,
retained first fault and external gates in every state. These are unexecuted
obligations: no fixtures, TEST/IMPL/build, runtime or reference trial now.

Stage worst case, controls/manifests,512-byte table metadata, both cell bands,
actual segment extents, persistent system/scrap objects, map/session/lifecycle/
cache state and cumulative stack all count separately. No final128K/ROM/physical
fit or historical compatibility follows from the arithmetic. Both required
System1/ROM boot directions remain untested. All24 originals/copies, prior builds/
edition/native/source/UART/parity/B2/human gates and Git-only wiki queue remain.
