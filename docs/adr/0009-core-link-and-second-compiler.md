# ADR-0009 — Current-core complete linkage and a second compiler

- Date: 2026-10-08
- Status: accepted for scoped development verification; self-review
- Sources: owner SRC-0030, tool/interface SRC-0031/0032; ADR-0003/0004/0007

## Context

B1/M0.1 passes, but per-object import checks do not prove a complete link.
One compiler does not establish the intended C portability. No secondary compiler
was found on PATH or in the checked Visual Studio/common compiler locations.
The next roadmap dependency is standalone linkage/runtime and a second core build.

## Decision

Use an original caller-storage integration probe with all four existing core
modules. Compile complete x86-64 and i686 PE link fixtures with Clang/LLD and
pinned portable GCC/GNU ld, at O0/O2, twice each. Exclude standard startup files
and all default libraries; retain every original function. Check core objects,
maps, bounded PE headers/directories and the actual entry RVA; independently
inspect imports with LLVM. Reject unresolved helpers, linked libraries, DLL/TLS/
IAT dependencies, wrong entry and malformed descriptors. An all-zero bounded
GNU ld import terminator is distinct from an imported DLL.

Fixtures use the native PE subsystem and a probe function taking caller-owned
storage. They are never loaded/executed as images. Their entry is not a kernel or
UEFI ABI and supplies no startup stack, memory map, firmware handoff or drivers.
The probe body executes in hosted behavioral tests with guarded storage instead.

Use w64devkit 2.10.0 / GCC 16.2.0 / Binutils 2.47.20260726 as optional pinned
development tooling. Extract the hash-verified package into ignored directories
using the Windows archive reader; do not run its launcher or change system settings.
Existing pinned CMake/Ninja remain authoritative. Run the four portable contracts
and probe under GCC Debug twins/Release/actual i686 with Windows adapters disabled.
Retain the full Clang hosted/sanitizer/debug matrix as separate evidence.

## Alternatives and limitations

Installed MSVC would avoid a download but was not found; a full SDK installation
adds a larger setup. Another Clang distribution would not provide an independent
compiler. GCC Windows viewer compilation remains outside this checkpoint after
an observed bundled HGDI_ERROR signed-conversion warning; warnings are not disabled.

No compiler-runtime helpers are needed by the current linked profile. Do not
silently add libgcc, CRT, libmemory or libchkstk when future modules require them.
Specify/review an original helper or explicitly reviewed dependency before use.
Hosted tests still use their toolchain startup/runtime; no test binary is a product
release. Tool licenses and future linked runtime rights require their own review.

## Verification and review conditions

TEST-0011 checks behavior, ownership, complete symbol/entry/import closure,
negative controls and matching twins per tool/architecture/optimization profile.
Different compilers need matching behavior, not matching machine-code hashes.
This does not prove native execution, other flags, ARM64 linked execution, m68k
runtime/size or any firmware support. Revalidate when core, compiler, flags,
runtime, target ABI or packaging changes. B2 and all physical edition gates remain open.
