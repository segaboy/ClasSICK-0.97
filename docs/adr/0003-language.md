# ADR-0003 — Primary language direction

- Date: 2026-10-07
- Status: selected for bootstrap
- Sources: SRC-0001, SRC-0006; independent language evaluation

## Context

The original 68000 and modern CPUs need native builds with controlled runtime
dependencies. The workstation initially had no verified compiler. Language choice
must remain revisable with target evidence.

## Decision

Plan a conservative freestanding C11 core and minimal assembly for unavoidable
entry/exception/context/special-instruction work. Avoid mandatory FPU, atomics,
threads and hosted libc services in core. Keep native ABI distinct from guest ABI.

## Alternatives

C++ may improve abstraction but needs a constrained ABI/runtime/size policy.
Rust may improve ownership safety but needs a proven original-68000 target/runtime
and reproducible toolchain. Assembly throughout sacrifices portability.

## Consequences and verification

C requires deliberate bounds/lifetime/overflow discipline and behavior tests.
Audit compiler-generated helpers and link/import maps. Confirm original 68000
output with a separately reviewed toolchain; do not assume LLVM-MinGW provides it.

## Review conditions

Before long-term language lock, measure native m68k size/runtime and modern-target
safety/maintainability. Record any change in a superseding ADR with actual evidence.
