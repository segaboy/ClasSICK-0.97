# Native outline copilot assignment -2026-10-18

Frozen public revision `141da1b3895209dc5d82d1df3256206ccaa9930a`; Windows11. Codex sole writer/integrator/verifier. Reviewed public/original contract and API only; no HQ,manuals,research originals,UART,implementation history,web,MCP,commands,edits,delegation or tests. Copilot may read only its prompt. Captures outside Git.

```text
INDEPENDENT ORACLE: Return only tests/quickdraw-outline-independent.c, <=320 lines. You never receive new implementation. Build an independent int64 inverse-mapped per-pixel oracle: map physical pixel back via view offsets and measure distance to original rect edges. Do not copy the production four-strip algorithm. Self-contained main uses stdlib/test libc, no external initialization needed: assign valid cs_qd_view fields explicitly. Tests may call only cs_qd_view_outline, not existing helpers. Cover matrix1-34 including all invalid field boundaries, all pen extremes, empty/inverted geometry and clips, no false clip edge, 380 perimeter pixels, byte boundaries, pattern preservation, saturation/erases, complete view/rect snapshots and outer canaries around view/port/rect (initialize padding before snapshot), two ports interleaving, deterministic >=512 cases (valid and refused) and signed16 extremes/offset extremes. Never manufacture invalid nonnull pointers. Counters expected by oracle saturate separately. Static assertions port size/offsets; view offsets relative pointer size (portable ABI). Count assertions/cases and fail loudly nonzero on any mismatch. No fixtures/files/network. No proposed CMake work. Include concise diagnostic naming. Keep output usable and within320lines.

Return JSON artifact/result/actionable_findings/tests_executed=[]/blockers/decisions_needed.
```

Exact packet hash `7dc38911e07e054306961c4722b1a49c36a7336d3ded3f9e02a96267480e1c70`, 19277 bytes. Inputs: SPEC-0048, quickdraw-rects.h,quickdraw-view.h and Codex frozen quickdraw-outline.h; Copilot additionally receives native-outline-test-plan.md, never implementation/logs. Boundaries and complete contract embedded in packet, EOF explicit.
