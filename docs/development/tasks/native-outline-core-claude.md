# Native outline claude assignment -2026-10-18

Frozen public revision `141da1b3895209dc5d82d1df3256206ccaa9930a`; Windows11. Codex sole writer/integrator/verifier. Reviewed public/original contract and API only; no HQ,manuals,research originals,UART,implementation history,web,MCP,commands,edits,delegation or tests. Copilot may read only its prompt. Captures outside Git.

```text
PRIMARY: Return only personality/system1/quickdraw-outline.c, <=180 lines. Implement the whole contract. No tests/header rewrite/build files. No external calls or mutable data even at -O0. Static internal helpers allowed; avoid multiword math/division or synthesized library calls. Simple physical-port pixel iteration with checked int32 geometry is acceptable and bounded. Keep validation ordered. Return a usable core, not pseudocode.

Return JSON artifact/result/actionable_findings/tests_executed=[]/blockers/decisions_needed.
```

Exact packet hash `321dea81ffc9996c3373b477bd3c5d83e8fdf57869cc5c4df8160c0e85fca4f2`, 14171 bytes. Inputs: SPEC-0048, quickdraw-rects.h,quickdraw-view.h and Codex frozen quickdraw-outline.h; Copilot additionally receives native-outline-test-plan.md, never implementation/logs. Boundaries and complete contract embedded in packet, EOF explicit.
