# Reproducible Windows 11 development

Status: bootstrap procedure for an ordinary x64 Windows 11 machine with Git.
It uses Windows PowerShell 5.1 and .NET supplied by Windows; PowerShell 7, Python,
Visual Studio, WSL, Docker, a package manager, and an emulator are not prerequisites.

## Minimal initial tools and their purpose

| Tool | Pinned version | Why required now |
| --- | --- | --- |
| LLVM-MinGW UCRT x64 package | 20260908; Clang/LLD 23.1.1 | Native Windows C compiler/linker and Windows headers/import libraries; portable archive |
| CMake / CTest | 4.4.4 | Declare target builds, share presets, run verification |
| Ninja | 1.13.2 | Execute generated builds without Visual Studio or Make |
| Git | Existing; founding build used 2.53.0.windows.3 | Clone, provenance/history review, guard index inspection |
| Windows PowerShell | Built in; founding verification used 5.1.26100.9549 | Inventory, downloads/hash checks, extraction, build orchestration |

Exact URLs, SHA-256 digests and purposes are in
[the toolchain lock](../../scripts/toolchain-lock.json). Digests were checked against
publisher release-asset metadata on 2026-10-07. Setup downloads about **234 MiB**
compressed; reserve at least **2 GiB** for archives, extraction and builds. The
space allowance is a conservative estimate, not an installation requirement probe.

LLVM-MinGW is maintained by its named upstream publisher rather than being the
official LLVM Windows SDK. Its documented hosted capabilities inform this choice
(SRC-0002); m68k and bare-metal SDK support remain separate research items. No
compiler/package source is copied into the public project. Tools and any future
linked runtime components have their own licenses and distribution obligations.

## Clone, prepare, verify

Run in Windows PowerShell. The source and test paths below are the owner's founding
workspaces; another developer can choose different writable directories.

```powershell
git clone https://github.com/segaboy/ClasSICK-0.97.git C:\Repos\ClasSICK-0.97
Set-Location C:\Repos\ClasSICK-0.97
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Inventory-Windows.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Setup-Windows.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Test-Repository.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Bootstrap.ps1 -BuildRoot C:\ClasSICK\bootstrap-local
```

The verification output directory must not already exist. Choose a new directory
for each fresh run; the script preserves previous output. `ExecutionPolicy Bypass`
applies to that process invocation and does not change machine policy. It may not
override centrally enforced organizational policy; resolve that with the machine
owner if applicable.

Setup verifies each downloaded archive before extraction. A SHA mismatch stops
setup. Tools go to ignored `.tools/`; archives go to ignored `.downloads/`. No
administrator rights, registry changes, persistent environment changes, or system
installers are required. It preserves an installation receipt and reuses prepared
tools; receipts do not independently attest every extracted file's integrity.
For a fresh integrity check, use a new checkout and the verified cached archives.

To set up offline, populate `.downloads/` with the exact named archives from the
lock and run `Setup-Windows.ps1 -Offline`. The same hashes are checked. Do not
populate this cache with reference Macintosh binaries or images.

For iterative builds in one PowerShell process:

```powershell
.\scripts\Enter-DevEnvironment.ps1
cmake --preset windows-hosted
cmake --build --preset windows-hosted
ctest --preset windows-hosted
```

Activation changes PATH only in that process. Close it to discard the change.
The verification wrapper restores its original PATH even when a step fails.

## What verification means

The CMake project currently builds a hosted console smoke executable and a
compile-only C11 freestanding width probe. CTest runs the smoke executable. Two
fresh output directories must produce identical executable hashes. Path remapping
and a fixed PE timestamp support that narrow reproducibility check. It does not
claim byte-identical future OS images across compilers, machines or all artifacts.

Results and exact limitations are recorded in [bootstrap evidence](bootstrap-evidence.md).
The hosted smoke links the supplied Windows startup/runtime and standard output
support. That dependency is **development infrastructure**, not part of the planned
freestanding OS core. Future bare-metal links must audit their helper dependencies.

## Tools deferred until a concrete need

| Tool/category | Purpose when introduced | Current state |
| --- | --- | --- |
| LLDB or another debugger | Breakpoints, guest/native state and symbols | Package includes LLDB; interactive debugging unverified |
| ASan/UBSan/fuzzing support | Memory/undefined behavior and decoder robustness | Must validate local target support and add purpose-built presets |
| QEMU plus reviewed UEFI firmware | Repeatable native x86-64 boot tests | Not installed by bootstrap |
| m68k cross compiler, linker and runtime helpers | Original 68000 native images and ROM packaging | Must select/review independently |
| ARM64 toolchain/runner | Native architecture parity | Runner/board unselected |
| Python or other scripting runtime | Needed only if a future generator requires it | No prerequisite; launch aliases are not an interpreter |

Do not install these merely because they appear in the roadmap. Describe purpose,
pin packages, review distribution/provenance, and add reproducible setup first.
