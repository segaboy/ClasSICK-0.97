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

## First Windows-native Mac build and paired model tests

Later on 2026-11-14 the owner supplied the archive, locally named
`C:/Repos/.task-temp/x-tools-v3-win (1).zip`. Its 110,878,379 bytes match the
publisher's SHA-256 above. A checked extraction into a new directory rejected
unsafe names, symlinks and duplicates and passed ZIP CRC checks. An inventory
independently compared all 1,441 extracted files (262,305,993 bytes) against
the archive before and after a second build; the two inventory reports are
byte-identical, SHA-256
`d2f02846d177e6815cfd5b9054fc3390efc67e6a2b415f90d34a55b18d7be3a5`.
The complete report is `C:/Repos/.task-temp/m68k-windows-inventory-20261114-a.json`;
`scripts/Inventory-WindowsM68kToolchain.py` reproduces the comparison with
explicit archive, extracted root and fresh output paths. The archive includes
GNU compiler and runtime license materials; their presence does not finish
third-party provenance, compiler trust or distribution review.

The Windows native intake identifies `m68k-unknown-elf` GCC 16.2.0, with
front-end hashes in `C:/Repos/.task-temp/m68k-windows-intake-20261114-a.json`.
Two independent invocations each built identical ROM twins from the current
window sources, using `--windows-native --window-drawing --window-fast-collect
--window-short-collect --window-loss-counts --dcd-invert 3`. The 65,536-byte
ROM SHA-256 is `6b622b2b163b86acdd3de54d1f656082ac55874830ebc1d58c204d06f36ef65a`;
the image uses 48,008 ROM bytes, 488 native data/BSS bytes, 964 workspace
bytes, and a calculated conservative stack bound of 4,736 bytes. Those are
build calculations, not a measured physical worst case. The selected link map
lists the project's objects and no `libgcc` or `libc` archive; review remains
open. The first and second complete builds are in
`C:/Repos/.task-temp/mac-windows-candidate-20261114-a` and `-b`; the first
manifest SHA-256 is
`dee2b29fdfe35b2259f71550f5664b16090d2e38bf0e9c1c75f0d9893a5337fd`.
Source fingerprints, flags, symbol map, tool identities, stack frames and
image-audit results are in the manifest and its adjacent files. GCC 16's
output size and behavior must not be assumed equivalent to the previous GCC 13
build merely because both target the 68000.

`scripts/Test-BootHushMac286Parity.py --mac-manifest` now verifies the image,
profile, normalized source fingerprints and diagnostic symbol bounds from the
build manifest, plus guest diagnostic tags. Four altered manifests—image,
source, profile and symbol bounds—were rejected before the guest ran in
`C:/Repos/.task-temp/mac-windows-admission-20261114-a`. This check is an
integrity comparison, not cryptographic proof that an untrusted manifest was
honestly produced. The original pinned-ROM mode also passes its regression.

At default 8,192 and explicit 4,096 modeled edge cycles, the new Mac ROM and
the current 286 diagnostic matched all eight natural-start input/state and
full-pixel checkpoints; the recorded Mac mouse trace replayed identically.
Receipts with commands and identities are
`C:/Repos/.task-temp/mac-windows-pc286-parity-20261114-c` and
`C:/Repos/.task-temp/mac-windows-parity-4096-20261114-b`. The 286 payload is
SHA-256 `9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`.
At 2,048 cycles the paired check failed at the title-position checkpoint:
guest pointer `(256,188)` instead of `(256,75)`, with 73 reported omissions.
The failed run is `C:/Repos/.task-temp/mac-windows-parity-2048-20261114-a`.
BootHush's separate, ready-checked four-direction speed investigation is
documented in `C:/Repos/boothush/docs/mac-mouse-witness.md`.

The port also used this Windows compiler for a separate source-informed native-C
movement diagnostic; `C:/Repos/Not-Super-Tilt-Bro/docs/windows-native-c-evidence.md`
contains its identities and limits. Its game-derived code is not a ClasSICK
implementation input. These Mac/286 results are headless emulator injection,
not human or physical input. Production 286 boot, fast-input behavior,
complete provenance and review, physical/human qualification, source-matched
release packaging and separate owner publication approval remain open.

### Optimization comparison

The native window builder now offers the opt-in `--window-optimize-speed`
experiment (`-O2`); the normal build remains `-Os`. With identical sources,
toolchain, other flags and test schedules, both options produced matching ROM
twins. The `-Os` control remained byte-identical to `6b622b2b...` and used
48,008 bytes. The `-O2` ROM SHA-256 is
`f76c37f793dfd2e2758471f7130a38d700d56a4e1158d66996ce5987c528d696`,
using 59,044 bytes. Both passed all eight Mac/286 checkpoints and replay at
4,096 modeled cycles per mouse edge. At 2,048, `-Os` failed the title state
with pointer `(256,188)` and 73 omissions; `-O2` failed with `(256,183)` and
81 omissions. These schedules and outcomes do not establish that one compiler
option is generally slower, nor a physical threshold. The size-optimized
profile remains the working candidate; neither profile closes fast-input or
release qualification.

