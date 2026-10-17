# TEST-0050 - Native drawing context conformance

2026-10-17; SPEC-0041 / ADR-0047 / IMPL-0045.
Independent oracle/refusal/state/extremes/isolation C suites exercise signed
translation and clips using per-pixel int64 inverse-coordinate membership,
complete pixels/counters and outer guards, not the production clipping algorithm.
Include256 random cases,32,400 offset/endpoint and625 clip cases, byte boundaries,
empty/inverted/no-effect refusals, saturation and independent/rebound/shared ports.
Old native/legacy/failure/presentation tests independently require unchanged scene.

Actual804 local CTests and70 width/optimization/endian audits pass; EFI twins,
27 corruption controls and19 omissions pass unloaded. Worker tests were proposals
and were not executed by workers. [Full receipt](../development/drawing-context-evidence.md).
CI/cross-build results are recorded separately after completion. No target trial.
