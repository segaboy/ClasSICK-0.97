# ADR-0077 - Admit a pinned compile-only C16 candidate

2026-10-23;SRC-0386..0389. Owner authorizes preparation for the286 port.
Use the pinned Open Watcom2026-10-01 Windows-hosted16-bit compiler as an isolated
compile-only candidate. Verify exact package/member identities;retain its local
license. No installer/system changes,compiler/runtime source inspection,shipped
helper admission,main CMake replacement or new emulator/firmware adoption.

Actual39-case matrix and2 harness refusal controls are tooling evidence only.
Production qualification stays partial:C11 features,helper closure,segmented
guest views,canonical pointer ownership,model/ABI,full maps/fit and actual286
execution/performance remain. Preserve shared semantics and existing target
builds. Next:C16 language/helper and segmented-backend contracts before loader
implementation. [Review](../development/pc286-toolchain-preparation.md).
