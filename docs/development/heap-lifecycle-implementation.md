# Hosted heap-lifecycle implementation

SPEC-0030 v2 / ADR-0034 / IMPL-0034 / TEST-0038; 2026-10-14.
Original engineering profile `classick-hosted-heap-lifecycle-v1`.
Lead self-review; human provenance review remains pending.

`core/memory/lifecycle.c/.h` coordinates an already valid SPEC-0023 manager.
It retires application ownership, preserves system allocations and admitted
memory-resident scrap, and publishes the application zone/limit. It never calls
manager initialization, runs guest instructions or binds a reset/launch trap.
The existing allocator, CPU, session, dispatcher, registry and boot entries are
unchanged. [Executed evidence](heap-lifecycle-evidence.md) is separate from this
API description and from the workers' unexecuted review proposals.

## Host protocol and storage

The trusted host supplies a zeroed controller, existing manager/map, one plan,
request, scrap descriptor, inventory, staging and report. Native objects must be
truthful, aligned, live, synchronized and disjoint; all borrowed objects remain
live through commit/cancel. The controller starts at epoch1 without RAM writes.
Tagged controllers cannot be reinitialized, including after a retained fault.

`cs_life_prepare` validates admission and exact placement before staging any
bytes. Successful preparation seals one owned plan and enters PREPARED. Commit
rechecks the request, descriptor, inventory values, manager configuration and all
records/cells/globals/map descriptors/gateway entries, exact layout and staged
scrap against its live source. A changed input consumes the current plan and
returns STABLE without additional protected writes or epoch advancement. This
does not repair caller mutation. The unchanged oracle is commit-entry state.
Cancel consumes only the current plan. Foreign/replayed requests cannot consume
another controller's live plan. An invalid mandatory native object or unsafe
report is rejected before report writes; a usable output is a caller prerequisite.

Plans and staging are owned immutable protocol state, not serialized untrusted
input. No cryptographic authentication or universal host-tamper detection exists.
In particular, jointly changing a staged byte and its live source violates that
ownership precondition and can evade their equality check. Opaque protected
contents and persistent system payloads remain caller-immutable; no full RAM
snapshot is kept. These limitations do not permit concurrent manager/guest use.

The first destructive boundary consumes the plan and advances a uint32 epoch
exactly once. Subsequent stores use prevalidated views and a bounded recipe:
retirement mask, complete application cell band, two global writes, optional
scrap payload/record/cell and final host descriptor. No new map resolution,
ordinary allocation, callback or notification occurs after that boundary.
Reset uses POPULATION for scrap recreation; teardown uses SCRAP_PRESERVE and
keeps COMMITTING. Successful completion returns STABLE. Unexpected failure
retains effects/staging/first diagnostics and locks the controller FAULTED.
There is no recovery or implicit reset operation in this profile.

`cs_life_admit` is the coordinated manager/session entry gate;
`cs_life_manager_call` applies it to the existing manager. Independent low-level
APIs are not intercepted. A host must enforce the gate and lifetime/context
preconditions before every coordinated entry. A NULL optional session/gateway
means none is retained in this closed world, not permission to omit active state.
No general application execution certificate follows from STABLE.

## Inventory and capacity

At most64 explicit entries have nonzero owners. Kind/action supplies the original
value/lifetime/access class; address/size/value or native span identifies storage.
Required GLOBAL entries name exactly the six existing SPEC-0023 views, width4:

| Address | View | Action |
| --- | --- | --- |
| 0x114 | HeapEnd | Preserve configured application end |
| 0x118 | TheZone | Rebind configured application identity |
| 0x130 | ApplLimit | Rebind preflighted L |
| 0x2A6 | SysZone | Preserve configured system identity |
| 0x2AA | ApplZone | Preserve configured application identity |
| 0x31A | Lo3Bytes | Preserve 0xFFFFFF |

NATIVE/GUEST preservation entries are immutable opaque control/input spans;
they do not admit manager records or retained heap-pointer semantics. Guest spans
inside retiring application storage or the six globals refuse. Native spans,
request/plan/report/staging, session/gateway controls and guest backing buffers
must be disjoint. Existing checked maps require writable backing equal to read
backing; a mirror pointing somewhere else is rejected by SPEC-0022 validation.

