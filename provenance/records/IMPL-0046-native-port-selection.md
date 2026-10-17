# IMPL-0046 - Pure native port-selection values

2026-10-17; SPEC-0043 / ADR-0049 / TEST-0051; SRC-0258..0261.
`personality/system1/quickdraw-select.h/.c`: lead-frozen API, Claude original C
proposal reviewed/integrated by Codex; Copilot API-only independent tests.
No runtime dependency, guest-memory access, mutable global or allocator. Original
GPL-3.0-or-later, no third-party implementation or sample code. Architecture-neutral
validation/selection, host flat-span preconditions and borrowed lifetime explicit.
[Artifacts, corrections and actual tests](../../docs/development/native-selection-evidence.md).