Builds and manifests are `C:/Repos/.task-temp/mac-windows-o2-20261114-a` and
`C:/Repos/.task-temp/mac-windows-os-speed-control-20261114-a`. The paired
receipts are `C:/Repos/.task-temp/mac-o2-parity-4096-20261114-a` and
`C:/Repos/.task-temp/mac-os-parity-4096-20261114-c`; failed faster runs and
their logs are `C:/Repos/.task-temp/mac-o2-parity-2048-20261114-a` and
`C:/Repos/.task-temp/mac-os-parity-2048-20261114-b`. Four manifest rejection
controls were rerun against the current `-Os` build in
`C:/Repos/.task-temp/mac-os-admission-20261114-b`.

`scripts/Test-BootHushMacStartup.py` samples the source-matched images' own
diagnostic stages every five million modeled cycles. In this setup the first
sample at ready stage 16 was 65 million for `-Os` and 55 million for `-O2`.
A separate stop-on-stage run observed stage 16 at modeled instruction
boundaries 60,936,878 (`-Os`) and 53,465,154 (`-O2`), within those brackets.
Both had fault zero at all samples. The ready-stage result
shows a startup tradeoff alongside the larger `-O2` ROM and worse fast-input
outcome above; it does not prove real-hardware timing. Commands, capture data,
source/manifest hashes and stages are in
`C:/Repos/.task-temp/mac-os-startup-20261114-b` and
`C:/Repos/.task-temp/mac-o2-startup-20261114-b`.

### Modeled VBL guard comparison

The paired runner accepts an explicit `--vbl-guard` so the input bridge's
guard can be compared without changing guest code. Failed runs retain their
exact emulator command, output and first failed assertion in the fresh output
directory; a pre-existing output directory is never used for that failure
record. On BootHush SHA-256
`370f7a12a31bed9908813686a0cbd6bb197065b5e5d3e443fc48ad843e69f28d`,
the same source-matched Mac ROM and 286 hard-disk session were tested at
2,048 modeled cycles per edge. At the title checkpoint, all four guards
failed the expected pointer `(256,75)`:

| VBL guard cycles | Observed pointer | Guest-reported omissions |
| --- | --- | --- |
| 0 | (256,189) | 76 |
| 1,024 | (256,191) | 74 |
| 2,048 | (256,185) | 74 |
| 4,096 | (256,188) | 73 |

At 4,096 cycles per edge and guard zero, all eight paired checkpoints and
mouse replay passed. The output directories are
`C:/Repos/.task-temp/mac-vblguard-GUARD-edgeEDGE-20261114-b`, substituting
the table values and `2048`, or `0` and `4096` for the passing case. The four
failure records contain `command.json`, `run.log` and `error.txt`; later
checkpoints were not evaluated after the first assertion failure. The passing
run's four Mac-manifest and four media-admission negative controls are in
`C:/Repos/.task-temp/mac-guard-admission-20261114-a` and
`C:/Repos/.task-temp/mac-guard-media-admission-20261114-a`.
Changing the modeled guard did not resolve this fast-input case; these few
schedules are not a physical speed threshold or a sole-cause diagnosis.

### Unadopted diagnostic-copy coalescing experiment

2026-11-15. The question was whether repeated foreground diagnostic copies
under interrupt exclusion materially contribute to fast modeled mouse loss.
An opt-in scratch change delayed only the foreground diagnostic copy until
the next VBL or at most 32 qualifying foreground checks. Terminal faults
remained checked, copied fields remained under the existing exclusion, and no
input or queue rule was intentionally changed. Diagnostic values could be
stale during the delay; this was an experiment, not a qualified trace contract.

The source baseline was commit `f9a0a90ef495d98dfd521c7f6eb6d664cef9af8e`.
The exact original experimental source files and patch remain in
`C:/Repos/.task-temp/mac-observer-experiment-source-20261115-a`; the
`window-entry.c` SHA-256 is
`ace2ce36066dc23ebf36c03c6a9674923e4fec65cdd873bc8558eeaf608b1331`,
and the builder SHA-256 is
`1cef04106fe9ed88ae1909222fb5abb9289e868850065cd419a2bccda0dca543`.
The experiment is not included in the working source. Both builds used the
Windows 68000 toolchain and the same `-Os` short/loss-count profile. Their
complete manifests, flags, source hashes and matching ROM twins are in
`C:/Repos/.task-temp/mac-observe-control-20261115-a` and
`C:/Repos/.task-temp/mac-observe-vbl-20261115-a`. The control ROM remained
`6b622b2b...`; the experimental ROM SHA-256 was
`b6f845d3edc07515b7d2f47ae928672e52955d24ba678da0d1fb5e93e521f9ab`.

With BootHush SHA-256
`370f7a12a31bed9908813686a0cbd6bb197065b5e5d3e443fc48ad843e69f28d`,
both images passed the eight paired checkpoints and replay at 4,096 modeled
cycles per edge. At 2,048, both failed the title checkpoint: the unchanged
control ended at `(256,188)` with 73 guest-reported omissions; the experiment
ended at `(256,194)` with 69. The expected position was `(256,75)`. These
different omission counts do not establish an improvement; neither image
passed the faster case. Commands, run logs and failures are retained in the
`mac-observe-{control,vbl}-edge{4096,2048}-20261115-a` directories under
`C:/Repos/.task-temp`. Initial invocations used an incorrect ROM subdirectory
and stopped before running guests; they created no output directories.

The trial was not adopted. It does not close any release gate, determine the
sole cause of missed phases, establish a physical speed limit or justify
relaxing the existing ambiguity rules. A complete source-level and physical
review would be required before any comparable diagnostic policy could be
used as a release candidate.
