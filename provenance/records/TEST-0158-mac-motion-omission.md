# TEST-0158 - Late-edge retry and held-drag motion omission

2026-11-05; SRC-0594 / SPEC-0096 / IMPL-0111. Original source-informed model,
whole-software-graph drag/release/cancel tests, compiler/sanitizer checks and
native twins are recorded in the [canonical receipt](../../docs/development/release-input-20261105.md#late-edge-race-diagnostic-masking-and-motion-omission).
The final native live drag check is incomplete. The model omits ambiguous
displacement deliberately; it is not a lossless capture or hardware proof.
Paired release and prior human failures remain open.
