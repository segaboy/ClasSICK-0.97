# Argon High supervised trial - lead review

2026-10-17; SRC-0249..0251. Documentation-only trial; existing project rules,
worker roles and implementation/target/release gates are unchanged. Argon always
uses explicit `--model gemini-4-argon-high --effort high` per the owner.

## Result and artifacts
Completed after the owner resumed the paused trial: one substantive assignment,
one continuation in the same conversation, no other worker. Argon supplied a
[20-row coverage matrix](guest-port-coverage-trial.json) spanning SPEC-0042
sections1..14 and an [18-line readiness draft](argon-trial-readiness.md).
Both are preserved as original **proposals**, not accepted requirements or passing
tests. The combined44 lines and136-word handoff meet the output budgets. All cited
case IDs exist. Cases remain unexecuted. No fixtures/code/registry binding/build/
emulator/VM/hardware/reference run or release was performed.

## Lead findings disposition
1. **Adopt as coverage gap**: binding-count bounds and two guest-record overlaps
   are explicit contract obligations (§4/9) without an explicit witness in the
   supplied C03/C20/C24 descriptions. This is not an observed runtime defect.
2. **Adopt as coverage gap**: complete206-byte globals reservation bounds and the
   other28 application-parameter checks (§5) lack explicit witnesses in this case
   plan. Existing implementation/checks outside the packet were not audited.
3. **Adopt as open classification question**: C22b's service-refusal label for a
   nonlive/truncated registered record needs distinction from post-admission trusted
   binding/map/lifetime corruption quarantined by §11. The trial does not settle it
   or amend SPEC-0042. Resolve before the relevant future binding is implemented.
4. **Adopt as coverage gap**: untouched registers/SR and retention of earlier
   effects/actual stopped frame on quarantine (§8/11) need explicit expectations
   beyond the supplied C01/C02/C10..C14 descriptions.
Matrix AR15 also notes missing explicit overflow-boundary coverage; retain as a
coverage observation, not authority to generate or execute a new case now.

## Matrix limitations (apply this review, not silent draft edits)
| Rows | Lead correction |
| --- | --- |
| AR02 | Mixed layers: canonical trap words/P4 are guest dispatch; an opaque record extent may be a core value. Split before use. |
| AR09/AR10 | Guest-cell writes, BE output and destination ownership belong to the owner boundary. Pure native core returns values/plans and cannot mutate guest RAM (§14). |
| AR13 | The combined alias table mixes native backing/value validation with guest stack/map/span duties. A single native-core label is insufficient. |
| AR20 | C10/C11/C14/C52 cannot establish the sequencing of contract acceptance. Dependency review is a scope check, not planned behavioral-test coverage. |
The11 "planned-covered" rows are Argon's proposals, not an accepted coverage
score. These classification errors limit the artifact's use without lead review.
The four readiness findings survive review; no general runtime/code/research
qualification or permanent worker reassignment follows from this single trial.

## Controls, completion and usage
Runtime init confirmed `gemini-4-argon-high`. Plan command expansion and sandbox
were requested; actual permission mode remained `request-review` with broad tools
exposed. No hard read-only allowlist is claimed. The initial call used one view of
the frozen reviewed-public packet; the continuation used no tools. No checkout
edits, commands/tests/web/MCP/skills/subagents, HQ/manuals/original research/UART
bodies/credentials/prohibited inputs. Codex alone validates and records results.

Initial capture stopped at240s (wall241.98s) with a timeout warning, exit0 and
raw SUCCESS but empty response. It is **incomplete**. The owner later requested
completion. Same-session continuation produced the artifact in68.64s, with no
timeout. Active capture wall time310.62s excludes the pause and inspections.
The CLI's cumulative duration includes the pause and is not model runtime.
Completed result reports56828 input/25849 output tokens (22613 thinking tokens,
82677 total, provider definitions). Preserve step/cumulative counters separately
in the [receipt](argon-high-trial.json); do not add overlapping snapshots or infer
subscription charges or ChatGPT quota savings. Price not reported. Account quotas
stay private. Raw streams/CLI logs remain outside Git.

Actual checks: output JSON/schema/row/section/case-ID/budget validation, observed
tool-call audit, unchanged worker checkout/original-input audit, ordinary Git
documentation and final preservation checks. These are artifact/documentation
checks, not execution of the proposed application cases. No test code generated.
Future implementation still needs its own owner-approved scope. OneNote/wiki
remain permanently retired; no new agent/chat/automation or rule change.
