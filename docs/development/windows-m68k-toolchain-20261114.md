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

### Direct omission trace and validated enqueue refinement

On 2026-11-15, the existing ROM `6b622b2b...` was traced at its internal
motion-omission field, rather than only the periodically published copy.
Its exact linked ELF has SHA-256 `818ca890f8e1665c01c83b3a75f0fd6cbf143aa34e5a56dcd6ed9b29cc096c9f`;
the Windows 68000 `nm` reports `motion` at `0xb0c` with size 168. A separate
compile of `offsetof` using this profile's SHORT, FAST and LOSS_COUNTS defines
reported offset 116 and structure size 168. The source, generated assembly
and commands are retained in `C:/Repos/.task-temp/mac-motion-offset-20261115-a`.
The resulting `0xb80:4` range is valid for this image only. Its own executable
disassembly identifies the increment/store at `0x4014d0`; no Apple code was
inspected. See `C:/Repos/boothush/docs/mac-mouse-witness.md` for the witness
reproduction and event ordering.

All four directions at 2,048 modeled cycles per edge had 24 internal
increments; the matching 4,096 cases had none. In the first +X omission, the
modeled SCC had a latched pending status across multiple phase changes and the
guest's collected pre/post-ack values differed before the counter write. This
is evidence of a modeled ambiguity, not a physical timing measurement or proof
of a unique cause. Traced and untraced frames and stopped guest results matched.

The inbox's strict and coalescing paths now share an internal validated enqueue,
so a non-merge motion fallback does not repeat metadata validation. The public
argument, epoch and state checks, precedence, exclusion and overflow semantics
remain as in SPEC-0097. Existing tests compare fallback to strict enqueue,
including malformed inputs, full queues and a claimed-record sentinel. With
the Windows native toolchain, two rebuilt ROM twins matched; the new ROM is
SHA-256 `a320e666352666326337d8fa9f41729b68a1a3e054ebec775fad7be4ecc0e4cd`
and uses 47,940 bytes versus the control's 48,008. The build manifest and
source/tool hashes are in `C:/Repos/.task-temp/mac-inbox-single-validation-20261115-a`.

The 4,096-cycle/4,096-guard BootHush paired sequence and replay passed all
eight state and full-frame comparisons in
`C:/Repos/.task-temp/mac-inbox-single-validation-edge4096-20261115-a`.
At 2,048, the title checkpoint still failed, pointer `(256,188)` instead of
`(256,75)` and 71 guest omissions, in sibling `edge2048` output. The prior
control reported 73 on this schedule; neither passes and the difference is
not a speed or physical-hardware claim. Twelve hosted tests passed in
`C:/Repos/.task-temp/classick-inbox-single-validation-host-20261115-c` after
building all named test and freestanding targets. Earlier isolated configure
attempts lacked `nm` and Python respectively; the first test run also omitted
the Mac freestanding object. Those attempts remain in sibling directories and
the corrected run's CTest logs. Release and fast-input gates remain open.

### Unadopted idle foreground polling experiment

With the inbox refinement as control, a separate scratch change skipped a
repeated empty foreground step while the interrupt producer's service counter
had not changed; a saturated counter never skipped. Terminal observation was
still checked. It did not change motion decoding, but would have required
additional concurrency and fault-detection review before adoption.

The source and patch are preserved in
`C:/Repos/.task-temp/mac-foreground-idle-experiment-source-20261115-a`;
the modified source SHA-256 is
`a028ad3f01807f35880b8a035d9c3bd66dbf7b35f44e2373a717b4f3bf1f77bc`.
Windows-built twins and their source/tool manifest are in
`C:/Repos/.task-temp/mac-foreground-idle-skip-20261115-a`; the ROM SHA-256 is
`3c472245ac5327143209f6840dea369dcf06156b9184e60caa77e1686a990161`.
At 4,096 modeled cycles per edge, the eight paired states, pixels and replay
passed in `C:/Repos/.task-temp/mac-fg-idle-skip-edge4096-20261115-a`.
At 2,048 the title checkpoint again failed, pointer `(256,191)` rather than
`(256,75)`, with 70 omissions; its command and failure are in the sibling
`edge2048` directory. The candidate was removed from the working source.
These observations do not establish a physical speed or a fix.

