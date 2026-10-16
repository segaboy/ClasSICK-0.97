# IMPL-0042 - Paired native drawing workload

2026-10-16; SPEC-0038 / ADR-0044 / TEST-0046; SRC-0219..0222.
Codex owns native workload/bindings, failure controls, native-only closure/tooling
and integration. Claude provides independent original test source from one frozen
contract/API packet; Grok provides one read-only boundary review. Codex reviews
and executes all adopted code/checks. Existing native rectangle semantics and
original legacy fixture are reused without new historical input or UART work.
No private source/manual/asset or emulator implementation is transferred.
[Evidence](../../docs/development/native-drawing-evidence.md).
