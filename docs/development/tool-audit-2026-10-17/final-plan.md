# Final ClasSICK multi-tool orchestration plan

Accepted planning record2026-10-17 after all four reviews; SRC-0252..0257.
This plan authorizes no runtime feature, build, trial or release. Public baseline
`b3e8cddc3ea436fb588212b572129dd5698b491b`. This whole project runs on
**Windows11/PowerShell**. Codex remains task selector, sole repository writer,
integrator and final verifier. Highest priority: reduce ChatGPT/Codex work by
adopting useful external artifacts, without weakening correctness or source gates.
This audit's all-four fan-out is explicit owner scope, not the normal workflow.

## 1 Evidence and model access
Three labels apply independently: catalog-listed, successfully invoked through a
specific service, and qualified on a particular project artifact. None implies the
other two. Self-reported capability and model preferences are hypotheses. Catalogs
are snapshots, not proof of future availability or account entitlement.

- Antigravity1.4.0 lists Argon4 Low/Medium/High, Gemini3.8 Flash Low/Medium/High,
  Opus5.5 Low/Medium/High, Sonnet5.5 Low/Medium/High, GPT-OSS120B Medium.
  Owner pins actual Antigravity work to **Argon4 High**, exact arguments
  `--model gemini-4-argon-high --effort high`; verify resolved model, never switch
  to xhigh or another family automatically. Other entries are unused inventory.
- Copilot1.0.98 picker shows GPT5.6 Terra/GPT6 Luna/GPT5.6 Luna/GPT5.4/5.4mini/
  5.3-Codex/5mini; Sonnet5.5/5, Haiku5.5/4.5; MAI-Code1.1-Flash; Gemini3.8/3.7
  Flash; Grok4.7/4.6/4.5; KimiK3; Auto and unclassified raw aliases. Traversal is
  not proven exhaustive. Current successful route: **GitHub Copilot/Sonnet5.5**,
  explicit `claude-sonnet-5.5`, low effort. Current receipts report provider id/kind
  `copilot` and this model; `isByok` is absent (prior qualification reported false).
  Funding/plan/balance are unknown unless explicitly reported. Six raw aliases and
  Sonnet4.6 visibly show plan denial; do not treat them as local/free or invoke them.
  Unknown routes are excluded; a GPT-like name proves neither ChatGPT usage nor
  GitHub funding. [Catalog and limits](model-access.json).
- Claude Code2.1.287: **claude-opus-5-5**, low effort, successfully invoked.
  CLI advertises fable/opus/sonnet aliases; full authenticated catalog and alias
  resolution remain unknown. Extra IDs in a self-report are not catalog evidence.
- Grok1.0.52 authenticated catalog: **grok-4.7**, grok-4.7-build-fast, grok-4.6,
  grok-4.5. Default/explicit4.7 has successful requests; use low effort for narrow
  reviews. Listed alternatives have no comparative project qualification.

Do not benchmark every model or infer performance/price from its name. Record
requested model, resolved model where reported, service route and effort per call.

## 2 Default allocation (task fit, not brand rankings)

| Work | Primary | Distinct-risk help only when needed |
| --- | --- | --- |
| Substantial reviewed contract or original portable C draft | Claude/Opus5.5 low | Copilot API-only oracle OR Grok narrow ownership/ABI review, selected for the actual gap |
| Independent test/oracle artifact | Copilot/Sonnet5.5 low | Codex checks the oracle against the accepted contract; no redundant second suite |
| Coverage, handoff, documentation, mechanical verification-script draft | Antigravity/Argon4 High | Explicit native-core/owner-boundary/guest-execution rubric; Codex corrects classifications |
| Focused defect, ownership/lifecycle or coupling review | Grok/4.7 low | Exact diff/one risk; return a concrete applicable hunk when a fix is clear and in scope, otherwise evidence/decision, not an invented patch |
| Build/link/compiler/CI failure explanation | Qualified patch author first; otherwise one Argon High trial | Sanitized exact logs and host/target/toolchain facts; Codex runs commands and verifies fixes |
| Approved-source research | Codex source admission/primary checks; external evidence-organization only within existing input limits | No worker receives manuals or excluded originals under this authority |