All512 supplied gateway slots are validated. The three existing default token
classes retain their configured identities; each additional builtin needs an
explicit owned identity from a separately trusted immutable registry. Never
derive that expected identity from the table being checked. Builtins obey the
existing service token encoding window/alignment. Guest patches, unknown/future
manager records, CLEAR/REFUSE actions and required system scrubs refuse. No RAM
scan for pointer-shaped values or private Macintosh heap reconstruction occurs.

The explicit even stack interval is mapped read/write and disjoint from both
full configured zones and manager globals. `L=min(application.end, reference-S)`
uses checked subtraction. H_min measures gross application payload capacity
before scrap population; separate exact extent/slot checks prove the recreated
scrap fits. Neither S nor H_min has a historical default. Placement uses the
existing original manager strategy: first available record after application
retirement, payload at the top of L. Thirty-two persistent system objects leave
no slot for a live application object in an already valid32-record manager;
no valid33rd-object case is invented.

NONE admits only size0/NIL. RESIDENT requires a matching owned live handle and
logical size. Application scrap stages exactly its logical bytes and recreates
locked/nonpurgeable/nonresource data. Live size0 reserves two physical bytes and
does not dereference a payload; NIL cells refuse. System scrap preserves its
cell flags, descriptor and payload. DISK/UNKNOWN, inconsistent tuples and names
requiring application relocation refuse. No Scrap Manager globals are bound.
Application cell addresses can be reused numerically across epochs; no universal
stale guest-pointer detector is claimed. Discarded payload contents are unspecified.

## Diagnostics and conformance seam

Reports carry cause, operation, phase, prepared/committed flags, before/after
epoch, write counts, scrap fields and actual six globals. `globals_valid` uses
one bit per row above. `scrap_valid` certifies completed preservation;
`new_handle_valid` separately says a host descriptor value was actually published,
including explicit NIL. A post-publication fault can report that value without
claiming STABLE or successful preservation. Invalid/not-reached fields are zero
with their validity tag clear. First-fault diagnostics never silently repair RAM.

Attempted/completed counts are semantic write units, not CPU stores or bytes:
epoch1, each retired five-field record1, each cleared four-byte cell1, each global1,
each copied payload byte1, recreated record1, recreated cell1 and host descriptor1.
The plan's total is `36 + retired records + (application scrap ? size+2 : 0)`.
No native fault/callback is caught. The test-only build injects an internal
invariant failure after each counted unit; production has no injection symbol,
callback or mutable global. Saved session/CPU-stop records remain separate and
unchanged even when later teardown deliberately changes the RAM they described.

## Independent review dispositions

SRC-0152: Claude read only the reviewed contract and original manager/map headers,
proposing12 failure cases and six gaps without executing them. Reject G1's claimed
foreign-plan contradiction: the explicit foreign/replay rule preserves the current
live plan; failed revalidation applies to that plan. G2/G3/G6 are narrow-packet
limitations: SPEC-0023/code already supplies the globals, publicly owned records
and lock encoding. Adopt G4 as the original test-only fault seam and clarify G5's
gross H_min accounting. Phase oracles need actual write boundaries; a valid33rd
object with32 system records cannot be constructed. The tests are independently
authored conformance cases, not executed copies of Claude's proposed cases.

SRC-0153: Grok read the contract and original new code/tests, mapping14 obligations
and reporting five candidate defects. D1/D2 and odd-L concern are rejected after
independent project-code checks: the readonly manager integrity probe validates
cells against record addresses, configured preserved globals and even zone ends.
D3's mirror-write alias is rejected by checked-map identity validation. D4 is an
unauthorized paired change to owned immutable staging/inputs, not a promise of
native authentication; document the limit explicitly. Adopt D5 with changes:
separate actually published handle validity from completed scrap preservation.
Additional original checks cover initialization around live allocations, corrupt
globals/cells/config, mirror backing, report safety on changed maps, retained-fault
manager entry and byte preservation through repeated population. Some proposed
coverage rows depended on omitted existing helpers; no blanket worker verdict
or historical certification is adopted. Raw reports remain outside Git.

No new historical question is answered. HL-Q1..20, AL/LQ/RA/ES/PG and seven-draft
MQ/WQ/SQ/OD questions, native applications, actual loader/manager/QuickDraw/MFS
bindings, callback closure, new VM/emulator/reference runs, UART bodies,
physical/full128K/parity/three editions/B2 criterion6 and human provenance review
remain separate gates. The next bounded runtime task is Resource Manager
service/lifetime design over the existing reader and allocator, before CODE loading.
