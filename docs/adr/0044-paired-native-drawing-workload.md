# ADR-0044 - Direct native drawing workload with separate legacy coverage

Accepted2026-10-16; owner SRC-0219, preceding parity clarification SRC-0217 /
ADR-0043. SPEC-0038 / IMPL-0042 / TEST-0046.

One original portable C workload calls the existing fixed rectangle service
directly and drives native PC/Windows and a distinct native Macintosh ROM scene.
Its result and phase meanings describe native work, not resource loading or
retirement. Keep the40-byte interpreted fixture and --drawing ROM as separate
compatibility coverage. Native feature operation has no interpreter dependency.

No generic native image/ABI/SDK or QuickDraw expansion. Retain the512x342 canvas
and input backends. Compile/hosted/link evidence is distinct from actual target
execution. Native candidates are not new boot observations or releases.
[Evidence and remaining gaps](../development/native-drawing-evidence.md).
