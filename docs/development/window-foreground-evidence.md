# Foreground coordinator and286 side-lane checkpoint

2026-10-23;SRC-0390..0393/ADR-0078. [Priority plan](release-and-pc286-plan.md),
[receipt](window-foreground-evidence.json). Current paired release is primary.

SPEC-0066/IMPL-0068 joins the inbox and dedicated capture helper through abstract
short-section hooks. Every inbox read/change is protected;owner/drawing calls
are outside exclusion. One claimed delivery may finish during newer loss;loss
recovery then cancels once before a later claim. First faults and unread prefix
survive;stop is terminal;invalid/exhausted restart has no descriptor effects.
Failed attempted restart stays closed with actual retained epoch/helper state.
No CPU/SR/ISR/MMIO adapter,ROM/EFI input or boot binding changed.

Actual local verification:338 full Clang64 Debug CTests plus3 GCC64 Release and
3 ASan/UBSan focused checks,total344 profile checks. These repeat profiles,not
344 distinct scenarios. New oracle276 assertions/real owner79 per profile cover
baseline/PRIMING staging,ordered samples,loss during delivery/baseline/cancel,
fault-over-loss precedence,stop/no replay,restart/no-wrap and refusal snapshots.
Real owner verifies pointer/window clamp reversal,drag/loss and held-release
suppression. Three compiled defective variants are rejected,valid control passes.
12 width/endian/EFI-ABI object audits pass with the exact inbox/capture import
allowlist,no mutable globals,delayed EFI-ABI twins and per-function512-byte bound.
Source-pinned Windows/68000 CI is pending;no native size/frame result yet.

The initial API-only oracle and adapted prior real-owner oracle were frozen
before reading primary code. Later integration extends cases;final suite is not
strictly author-blind. Claude's334 combined code/contract lines are integrated
with actual resume-success BASELINE correction and stricter restart/helper-phase
preflight. Its inbox return assumptions are checked against the existing module;
no worker test claim is credited. An initial configure failed because Python was
not on PATH;passing configure explicitly selects the existing pinned interpreter.

SPEC-0067 is the parallel286 outcome:reviewed C16 language/type/mask/shift/divide/
native-chunk/alias/alignment/helper policy. Grok's76-line1133-word draft is adopted
with signed-shift safety,all-target performance,large logical guest-span versus
small native chunk,owned C multiword versus compiler-private ABI,and reuse/no-
redundant-requalification corrections. TEST-0081's8 cases remain UNEXECUTED.
No adaptation header,helper body,link,new286 test matrix,boot or timing claim.

One Claude Opus5.5 low primary125.30s,26214B525line packet,334 lines/533 contract
words;one distinct286 Grok4.7 low/resolvedgrok-4.7-build96.28s,14165B234line packet,
76lines/1133words. Both within caps,zero tools/edits/tests/retry/requalification.
Claude added a prose/fenced-JSON wrapper,removed during packaging. Codex supplies
the frozen API/header,independent oracles,integration corrections and verification.
No manuals,HQ,credentials,untracked originals,Uart or prohibited implementation
were sent. Reported$0.4192408/$0.02194564 list/session costs are not balances or
measured Codex savings;Copilot/Argon idle,all-four live remaining allowances
unverified. Raw transcripts/builds/objects/negative executables stay outside Git.

MacM1-M7,actual CPU ordering/non-nesting/quiescence/latency,whole128K ROM/RAM/
foreground+interrupt stack,PC GUI/human/CWP1 and paired visible qualification remain
open/partial. Software callbacks and hosted models do not prove those properties.
No target trial,release,GIF or new screenshot;no changed visible product behavior.
Old assets/tags/records,24 untracked originals,held brief,Uart and retired wiki
preserved. Native-app,edition/historical,physical,B2,BIOS/Linux and both required
System1/ROM directions remain. Next release work is separately reviewed CPU/SCC
binding and cost accounting;286 moves to the bounded C16 facade/width proof.
