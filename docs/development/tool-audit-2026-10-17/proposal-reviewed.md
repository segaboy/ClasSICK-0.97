# Proposed ClasSICK orchestration after four-tool capability audit

Draft2026-10-17 for all four workers' review. Public baseline
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
  explicit `claude-sonnet-5.5`, low effort, prior/current provider receipt checked
  for `isByok=false`. Unknown custom/local routes are excluded from quota-saving
  routing; a GPT-like name alone proves neither ChatGPT usage nor GitHub funding.
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
| Focused defect, ownership/lifecycle or coupling review | Grok/4.7 low | Exact diff and one risk; no broad review of every file |
| Build/link/compiler/CI failure explanation | Argon High first bounded useful trial, or Claude if tied to its patch | Sanitized exact logs and host/target/toolchain facts; Codex runs commands and verifies fixes |
| Approved-source research | Codex source admission/primary checks; external evidence-organization only within existing input limits | No worker receives manuals or excluded originals under this authority |

ABI/kernel/driver/loader and new roles remain task-specific and need reviewed
contracts and meaningful verification. Existing narrow qualifications are reused:
Claude's earlier independent tests remain qualified even though its self-report
understated them. Copilot's historical Sonnet5.5 artifacts retain their recorded
scope. A test author must be independent of the implementation artifact, not
merely another vendor label. Withhold implementation from API-only oracle work.
Review votes and provider diversity do not prove behavior or historical facts.

## 3 Model alternatives and fallbacks
For Copilot, catalog-listed Haiku5.5, GPT6 Luna or Gemini3.8 Flash are **candidate**
models for simple mechanical artifacts/triage, not proven cheaper/better choices.
Grok4.7-build-fast is a candidate narrow-review/log-triage trial, not a speed claim.
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

Workers return stdout proposals and concise result/findings/tests-actually-run/
blockers/decisions. Actual execution stays with Codex in this supervised setup.
Codex checks source, ownership and semantics, integrates sound work with necessary
corrections, runs focused plus required repository/CI checks, and records actual
results. Planned cases are never passing tests. Do not repeat unchanged tests or
qualifications; failed builds justify a focused log-analysis task.

## 5 Windows, controls and bounded usage
Never assume bash, WSL/MSYS2, local68000 tools or Linux-host behavior. Identify the
actual compiler, ABI, line endings/encoding, PowerShell quoting and native paths.
Linux CI cross-build evidence is distinct from local Windows verification and
target boots. Do not install/update/configure providers, buy credits or change
subscriptions. Existing authenticated services only.

Use established CLI controls, no bypass flags. Claude: explicit Read-only tool
set, hooks/memory off, empty strict MCP, no persistent session for routine calls.
Copilot: view-only, shell/write/URL denial, MCP/custom instructions/remote/update
disabled, low effort; soft30-credit minimum session cap is NOT account balance or
a hard spending ceiling. Check service route in receipts; do not expose credentials.
Grok: explicit model/low effort, Read-only, no web/subagents, bounded turns.
Antigravity: explicit Argon High, normal permission review/sandbox, no conflicting
plan+disable-slash combination; broad exposed tools are not hard read-only isolation.
Prompt restrictions supplement controls, never replace them. Audit actual calls.

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
