# Controlled owned-image execution lead review - 2026-10-15

SPEC-0034 / ADR-0038 finalizes an unimplemented controlled-image execution contract. One generation-bound root, strict guarded accesses/commit, explicit image/inventory authority, balanced return, distinct guest-fault/quarantine states and quiescent retirement precede any execution. X-1 guarded interpreter transactions are the first dependency. Claude authors the first draft; Grok checks one boundary; Codex integrates and remains the sole repository writer. Two bounded CLI calls, no truncated reads. No code/fixtures/TEST/IMPL/build, application/emulator/VM/reference run, native app or boot change. All24 originals/copies, compiled/build/edition/project pointers and both untested System1/ROM boot goals remain.

SRC-0179 owner scope, SRC-0180 Claude draft, SRC-0181 Grok check, SRC-0182 lead
integration. [Final SPEC34](../specifications/SPEC-0034-owned-image-execution.md),
[ADR38](../adr/0038-owned-image-execution.md), [identity/usage record](owned-image-execution-review.json).
Prior compiled99d4a02/public ff3d3f7/HQ ffa34be, CS-EVD-0067/CS-BLD-0037 remain
exact dated evidence:2010 local/2482 CI belong to that implementation, not SPEC34.
No runtime or historical claim is promoted by this contract.

## Worker roles and inputs

The owner asks for better use of platforms and accepts repo-referenced briefs.
[Claude brief](tasks/owned-image-execution.md) gives a primary full-contract draft
assignment from selected SPEC33/SPEC28/API header ranges. It does not duplicate
all source text in a message. Claude performs five bounded Reads in six model
turns. Its1810-word initial draft exceeds the1600-word maximum; private exact
bytes are retained before lead changes. The optional SPEC29 Read is not used;
its claim that typed A9F4 support is absent is therefore checked and rejected.

[Grok brief](tasks/owned-image-execution-check.md) asks only for fetch/write/stop/
retirement risks. It reads the short brief and two100-line draft chunks, then an
unnecessary third chunk past142-line EOF. Four completed Reads in three model
turns;467 words exceeds350. This call remains within its read budget and nothing
is truncated. Future briefs should state the exact end line to avoid that probe.
Neither worker edits/runs code or reads another file, HQ text, raw drafts, manuals,
credentials or prohibited material. Native version queries confirm the known CLI
versions; record only model/tool/usage data, no account/authentication details.
The initial read identities are archived outside Git; the final task brief drops
an irrelevant checkout label. The checked draft's digest is distinct from the
lead-final contract's Git-LF digest. Final amendments are lead-reviewed, not a
third worker pass. Reports are engineering input, not authority/certification.

## Lead dispositions

| Input | Disposition and final behavior |
| --- | --- |
| Claude structure, root-only/RTS/no services, X-1..X-5 ordering | Adopt with changes; distinguish new required policy/session/image/retirement APIs from current behavior |
| Claude entry row address /32-entry table /extent payload | Correct: first row+2,64 rows<=512 bytes, segment payload ends at logical length, not even-rounded extent. Headers/padding/ID/extension opcode starts cannot execute |
| Claude per-access hook alone | Extend: actual instruction-class refusal and proposed A7/SR/target checks must precede staged commit too; register-only escapes and legacy A-line dispatch are not solved by memory filtering alone |
| Claude map/token assumptions | Require return-token RX storage in the original map before image init; bind effective fetch authority without changing non-EXEC map descriptors/flags |
| Claude null dispatch forbids traps | Reject premise after own bus inspection: null dispatch falls back to cs_m68k_aline. Explicit deny before either dispatch path is mandatory |
| Claude absent typed ExitToShell /auto-pop premise | Reject after SPEC29 lines73..116: A9F4 already works through the trusted root driver without auto-pop. Deferring policy-aware integration is still a valid narrow-profile choice |
| Claude conditional parameter writes /invented loaded digest | Reject ambiguity/authentication claim. Only below-A5/stack writable; above-A5 stays protected. Existing SPEC33 has no retained payload digest. Mediation plus truthful immutable-host protocol is not arbitrary payload authentication |
| Claude release of all execution faults /stack scrub | Replace with STOPPED-GUEST-FAULT versus QUARANTINED invariants. Explicit proven quiescent handback may retire the former; quarantine has no recovery. Preserve stack/snapshots; scrub only writable below-A5 before ordinary release |
| Lead sealed inventory /second bridge /entry frame | New explicit X-3 participant and lease API must reconcile registration changes without forging old seals. Associate one bridge per generation, consume before first token write, construct separate startup frame, preserve original caller frame |
| Grok G1 row PC /table width | Adopt; independently agrees with lead corrections |
| Grok G2 logical/padding and alias spans | Adopt complete-byte checks including extensions, conservative distinct-region backing alias refusal, no cross-row/JMP extension opcode entry |
| Grok G3 late effects and rollback | Adopt the required no-effect failing instruction, reject execute-then-rollback mechanism/reachable-current-bug premise: own SPEC22/bus already stages the frame and at most one store. Extend all new policy checks before commit; preserve earlier completed steps only |
| Grok G4 consume state before execution | Adopt single-use claim before token write. Do not label a not-yet-executed image EXECUTED or mutate current image enums; introduce explicit bridge CLAIMED/RUNNING/stopped/quarantined states |
| Grok G5 A9F4, digest and stack | Adopt explicit pre-dispatch denial, existing typed-support fact, rejection of an assumed digest, preserved stack and canonical handback |

The lead reads only relevant original contracts and the original bus tail to
verify actual fallback/staged-commit mechanics. No new historical source or
manual sample is used. All future test ideas remain unexecuted; no fixtures or
code are generated. Native alias/output protection and truthful coordinator
coverage remain preconditions/obligations, not a sandbox or universal pointer scan.

## Usage and next step

Claude Code2.1.287 reports claude-opus-5-5 low effort:25945 aggregate reported
tokens,5093 output including138 thinking, CLI list estimate0.1867988USD. Grok
CLI1.0.52 stable0920c8169e27 reports grok-4.7-build low effort:28753 total including
5816 output/4974 reasoning, estimate0.02041156USD. Exact usage/read ranges/input/
report/final hashes are in the manifest. These are not billed amounts, remaining
subscription quotas, interchangeable platform allowances or a controlled quality/
usage benchmark. Two calls/no retries/no truncation replaces the last slice's
three calls; the deliverables differ, so no percentage-savings guarantee follows.

First propose original X-1 guarded interpreter access/class/control/commit plumbing
and conformance against existing ISA coverage, before session/image integration.
Current APIs/boot artifacts/gates remain unchanged. R-001/AL-2/LQ and prior RF/HL/
RA/ES/PG/MQ/WQ/SQ/OD historical questions, human review, native app, physical/full128K/
parity/three-edition/B2criterion6 and both ROM/System1 goals remain open. Same19
wiki pages queued, no OneNote, extra agent/chat or scheduled automation.
