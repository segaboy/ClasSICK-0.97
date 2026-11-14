<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Windows-native 68000 build transition

2026-11-14: the owner directs no further WSL use. The Mac/286 interactive
preview remains the release goal; this decision does not waive any release,
human, physical, reference, provenance or publication gate. The existing
Linux-hosted toolchain and its records remain historical.

## Experimental path

[SPEC-0106](../specifications/SPEC-0106-windows-m68k-tool-intake.md),
IMPL-0133 and TEST-0187 introduce an explicit `--windows-native` option in
`scripts/Build-LoadedAppROM.py`. Under that option `--tool-root` names the
Windows compiler's bin directory; `--windows-prefix` defaults to
`m68k-unknown-elf-`. The builder inspects the four frontends before building,
omits the Linux sysroot, retains its existing image audits and twin comparison,
and checks their identities again before writing the manifest. Its separate
`scripts/m68k_windows_toolchain.py` can inspect a candidate and write a new
identity report. This is an experimental entry point, not an admitted compiler.

Hosted tests were run on Windows with the project-local Clang SHA-256
`93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4`:

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$cc = 'C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe'
& $py -B tests/m68k-windows-toolchain.py --cc $cc
& $py -B scripts/Build-LoadedAppROM.py --help
```

The test builds harmless PE fixtures: the claimed m68k target is accepted for
identity inspection, while a wrong target, malformed PE and missing executable
are refused. An incapable fixture cannot create a ROM manifest; wrong-target
refusal occurs before the build output directory is created. This does not test
an actual 68000 compiler, linker or guest.

## Candidate and open work

The NeoGeoSDK project publishes a Windows 64-bit `m68k-unknown-elf` GCC
16.2.0 bundle, `x-tools-v3-win.zip`, in its v1.7.0 release. The release lists
110,878,379 bytes and SHA-256
`18d36e1233b54fb2346e134ac849edc436db4e3f1cd50fdea5435aa02c1e5ba2`:
https://github.com/eaglesoftware777/neogeosdk/releases/tag/v1.7.0 . These are
publisher metadata, not an admission or a verified local archive. A search of
the local ClasSICK and repository trees found no Windows-hosted m68k GCC.
Command-line network access could not resolve GitHub; a background browser
download timed out. No compiler was obtained or executed. The toolchain's
provenance, licenses and any linked runtime remain unreviewed.

Next, obtain and hash-check an eligible Windows compiler outside Git; review
its packaging, executable dependencies and licenses; run the intake; build the
real window ROM twice and perform existing image/stack audits; then execute
the source-matched ROM in bounded headless BootHush tests and compare it with
the 286 candidate. Record the complete archive and subordinate compiler,
assembler, linker, header and runtime identities. Four frontend hashes alone
do not establish a reproducible toolchain. Until those steps and all other
milestone criteria pass, there is no qualified paired release.

The separate Not Super Tilt Bro project continued a branchless mouse-sampling
experiment while this toolchain route was investigated. Its tests and rejected
change are recorded in `C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md`.
The result is a BootHush-model observation and is not a ClasSICK hardware
specification or release result; game code was not transferred.

The port subsequently used BootHush's mouse witness to count multiple modeled
phase changes between successive SCC-A reads during its fast-input case. See
the port's same evidence record for identities, controls and limitations.
That event-count technique may be applied to a source-matched ClasSICK ROM
once available; neither its game-specific addresses nor its observations are
admitted as physical hardware behavior.
