# IMPL-0012 — Owned x64 terminal exception state

Date 2026-10-08. SRC-0042/0043; SPEC-0008 / ADR-0012 / TEST-0014.
Codex GPT-6 AI-assisted original C/assembly/CMake/PowerShell/tests/documentation;
implementation lead self-review, human provenance review pending. GPL-3.0-or-later.

Paths: platform/pc/x64/state.h/.c, exceptions.S, README; platform/uefi/entry.c and
loader.h; tests/x64-state.c, image audit/build scripts, Verify-X64Exceptions.ps1,
CMake/workflow and records. Existing core/preboot/orchestration algorithms unchanged.

Original byte emitter writes a finalized explicit layout after validating seven
logical ranges. No packed structs or logical-address dereferences. Independent
test bit decoders inspect every field/reserved byte, guarded/unaligned storage,
boundary and overlap failures. Original assembly installs descriptors and captures
the first fault in fixed storage without callbacks or calls; native code is never
run by hosted tests. Image audit checks exact assembly and all vector destinations.

Exposure: Intel public ISA prose/diagrams/instruction pseudocode only, rendered
diagram pages and scoped text extraction. No Intel sample implementation, other
OS loader, external headers or firmware adopted. No Apple source/ROM/executable,
disassembly/decompilation, copied graphics or protected reference input used.
Local rendering uses bundled PDFium after PyMuPDF absence; tooling is not shipped.

Native descriptor/exception behavior, transition-window/NMI/nesting/page-permission
safety and device diagnostics require hardware observations. Current code is an
unloaded terminal scaffold, not a working kernel or completed boot edition.
Exact firmware review remains open. Detailed test/revision results: TEST-0014.
