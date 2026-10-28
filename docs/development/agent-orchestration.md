# Agent orchestration

Compacted 2026-10-28. Earlier Git text is preserved in the [byte-identical snapshot](agent-orchestration-history-2026-10-28.md) (base `11ad7a9`). Dated observations there are evidence, not current claims about model access or isolation. Later explicit owner amendments take precedence. Any other unsuperseded rule still applies. Do not reread the archive routinely. Read this file, [recordkeeping](recordkeeping.md) and the [final routing plan](tool-audit-2026-10-17/final-plan.md) once, and again when they change.

## Authority

- Conserve total ChatGPT/Codex effort, including briefing/context, review, integration and records. Codex alone selects tasks, decides, writes repositories, integrates and verifies; adopt sound worker artifacts instead of recreating them. Owner resumed approved own-candidate trials under SRC-0521;this protocol itself authorizes no feature,trial or release.
- Standing worker permission: SRC-0149/0150/0179 extended by SRC-0223..0226 to Claude, Grok, Antigravity (`agy`) and Copilot. Workers receive only reviewed public ClasSICK contracts and original project code from the active owner-authorized slice.
- Excluded from all workers: HQ, manuals and manual pages, untracked originals, credentials, held UART bodies, ROMs, media, disassembly, emulator source, and Apple or Microsoft implementation material. Reviewed paraphrases are allowed under the clean-room exposure policy.
- Unqualified external kernel, driver, ABI, loader or runtime code is not assumed safe.
- Do not reinstall tools, buy credits or change subscriptions. No Codex extra agents, new chats or automation.

## Dispatch

- Do small tasks directly when coordinating them would cost about as much.
- Give one worker a substantial bounded primary deliverable. Add a second worker only for a distinct risk or an unresolved gap. The 2026-10-17 four-way audit fan-out is not routine.
- Reuse standing qualifications ([qualification](worker-orchestration-qualification.md), [audit](tool-capability-audit.md)). Do not run repeat qualification calls without cause.
- Never equate provider tokens or credits with ChatGPT quota. Do not claim net savings without supporting measurements.

## Roles (narrow, task-specific)

- Claude: substantial contracts and original C code and tests. Always set effort explicitly. Keep output to 450 lines or fewer, as deltas or exact replacements. Use partial-message capture. Timeouts must match the deliverable.
- Grok: small, precise boundary or defect review. Use low effort and short output.
- Copilot: independent API-only tests, oracles and coverage. Its output is useful but is not authority.
- Antigravity: build and verification scripts, mechanical analysis and documentation drafts. Complex runtime code is unqualified for it.
- Research is an unqualified candidate role. The task must name eligible sources and controls. Worker opinion cannot settle historical claims.
- These are default routes, not exclusive roles. Reuse later task-specific qualifications, including [Copilot's original helper artifacts](owned-window-input-evidence.md); no brand gets blanket kernel/runtime qualification.

## CLI controls (no hard-isolation claim)
Prompt restrictions are not security isolation. Verify the actual control state. If a control is unavailable, refuse the task rather than broaden access. No bypass, autopilot, fleet, all-tools or all-paths flags. Existing services only; never expose credentials.
- agy 1.4.0: ALWAYS Argon4 High: `--model gemini-4-argon-high --effort high`. Verify the resolved model. Never use xhigh, because it resolved to Medium. `--mode plan` has no effect together with `--disable-slash-commands`, so avoid that combination. Keep request-review, deny extra permissions, and use a frozen public-only working directory with transcript audit.
- copilot 1.0.98: `-p`, JSONL, `--available-tools=view`, shell/write/URL denied, built-in MCP, custom instructions, ask-user, remote and update disabled. Use low reasoning and 0 continuations. `--max-ai-credits 30` is a soft cap, not a guarantee. Keep default context, prompt memory disabled, no eager PowerShell resolution, process deadline and usage JSON. The30-credit minimum is no purchase or hard spending guarantee. Historical authentication/model access is not a paid-plan or continued-entitlement claim; plan and allowance stay unknown when unreported. It once made an extra directory listing, so keep work directories free of excluded inputs.
- Claude: established `claude-opus-5-5`, explicit Low effort, bounded print/stream, Read-only permissions with dontAsk, hooks/memory/slash commands/session persistence disabled, empty strict MCP; complete frozen prompt via stdin and audit tools/output. Grok: established `grok-4.7` Low, Read/dontAsk, web and subagents disabled, bounded turns/stream. Use the exact qualified controls in the [routing plan](tool-audit-2026-10-17/final-plan.md), not guessed switches. These are dated routes, not promises of current model availability.
- Details: [final plan](tool-audit-2026-10-17/final-plan.md), [mouse receipt](mouse-input-evidence.md), [controller receipt](pc-mouse-evidence.md).

## Handoff

- Write briefs in `docs/development/tasks/` with the exact commit, paths, line ranges and EOF/line counts. Use a frozen public-only input directory, never whole-repository access. Capture logs and usage in a sibling directory outside the permitted paths.
- Workers return proposed contents plus a handoff of 180 words or fewer: `result`, `actionable_findings`, `tests_executed` (only tests actually run; `[]` for read-only), `blockers`, `decisions_needed`.
- Integration: inspect the output, make only the corrections needed, then run focused checks and final verification. Stop on unusable output or an auth, rate, permission or budget failure. Retry only for a concrete correctable blocker within the bounded call budget; no unproductive retries.
- Before semantic review check schema, declared paths, encoding, completeness and output budget; check a diff against its pinned baseline before applying it. Reuse test evidence only when relevant source/dependency inputs, flags, toolchain and profile identities match. Changed relevant inputs invalidate it; required checks still run. Windows11/PowerShell verification, Linux cross-builds and target execution are distinct.
- Record wait time and active time separately when actually measured, plus provider-reported counters, deviations and integration effort. Unknown values stay unknown. Provider counters/list costs are not subscription charges, balances, causal speedup or Codex quota savings.

## Records
Follow [recordkeeping](recordkeeping.md): one canonical receipt per meaningful slice; update status/roadmap/HQ only for changed facts or required intake, with short references. Keep per-call findings out of AGENTS and this protocol. Keep raw transcripts and private captures outside Git. OneNote and the Git wiki queue are retired. Existing wiki files are frozen history.
