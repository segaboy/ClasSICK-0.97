# TEST-0046 - Native drawing and separate legacy conformance

SPEC-0038 / ADR-0044 / IMPL-0042;2026-10-16; SRC-0219..0222.
Seven native-drawing CTest suites: native, refusal, repeat, legacy, only, faults,
freestanding. Independent rectangle-membership oracle checks all8192 pixels/
1024 bytes, actual FNV/counts, counters, output fields and canaries. Poisoned
re-entry replaces prior state; null calls preserve available objects. Legacy
comparison uses a fresh original resource/CODE fixture and gateway, without
equating loader/native phase meanings. Failure controls replace only the helper
in a dedicated test link and check retained phase/effects and unchanged surfaces.
The native-only target links the workload/service without the interpreter.

Nine compiler profiles, import/width/endian audits, isolated68000 closure/ROM
twins, x86 EFI/corruption/missing-object checks and updated viewer LLDB evidence
are recorded in [results](../development/native-drawing-evidence.md). Whole
regressions include existing compatibility tests; UART bodies are not inspected.
No model-authored test is credited until the lead executes it. No native target
boot, reference/physical/human input or complete OS/parity qualification is implied.
