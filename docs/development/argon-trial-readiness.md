# Argon Trial Readiness Draft: SPEC-0042 Coverage Audit

- **Baseline revision**: `ffecc003c3a142e88366b6a5f7e7e392a213fd9b`
- **Status**: Draft for Codex/lead review only (`tests_executed: []`; all cases unexecuted; grants no implementation, fixture, or trial authority).
- **Result**: `SPEC-0042` and `guest-port-cases.md` cleanly separate the pure shared native selection core from owner/guest-boundary coordination while preserving `SPEC-0036` fixed-port and `SPEC-0041` view contracts unchanged. Explicit `[X]` deferrals (`P1`..`P6`) are properly gated.

## Actionable Gaps and Ambiguities (Max 4)
1. **Record-to-record guest overlap & binding count bounds (`SPEC-0042` §4, §9; `C03`, `C20`, `C24`)**: §4 allows `1..2` bindings and §9 forbids `SetPort ID record` overlapping `other record`, but `C20`/`C24` do not explicitly cover two guest records overlapping each other in below-A5 memory or prepare-time refusal for `0` or `>2` bindings.
2. **Globals reservation lower bound & 28-byte app parameters (`SPEC-0042` §5; `C14`, `C24`)**: §5 requires `[g-202, g+4)` inside `CODE0` below-A5 space and preserves existing checks on the other 28 above-A5 application-parameter bytes, which lacks an explicit prepare/admission refusal witness in `C14`/`C24`.
3. **Mid-run nonlive/truncated current record refusal class (`SPEC-0042` §5, §11; `C10`, `C22b`)**: `C22b` places a nonlive/truncated registered current record under guest/service refusals, whereas §4/§6 validate extent/liveness at admission and §11 assigns mid-run binding/view/map changes to owner quarantine (which preserves prior effects).
4. **Register preservation & quarantine state retention witnesses (`SPEC-0042` §8, §11; `C01`, `C02`, `C10`..`C14`)**: §8 requires preserving untouched `D0-D7`/`A0-A6` registers and §11 requires retaining prior effects and stopped frame on quarantine; neither has an explicit assertion in the positive (`C01`/`C02`) or quarantine (`C10`..`C14`) case descriptions.

## Decisions Needed
- Clarify whether `C22b` represents a pure native-core input validation refusal in Slice 1 unit tests or an owner-quarantine event (`§11`) when reached during an admitted guest run.

## Next-Step Prerequisites by Layer
- **Slice 1 (Pure shared native core)**: After lead review of `C22b` classification and record/binding bounds (`C20`/`C24`), review pure selection validation/plan logic on 68000/x86 with no guest RAM or trap wiring (`P3` disjointness).
- **Slice 2 (Owner/guest boundary)**: Requires separate contract amendments for `P1` (`SPEC-0036` `(A5)` expected-`g` header clause), `P2` (`SPEC-0029` owner-coordinator environment span), `P4` (lifecycle participant coverage), `P5` (trap registry admission), and `P6` (`CODE0` below-A5 declaration).
