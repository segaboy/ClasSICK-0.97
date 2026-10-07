# ADR-0004 — Reproducible hosted Windows toolchain

- Date: 2026-10-07
- Status: selected for bootstrap
- Sources: SRC-0002, SRC-0003, SRC-0004; observed workstation inventory

## Context

Only Windows and Git were assured. Hosted native development should run without
rebooting, undocumented machine setup, or assuming an IDE, SDK, Python or WSL.

## Decision

Pin portable LLVM-MinGW UCRT x64, CMake/CTest and Ninja archives with verified
publisher digests. Prepare them with built-in PowerShell inside ignored checkout
directories. Share build presets; run bootstrap probes out of tree. Do not alter
system PATH/registry. The exact initial versions live in the toolchain lock.

## Alternatives

MSVC Build Tools/Windows SDK is a credible secondary compiler with a larger
installed setup. Standalone Clang needs a Windows library/header environment.
MSYS2 is useful but adds package/environment management. Ad hoc build scripts
do not scale well to multiple target/linker configurations.

## Consequences and verification

Trust depends on publisher release provenance and pinned hashes. Archives are
large and not public repository content. Verify setup under Windows PowerShell
5.1, two fresh native build/run results and identical probe hashes. Hosted startup
runtime is allowed only in the development probe/adapter, not freestanding core.

## Review conditions

Validate debugger/sanitizers, a second compiler and distinct m68k/UEFI/ARM64
toolchains as needed. Windows-targeting compiler support is not native-OS proof.
