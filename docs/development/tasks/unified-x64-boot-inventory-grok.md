# Grok unified x64 dependency inventory brief

2026-10-21;SRC-0344;one read-only primary. Frozen public revision `fdc6d3e983b55f7a3c3bb38da345ab71e34b7944`;37731 bytes/610 lines;SHA-256 `51c6ad4bfdee9c7e866272f3265b542c6e3861daa7bc85c2d922205afc980bbd`.
Exact selected original input ranges/hashes are in the [receipt](../unified-x64-boot-inventory.json);packet and verbose captures stay outside Git.

```text
You are a supervised read-only worker for ClasSICK 0.97 on Windows 11.
Codex is sole repository writer/integrator. Your single primary deliverable is a usable
architecture inventory draft for one native x86-64 OS payload loaded by BIOS or UEFI.
This is a documentation task, not loader implementation or external research.
Owner goal: one 64-bit PC build/distribution. Broad hardware coverage is a goal;
never promise every PC/peripheral works. Current PC image is UEFI-only. Immediate
paired-native-preview-2 (native Mac68000 + native x64 PC draggable-window/cursor)
remains the checkpoint; BIOS is a longer-term goal, not a new release requirement.
Shared window/input code and per-poll controller receipts are hosted/compiler-verified;
the new window workload is not boot-bound on either target. Existing native-app gate
and exact-image trial/release permissions are unchanged.

Use ONLY the frozen original public inputs below, through EOF. No other file reads,
discovery, commands, edits, shell, web, MCP, subagents, tests or generated fixtures.
No HQ, manuals, research originals, proprietary/third-party implementation, UART
implementation or test bodies are supplied or permitted. Names/imports alone do
not prove behavior. Missing BIOS/media/CPU source review is an open prerequisite;
do not fill it with remembered recipes, interrupts, register values or boot sectors.

Produce JSON with keys draft_md and handoff. draft_md is ready-to-integrate Markdown
(max140 lines,1100 words):
1. Source-pinned current facts: one EFI-linked image, what is firmware-bound vs
   reusable native code, exact selected path/line locators. About ten inventory rows.
2. Proposed neutral handoff boundary: fields/categories, ownership, address/mapping
   validity, CPU/stack/exception readiness, capabilities, failure/lifetimes. Preserve
   existing safety checks; BIOS must not fake successful ExitBootServices. This is
   semantic design only, no binary ABI/layout/format/CPU baseline decision or C code.
3. Small staged migration order that keeps one identical payload and existing
   current-release behavior/evidence. Identify what can proceed without BIOS code.
4. Specific unresolved decisions/prerequisites (including firmware bitness, display,
   PS/2/ACPI dependencies, identity mapping, payload loading/relocation and ownership).
No new runtime/test/BIOS completion claims. Mark suggestions as proposals.
handoff: result, actionable_findings, tests_executed (must be []), blockers,
decisions_needed (total under180 words). Do not repeat supplied source contents.
```

Outcome:completed93.23s,64 lines/1090 words,no observed tools/writes/tests.
Lead adopts with memory/mapping/readiness/gate/migration corrections;no retry or
second provider. [Reviewed inventory](../unified-x64-boot-inventory.md).
