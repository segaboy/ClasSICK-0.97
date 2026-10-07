# Verified SPEC-0001 build snapshot

Date: 2026-10-07. Classification: project verification, not historical behavior.
Executed implementation/build inputs: source revision
`10acecb1ac8bb00ccbfcbc4310c0df1fa69216b2`. The final local matrix exercised the
same code, tests and build scripts before that commit; its remaining additions
record results/documentation. This later evidence commit changes no build inputs.

Tools: existing SHA-256-pinned LLVM-MinGW 20260908 / Clang 23.1.1, CMake 4.4.4,
Ninja 1.13.2. Host: Windows 11 x64, built-in Windows PowerShell 5.1. No added tools.
Local output remains outside repositories; no compiled payloads are committed.

- Two fresh x64 debug builds: all six CTest checks pass in each.
- x64 release, i686 Windows execution and x64 ASan/UBSan: all six checks pass each.
- 8,424 whole-buffer matrix cases per configuration plus literal/error/ownership
  suites. Overflow tests execute with both 32-bit and 64-bit native sizes.
- Deliberate heap-overflow and signed-overflow probes each fail with the expected
  sanitizer report before the actual suite passes. Core and tests are instrumented.
- Optimized freestanding x64/i686 objects and little/big-endian ARM64 ELF objects
  have no undefined symbols or global data. ARM64 is compile-only.
- Debug archive and include dependencies inspected; no core runtime/platform imports.
  Independent negative audit probes reject an external function and mutable global.
- Strict warnings/errors and the 88-file indexed repository guard passed before
  the implementation commit. The evidence-only addition is guarded before publication.

Identical fresh x64 surface test executable SHA-256:
`6c44fa17d22f384f7a9e1837f887f39658df5adf30948fdc23a33b7ce4bcc667`.

The two final debug archive SHA-256 values were:

- `e1e280da7366e2c73e5963347c365f314d3e51d90105d1db6049ea0182c3e0a3`
- `6dc847670760017ed49b4a17c22d9c918e4e0dc94d34ddeb32eadada2eac98c8`

They differ because of COFF object timestamps; archive/OS-image reproducibility
is not claimed. Earlier failed archive-equality and command-parsing attempts are
retained in [TEST-0004](../../provenance/records/TEST-0004-surfaces.md).

Reproduce using `scripts/Verify-Surfaces.ps1 -BuildRoot <fresh-directory> -Sanitizers`
after the pinned setup. CI now runs this matrix in addition to the original
bootstrap verification. Remote CI results, when available, are separate evidence
from this local execution; this record does not predeclare a remote pass.

Inputs and review: owner requirements SRC-0001/SRC-0016, SPEC-0001 v1,
IMPL-0002, TEST-0004; Codex self-review only, human review pending. No external
graphics source/assets, Apple implementation or reference-system input. All
new code/tests/docs are project-owned GPL-3.0-or-later.

B1/M0.1 remains partial. Original-hardware, native-PC without Linux and Linux-PC
boots remain not-started; historical identities, QuickDraw, edition parity,
m68k/ARM64 runtime and a second compiler are unverified. Recommend a separate
Windows presentation adapter contract and original scene next, followed by
arenas/events/clock and the remaining B1 acceptance checks.
