# IMPL-0065 - Original ordered window capture/drain

2026-10-22;SPEC-0063;ADR-0072;SRC-0366..0368;TEST-0076.
Original files:apps/window-scene/capture.h/.c;separate CMake targets and compiler
audits. Claude supplies259 production lines from frozen original/public types;
Codex sole integrator/verifier fixes ready-owner stop cancellation and asserts
power-of-two capacity.16 scalar samples,borrowed existing owner,epoch/no-wrap,
sticky software loss,first-fault retention. No allocation/MMIO/callback/ISR or
direct pointer/window leaf delivery. Foreground routes sample/cancel once via
existing shared owner;producer never dereferences it. GPL-3.0-or-later.
No manual/HQ/research-original/prohibited inputs;no target run or boot binding.
[Actual verification/limits](../../docs/development/window-capture-evidence.md).
