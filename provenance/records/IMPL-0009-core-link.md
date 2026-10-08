# IMPL-0009 — Current-core complete link and second-compiler workflow

Date 2026-10-08. Owner authority SRC-0030, development/interface sources
SRC-0031/0032, ADR-0009, existing SPEC-0001/0003/0004/0005, TEST-0011.
Codex original AI-assisted C/PowerShell/CMake and documentation; self-review,
human provenance review pending. Permitted inputs: existing independent core
contracts/code, public compiler/PE interface prose, tool metadata and declarations.

Paths: tools/core-link/, tests/core-link.c, CMakeLists.txt,
scripts/Setup-SecondCompiler.ps1, gcc-toolchain-lock.json, Test-LinkImage.ps1,
Verify-CoreLink.ps1 and workflow/documentation.

The probe reserves bounded RGBA/mono/typed input storage from exactly 128 caller
bytes, checks literal pixels/padding, clipping, FIFO/source/sequence/overflow,
elapsed clock arithmetic and reset/ownership. Hosted tests independently guard
two regions and invalid capacities. Probe initialization uses volatile stores;
no bulk-helper implementation or external runtime is substituted.

The workflow compiles all four unchanged core modules plus the probe, retains
all eighteen public/probe functions, excludes startup/default libraries and checks
the full link at two widths/optimizations under two independent compiler/linkers.
An original bounded PE/map auditor checks machine, timestamp, entry section/RVA,
library absence and zero imports/TLS/IAT/delay dependencies. GNU ld's all-zero
bounded terminator is accepted; actual imports are rejected and LLVM separately
inspects import descriptors. Negative fixtures are original and never loaded.

GCC core-only conformance disables Windows development adapters through a new
default-on build option, retaining strict warnings. Optional tool setup verifies
archive size/hash, inventories path bounds, stages extraction and checks compiler/
linker hashes. No system environment change, external launcher or new build-system
selection. Existing CMake/Ninja and Clang remain pinned.

No core API/algorithm change, Apple implementation or copied assets. No third-
party runtime code is linked into standalone fixtures; external tools and hosted
test startup remain separately licensed development dependencies. Original project
work is GPL-3.0-or-later. No kernel/firmware ABI, native execution or general helper
approval. Exact tested source/results and failures are retained in TEST-0011.
