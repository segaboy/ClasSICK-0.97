# Guest port selection - test cases (unexecuted)

Not run. No fixture or test code exists. Each case expects pre-effect refusal
(state/RAM/frame/native unchanged) unless it says otherwise.

## Positive
- C01 GetPort with initial current 0 writes 00000000 to dest; PC+2, A7+4; one step.
- C02 SetPort(ID1) then GetPort returns ID1; only declared data outputs, scratch and the committed frame changed.
- C03 Switch ID1->ID2->ID1; two disjoint views and native port backings.
- C04 Direct guest store of ID2 to g; GetPort returns ID2 (no caching).
- C05 Native68000 and x86 cores give identical plans/results for C01-C04.
- C06 Budget stop after SetPort keeps the selection; retire scrubs the cell.
## Capability/owner threats (quarantine)
- C10 Forged capability digest; mutated binding/view after admission.
- C11 Stale generation; stale epoch; recycled image address with a new generation.
- C12 Binding view aliases capability, controls, report or a guest map backing.
- C13 Exact sharing or partial overlap of two native port backings.
- C14 (A5)!=g at service time; header altered; zero-header expected in fixed mode.
- C15 Fixed/default mode given a g header -> existing refusal unchanged.
## Guest/service refusals
- C20 Duplicate IDs, odd ID, high (>24-bit) ID, ID 0 registration -> prepare refused.
- C21 SetPort(NIL), SetPort(unregistered), SetPort(record extent past below-A5).
- C22 Unknown value stored in g, then GetPort/SetPort refuse; no old-view fallback.
- C22b Inactive preparation/pure values: a truncated or invalid supplied record refuses on either call. After owner admission, trusted record/map/lifetime/binding corruption quarantines on either call; preserve prior effects and the actual stopped frame. A malformed guest ID alone remains a service refusal.
- C23 GetPort dest odd, in stack, in a record, in g reservation, in A5/header.
- C24 Record overlapping the g reservation or stack/CODE/jump table -> prepare refused.
- C25 Parameter at root token; scratch below stack base; S+4 > root.
- C26 Continuation PC+2 not an owned opcode; SR supervisor/mask mismatch.
- C27 Guest MOVE to (A5) refused by existing write policy.
## Lifecycle
- C30 Setup while active; capability install during the run; reentrant run.
- C31 Run after retirement or teardown with the old capability.
- C32 Second execution in a consumed generation.
## Trap surface (no implicit expansion)
- C40 A8A2/A8A3 in the new lane refuse; Get/Set in the SPEC-0036 lane refuse.
- C41 Auto-pop, bit9, OS-format, unknown, InitGraf/OpenPort words refuse.
- C42 Patched A873/A874 slot at admission and mid-run refuses; no GetTrapAddress.
- C43 JSR to a would-be builtin identity: none exists; normal policy applies.
## Commit atomicity
- C50 Dest overlapping scratch or parameter -> refused before any write.
- C51 Map access fault injected at each preflight step leaves everything unchanged.
- C52 No generic SPEC-0029 handler can reach g or dest (span isolation).

C10-C15 depend on owner-capability admission; C20/C24 are inactive setup refusals,
not guest CPU steps. C51 covers ordinary service-span faults before commit; trusted
map/configuration corruption uses owner quarantine. No test has been implemented or run.
