# IMPL-0028 - Original built-in guest memory trap execution

2026-10-13; SPEC-0025 / ADR-0029 / TEST-0033 / SRC-0110/0111.
Codex-assisted original integration; lead self-review, human review pending.
Approved prior project contracts and reviewed published numeric API facts only.
No additional source exposure, external implementation or Apple/Microsoft
code/asset/reference executable used. Claude's callback drafts remain research;
no guest callback convention or executable is inferred from them.

CPU options preserve the strict legacy API and add opt-in data-only low24-bit
resolution with full register retention. Personality-owned typed logical tokens
route eleven routines through the existing semantic/register implementation.
Get/Set remain observable; wrong built-in signatures and guest patches refuse
without execution. Saved originals have an explicit value-frame direct path.
Preflight exception scratch outside low memory/heaps and BlockMove destinations
before any service mutation. Original instruction fixture and independent
assembler encoding oracle exercise the CPU/service boundary.

Paths: core/cpu/m68k-bus.c/.h, personality/system1/memory-traps.c/.h,
apps/guest-heap-probe/program.S/.h, tests/memory-traps.c, compiler verifiers,
CMake and pinned CI integration. No boot entry is linked to the new runtime;
no UART bodies, x86 application, guest callback implementation or native SDK.