### Unadopted SCC-first interrupt experiment

2026-11-15. To test whether VBL bookkeeping before SCC servicing contributed
to modeled fast-input loss, a scratch build serviced the motion path first
for SCC/combined interrupt entries, then handled the VIA pending state. It
passed the eight paired state and full-pixel checkpoints plus replay at
4,096 cycles per edge. At 2,048, the first title movement still failed:
pointer `(256,181)` instead of `(256,75)`, with 74 guest omissions and no
reported hard loss. The preceding shared-art build reported `(256,189)` and
70 omissions on its own corresponding run; these different outputs do not
establish an improvement or a physical rate.

Experimental ROM SHA-256
`16e4e8b8588b4595cda458bcdcaec411a41b1145e24c6481ddf03a35838440f4`;
BootHush SHA-256
`3aec7ffc0062d950f8f62088baf5c17ed95c70a9f1737776157a5a38568aa7f8`.
Build and input identities are in
`C:/Repos/.task-temp/mac-scc-first-build-20261115-a`,
`mac-scc-first-normal-20261115-a` and `mac-scc-first-fast-20261115-a`.
The original and modified source copies are in
`C:/Repos/.task-temp/mac-scc-first-source-20261115-a`; the working file was
restored byte-for-byte. This experiment changes interrupt ordering and was
not adopted or qualified on physical hardware.

## Interrupt mask and instruction cost investigation - 2026-11-16

Question: how much of the failing fast mouse schedule is spent with the guest
unable to accept another SCC interrupt, and which original routines consume
that interval? This increment implements original SPEC-0097 checked arithmetic
and private-helper inlining, plus a reproducible symbol-correlated analyzer.
It does not change input acceptance, sample validation, queue capacity, device
guards or the release criterion. Source baseline is
`56856406f43314cacb4c57e0f60dbb3029e97ef7`; unrelated local work was preserved.
Codex performed this work without external AI tools, WSL, reference software,
physical trials or prohibited implementation inputs.

### Tool evidence and eligibility

BootHush commits `3b7f25b` and
`a746c1fc79967cc13b62f6087df3a2f25033b14a` add SR-mask observations and optional
instruction boundaries to the existing bounded detailed witness. Its canonical
format and limits are in `C:/Repos/boothush/docs/mac-mouse-witness.md`.
The final Windows active-profile twins produce executable SHA-256
`cd3992737cc78ef5758a376872cac8005e7b959157b4be69349f31141c68fe9b`;
the exact build inputs are in
`C:/Repos/.task-temp/boothush-step-active-20261116-b/report.json`.
Forty-five focused tests pass; two optional external-fixture cases skip.
The original BootHush observer uses the CPU dependency's public SR accessor
declaration; no third-party CPU implementation was copied into ClasSICK or used
to design its inbox change. This is permitted project-owned tool work under
the owner amendment, not admission of third-party implementation or proof of
hardware timing.

The original port title lookup at
`5b285d2bc712f39939d21422e2d7630799f91c85` and final regression record
`c3512be56a44e86b5b896892ba71f54cd87be0bc` provide a separate polling control.
`C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md` records shorter
modeled SCC-read intervals, all 160 phase/boundary tests and the continuing
fast-input losses. Its title stays at IPL7 while polling, whereas ClasSICK uses
interrupts. The transferred finding is this distinction and the measurement
method. No port implementation or game-derived source was imported.

### Retained implementation and checks

