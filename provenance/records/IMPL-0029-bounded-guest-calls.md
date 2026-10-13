# IMPL-0029 - Original bounded returning guest-call service

2026-10-13; SPEC-0026 / ADR-0030 / TEST-0034 / SRC-0116/0118/0119.
Codex-assisted original code from reviewed published ISA and calling facts;
lead self-review, human provenance review pending. No Apple/Microsoft code,
manual implementation sample, emulator source or copied asset is an input.

The bus interpreter resolves canonical JMP/JSR control operands separately from
data addressing, checks signed arithmetic and all fetched/target/store spans,
and preserves transaction semantics. A separate bounded-step API adds stack
interval checks without changing the prior options ABI or data-address policy.

The caller-owned service reserves readonly logical return tokens, accepts an
owned ordinary proc0 signature/context, bounds stack and active depth, and
charges attempts to all ancestor budgets. Success requires the expected RTS
and balanced A7; caller controls are restored. Failure restores caller registers
and retains completed guest side effects. No unbounded memory snapshot, guest
pointer/native-function cast, private historical data structure or algorithm.

Paths: core/cpu/m68k-bus.c/.h and m68k-call.c/.h, tests/m68k-call.c,
apps/guest-call-probe/program.S/.h, CMake, compiler verifiers and pinned CI.
The probe is independently authored and assembled as an encoding oracle, not
copied from a manual or Apple application. Internal trusted OS hooks are core
plumbing; no x86 application ABI/header/SDK or native-entry registry is introduced.

Only zero-parameter/no-result ordinary returning calls are implemented. The
minimal legacy interpreter, manager callbacks, marshalling, guest patch routing,
CODE launch, QuickDraw and interrupt/nonreturning classes remain later work.
No boot entry is linked to this service; no emulator/VM run or UART body work.
Claude's six callback/QuickDraw/resource-fork drafts remain unchanged/untracked.
