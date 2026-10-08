# Reproducible Windows 11 development

Status: pinned build and headless core procedure for an x64 Windows 11 machine with Git.
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

The CMake project builds a hosted console smoke, a freestanding width probe, the
freestanding surface library, an optimized surface import-audit object and hosted
conformance tests. CTest runs the smoke, four surface suites and an object audit. Two
fresh output directories must produce identical executable hashes. Path remapping
and a fixed PE timestamp support that narrow reproducibility check. It does not
claim byte-identical future OS images across compilers, machines or all artifacts.

Results and exact limitations are recorded in [bootstrap evidence](bootstrap-evidence.md).
The hosted smoke links the supplied Windows startup/runtime and standard output
support. That dependency is **development infrastructure**, not part of the planned
freestanding OS core. Future bare-metal links must audit their helper dependencies.

For the surface verification matrix, use a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Surfaces.ps1 -BuildRoot C:\ClasSICK\surfaces-local -Sanitizers
```

This runs fresh x64 debug twins (matching test executable hashes), x64 release,
x86 Windows execution, ARM64 little/big-endian compile-only checks and import/data
audits. `-Sanitizers` first verifies intentional heap/signed-overflow detection,
then tests x64 surfaces with ASan/UBSan. Sanitizer runtime/stdio/startup belongs
only to development executables; the audit object is deliberately uninstrumented.
No new tool packages are required. `CLASSICK_SURFACE_SANITIZERS=ON` is the CMake
option after support validation. Core archive bytes are not reproducible yet:
COFF object timestamps differ. See [TEST-0004](../../provenance/records/TEST-0004-surfaces.md).

## Run and validate the original surface viewer

After tool setup, choose a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Presentation.ps1 -BuildRoot C:\ClasSICK\presentation-local -Sanitizers
Start-Process C:\ClasSICK\presentation-local\debug-a\classick_surface_demo.exe
```

The wrapper now runs 24 CTest checks in fresh x64 debug twins, Release, i686 and
validated x64 ASan/UBSan configurations, plus the existing core compile/import
audits. It compares both presentation executable hashes and ends with
`SPEC-0002 verification PASS`. Logs/hashes stay in the chosen output directory.
CTest uses GDI memory targets and hidden windows; it does not require clicking UI.

For manual review: Space switches color/monochrome; holding Space should switch
once per press. Resize larger for sharp integer zoom with a dark centered border;
resize below scene size for centered 1x crop. Minimize/restore and cover/uncover:
the scene should repaint. Escape or Close exits. At different desktop DPI settings
pixels remain integer-scaled; actual cross-monitor changes await manual review.
The two scenes are original geometry, not a reconstructed Macintosh desktop.

The viewer now supplies one host-owned memory region to SPEC-0003 and reserves
its color, mono, scratch and sixteen input records through our bounded core arena.
SPEC-0004 maps Windows Space/Escape messages to core press/release/repeat records;
synthetic input uses the same queue/consumer. Clocks, full input and debugger work
remain open. The 1,342,744-byte development pool is not a Macintosh RAM budget.

For the current complete core verification matrix, use a fresh output directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Verify-Input.ps1 -BuildRoot C:\ClasSICK\core-local -Sanitizers
```

This also compares input-test executable hashes and audits ARM64 little/big-endian
input objects. All current suites run, including actual viewer overflow/recovery,
synthetic/native-message input and memory exhaustion. See [input evidence](input-evidence.md).
The hidden viewer checks can be run individually with --verify-input and
--verify-input-memory. They do not inject input into other applications.

## Tools deferred until a concrete need

| Tool/category | Purpose when introduced | Current state |
| --- | --- | --- |
| LLDB or another debugger | Breakpoints, guest/native state and symbols | Package includes LLDB; interactive debugging unverified |
| ASan/UBSan/fuzzing support | Memory/undefined behavior and decoder robustness | x64 ASan/UBSan verified for surfaces; other targets and fuzzing pending |
| QEMU plus reviewed UEFI firmware | Repeatable native x86-64 boot tests | Not installed by bootstrap |
| m68k cross compiler, linker and runtime helpers | Original 68000 native images and ROM packaging | Must select/review independently |
| ARM64 toolchain/runner | Native architecture parity | Runner/board unselected |
| Python or other scripting runtime | Needed only if a future generator requires it | No prerequisite; launch aliases are not an interpreter |

Do not install these merely because they appear in the roadmap. Describe purpose,
pin packages, review distribution/provenance, and add reproducible setup first.
