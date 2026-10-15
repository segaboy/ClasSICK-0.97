# Hosted resource residency implementation - 2026-10-14

SPEC-0031 / ADR-0035 / IMPL-0035 / TEST-0039 / SRC-0159..0162.
Lead self-review; human provenance review pending. Original engineering profile,
not a historical Resource Manager implementation or application-load result.

## API and ownership

`personality/system1/resources.h/.c`: zeroed caller service, one immutable borrowed
source per OPEN,16 cache entries,32 canonical leases and one immutable first-fault
snapshot. `cs_res_init` binds once; `cs_res_execute` takes one typed request/report.
The report embeds ACQUIRE's ticket; copy it to disjoint native input for later
DESCRIBE/RELEASE. A copy grants no additional lease. Lookup keys are uint32 type
and signed16 ID. Every loaded local copy is locked, nonpurgeable and nonresource;
file flags stay metadata apart from bit7/system-heap refusal. Zero has a two-byte
reservation and a tagged EMPTY description without a guest zero-span request.

The profile supplies positive uint32 source/resource limits and a nonzero native
owner ID. A truthful authoritative FUTURE/REFUSE acknowledgment precedes OPEN and
stays through faults. Ordinary context, inactive optional session, stable lifecycle,
same heap epoch, application TheZone, complete manager integrity and cached handle/
cell/extent checks precede operations. Borrowed typed objects are aligned/disjoint;
both original and changed map backing ranges protect refusal output destinations.
Two read-only retained/incoming source spans may coincide on a state-refused OPEN.
The service never authenticates native inputs or payload contents. Constructor map
descriptors/backing identities and zone configuration remain fixed; reset may
change epoch/application limit and a CLOSED service reopens with fresh source args.

One admitted NEW_HANDLE may compact other unlocked application objects. Its actual
movement count survives the later HLOCK reply. Ordinary memFullErr is no effect;
unexpected native/invariant failures retain the actual allocation, locks, copied
prefix, cache/ticket publication and disposal/retirement masks. Fresh post-lock
record/cell/span checks and a prepublication check precede resident publication.
Reply fields are copied individually so unused native padding is not disclosed.

Release retires only a ticket. Evict/close need zero leases; close prevalidates all
objects, disposes ascending cache slots and records separate disposed/retired masks.
Native CLOSED publication precedes the last counted unit, but the source lifetime
acknowledgment remains conservative until successful return. Faults have no retry,
close, reinit or recovery. The failing call keeps its real effects in first_fault;
subsequent calls produce fresh no-effect refusals with retained_fault=1 and the
current requested operation, never a newly usable ticket/span. Captured and current
epochs and generation validity are separate. cs_res_external_admit is a trusted
CLOSED-only execution/reset coordinator gate, not low-level API interception.

## Bounded reviews and lead dispositions

Full copyable prompts preceded each authorized CLI launch. Each worker reads one
frozen packet only; identities, actual calls, versions, output and usage are in
SRC-0160/0161. No HQ, manuals, raw originals, credentials or prohibited input is
sent. No code, command, web, extra-file or subordinate agent action occurs.
The lead amendments and final source are not claimed independently re-reviewed.

| Finding | Lead disposition |
| --- | --- |
| Claude C1, replayed fault report can look like current effects | Adopt with changes: fresh refusal, separate immutable first snapshot; no duplicate borrow. |
| C2, unwritten native manager result/lost lifecycle outcome | Adopt: separate tagged outcomes; before-call fault seams publish no dispatch result. |
| C3, stale post-lock views under an unexpected manager violation | Adopt defensively: exact saved record/extent/address, fresh mapped span, repeated prepublication validation and original corruption controls. No real manager malfunction claimed. |
| C4, reinit/profile precedence | Adopt after mandatory pointer safety: tagged STATE precedes profile-field checks. |
| C5, generation/epoch validity | Adopt: non-live CLOSED diagnostics and separate captured/current epoch fields. |
| Claude auxiliary concerns | Existing MM_CELL_BYTES is exactly4*32; the view has only bytes/writable; reader REFERENCE clears data span; lifecycle reset retains zone configuration. Original tests cover reset/reopen and reference refusal. Test seam is deliberately rearmed per case. |
| Grok G1..4, precedence/fault paths/survivors/snapshots | Adopt with original cases, including unsafe output before retained fault and context before registration conflict. Corrupt manager state may refuse lifecycle before FUTURE classification. |
| G5, length boundaries | Adopt exact65000 profile maximum. Huge valid2GB/uint32 payload/source fixtures remain unexecuted; code predicates/width audits are not such fixtures. Resource length requires lookup, so generic profile-before-resolution ordering is rejected. |
| G6, shared-span lifetime | Adopt: release/evict one independent key while the other still describes unchanged bytes and source stays immutable. |

Compiler bring-up caught GCC misleading-indentation warnings, corrected before
verification. First fixture initialization needed the original sentinel NOP bytes;
an initial Python discovery failure and early fault/source alias issue are retained
privately, not claimed passing. Final report-padding correction is followed by all
90 affected checks/object audits and new final68000 outputs. Final profile-limit
fixtures pass their limits to the constructor instead of mutating owned profile
controls; those corrected cases receive the final90-check repeat. Earlier outputs remain.

[Exact execution evidence](resource-residency-evidence.md) records storage/frame
costs and publication CI separately. No new historical questions are answered.
Next propose a bounded CODE-only launch/segment contract; dynamic LoadSeg and
transitive executable-resource/callback ownership remain later obligations.
