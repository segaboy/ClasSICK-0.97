# Agent Orchestration Protocol

## Current routing plan - 2026-10-17

Owner SRC-0252 requested the completed all-four capability audit and review round.
[Final plan](tool-audit-2026-10-17/final-plan.md) is current for routing, model/route
evidence, Windows11 packets, test independence and control limitations where it
corrects dated language below. [Full comparison/reviews](tool-capability-audit.md).
No feature/source/target permission expands: current runtime scope is the subsequently accepted
SPEC-0045 integrated prepared-port implementation and independent compiler/test verification; target trials/releases remain separate. The audit's four-way fan-out is not routine policy.
Always pin Antigravity to Argon4 High. Keep Codex sole writer/integrator/verifier;
one normal primary and optional distinct-risk worker. Historical observations below
remain dated evidence, not claims of current model access or hard tool isolation.

## Authority and Standing Scope
Current owner continuation SRC-0266 permits the bounded SPEC-0045 integrated owner/coordinator,original fixture,independent tests/compiler/CI and ordinary records; no target trial or release; SRC-0227 permanently retires OneNote/wiki maintenance. Worker authority SRC-0223..0226 extends SRC-0149/0150/0179 to four supervised external CLIs. Codex alone writes repositories. All existing implementation, emulator/hardware/reference, source, release and publication gates remain; each target trial still needs fresh owner approval. Private HQ, credentials, untracked research originals, manuals, prohibited implementation/assets and UART implementation/test bodies are excluded from worker inputs.

Standing service scope includes **Antigravity (`agy`)**, **GitHub Copilot CLI (`copilot`)**, **Claude** and **Grok**, using existing authenticated services. Do not reinstall working tools, purchase credits or change subscriptions:
- **Source Research & Manuals**: Source research requires eligible admitted publications. No worker currently receives manuals, ROMs, media, disassembly, emulator source, or Apple/Microsoft implementation material.
- **Kernel & Driver Code**: Unqualified kernel, driver, ABI, loader, or runtime code from external workers is not assumed safe.

## Quota Conservation & Division of Labor
Highest priority is conserving **ChatGPT / Codex usage**:
1. **Codex Authority**: Codex alone holds repository edit, integration, decision, and final verification authority. Codex must never recreate a sound worker artifact.
2. **Focused Worker Dispatch**: Give one worker a substantial bounded primary deliverable, which may contain several files. Add a second worker only for a distinct risk or unresolved gap. Do not send every task to every tool. Reuse prior qualifications and completed artifacts. This supersedes the old Codex-first implementation allocation.
3. **No Cross-Provider Quota Equivalence**: Never equate external provider token counts or credits to ChatGPT quota. Do not spawn new ChatGPT chats or subagents.

## CLI Controls & Isolation Reality
Prompt-only restrictions are never treated as hard security isolation. Do not claim unavailable CLI controls exist, and refuse unavailable controls rather than broadening access.

### Antigravity CLI (`agy 1.4.0`)
- **Owner preference2026-10-17 (current)**: always use Argon4 High: `--model gemini-4-argon-high --effort high`. Verify the resolved model before work; no automatic switch to xhigh. The earlier very-high request is superseded. Installed agy1.4.0 only catalogues Low/Medium/High Argon variants; xhigh conflicted and resolved to Medium during a no-inference startup check.
- **Observed**: version1.4.0; authenticated model catalog and bounded print task succeeded. Used existing default model (actual model ID not reported), low effort, sandbox,180s print timeout and JSON stream capture. One packet read; no command/write/web/MCP/subagent call observed.
- **Control limitation**: the CLI explicitly warned that `--mode plan` has no effect together with `--disable-slash-commands`; runtime permission mode was `request-review`, with broad tools exposed. Do not claim this was an enforced read-only toolset. Future calls must avoid that conflicting combination, retain normal permission review, deny extra permissions and verify actual control state; stop if stronger isolation is required and unavailable. No bypass flags. Frozen public-only working directories and transcript auditing remain necessary.

### GitHub Copilot CLI (`copilot 1.0.98`)
- **Observed**: version1.0.98; one useful task succeeded through GitHub Copilot, model `claude-sonnet-5.5`, `isByok=false`. This confirms usable authentication/model entitlement at that time. Paid/free/trial plan label and remaining account allowance were not reported; do not assume a paid plan or continued availability.
- **Controls**: `-p`, JSONL, `--available-tools=view`, explicit shell/write/URL denial, explicit disabling of built-in MCP/custom instructions/ask-user/remote/update; prompt memory is disabled by default. No all-tools/all-paths/autopilot/fleet/bypass. Low reasoning, default context,0 autopilot continuations, process timeout and usage JSON. `--max-ai-credits 30` is the documented minimum and a soft response-accounting cap, not a purchase or hard spending guarantee. Runtime exposed only `view`; one extra directory listing occurred beyond the requested packet read, with no other file content read. Record that deviation and keep work directories free of excluded inputs.

