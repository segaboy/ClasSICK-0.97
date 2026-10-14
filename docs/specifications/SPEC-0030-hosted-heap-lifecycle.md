# SPEC-0030 - Hosted application-heap lifecycle

Finalized engineering v1, 2026-10-14; lead self-review, human provenance review
pending. Profile `classick-hosted-heap-lifecycle-v1`; SRC-0146..0148 / ADR-0034.
**Contract only, unimplemented.** This is runtime prerequisite 3 after the
bounded session and typed Toolbox boundary, not a loader or historical reset ABI.

## Basis and boundary

Use SPEC-0022's checked guest map, SPEC-0023's original bounded manager,
SPEC-0028's inactive-session/stop ownership and SPEC-0029's explicit completion
boundary. The reviewed HL-E1..16, RA-E5 and PG-O5 choices supply requirements,
not Apple algorithms. Existing specifications and executable interfaces retain
their behavior. No new primary-source reading, sample, implementation, binary
or reference run informs this contract.

Only a trusted host coordinator may prepare, commit or cancel a reset or teardown.
No active root session, returning callback, patch execution or manager operation
is allowed. Known or unknown interrupt context refuses before effects; caller
supplies explicit ordinary context, not an inference from SR. Guest InitApplZone,
SetApplBase, SetApplLimit, Launch and Chain admission remains unsupported.
Existing ordinary SPEC-0023 calls keep their behavior outside a coordinated
lifecycle operation. No guest argument becomes a native function pointer.

Reset makes a fresh empty application allocation state, optionally recreating
resident scrap. Teardown has the same lifetime transition without application
population. Neither starts a session, loads CODE, creates Finder information,
runs goodBye/close callbacks, purges resources or binds a trap. In this profile
population means scrap recreation only. A later loader owns all other population.
Current A9F4 ExitToShell still ends a root session without teardown.

## Owned state and admission

Caller owns the existing manager/map/config, one lifecycle controller, immutable
request and inventory, bounded native plan, staging bytes and reports. These
objects/buffers are truthful, synchronized, live and mutually disjoint; no host
heap allocation, mutable process global, concurrency or guest execution occurs.
Plan/report/staging must not alias guest live data, either zone/cell band, gateway
storage, control/stack/token storage, input bytes or saved session reports.
The map and manager must already pass SPEC-0023's complete integrity checks.
An uninitialized or corrupt manager is refused, not silently reinitialized.

Native lifecycle state is STABLE, PREPARED, COMMITTING, POPULATING or FAULTED.
Initialization around an existing valid manager assigns epoch 1 without clearing
its records or guest RAM. One prepared plan belongs to one controller/manager;
it cannot be replayed, applied to another instance or consumed twice. While
PREPARED, the caller permits no manager/guest/gateway mutation. Commit revalidates
the entire relevant configuration, input/global/cell/record values and plan
capacity before the first destructive write. Changed inputs refuse and require
fresh preparation. Cancel before commit changes only owned plan/staging/report.

Epoch is unsigned 32-bit native metadata. A reset or teardown advances it exactly
once at its destructive boundary; epoch 0 is never published. At 0xFFFFFFFF,
preflight refuses before staging. There is no wrapping or implicit reinitialization.
An admitted manager identity is not an epoch-tagged guest address: reused raw
cell/payload numbers may identify new objects. No universal stale-pointer detector
or whole-memory generation tag is claimed.

## Reference/global inventory and continuous validity

An original closed-world inventory has at most 64 entries, owned by the host.
Each entry names an owner, native/guest storage span, width and access permissions,
value class, zone/lifetime, mutability, current validity, action and dependencies.
Actions are PRESERVE, CLEAR, REBIND or REFUSE. Span arithmetic, widths, alignments,
aliasing and permitted values must be preflighted, including transitive owned
references. The bounded plan records every actual write. It does not scan RAM
for pointer-shaped values or reconstruct private Macintosh records.

| Existing/possible state | v1 action |
| --- | --- |
| SPEC-0023 application records and full 128-byte application cell band | Retire all application records; clear the whole owned application cell band, then publish only recreated scrap's new record/cell. |
| System records, payloads and 128-byte system cell band | Preserve byte-for-byte. v1 admits no system allocation/disposal, movement, callback or system-storage scrub. If a required scrub would touch them, REFUSE. |
| TheZone | Publish the configured application identity after retirement. |
| ApplLimit | Publish the newly preflighted even limit; do not change fixed HeapEnd into it. |
| HeapEnd, SysZone, ApplZone, Lo3Bytes | Validate configured values and preserve. No zone expansion or physical layout change. |
| Owned gateway entries and their immutable builtin identities | Preserve only explicitly inventoried owned entries. Any installed guest patch or unknown retained code path REFUSES v1; no guessed patch reset or transitive callback safety. |
| Separately owned SPEC-0028/0029 controls, reports, stacks and tokens | Preserve; none may be inside retiring heaps or staging/report destinations. A saved stop frame remains unchanged. |
| Host scrap descriptor | Stage and preserve/rebind as below. This is original coordinator metadata, not a newly bound Scrap Manager global. |
| AppParmHandle, old A5 parameter slot, application identity/A5/jump-table references and future manager records | Unsupported registrations REFUSE v1. No current population creates them. A later contract must define their exact CLEAR/REBIND spans, including the old +16 slot, before admitting them. |

