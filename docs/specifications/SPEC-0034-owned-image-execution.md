# SPEC-0034 - Controlled owned-image execution

Engineering contract v1, 2026-10-15; `classick-owned-image-execution-v1`.
SRC-0179..0182 / ADR-0038. Finalized, **unimplemented**; human review pending.
No code, fixtures, TEST/IMPL/build, application execution or new trial is authorized
or recorded here. [Lead review](../development/owned-image-execution-review.md).

## 1. Narrow profile

Extend SPEC-0033 with one explicitly admitted, single-use execution lease for one
LOADED image, one ordinary supervisor root, depth1 and budget1..1000000. Clean
completion is balanced RTS only. Defer callbacks, patches, interrupts, re-entry,
all guest traps/services, MM operations, dynamic LoadSeg/UnloadSeg and Finder.
Strict24 only; DATA24 is deferred. CPU instruction coverage stays unchanged.
This is original engineering, not historical startup or application compatibility.

ExitToShell is deliberately deferred: SPEC-0029 already implements typed root-only
A9F4 through a trusted driver, without handler/auto-pop. It lacks this new guarded
execution boundary. Refusal here is a profile choice, not missing existing support.

## 2. Required extensions, not existing authority

| Dependency | Required future behavior |
| --- | --- |
| X-1, first | Guard actual opcode/extension/data accesses, instruction class, proposed control state and staged commit inside the existing bounded bus transaction |
| X-2 | Bind that policy through session init, entry, initial token push, RTS validation and stop; no unguarded default/trusted executor substitution |
| X-3 | Explicit bridge-aware SPEC33 admission, exclusive participant/lease inventory and single-use claim; retain original EMPTY-only external gate for every other operation |
| X-4 | Give only this guarded path effective fetch permission over owned ranges despite base non-EXEC; preserve original map descriptors/backings/flags |
| X-5 | Explicit quiescent world-scrub/retirement handback; do not weaken ordinary SPEC33 RELEASE |

Current bus/session APIs cannot supply these guarantees. No boolean acknowledgment,
raw PC, map EXEC bit or copied description grants a lease. Do not flip an entire
heap executable or squeeze16 segments into an eight-region overlay. Guarded fetch
requires underlying READ plus precise X-4 authority; ordinary map EXEC access is
unchanged. No second decoder predicts effects; the actual interpreter supplies
access/operation/proposed-state events before commit. Guards have no guest/service
effects and cannot re-enter or receive guest-selected native function pointers.

## 3. Ownership and bounded access policy

Bind the actual image/controller/manager/map/session/gateway, generation, nonzero
heap epoch, fixed profile, instruction budget, inventory, controls and all backing
identities. Policy spans derive only from constructor-owned manifest/layout and
fresh record/cell/extent validation, not arbitrary caller-supplied allowlists.
Revalidate64 entries/16 segments, all logical lengths and native counts before
indexing; policy construction has checked explicit caller storage/capacity.

| Guest span | Fetch | Read | Write |
| --- | --- | --- | --- |
| Segment `[address+4,address+logical_length)` | Opcode and extensions | Yes | No |
| Four-byte segment headers and physical extent padding | No | No | No |
| Each eight-byte jump row | Opcode only at row+2; its four extension bytes only for that row's JMP | Yes | No |
| World below A5 | No | Yes | Yes |
| Entire above-A5 world: parameters, table, gaps/slack | Only row JMP as above | Yes | No |
| Reserved stack | No | Yes | Bounded explicit/implicit accesses |
| Owned return tokens | Intercepted, never executed | No guest data access | No |
| Cells, low memory, system/scrap/native controls and other storage | No | No | No |

Logical size excludes even-round allocation padding. Require every byte of each
access, including a four-byte extension, to fit an admitted range and one inherited
map window without wrap. A jump-row segmentID is data; entering row+4 or row+6 as
an opcode refuses. A row instruction cannot fetch into another row. Full decoded
instruction bytes must remain within their originating payload or exact JMP slot.
Targets must authorize at least an opcode word before any control/stack commit.

