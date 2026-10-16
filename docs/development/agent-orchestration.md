# Agent Orchestration Protocol

## Authority and Standing Scope
Current authority: SRC-0223..0226, extending SRC-0149/0150/0179 to four supervised external CLIs. Codex alone writes repositories. All existing implementation, emulator/hardware/reference, source, release and publication gates remain; each target trial still needs fresh owner approval. Private HQ, credentials, untracked research originals, manuals, prohibited implementation/assets and UART implementation/test bodies are excluded from worker inputs.

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

## Handoff, Transcript & Wiki Hygiene
- **Exact Inputs**: Supply bounded packets with exact commit refs, file paths, and line ranges.
- **Repository briefs**: use `docs/development/tasks/`, exact public revisions and selected functions/sections. Use a frozen public-only input directory when repository-wide access would expose research originals; never grant whole-repository access just to avoid assembling a small brief. State EOF/line counts. Keep the working directory input-only and capture streams/logs/usage in a separate sibling directory, outside the model's permitted paths. Directory discovery cannot reveal a deliberately withheld implementation. Files do not remove context or token limits.
- **Captured Output Only**: Workers return proposed file contents and a concise structured handoff (`result`, `actionable_findings`, `tests_executed`, `blockers`, `decisions_needed` under 180 words).
- **Honest Test Accounting**: Report only actually executed tests in `tests_executed` (empty `[]` in plan/read-only runs). Never count proposed tests as executed.
- **Integration**: inspect accepted artifacts and actual tools, make only necessary corrections, run focused checks plus required final verification, and retain a concise disposition. Stop on unusable output, auth/rate/permission failure or budget exhaustion; retry only for a concrete correctable blocker. Keep elapsed time, provider-reported counters, instruction deviations and integration effort. These counters are not comparable quotas or a measurement of ChatGPT savings.
- **Records & Wiki**: Keep raw transcripts private. Stage documentation updates in the Git wiki queue only; touch OneNote exclusively upon the exact owner instruction `"update wiki"`.

[Current qualification and verification](worker-orchestration-qualification.md);
[dated prior protocol and Claude/Grok qualifications](agent-orchestration-2026-10-15.md).
The present qualification continues SPEC-0038 verification only: no runtime change,
new native target trial, driver implementation, release or historical claim.