`apps/window-scene/inbox.c` retains canonical validation, monotone direction
checks, int32 bounds, button boundaries and strict fallback. Supported compilers
use checked addition and inline the producer's private helpers; the existing
guarded C arithmetic remains the portable path. `tests/window-inbox-motion.c`
adds a separately calculated 64-bit oracle for 6,561 two-axis combinations of
nine signed/boundary values, comparing the entire resulting inbox. All 70,242
checks pass. Thirty focused motion/collector/inbox CTests pass in
`C:/Repos/.task-temp/classick-inbox-single-validation-host-20261115-c`;
the current logs are `C:/Repos/.task-temp/classick-inline-host-test-20261116-a.log`
and `classick-inline-host-build-20261116-a.log`.

Windows-native `-Os` Mac twins in
`C:/Repos/.task-temp/mac-inbox-inline-20261116-b/manifest.json` match at SHA-256
`933fe32575ad2a56e0d5ffe16f5d7bc9c327d856f363e95661f1a2489aa88172`.
The ROM uses 47,916 bytes (28 fewer than control), BSS 488 bytes, workspace
32,768..33,732, and conservative nonrecursive stack bound 4,720 bytes within
the 24,320-byte reserve; largest individual frame is 300 bytes. These are
compiler/linker budgets, not measured physical stack use. All existing image,
alignment, veneer and corruption guards remain active. Build command:

```powershell
python -B scripts/Build-LoadedAppROM.py --tool-root M68K_BIN --windows-native --window-drawing --window-fast-collect --window-short-collect --window-loss-counts --dcd-invert 3 --output NEW_MAC_BUILD
```

The source-matched 286 whole-disk run in
`C:/Repos/.task-temp/pc286-inline-session-20261116-a/report.json` passes all
eight hosted-state/full-EGA comparisons and two pixel mutation controls.
The payload hash is `bb0f402bbf7b9d22d7999f7afc6e44f8defdc74de4acc012af2f349818dfd336`;
disk hash is `a01bb95b6a3840a0a5a889951d75c421bb4823b1189a8cfae1e17d7b4089350e`.
The same native Mac/286 sequence passes all eight 175,104-pixel normalized
canvas comparisons, white margins and Mac replay at edge/guard 4,096 in
`C:/Repos/.task-temp/mac286-inline-4096-20261116-a/report.json`.
At edge 2,048 it still fails the title checkpoint: pointer `(256,197)` instead
of `(256,75)`, 54 omissions, no hard loss, stage 16 and fault zero. The command
and failure are retained in `mac286-inline-2048-20261116-a` and its sibling log.
This is not a fast-input fix or a production 286 boot/installer.

### Same-build timing comparison

Both runs request +32 X pixels at cycle 100,000,000, stop at 105,000,000, and
trace 99,999,000..100,300,000 with inversion 3 / VBL guard 4,096. The original
control ROM is `32d7a5e8c897aae3bdd02ac2dc551c2eca23578bd3e942a2a99d869dbd6ad944`.
Exact commands, JSON, complete frames and traces are in
`C:/Repos/.task-temp/classick-baseline-steps-20261116-a/report.json` and
`C:/Repos/.task-temp/classick-inbox-inline-mask-20261116-a/report.json`.
Every traced/untraced pair matches in complete stopped JSON and frame bytes;
all selected ranges finish with no dropped witness rows and 64 delivered phases.

| Edge cycles | Control longest complete IPL7 span | Revised span | Control/revised final X | Control/revised omissions |
| --- | --- | --- | --- | --- |
| 2,048 | 5,450 | 4,836 | 252 / 252 | 22 / 16 |
| 4,096 | 5,464 | 4,834 | 288 / 288 | 0 / 0 |

Expected X is 288. The measured complete spans fall about 11%; the fast final
position remains wrong. These are schedule-specific instruction-boundary model
observations, not a physical speed envelope or a causal percentage of losses.
The revised fast SCC span still exceeds its 4,096-cycle DCD-change interval.

