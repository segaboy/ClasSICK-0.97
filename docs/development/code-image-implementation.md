# Original owned CODE image implementation - 2026-10-15

SPEC-0033 / ADR-0037 / IMPL-0037 / TEST-0041 / SRC-0175..0178.
[Executed evidence](code-image-evidence.md).

`personality/system1/code-image.h/.c` defines the host ABI: init, execute and
external_admit; five operations PREPARE/LOAD/CANCEL/DESCRIBE/RELEASE. Numeric
result/state/phase values and native records are in the header. No guest trap,
SDK, allocator, instruction runner, process mutable global or callback is added.
The configured data-only profile retains the reviewed64-entry/16-segment bounds.
An additional explicit implementation limit allows8 associated resource owners.
maximum_world bounds the entire reserved stack plus A5 world, not only globals.
These are ClasSICK engineering limits, not Macintosh historical facts.

Native caller storage is truthful, live, aligned, synchronized and disjoint.
Opaque immutability, complete inventories and absence of hidden raw payload
pointers are host preconditions, not authenticated memory or pointer discovery.
Mandatory envelope checks protect output/scratch/input/native buffers against
original/current map backings, manager/lifecycle/session and pending lifecycle
plan inputs, inventoried native storage and resource controls/source borrows.
Unsafe envelope refusal preserves report too. Canonical description changes only
on success; safe scratch/report/staging may change on an ordinary refusal.

One image binds once to the same map/descriptors/backing identities, manager,
lifecycle, fixed zone configuration and positive native profile/owner identity.
Every operation is ordinary/inactive. All but native-only CANCEL require actual
STABLE lifecycle/MM integrity and binding/current application-zone/limit gates.
The authoritative inventory must include this exact FUTURE/REFUSE owner before
PREPARE, throughout all ownership, until successful EMPTY. All associated caches
must be CLOSED/reference-free and individually inventoried on this same runtime.
Supported other entries are known GLOBAL/PRESERVE views, truthful NATIVE/PRESERVE
controls without raw heap payload references and stable GUEST/PRESERVE spans
outside both configured heaps and the reserved stack/world. Physical aliases to
those destinations refuse too; native registrations cannot hide guest backings.
All other owner kinds/actions refuse. Completeness cannot be inferred from RAM.
The coordinator enforces fresh composite gates before every external entry;
independent low-level APIs do not automatically consult this image owner.

PREPARE runs fresh existing inspection/layout, checks capacity, stages only the
selected CODE0 header/rows and complete selected segments, then publishes its
sealed manifest/layout/MM records/globals/cells/inventory/session. Source input
ends at return; the owner stores no source/view/name pointer. Workspace source
pointers are cleared after inspection too. Non-PREPARE requests clear source,
length, staging and staging_capacity; only the owner retains the staging borrow.
LOAD repeats the sealed B/S/H_min, compares the complete newly checked layout,
all seals and staged metadata without source lookup. Capacity counts the shared
32 slots and occupied physical extents below current ApplLimit; it does not
reserve placement or guarantee allocation through immovable fragmentation.

NEW_HANDLE uses flags0 through lifecycle; capture its actual slot/handle/address/
extent immediately, HLOCK through lifecycle, validate exact0x80 cell flags, copy
logical bytes only. Fresh validation of earlier owned entries precedes later
allocations/publication; no borrowed guest window crosses a manager call.
Preexisting unlocked unrelated handles may compact. A native reply -108 is tagged
separately from native result/lifecycle status and writes no guest MemError/CCR.
First ordinary OOM is a no-effect PREPARED refusal. Later ordinary OOM retains
PARTIAL, staging and actual acquired handles; no world has been written. PARTIAL
allows DESCRIBE/RELEASE, never continuation/CANCEL. Cleanup does not undo compaction.
Unexpected failures retain FAULTED, even before the first guest effect.

All configured application/world/stack-intersecting regions lack EXEC; executable
physical aliases refuse. The implementation conservatively checks whole backing
regions for such EXEC aliases and also excludes application/system/global physical
aliases. Each admitted segment stays pinned. After all copies, zero exactly the
above/below-A5 bytes, write BE segmentID/JMP0x4EF9/owned-resource-start+literalOffset
rows and revalidate owned cells, bytes and world before LOADED publication. No
stack/frame/sentinel, launch low-memory global or CPU register is written.
All32 parameter bytes, QD+0, Finder+16 and slack are zero except actual table rows.
All execution_authorized fields remain0; addresses in descriptions are scalar
diagnostics, never executable or writable leases. Successful LOAD ends staging.

RELEASE first validates all actual live ownership and LOADED table/zero fields;
PARTIAL must have zero world writes and skips clearing it. LOADED clears its world,
then both dispose only owned handles in reverse segment order. Check actual
zeroed records/cells before native retirement. No payload scrub, compaction or
heap reset. System cells/objects, unrelated application objects, scrap, CPU stop
snapshots, stack and earlier compaction survive. Successful retirement reports
keep the retired generation/epoch and actual masks while EMPTY descriptions have
no live identity. live/disposed/retired are separate, including last-publication
faults. Failed cleanup retains reservations/borrows and its immutable first fault;
no retry, stale-handle disposal, hidden rollback or reset recovery exists.

