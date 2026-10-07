# Bootstrap verification evidence

Date: 2026-10-07. Classification: **independent local development experiment**,
not an original Macintosh reference-system observation.

## Repository and policy

Public repository: [segaboy/ClasSICK-0.97](https://github.com/segaboy/ClasSICK-0.97).
Founding safeguards were committed and pushed as `d3f3af8` before build probes or
any OS implementation. All source remains text; no Apple code, private reference
payload, tool binary, credential or license file is included.

## Procedure and exact environment

Built-in Windows PowerShell 5.1.26100.9549 ran `Setup-Windows.ps1` and
`Verify-Bootstrap.ps1`; it did not depend on Codex's bundled Node/Python/PowerShell.
The latter built into two previously nonexistent directories beneath the owner's
`C:\ClasSICK` test workspace. The toolchain lock specifies the original publisher
downloads and SHA-256 digests. All three archives matched before extraction.

- Compiler: Clang 23.1.1 from LLVM-MinGW 20260908, native target
  `x86_64-w64-windows-gnu`.
- Build: CMake 4.4.4 and Ninja 1.13.2; C11 extensions disabled, warnings as errors.
- Compile-only probe: fixed-width/byte/pointer integer assertions with
  `-ffreestanding`; this has no OS runtime or historical semantics.
- Hosted probe: native executable printed `ClasSICK 0.97 toolchain smoke: PASS`
  and returned success. CTest passed **1/1 tests in each fresh build**.
- Both fresh Debug executables had SHA-256:
  `0bc09f5298526c6f9a6cd0f616bc9ead98e88a461f349502ae6ce567f0053caa`.
- Setup made no persistent PATH/registry change. Verification restored PATH.

The source files and lock are retained in Git; the commit containing this record
ties the first evidence to the exact bootstrap source. This is a reproducible
procedure plus a narrow executable identity check, not a universal future image
determinism claim.

## Repository guard and clean-clone replay

The staged-index guard passed for 78 source/text files, checking required documents,
local Markdown links, JSON, excluded payload paths/extensions and common credential
signatures. An isolated alternate index using an existing harmless C source blob
under a simulated `.rom` path was rejected as expected. No reference ROM or binary
was created. The normal index was preserved and separately rechecked successfully.
Setup reuse in offline mode also passed.

A fresh local Git clone of the committed bootstrap was made in the owner's test
workspace. Its ignored tool directories were initially absent; only the three
hash-verified downloaded archives were copied as an offline cache. Built-in
PowerShell re-extracted the tools, passed the repository guard, and built/tested
twice under a second source path. Both executables matched the same SHA-256 above.
This produced **four passing native probe builds** across two source checkouts.

## Public publication and CI

The completed documentation and build scripts were published as `a7f028f` after
the founding safeguards. GitHub workflow scope was subsequently authorized through
the provider's device flow, allowing the reviewed Windows workflow to be published
separately. It uses an official checkout action pinned to a verified commit,
read-only repository permission, and the same package lock/scripts as local tests.

The initial [remote CI run](https://github.com/segaboy/ClasSICK-0.97/actions/runs/37669248823)
compiled and ran both probes successfully but rejected different executable hashes.
The difference was reproduced locally when output directories were nested inside
the source checkout. Extracted debug strings contained `./build/.../a` versus
`./build/.../b`: the broad source prefix map took priority over the narrower build
map. Reversing the option order removed those build-specific paths. Two fresh
builds with the nested CI layout then matched the same licensed executable hash
as the external-output layout. The corrected remote run is being verified.
No OS boot or Macintosh compatibility has been achieved. Interactive debugging,
sanitizers, alternate compilers and m68k/ARM64 execution remain future gates.

## Owner-approved license

The owner explicitly selected GPL-3.0-or-later on 2026-10-07 after reviewing the
decision comparison. The standard GPL v3 document was obtained from its FSF
publisher and left verbatim; README/COPYRIGHT.md and source SPDX notices state
version 3 or later. Adding source notices changes debug metadata, so the original
probe hash above remains evidence for the pre-notice bootstrap source; the licensed
revision's fresh-build result is recorded separately below.

After adding the notices, two further fresh Debug builds each passed CTest 1/1 and
produced identical executable SHA-256:
`af1f537373b43aceaccabbba553d3887b8900ea2b0049daf87f38a7db0871802`.
The source guard additionally requires LICENSE and COPYRIGHT.md after approval.
Windows PowerShell download progress is suppressed and each download has a
300-second request timeout; this keeps large CI downloads practical and bounded.
