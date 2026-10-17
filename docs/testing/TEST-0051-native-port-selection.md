# TEST-0051 - Independent native selection conformance

2026-10-17; SPEC-0043 / IMPL-0046 / ADR-0049; SRC-0258..0261.
`tests/quickdraw-select-independent.c` is Copilot's independent contract/API-only
artifact. No implementation or implementation logs supplied, including retries
(none). Codex reviews oracle/assertion quality and executes strict hosted builds,
ASan/UBSan, compiler-width/endian import audits and broken-core controls.
No worker-executed tests claimed. Successful68000 CI is only cross-compilation;
guest CPU/frame/current-cell/owner-quarantine cases remain unexecuted plans.
[Actual results](../development/native-selection-evidence.md).