## Role Qualifications & Candidate Boundaries
Worker roles remain provisional until qualified on specific narrow tasks; qualification validates only the narrow role, not general model superiority:
- **Claude (Historical Qualification)**: Qualified for adopted independent pixel, canary, and legacy C test suites. Do not rerun or rewrite adopted suites without cause.
- **Grok (Historical Qualification)**: Qualified for boundary and coupling review (previously identified synthetic-result and allowlist coupling defects, though with higher reasoning overhead and an extra EOF read).
- **Antigravity**: initial primary for bounded build/verification scripts, mechanical failure analysis and documentation drafts. It supplied the guard and protocol draft; integration required two parser fixes and adapting its illustrative CI patch to the actual workflow. Complex runtime/kernel/driver generation is unqualified.
- **Copilot**: initial independent test-artifact and requirements-coverage worker. Its17 original tests exposed the guard's whitespace defect; Codex added one exact-name case. Prefer it when a concrete oracle/parser/coverage risk justifies a second worker; it is not a mandatory review after every primary.
- **Claude**: use the established relationship for substantial contract drafts and original C test/code artifacts that fit its demonstrated strengths. The native-drawing qualification already produced adopted independent tests; no repeat qualification call here.
- **Grok**: use a small selected defect/boundary review when there is a distinct risk. Prior useful findings coexist with substantial reasoning overhead; use precise scopes, low effort where supported and short outputs. No repeat qualification call here.
- **Research**: approved-source research is a candidate role, not newly qualified. A task must identify eligible source categories/locators and permitted retrieval controls. Historical claims still need eligible primary evidence; worker opinion and implementation behavior cannot settle them. No manuals were transferred in this qualification.

## Handoff, Transcript & Record Hygiene
- **Exact Inputs**: Supply bounded packets with exact commit refs, file paths, and line ranges.
- **Repository briefs**: use `docs/development/tasks/`, exact public revisions and selected functions/sections. Use a frozen public-only input directory when repository-wide access would expose research originals; never grant whole-repository access just to avoid assembling a small brief. State EOF/line counts. Keep the working directory input-only and capture streams/logs/usage in a separate sibling directory, outside the model's permitted paths. Directory discovery cannot reveal a deliberately withheld implementation. Files do not remove context or token limits.
- **Captured Output Only**: Workers return proposed file contents and a concise structured handoff (`result`, `actionable_findings`, `tests_executed`, `blockers`, `decisions_needed` under 180 words).
- **Honest Test Accounting**: Report only actually executed tests in `tests_executed` (empty `[]` in plan/read-only runs). Never count proposed tests as executed.
- **Integration**: inspect accepted artifacts and actual tools, make only necessary corrections, run focused checks plus required final verification, and retain a concise disposition. Stop on unusable output, auth/rate/permission failure or budget exhaustion; retry only for a concrete correctable blocker. Keep elapsed time, provider-reported counters, instruction deviations and integration effort. These counters are not comparable quotas or a measurement of ChatGPT savings.
- **Observed mouse slice**: Claude supplied the substantial original C core/decoder, contract, primary tests and native binding edits; Copilot supplied199 independent C test lines from the API alone, covering8,388,608 report combinations. One Claude request timed out at240s without an artifact; one retry with supported `--effort low` completed in184.91s. Set effort explicitly on future Claude tasks and keep deadlines/output budgets matched to the deliverable. A requested thinking-token environment limit was not enforced (the result reported11,198 thinking tokens); do not claim it is a hard limit. Both successful calls read only their packet and made no edits/tests. Codex integrated sound artifacts, normalized the spec path and added an independent pixel-bound check; full results are in [the mouse receipt](mouse-input-evidence.md). No new Antigravity/Grok calls were needed.
- **Records**: Keep raw transcripts private; maintain ordinary project/HQ development and provenance records. Owner retirement2026-10-16 supersedes all wiki instructions: never contact OneNote or add/maintain its Git wiki queue, mirrors or manifest. Existing wiki files are frozen history, not current authority.
- **Controller allocation result**: [SPEC-0040 evidence](pc-mouse-evidence.md) qualifies Claude's narrow original controller/binding draft after integration/tests. A combined900-line code/native/test request timed out at360s; one retry limited to code and delta patches completed in145.73s. Prefer at most450 output lines and exact/global replacements over whole unchanged files; use supported partial-message capture so timeout diagnosis retains available output. Keep independent tests on the distinct-risk worker; Codex handles final native integration fixtures. Copilot's273-line test proposal needed two expected-outcome corrections and strict-build/license edits; its implementation independence is useful, not authority. No new Grok/Antigravity calls; reuse their qualification unless a distinct gap warrants one.

[Current qualification and verification](worker-orchestration-qualification.md);
[dated prior protocol and Claude/Grok qualifications](agent-orchestration-2026-10-15.md).
The qualification is dated SPEC-0038 evidence. The prior continuation permitted
SPEC-0041 drawing context/workload implementation and native candidate checks; the current slice is SPEC-0042 documentation only; target trials,
releases, general native ABI/SDK and historical claims remain separately gated.