Use one log-analysis worker per failure: qualified patch author first, otherwise
Argon for one useful bounded log-analysis trial. Do not send the same logs to both.
Argon portable-C scaffolding, fixture-generation or ABI/layout checks remain
candidate useful trials only when that feature/implementation is authorized and
meaningful independent verification exists; no automatic ABI/kernel promotion.

ABI/kernel/driver/loader and new roles remain task-specific and need reviewed
contracts and meaningful verification. Existing narrow qualifications are reused:
Claude's earlier independent tests remain qualified even though its self-report
understated them. Copilot's historical Sonnet5.5 artifacts retain their recorded
scope. A test author must be independent of the implementation artifact, not
merely another vendor label. Withhold implementation from API-only oracle work.
Review votes and provider diversity do not prove behavior or historical facts.
Freeze the accepted contract/public API before implementation and oracle dispatch;
record their hashes in both packets. The independent oracle author receives no
implementation, implementation commentary, implementation-side logs or session
history, including retries. Interface-level outcomes or its own compilation
diagnostics may be sanitized for a test correction. If independence is lost, label
those tests implementation-aware and obtain a fresh independent artifact only if
the concrete risk requires it. Do not alter an oracle merely to match the code.

## 3 Model alternatives and fallbacks
For Copilot, catalog-listed Haiku5.5, GPT6 Luna or Gemini3.8 Flash are **candidate**
models for simple mechanical artifacts/triage, not proven cheaper/better choices.
Grok4.7-build-fast is a candidate narrow-review/log-triage trial, not a speed claim.
No broad benchmarking sweep: at most one useful alternate trial in a normal task;
revisit only on a concrete gap, availability change or owner request.
Before an alternate is used: verify current availability and service/funding route,
choose a small useful deliverable within authorized scope, review its artifact and
verify any applicable behavior. No blanket new owner approval for routine scoped
selection is introduced, but existing pins, task/implementation and spend gates
still apply. No switching Antigravity away from Argon High under this plan.
If a preferred worker is unavailable, use a qualified role-compatible alternate or
shrink a useful qualification task; do not push unqualified high-risk work through
to preserve throughput. Unknown availability is not zero remaining allowance.

## 4 Dispatch and integration
For normal work pick ONE bounded primary deliverable. Add at most one worker for
an explicit unresolved risk/independence need, not an automatic review ritual.
Packet: exact public revision/paths/sections, Windows11 + PowerShell, authorized
feature scope, native Mac/native PC/legacy boundary, acceptance criteria, allowed
inputs/tools and response budget. Prefer original patches/usable files over advice
or repeated full-file/history dumps. Do not rebuild a sound worker artifact in Codex.
Inline compact context is acceptable; file packets do not remove context limits.
Frozen input-only directories; capture/log siblings outside Git and worker inputs.

Before semantic review, validate output schema, exact declared file list/allowed
paths, encoding, completeness and budget. For unified diffs run `git apply --check`
on the pinned baseline; full-file artifacts get content/schema checks instead.
Never blindly normalize binary fixtures or hashes. Reuse versioned per-feature
briefs and send deltas; keep oracle context separate from implementation context.

Workers return stdout proposals and concise result/findings/tests-actually-run/
blockers/decisions. Actual execution stays with Codex in this supervised setup.
Codex checks source, ownership and semantics, integrates sound work with necessary
corrections, runs focused plus required repository/CI checks, and records actual
results. Planned cases are never passing tests. Reuse test evidence only when
relevant source/dependency inputs, flags, toolchain and profile identities match.
A changed relevant input invalidates that evidence; unrelated documentation or
Git revision alone does not require a full rerun. Required checks still execute.
Failed builds justify a focused log-analysis task, not duplicated artifact work.

## 5 Windows, controls and bounded usage
Never assume bash, WSL/MSYS2, local68000 tools or Linux-host behavior. Identify the
actual compiler, ABI, line endings/encoding, PowerShell quoting and native paths.
Name the actual PowerShell version (audit host7.6.5; do not assume all hosts match).
Text proposals use explicit UTF-8 without BOM/LF and repository-relative forward
slashes in diffs. Avoid shell default redirection encodings; handle CRLF checkout
settings, case-insensitive collisions, spaces and path lengths during integration.
These are format checks, not a mandate to rewrite unchanged files.
Linux CI cross-build evidence is distinct from local Windows verification and
target boots. Do not install/update/configure providers, buy credits or change
subscriptions. Existing authenticated services only.