Resolve physical as well as logical spans. Conservatively reject overlapping
backings of distinct map regions, native-control/backing overlaps and aliases
between mutable/protected roles at admission; revalidate their identities during
execution. An unowned logical alias grants nothing. No opaque payload hashing,
Apple private heap reconstruction or universal stale-address authentication is
claimed. Caller guarantees truthful live synchronized storage, immutable code and
complete reference/coordinator coverage; there are no hidden writable spans.

## 4. Fresh admission and initial state

Failure order: mandatory native bounds/alignment/output and original/current
backing protection; known owner/state; ordinary inactive context/registration;
actual lifecycle/image/cache/binding/epoch integrity; policy/layout/budget/entry/
stack/token checks; then the execution claim. Unsafe native refusal preserves
reports too; other pre-entry refusals may change diagnostics only.

Require actual LOADED ownership, all image handles pinned/exclusive, same runtime,
STABLE lifecycle/MM integrity and CLOSED/reference-free associated caches. Check
loaded rows, all protected zero world fields and current globals/record/cell state.
No pending exit/root/callback/patch/manager operation or prior execution lease.
Before every instruction, revalidate binding, protected globals/ownership and the
lease; caller statements alone do not certify a healthy manager.

First root enters **first row+2**, whose segmentID is1 and whose recomputed literal
resource-start target matches the owned logical payload. No arbitrary entry/native
pointer. A5=layout A5, A7=reserved stack top, D0..D7/A0..A4/A6=0, SR=0x2700.
These are engineering choices; QD+0/Finder+16 remain zero and are not services.
Construct a separate native starting frame; preserve the caller frame on all paths.

The four immutable SPEC26 return tokens require their own nonaliasing RX storage
already present in the original map **before** image initialization/preparation.
Do not append a token region after LOAD. Trusted control-plane validation can
inspect tokens, cells and globals; it does not grant guest data permissions.
Preflight the selected token's guarded four-byte stack push and all session checks.

A new image participant/lease API must explicitly reconcile the sealed inventory
with bridge registration and NATIVE-to-FUTURE/REFUSE transitions. Current SPEC33
rejects unknown FUTURE owners and changed inventory. Do not forge its seal, omit
owners or relabel active borrows as reference-free. Associate exactly one bridge
identity per image generation; a second bridge or reinitialization cannot bypass
single-use. X-3 must validate legal transitions and the same complete inventory.

## 5. Claim, instruction and stop boundaries

| Bridge state/event | Permitted transition |
| --- | --- |
| READY, ordinary pre-entry refusal | READY; no claim or guest effect |
| All checks pass | CLAIMED; consume this generation's run authority before any token write |
| Guarded initial token push | RUNNING; first guest effect, retained even with zero executed instructions |
| Balanced root return | STOPPED-CLEAN after verified quiescence |
| Budget/CPU/unsupported/policy failure, integrity still proven | STOPPED-GUEST-FAULT; retain first stop, no resume |
| Unexpected invariant/publication/guard failure or lost ownership | QUARANTINED; retain first invariant fault and reservations; no run, scrub, reset recovery or release |
| Explicit quiescent handback/retirement | RETIRED; no executable authority remains |

These are new bridge states, not edits to the current image enum. Keep the image
owned and pinned throughout. Existing image/session/life/cache APIs are not
automatically intercepted: every coordinator entry must enforce the new composite
gate, including direct manager/reset/retirement attempts. No active entry bypass.

Every opcode attempt charges the existing session budget, including a denied trap.
Deny A-line at actual dispatch **before** gateway/OS/typed handler entry; a null
options.dispatch is insufficient because the legacy bus has another A-line path.
F-line/TRAP/exception dispatch and every other service path are also unavailable.
Future coverage cannot silently open these paths. BlockMove/service-output/native
helper writes require the same mediation or explicit refusal in a later profile.