The reviewed package inventory is a coverage checklist, not 31 supported fields:
DoubleTime, CaretTime, TEScrpHandle, TEScrpLength, DlgFont, ACount, ANumber,
ScrHRes, ScrVRes, ApplLimit, SysZone, ApplZone, GZRootHnd, MemTop, AppParmHandle,
SysEvtMask, EventQueue, FSQHdr, VCBQHdr, DefVCBPtr, DrvQHdr, PrintErr, CurPitch,
SdVolume, VBLQueue, Time, SysParam, ROMBase, CurrentA5, UTableBase and SFSaveDisk.
Except the existing manager fields above, v1 defines no addresses/widths or guest
bindings for this list. A later admission must define each exposed field's owner,
width, mutability, epoch and validity and preserve coherence before every guest
instruction, including direct library accesses. Blanket zeroing is forbidden.

STABLE after reset certifies the bounded manager state only, not that arbitrary
Macintosh application code may run. Any later consumer must prove unsupported
global/record views and re-entry paths inaccessible or refuse execution; a broad
mapped RAM span does not itself provide that proof. Future ROMBase/version views,
Restart, queues, ports/windows/menus/TextEdit hooks and system resource/driver
references need separate contracts. No MemError global is invented. Generalized
system scrubbing, closure and notifications remain outside v1's no-callback commit.

## Explicit limit and capacity profile

The caller supplies an even ordinary stack interval, an even host stack reference
within its closed endpoint range, explicit even nonnegative reserve S and explicit
even nonnegative minimum payload capacity H_min. None has a historical default.
S is immutable host profile input, not a read of guest DefltStack/MinStack. This
chooses that authority for this engineering profile without closing HL-Q2/17.

Compute L = min(configured application end, host stack reference - S), using
checked arithmetic. Require L even, at or above the configured application's
payload start, no overlap with the full independently reserved stack, and
L - payload start >= H_min. The stack/configured zones remain disjoint; do not
cut live system objects or change the session's stack bounds. Retiring application
objects need not fit under L because they are discarded, not resized. v1 does
not change SPEC-0023's rule for ordinary direct limit changes around live objects.
No numeric BufPtr, B, H_min, 8K default, private overhead or final 128K budget is
selected. The formula is an original host policy, not a historical initializer.

Preflight computes exact post-retirement placement and record/cell assignment for
every recreated object under SPEC-0023's original strategy. At most 32 live
objects across both zones remains the limit; persistent system objects consume
those slots. Include even rounding and the two-byte physical reservation for a
logical zero-byte block, staging size, plan/inventory/report and native metadata.
Do not accept a sum-of-logical-sizes estimate as an allocation proof. No ordinary
fallible allocation or compaction call is permitted after commit begins.

## Memory-only scrap matrix

The host supplies a snapshot descriptor: signed logical size, residency tag
(NONE, RESIDENT or DISK/UNKNOWN), optional logical handle, and optional name view.
These are original typed coordinator inputs. A historical raw ScrapState is not
silently converted to these tags: zero alone does not mean disk-only. If a future
adapter supplies guest ScrapHandle/size/state, its own contract must freshly read
authoritative values and establish the same predicates without guessing.

| Size / residency / handle | v1 result |
| --- | --- |
| Negative, unknown/inconsistent residency, foreign/odd/forged handle, corrupt cell or mismatched manager logical size | Precommit refusal. |
| 0 / NONE / NIL | No scrap object; preserve explicit absence. A name view is independently checked. |
| Positive / NONE / NIL, or any DISK/UNKNOWN content | Refuse unavailable content; no silent empty conversion or disk readback. |
| Nonnegative / RESIDENT / NIL handle or externally NIL payload cell | Refuse. A NIL cell is not a live zero-size allocation. |
| 0 / RESIDENT / live application handle with matching logical size | Stage descriptor; recreate a live zero-size application handle with its two-byte reservation. |
| Positive / RESIDENT / live application handle with matching size | Stage all logical bytes; recreate a handle and exact bytes. |
| Nonnegative / RESIDENT / live system handle with matching size | Preserve that owned system object, its handle, cell and bytes; no application recreation. |
| NONE with non-NIL handle, nonzero unavailable size or inconsistent tuple | Refuse, rather than infer discarded content. |

Application scrap recreation chooses a locked, nonpurgeable, nonresource cell
(flag byte 0x80); this is an engineering choice. No old cell/handle identity is
promised, although numerical reuse is possible. System scrap preserves its flags.
Discarded payload bytes are unspecified and receive no deterministic fill.

A name view, if present, supplies an explicit complete immutable bounded span;
v1 preserves it only outside the retired application zone/cell band and every
planned write. A name or retained interior pointer requiring relocation REFUSES
v1; future relocation needs a typed contract. Staging consists of exactly the
application scrap's logical bytes and native planning data; it cannot share the
old source or any final destination. Zero size requires no payload dereference.

