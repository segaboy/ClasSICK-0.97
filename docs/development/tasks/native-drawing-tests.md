# Claude task prompt - native drawing conformance

Read only packet.txt in the provided working directory. It contains the lead-reviewed
SPEC-0038 and original public API excerpts. Codex owns implementation/integration.
Write an original C11 test artifact for tests/native-drawing.c, without viewing
the new implementation body. Return one fenced C block, at most240 lines, then
at most150 words listing assumptions. Do not edit or execute anything.

Use the supplied public headers via ../apps/native-drawing/workload.h,
../apps/drawing-rom-app/probe.h and ../apps/service-probe/probe.h. Main accepts
exactly one suite name: native, refusal, repeat or legacy. Return nonzero and
print a useful diagnostic on failure. The lead will compile with strict Clang/GCC
warnings including conversion/sign-conversion,32/64-bit and sanitizers.

Native: independently construct each expected pixel from rectangle membership,
not the production helper's bitmask algorithm. Compare all1024 bytes, counters,
actual hash/black count, phase6/value97/success1 and surrounding object canaries.
Refusal: null arguments preserve each available object byte-for-byte.
Repeat: poison initial port/report, run, mutate/poison again, run again; ensure
deterministic replacement with no stale pixels/counters. Retain both guards.
Legacy: allocate a fresh original drawing workspace, initialize its gateway with
cs_service_probe, run cs_drawing_app_probe, compare its complete pixel image and
actual result/hash/counters with native output. The legacy probe has different
phase semantics; do not equate its loader phases with native workload phases.

Only Read is permitted. No shell, writes, web, MCP, subagents, Git, extra files,
credentials, manuals, HQ, untracked research, UART bodies, Apple/Microsoft code,
ROM/media, disassembly, emulator source or sample code. No VM/emulator/reference
run. Supply original tests only; no historical facts, implementation or stable IDs.
No broad native application ABI/SDK/loader. Stay within four model turns; the
owner explicitly asked Codex to make bounded use of Claude and Grok. State no
test passed: Codex must review and actually execute the delivered artifact.