Preserve SPEC22 staged-instruction atomicity: check all operands/extensions, target,
A7 bounds and proposed SR before any staged write or frame publication. Keep
S=1/T=0/I=7 and even A7 within the reserved window, including register-only A7
changes. A rejected instruction publishes no memory/register/gateway effect;
retain effects only from earlier completed instructions. Do not execute writes
then attempt a whole-session rollback. Guard/proposed-state checks after a
fallible service are not a substitute for pre-service refusal.

Balanced root RTS is the sole token-target exception: validate the selected token,
actual preceding successful RTS and exact incoming top A7; intercept completion
before token opcode fetch. Branch/JMP/JSR or malformed RTS to any token faults.
No guest frame scan or claim of authenticating arbitrary subroutine history.
CPU/policy/integrity failure precedes completion, including the last budget attempt.
Unexpected EXITED is an invariant fault, not accepted ExitToShell.

## 6. Reports, quiescence and explicit retirement

Report actual entered/token-written/claim validity, generation/epoch, attempted
instructions, stop PC/frame, CPU/memory/policy cause, access kind/width/address,
physical conflict and precise state/effects. No inherited value is valid without
its tag. Preserve completed stack/global effects and the native stop snapshot.
First stop and first invariant-fault snapshots are distinct and immutable; later
refusals grant no fresh spans/authority. Caller frame restoration stays complete.

STOPPED requires root/service depth0, no active callback/patch/driver/native call
and no pending exit. Session unwind alone does not prove ownership integrity;
verify it separately. Any failure to establish quiescence quarantines the lease.
No automatic resume, second root, heap teardown or cleanup.

X-5 accepts an explicit STOPPED-CLEAN or STOPPED-GUEST-FAULT retirement only when
integrity and quiescence still hold. Validate every actual owned object, protected
row/above-A5 byte and all references **before** writes. Do not invent a retained
LOADED payload digest: SPEC33 has none after staging ends. Code immutability is
established by the admitted mediation plus the stated host protocol, not arbitrary
post-hoc authentication. QUARANTINED/image-FAULTED cases have no recovery here.

Scrub exactly the guest-writable below-A5 range to its canonical zero state;
preserve stack bytes and native stop snapshots. With no admitted execution/
external references remaining, X-3/X-5 explicitly hand the image back to its
original inventory and validated canonical state, then use ordinary SPEC33 RELEASE
for world clear/reverse disposal. Do not patch the inventory while active or call
RELEASE on a dirty world. Retain actual scrub/disposal/retirement counts/masks;
failure after any irreversible/native publication retains quarantine/reservations,
never a guessed EMPTY state. No automatic cleanup on root stop.

## 7. First implementation and unexecuted obligations

Implement X-1 first, as a separately approved bounded core slice: access kinds,
strict spans/aliases, class refusal and staged control/commit enforcement. Then
X-2 session plumbing, X-3/X-4 image authority and X-5 handback. Existing execution
APIs/gates stay unchanged until each dependency is implemented and verified.

Future original tests must cover opcode/extension/target header/padding/row+0/+4/
+6 and cross-row refusals; exact64/16/native capacity; logical/physical aliases;
register-only A7/SR escape; implicit stack accesses and late-operand transaction
failure; A-line denial before all dispatch paths; missing/predeclared tokens;
claim/token/publication faults; budget edge and balanced/forged token return;
single-use/new-bridge attempts; continuous globals/epoch/cells; every busy/root/
callback/patch/pending-exit gate; clean versus guest-fault versus invariant stops;
dirty-world scrub, preserved stack/snapshot, seal-aware inventory handback and
partial retirement faults. These are obligations, not fixtures or executed tests.

All24 original drafts/copies, prior compiled/build/edition/UART records, human
review and historical/native/source/physical/full128K/parity/B2 gates persist.
R-001, AL-2/LQ and earlier questions stay open. Both required System1/ROM boot
directions remain untested. No Apple/Microsoft implementation, manual sample,
raw research or emulator source; no new chats/agents/automation/OneNote.