## Phases, failures and reporting

1. **PREFLIGHT:** validate every input, current state, inventory, map/span, report,
   source/destination, counter, size, layout and capacity. No staging write.
   All predictable admission failures must occur here. Invalid mandatory native
   pointers or an unwritable/aliasing report leave the report unchanged too.
2. **STAGING:** copy admitted application scrap to disjoint owned staging and seal
   the plan; retain precommit inputs until revalidation. Failure/cancel may leave
   staging/plan/report bytes changed, but manager, RAM, gateway, globals, session
   state and epoch unchanged. Release of staging does not release guest objects.
3. **COMMIT:** after full revalidation, enter the first destructive boundary by
   advancing the native epoch and retiring application ownership. Clear the
   application cell band and perform the exact preplanned global/metadata writes.
   Exclude callbacks, guest instructions, notifications, resource closure,
   ordinary allocation and new fallible map/placement checks. Do not call generic
   manager initialization, which would clear the persistent system band too.
4. **POPULATION:** reset recreates only the preplanned application scrap record,
   payload/cell and host descriptor. For teardown, the same necessary recreation
   belongs to COMMIT's SCRAP_PRESERVE subphase and POPULATION is skipped.
   Publish a non-NIL rebound host scrap handle only after the complete payload
   and cell are valid. System scrap and explicit absence require no allocation.
5. **STABLE:** verify internal resulting-state invariants before handing
   control back; this is not a new fallible admission/allocation check. Unexpected
   invariant failure follows the retained-fault rule. This is not automatic
   session entry or application readiness.

For a post-boundary internal invariant failure, retain actual completed changes;
no rollback, automatic teardown or resumed session. Mark FAULTED with phase and
first cause, preserve staging and the first failure report, and refuse further
reset/teardown/manager/session entry through this coordinator. v1 has no recovery
operation: do not turn corrupt manager state or incomplete scrap into a fresh
initialization. Explicit recovery or reconstruction needs a later contract.
Cancellation is possible only before commit; interruption during commit is a
retained fault, not a no-effect cancellation. Independent low-level APIs are not
magically intercepted: their trusted caller must enforce the coordinator's gate.

Faults may leave only a partial cell/global/scrap recreation. Report no fabricated
new handle, stable state, complete preserved scrap or successful population.
Preflight/refusal and successful teardown must not alter a separately owned saved
CPU stop frame. Later deliberate teardown changes RAM that the frame described;
the frame is never described as an immutable heap/RAM snapshot.

Reports distinguish operation, phase/subphase, cause, prepared/committed flags,
before/after epoch, attempted/completed write counts, old/new scrap handle and
size, actual global values and retained fault state. Fields carry explicit valid,
unavailable or not-reached tags. Revalidation refusal has committed=false and the
old epoch. A post-boundary fault has committed=true and records actual effects.
Diagnostics are original host results; no historical system-error number or D0/
CCR mapping is assigned. A repeated successful teardown can advance epoch,
recreate scrap, change its handle/cells and publish a new limit, not merely change
epoch. It preserves admitted scrap content, not object identity.

## Verification obligations and deferred work

This document defines acceptance obligations, not executed cases or new TEST/IMPL
records. Preserve the independently authored original HT-A..F and RA-S1..22 as
unexecuted planning input. A separately authorized implementation must cover:

- Complete scrap matrix, live-zero versus NIL-cell distinction, staged byte
  preservation, physical rounding, persistent system scrap and name-view refusal.
- Exact placement/slot/limit boundaries, counter saturation, report/staging/input/
  stack/control aliases and all precommit unchanged-state guarantees.
- Changed inputs between preparation/commit, cross-instance/replayed plans,
  cancellation, active/interrupt refusal and unsupported reference registrations.
- Partial commit versus population faults, truthful unavailable report fields,
  retained staging and blocking fault state; no implied repair or rollback.
- Repeated teardown, system-band/payload equality, application-cell retirement,
  reused raw-address aliases, unchanged CPU stop snapshots and later changed RAM.
- Existing manager/session/dispatcher regression checks; explicit native plan,
  staging and inventory RAM/stack accounting. Compiler evidence alone cannot prove
  final ROM/128K fit or reference behavior.

HL-E9 segment flags/purging, HL-E14 Finder population, goodBye/driver/resource
closure and guest resets remain later contracts, with no new trap/registry/boot
binding. HL-Q1..20; AL-2/4/5; LQ-1..15; RA-Q1..14; ES-G1..11; PG-Q1..9 and
earlier callback/QuickDraw/SPEC-0017 questions retain their historical status.
SetApplLimit memFullErr and locked HPurge keep their 1985-only annotations;
three A0/A1 edition differences remain unresolved. Native applications, protected
inputs, new VM/emulator/reference runs, UART body work, physical/full OS/parity/
edition/B2 criterion 6 gates and human provenance review remain separate.