## Drawing-context allocation result - 2026-10-17

One Claude primary completed266 proposed lines in64.95s from a193-line packet;
one API-only Copilot artifact completed in86.05s from55 lines. No retry/new
qualification or Grok/Antigravity call. Core/native delta adopted, test oracle
retained after removing nonassertions and correcting one expectation. Copilot
exceeded260 lines with351; budget compliance and assertion quality require review.
Prefer bounded core+small binding replacements and a distinct-risk tester; no
whole histories or redundant provider reviews. [Receipt](drawing-context-evidence.md).
Current native context candidates do not authorize new target trials or releases.

## Prepared-port contract allocation - 2026-10-17

One Claude primary supplies the three usable contract/decision/case drafts;
one Grok review only for the distinct A5-seal/environment-write/owner-lifetime risk.
Codex performs local eligible-source checks, decisions, integration and documentation
verification. No Copilot/Antigravity calls, repeat qualification or duplicated draft.
No manual/HQ/untracked-original/UART inputs. Captures outside Git, bounded calls;
line budgets and provider-reported usage are recorded without quota equivalence.
Claude completed in83.58s; Grok reached180.11s without a finished report, so no
independent finding was adopted and no retry followed. For future Grok reviews use
the now-confirmed `--reasoning-effort low` control, a smaller selected risk packet
and the same hard timeout; do not repeat this406-line second-review packet.
[Dispositions](guest-port-lead-review.md); [receipt](guest-port-review.json).

## Owner-requested Argon High trial - 2026-10-17

One bounded coverage-audit assignment timed out after240s with exit0/SUCCESS but
empty response; honest completion checks reject that status as a deliverable.
After owner resume, a same-session continuation completed in68.64s, no reread or
other tools, returning20 matrix rows and a readiness draft. Four useful coverage/
classification findings retained; several matrix layer/coverage claims require
lead correction. Existing roles/rules stay unchanged; no automatic qualification
for runtime code or role reassignment. [Lead review](argon-high-trial.md),
[receipt](argon-high-trial.json), [actual prompts](tasks/argon-high-trial.md).

## Native selection allocation result - 2026-10-17

Claude primary core and independent API-only Copilot tests, one bounded call each;
zero worker tools/writes/tests, frozen hashes checked before/after. No redundant
Grok/Argon review. Lead integrated artifacts and ran checks. [Receipt](native-selection-evidence.md).

One qualified-author Claude follow-up corrected an O0 native68000 import-audit
failure without API changes. Three model calls total for this feature; no new
oracle exposure or redundant boundary reviewer. Exact symbol was not printed in
the first failure; improved diagnostics retained. Final tests are rerun on the fix.

## Prepared-port ownership allocation result - 2026-10-17
One substantial Claude draft plus one focused Grok review for stopped-lifetime/
retirement handback. Two calls,zero tools/writes/tests,no retry. Lead adopts the
draft with explicit14 O/3 G dispositions;40 cases stay unexecuted. Copilot/Argon
qualification reused,no duplicate suite or all-tool review. [Review](prepared-port-owner-lead-review.md).

## Integrated prepared-port allocation - 2026-10-17
Claude supplies API then original owner implementation; independent Copilot oracle
receives the frozen interface/contracts and old helpers,never new implementation.
Codex freezes necessary decisions,integrates sound artifacts and executes checks.
No redundant Grok/Argon call; qualifications reused. [Actual receipt](port-owner-evidence.md).

Integrated-owner result: one API call,one implementation timeout,one bounded
qualified-author correction and one independent Copilot artifact; no extra tool
qualification or redundant reviewer. Prefer <=450-line dependent author artifacts
behind frozen private hooks within one accepted milestone. Keep unchanged packet
prefixes stable; append follow-up deltas to preserve potential caching. Test worker
never receives implementation/history during those phases. [Receipt](port-owner-evidence.md).

## Selected-port drawing allocation -2026-10-18
Reuse qualified Claude Opus5.5 low for one bounded coordinator delta and Copilot
Sonnet5.5 low for the distinct independent pixel/routing/refusal oracle. Freeze
the explicit opt-in API/contract first; keep new implementation out of the oracle
packet. No duplicated Grok/Argon call. Codex integrates and executes checks.
[Actual receipt](selected-drawing-evidence.md). Existing no-writes/input/usage rules remain.

## Paired visual workload allocation -2026-10-18
Claude supplies one bounded native C/binding delta; Copilot independently checks
two-port pixels and Mac/PC output from frozen API/contracts only. Both calls
returned artifacts without retries. Smaller exact packets avoid repeating owner
history; Codex integrates rather than recreating sound code. No new Grok/Argon
call or qualification. [Actual results/limits](paired-selection-evidence.md).
