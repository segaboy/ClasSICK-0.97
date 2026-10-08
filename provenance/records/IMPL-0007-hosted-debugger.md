# IMPL-0007 — Hosted debugger workflow and viewer argument fix

Date 2026-10-07. Owner authority SRC-0025. Existing SPEC-0002 presentation and
SPEC-0003/0004/0005 core contracts; TEST-0009. Author/reviewer: Codex, self-review;
human provenance review pending. AI assistance in original code/tests/docs, using
only current eligible project sources and declared host/tool interfaces.

Paths: apps/surface-demo/windows-main.c, scripts/Test-Debugger.ps1,
scripts/Verify-Debugger.ps1, workflow and development documentation.

Debugger launch exposed raw WinMain '--verify-clock ' rejection (exit 2).
Use the already linked host-runtime argument vector: zero options opens the viewer,
one recognized verification option selects its mode, unknown/extra/empty options
reject before UI. Existing option names and core behavior are unchanged. SRC-0027
records runtime interface prose and existing header declarations; no parser copied.

Original replay creates LLDB command files from existing source names, maps source,
inspects typed clock/queue/viewer state and stack/step transitions, checks child exit
and saves local transcripts/artifact hashes. Disabled startup files and bounded
owned-process launches avoid personal configuration dependencies. The full wrapper
adds debugger replay after the existing conformance/sanitizer/freestanding matrix.

LLDB remains an external tool inside the existing pinned package, not linked core
code. No new install/tool dependency or adopted tool implementation. SRC-0026
discloses interface examples and a diagnostic-only upstream issue. No Apple code,
ROM/source/disassembly, copied assets, private reference payload or legacy fixtures.
Project code/docs remain GPL-3.0-or-later; external tool terms remain separate.

The optional WOW64 trial failed and is retained as a limitation; supported x64
replay passed. No general debugger, historical behavior, B1 or OS-edition claim.
Exact immutable source/result/CI pointers are in the debugger evidence snapshot.