CANCEL performs no guest/MM lookup or integrity call. Under a safe native envelope,
inactive context and truthful acknowledgment it retires a genuine allocation-free
PREPARED plan even with changed manager/map/epoch. It does not clear an independent
lifecycle/cache fault. Generation starts1, increases only successful PREPARE and
saturates rather than wrapping. The external gate accepts EMPTY only; all remaining
coordinator/lifecycle/resource gates still apply, including terminal image faults.

## Lead disposition of bounded advisory reviews

Each worker reads only a frozen reviewed-public/original packet, no other files,
commands/edits/tests/web/MCP/subagents. Claude's first Read is truncated at its
reader limit; its partial review covers admission/helpers only. A separately
shown20,001-byte follow-up packet covers the complete remaining implementation
with reviewed requirements/dependency summaries. Both use one Read/two turns;
Grok uses one completed read_file/two turns. Reports/usage are audited separately
and included in evidence. Claude reports exceed600/500-word requests; Grok's598
words fit600. These are advisory reviews, not test execution or certification.

| Finding | Lead disposition |
| --- | --- |
| Claude first: changed LOAD B/S/H_min | Already rejected by complete fresh-layout seal comparison; executed changed B/S tests |
| Claude first: ordinary OOM validity | Reject conditional: own MM returns CS_MM_OK with reply.code=-108; real first/later OOM tests verify reply_valid |
| Claude first: repeated staging pointer false refusal | Clarify native ABI: non-PREPARE source/staging fields are NULL/0; owner retains the distinct staging borrow |
| Claude first: capacity subtraction / uninitialized diagnostic | Earlier actual layout proves candidate>=app.begin+128; native envelope intentionally precedes identity/diagnostics |
| Claude first: successful retirement masks | Independently retain actual tombstones and retired identity in report; add exact successful-release assertions |
| Claude follow-up: workspace source pointer | Owner already has no source lease; adopt additional hygiene, clear scratch inspection/view pointers on success and source-error paths |
| Claude follow-up: negative CODE IDs | Reject: SPEC32 requires selected segment IDs1..32767, first1; CODE0 is0. No admitted negative segment can reach staged() |
| Claude follow-up: source alias/count bounds | Earlier native_safe rejects source/staging overlap and counts>64/8 before writes; tests preserve output/report on unsafe envelopes |
| Claude follow-up: disposed free-list link | Reject historical/private-heap claim as implementation evidence. Own SPEC23 manager clears record/cell; do not reconstruct Apple private heap structures |
| Claude follow-up: nonzero free record / disposal epoch | Own manager represents free records with kind0 and disposal does not advance lifecycle epoch; all real cleanup tests pass |
| Claude follow-up: CANCEL helpers | inactive reads native control; cancel-aware acknowledgment skips guest reads/windows/physical checks, as tested with faulted life and changed manager/map/epoch |
| Grok: PTR fragmentation invalid | Reject premise: own original PTRs are immovable. Add real locked-handle fragmentation nevertheless; exact first/later -108 and world preservation execute |
| Grok: PARTIAL restoration oracle | Original stationary-pointer fixture permits equality; add separate real compaction-then-later-OOM fixture proving moved unrelated handle/payload survive cleanup |
| Grok: unit-counter loop self-oracle | Adopt independent135 LOAD /85 RELEASE counts, per-boundary masks/world/table progress and phases, plus6 before-call fault seams; attempted failure counts differ from completed |
| Grok: physical EXEC alias scope | Use exact app-payload backing alias at unrelated logical address, avoiding ambiguity of the earlier conservative whole-region test |
| Grok: shared slots / scrap / generation / external gate | Add exact30 mixed system/application occupied slots,31-slot refusal, resident scrap descriptor preservation, changed generation, no OOM generation consumption and PARTIAL/LOADED external refusal |
| Grok: full world / trailing / patterned data | Add136 partitions with whole-world byte oracle, all parameter/slack bytes, adjacent markers, full logical payloads and exact staging counts excluding CODE0 tail; retain odd payload padding check |
| Grok: inventory/resource/context gaps | Add owner/kind/raw backing/physical GUEST alias refusals, faulted/open/borrowed/foreign cache controls, actual inactive-session construction and active/depth/pending/config-change refusals |
| Grok: real unexpected MM errors beyond seams | No external hardware/native-error injection claim: internal before-call seams model unexpected failures. Real ordinary OOM and actual MM calls execute separately |
| Lead: physical alias and scalar guards | Reject app/system/global backing aliases and physically aliased GUEST/native inventory; validate copied manifest/layout on RELEASE/DESCRIBE before indexed access |

Final amendments receive lead review and final-source local/CI checks, not another
external-review claim. Initial failed strict GCC test comparison, one inherited
manifest filename, and a preflight-vs-owner error-precedence oracle were corrected;
pre-final failures are not passing evidence.

## Remaining work

Next design the execution bridge separately: admit only owned instruction bytes
including extensions/targets/trusted exits, exclude headers/padding, protect every
guest/service/BlockMove/trap write and physical alias, filter every manager entry,
and establish full root/callback/patch quiescence before retirement. Entry frames,
exit policy, launch globals and dynamic LoadSeg/UnloadSeg remain separate. Current
non-EXEC profile and CLOSED-only cache gates cannot simply be bypassed.

No January1984/AL-2 interpretation is confirmed by these choices. R-001, LQ1..15,
prior RF/HL/RA/ES/PG/MQ/WQ/SQ/OD questions, three editions, physical/cumulative-stack/
full128K fit, B2criterion6 and human provenance remain open. Both required ROM/
System1 boot directions are untested; no new trial is authorized or run.