Use established CLI controls, no bypass flags. Claude: explicit Read-only tool
set, hooks/memory off, empty strict MCP, no persistent session for routine calls.
Copilot: view-only, shell/write/URL denial, MCP/custom instructions/remote/update
disabled, low effort; soft30-credit minimum session cap is NOT account balance or
a hard spending ceiling. Check requested/reported model and provider id/kind/label;
record absent funding/isByok fields as null, not inferred. Do not expose credentials.
Grok: request explicit model/low effort, Read tool restriction, no web/subagents and
bounded turns. Generic tool/MCP/skill gateways mean hard file-only enforcement is
not established merely by those flags; record actual exposed and used tools.
Antigravity: explicit Argon High, normal permission review/sandbox, no conflicting
plan+disable-slash combination. Plan expansion is observed; a worker claim about
brain artifact writes is unverified, not a granted permission. Avoid plan injection
for stdout-only tasks only after a normal-review alternative is verified; never
switch blindly to auto/accept-edits. Explicitly forbid model artifact/file writes.
Input-only CWDs are organizational boundaries, not proven filesystem confinement.
No provider/model is intrinsically read-only. Keep excluded material absent from
worker inputs; retain normal denial/review, inspect exposed/used tools and stop on
unexpected use. For these inline audit calls all tools were forbidden and all8
recorded zero calls. A task requiring stronger isolation must wait for verified
controls, not expanded access. Capture/model/usage metadata already in stdout is
preferred over speculative private app-log paths. Prompt rules are not hard
security controls; no claim that broad tool exposure was repaired by this plan.

Start from demonstrated compact inputs and outputs (roughly300 changed lines or
600-word docs; adapt deliberately). Suggested wall caps: Argon420s, Claude240-360s,
Copilot240s, Grok180-240s, adjusted to the bounded deliverable and recorded. Empty
or invalid artifact, timeout, denied tool or unexpected route is not completion,
even if exit0/SUCCESS appears. Stop unproductive retries; one bounded continuation
only for a concrete correctable blocker, with no new context/permission expansion.
If scope needs owner approval, prepare a concrete result and retain that gate.

Record latency, requested/resolved model and route, provider counters/cost basis,
budget compliance, tool deviations and integration corrections. Keep provider
units separate; no token-to-ChatGPT-quota equivalence or inferred subscription bill.
Use actual/cached dated allowance where available; null means unknown, never zero.
Check quotas before a costly burst or after rate failure, not before every small
call. Account details stay private HQ; transcripts outside Git. Summaries concise.

## 6 Boundaries and next use
No private HQ, credentials, untracked research originals, manuals, Apple/Microsoft
implementation/assets, media/ROMs/disassembly/emulator source or UART bodies to
workers. No historical claim from model memory or our implementation. Existing
source/role/edition/physical/full128K/B2/human/System1/ROM gates persist; fresh owner
go-ahead for each target/VM/emulator/reference run. Native Mac/PC advances remain
paired; interpreter is for legacy applications. No extra Codex chats/subagents,
automation, OneNote or Git wiki maintenance. Only ordinary Git/HQ plan records.
This audit does not authorize selection-core code, fixtures, builds, target runs
or release. Next actual feature task still needs its accepted scope and contract.

## 7 Review outcome and operating checklist
All four reviewers returned approve-with-changes. Two called control-assurance
issues blocking; this final plan removes unsupported isolation claims and retains
the limitations explicitly. It does not claim new OS/CLI enforcement. Lead rejected
claims that Argon stdout lacks model/token metadata (the captures contain it), and
did not adopt unverified internal-log paths or blanket reruns on every revision.
[All18 review dispositions](review-dispositions.md), [capability comparison](../tool-capability-audit.md),
[receipt](audit-receipt.json). The same frozen proposal was reviewed by all four;
the final plan is the lead synthesis, not an additional unanimous worker vote.

For each next authorized task: select one usable artifact and role; pin model and
service; freeze a compact Windows11 contract/API brief; withhold implementation
from the independent tester; validate the returned artifact; integrate and verify
the relevant behavior once; record concise corrections, costs/counters and remaining
unknowns. Use the best-evidenced model for the task, not a global brand ranking.
The current runtime milestone is still SPEC-0042 contract-only; this audit supplies
no implementation permission. OneNote/wiki remain retired.