`scripts/Analyze-MacInterruptWitness.py` checks the ROM/manifest identity,
linked text-symbol addresses, mask-chain continuity, dropped/range flags and
complete step coverage before attributing the longest identified SCC span.
Two synthetic analysis tests include incomplete, censored and inconsistent
evidence controls. Its final result is
`C:/Repos/.task-temp/classick-inbox-inline-profile-20261116-a.json`: 4,836
cycles comprise motion service 2,212, inbox push 1,172, collector 716, interrupt
wrapper 330, veneer 328 and diagnostic increment 78. Calls are charged to their
caller; returns to their callee. This identifies the next optimization targets
without weakening the device checks or assuming a foreground-only cause.

### Preserved, unadopted experiments

Direct sample storage, a single stability comparison, and selective O2/O3/LTO
compilation did not resolve the fast case. The working motion source and build
script were restored before the retained inbox change. Each scratch build has
matching twins, a manifest and a sibling `-source` directory containing all
49 inputs verified against that manifest; no existing source/evidence was lost.
The compiler variants change only `window-entry.c`, `motion.c` and `inbox.c`
optimization; the rest remains `-Os`. LTO also changes the diagnostic layout,
so its results were decoded through its own manifest symbols.

| Scratch build under `C:/Repos/.task-temp/` | ROM SHA-256 | Result |
| --- | --- | --- |
| `mac-direct-capture-20261116-a` | `4a48db6f4432208eb59190dc8274de4b19a2b414b3d32cd9ddbc7eb32ae568d5` | Short trace only; longest observed IPL7 span 5,300; no full parity claim |
| `mac-single-stability-20261116-b` | `107990255e8d2cca915169db5cbad452d0cc5f14b48804c256ec2ccea9676ae7` | Fast +X ends 250 with 20 omissions |
| `mac-input-o2-20261116-a` | `0b9f343152dd1a73e00e632397893c75bb608aebf909118edc0a8c426ee48dae` | Fast +X ends 251 with 25 omissions |
| `mac-input-o3-20261116-a` | `45e1a7e99384c2be1d7fdc52c85a502dae01fd3406c115b4e397967b8a7a8500` | Fast +X ends 255 with 21 omissions |
| `mac-input-lto-20261116-a` | `26b0077ce0e74f7751875015b0199eb6cfd3cd075bf1f3d488ea8ac3248bdf83` | Fast +X ends 250 with 16 omissions |

The four full comparisons pass their slow +X control at X=288 with no omissions
and preserve traced/untraced equality. Their exact commands and frames are in
the corresponding `classick-single-stability-mask-20261116-a`,
`classick-input-o2-mask-20261116-a`, `classick-input-o3-mask-20261116-a` and
`classick-input-lto-mask-20261116-a` reports. An intermediate checked-add-only
ROM (`a60b5b5c716fdf08250ae2167c67e317701247de3a59f718c181c371c5009d3b`)
is likewise preserved in `mac-inbox-overflow-20261116-a`; its independent
instruction attribution identified the remaining helper-call overhead.

The paired milestone remains incomplete. Production media, fast input,
source/tool/provenance review, physical and human qualification and separately
approved publication remain open. The scoped model improvements above do not
waive or satisfy those gates.

### Repository guard control

The indexed guard initially rejected the already tracked original 286
`arch/i286/watcom-arithmetic.asm` (introduced at
`6681abd48060e389716bf7efc4d6b90affe23acc`). Its source whitelist now admits
`.asm` alongside `.s`; UTF-8, NUL, size, provenance and secret checks remain.
`scripts/Test-RepositorySourceTypes.py` tests the real guard using isolated
copies of the index: original assembly passes, NUL-containing assembly and
an executable extension fail. All three controls pass and the real index is
unchanged, as recorded in
`C:/Repos/.task-temp/classick-source-type-guards-20261116-b/report.json`.
The initial control run correctly rejected NUL data but expected the wrong
error stage; that assertion was corrected without weakening the guard.
