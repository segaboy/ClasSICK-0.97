# IMPL-0033 - Original typed Toolbox dispatcher

SPEC-0029 / ADR-0033 / TEST-0037 / SRC-0139/0140; 2026-10-13.
Codex-assisted project lead; lead self-review, human provenance review pending.
Project-owned GPL-3.0-or-later; no third-party implementation/assets.

Original personality/system1/toolbox.c/.h validates caller-owned typed records,
decodes mixed-width arguments and checked data spans, stages fixed output/result
bytes and completes bounded returning/auto-pop calls. Trusted root ExitToShell
uses the previously reviewed session exit request. Original data structures and
algorithms start from SPEC-0029 and reviewed project interfaces, without new
manual/sample/executable/implementation input. No private heap/graphics state,
callback mechanism or Apple instruction reconstruction is used.

Original tests and assembly/header fixtures exercise the OS/core boundary only.
No manager API/registry expansion, installed stack patch execution, loader,
heap reset, native application/SDK, boot binding, UART body change or reference
run. Preserve thirteen research originals and concurrent re-entry drafts. All
source/role/native/physical/128K/parity/three-edition/B2criterion6 gates remain.
Final execution and compiler evidence is recorded separately, not inferred from
this implementation description or Claude's planned research scenarios.
