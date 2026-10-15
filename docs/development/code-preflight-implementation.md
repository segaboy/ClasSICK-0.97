# Original CODE preflight implementation - 2026-10-15

SPEC-0032 / ADR-0036 / IMPL-0036 / TEST-0040 / SRC-0167..0170.

`personality/system1/code-preflight.h/.c` assigns the original host numeric ABI:
cs_code_inspect and cs_code_check_layout, cs_code_result0..15, and caller-owned
workspace with byte capacity at least sizeof(cs_code_workspace). The header is
the complete native record/enum declaration; there is no guest trap ABI or binding.
Inspection needs a truthful map and explicit inactive ordinary context; a lifecycle
may be absent for standalone inspection without a manager. Supply every relevant
live runtime control. Layout requires a lifecycle and independently admitted map.

The canonical manifest/layout publishes only after all checks pass. Every failure
preserves it and all protected state; unsafe native envelopes also preserve the
report/scratch. Other refusals may initialize diagnostics or bounded workspace.
Source fields are checked offsets, never dereferenced file pointers. A manifest
is copied scalar protocol data, not authenticated source ownership. Layout has
no source lookup and always reports source_revalidated_for_layout=false. All
execution_authorized fields remain0. No allocation, reset, instructions, callbacks,
global process state, retained resource lease or executable entry is introduced.

The original scalar scanner checks native aliases, CODE0 semantic order, unloaded
entry forms and distinct segments before per-segment lookup/group/target checks.
The independent snapshot validator rechecks counts, indexes, scalar spans, policy,
groups and derived warnings before layout. The layout admits stable lifecycle
integrity first, then checked arithmetic, strict24 bounds, full configured-zone
stack exclusion, logical/physical aliases and actual READ/WRITE windows. Above
and below A5 may occupy separate valid windows; parameter/table containment is
intentional. Physically overlapping above/below windows refuse. No execute access
is requested. Pending reset inputs/staging/native inventory remain protected even
on state refusal. The existing manager's32 cells per zone cost128 bytes each.

## Lead review of bounded workers

Each CLI reads one frozen reviewed-public/original packet, in two model turns.
Claude receives contract/header/implementation; Grok also receives the original
tests. No worker reads HQ/raw research/manuals or performs edits, tests, commands,
web/MCP/subagent operations. Packet/report identities and actual usage are in
[evidence](code-preflight-evidence.json). Both reports exceed the requested
700-word target; bounded calls and input scope are verified independently.
The reports are advisory input, not authority, test execution or certification.

| Finding | Lead disposition |
| --- | --- |
| Claude D1 stale field after later missing segment | Adopt: clear field in lookup; regression checks missing CODE2 and no stale position |
| Claude D2 conditional bit7 mismatch | Reject as a defect: the existing reader returns REFERENCE on bit7; selected reference0/1/2 and combined0xC0 refuse. All64 admitted attribute combinations round-trip through layout |
| Claude D3 inaccurate shared-header comment | Adopt wording correction; keep independent snapshot validation |
| Claude incomplete protected-plan knowledge | Independently inspect own closure: protect active plan borrowed request/scrap/staging/inventory/native registrations; test pending reset staging alias and ordinary STATE refusal |
| Claude unverified integrity assumptions | Own manager/lifecycle map/config checks establish bounds before backing arithmetic; GET_PTR_SIZE0 integrity path writes no guest/manager state. Prior tests and new refusal snapshots verify the contract, not arbitrary native pointer authentication |
| Grok isolated warnings and boundary cases | Adopt: each warning separately, CODE1/2/1 mismatch, exact32766 routine in32768 bytes, four-byte segment refusal, ID32767, extra malformed snapshot tags/policies, odd sizes, duplicate types and late bad push |
| Grok stale/low-memory/readonly/session suggestions | Existing plus added strict24-end and read-only/physical alias cases cover selected boundaries; unadded combinations remain future targeted cases, not claimed executed |
| Grok dead implied arithmetic predicates | Keep defensive checks; valid prior invariants imply some cannot fail independently. Do not forge source provenance to exercise them |
| Grok wrapped backing arithmetic | Reject as a reachable defect: the earlier actual lifecycle integrity validates the whole map and rejects wrap as STATE |
| Grok truncated-prefix concern | For this specific independently encoded fork every shorter length truncates its declared map. All410 lengths correctly return SOURCE; no claim about every possible resource fork |
| Grok cell-band128-only and stack_base0x400 success proposals | Reject those oracles under present constraints: initialized zones require130 bytes; disjoint stack makes128-only candidate impossible, and a giant stack crossing configured zones is UNSUPPORTED. H_min0 itself is admitted |
| Lead separate above/below windows | Fix an unnecessary combined world-window requirement and reject their physical alias; both cases execute in layout suite |

Final amendments receive lead review and final-source local/CI checks, not a second
external-review claim. Initial test-oracle/encoder mistakes and one GCC indentation
warning were corrected; failed/pre-final attempts are not passing evidence.

## Limits and next contract

The64/16 capacities, header-inclusive32K ceiling, complete contiguous groups and
literal resource-start policy are engineering limits. AL-2/LQ-1..15, R-001 and
prior historical questions remain open. The >UINT32_MAX scalar guard exists
before narrowing/reader access; the compact fixture's false4GB span instead
hits the earlier mandatory alias refusal, so it is not a direct test of that
guard. No full RAM/linked ROM/cumulative stack or final128K fit is claimed.

Next propose executable-image ownership and launch/population transactions,
including source revalidation, cache/gate composition, guest mutation/manager
filtering, main-segment and transitive return lifetime, fault/teardown behavior,
and separately reviewed dynamic continuation. The current CLOSED-only resource
execution/reset gate and typed LoadSeg refusal are unchanged. The owner-required
two ROM/System1 boot directions and native-application gate remain independent.
