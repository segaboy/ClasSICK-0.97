# IMPL-0045 - Native drawing context

2026-10-17; SPEC-0041 / ADR-0047 / TEST-0050; SRC-0240..0243.
Claude supplies the original native view/header and updated small workload.
Copilot supplies independent tests from the contract; Codex corrects test defects,
reviews geometry/lifetime/overflow and integrates build/import/ROM/EFI controls.
No new guest ABI or rasterizer. Old native and interpreted pixel oracles remain.
[Actual evidence](../../docs/development/drawing-context-evidence.md).
