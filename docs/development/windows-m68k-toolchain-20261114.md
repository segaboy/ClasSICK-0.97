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

### Follow-up sampling control

After the retained inbox change, a separate original experiment evaluated the
eligible three-sample stability predicate once in a non-inlined byte helper.
All 30 focused host tests and native image twins passed, but the same-build
timing comparison regressed: longest complete IPL7 spans were 5,312 / 5,310
cycles at edge intervals 2,048 / 4,096. The fast +32 X request ended at X=257
with 25 omissions; the slow control reached X=288 with none. Both traced/plain
pairs matched. This experiment was rejected and `motion.c` restored exactly.

ROM SHA-256 `0de1d1ac21e69dbf11221eac04d286e1e5220415addeb2eace5ac205a34ba48e`;
47,900 ROM bytes. Manifest and all 49 hash-verified source inputs are preserved
in `C:/Repos/.task-temp/mac-masked-stability-20261116-a` and its `-source`
sibling. Commands, states and frames are in
`C:/Repos/.task-temp/classick-masked-stability-mask-20261116-a/report.json`;
host logs are `classick-masked-host-build-20261116-a.log` and
`classick-masked-host-test-20261116-a.log` in the same scratch root. The
previously recorded engineering snapshot remains the retained result.

Inspection of our generated interrupt boundary also identified duplicate
register saves between the all-register veneer and compiler-generated C
prologues. No narrower save mask was adopted: that would require a separate
calling-convention contract and register/SR/stack preservation controls. It
is an optimization candidate, not a proven fix or a changed hardware claim.

## Native interrupt preservation and queue validation - 2026-11-16

This follow-up implements and tests that experiment; the preceding no-adoption
statement describes its earlier checkpoint. Baseline public source is
`86e6d2c74783d661492c08fce44745841a1a02e2`. Codex independently wrote and
self-reviewed the changes from SPEC-0072, SPEC-0065 and the project's own
generated code. No Apple/Microsoft code, protected assets or game-derived
implementation informed the changes. Human provenance review remains open.

BootHush `40b4279` now exposes stopped D0-D7/A0-A7/PC/SR through
`--mac-cpu-state 1`. Its canonical interface, original tests and build receipt
are `C:/Repos/boothush/docs/mac-cpu-state.md`. The tested twin executable is
`C:/Repos/.task-temp/boothush-cpu-state-active-20261116-a/a/boothush-active.exe`,
SHA-256 `a4c51633e9ee1a2a935eae270f54959d2d72f5ea3f6bd8c5a085264904d0edd7`.
BootHush's adapter uses its existing, separately licensed CPU public getters;
no third-party emulator implementation was imported into ClasSICK. Its focused
suite passes 48 tests, with two optional fixture skips.

### Preservation contract and controls

`--window-c-abi-irq` requires SHORT and remains opt-in. The veneers save
D0/D1/A0/A1; the pinned compiled C graph preserves D2-D7/A2-A6. The complete
entry/return contract still preserves all registers and interrupted SR/stack,
raises IPL7 before shared state, passes the same 32-bit source and uses RTE.
The default full-register save set and conservative stack allowance remain.
The builder audits exact native save/restore/mask/call/RTE bytes for all three
veneers. Its alignment check now rejects odd static SP-relative word/long
accesses as well as FP-relative accesses; dynamic addresses remain unproved.

`scripts/Test-MacInterruptABI.py` executes the actual assembly veneers using
original TRAP-generated exception frames and a C stress handler. It checks all
registers, complete SR, source argument, active IPL7, return PC and stack for
three sources, two initial SR values, both save sets and Os/O2: 24 positives.
Eight controls cover missing D1 preservation, RTS instead of RTE, absent IPL7
and an assembly callee that violates D7 preservation. That bad callee is
correctly tolerated by the full wrapper and rejected by the C-ABI wrapper.
The forced compiler frame actively exercises A6, not merely its sentinel.

Final commands, generated original source, disassembly, ROM hashes and stopped
states are in `C:/Repos/.task-temp/mac-irq-abi-controls-20261116-e/report.json`.
Two earlier fixture attempts hit GCC's m68k.cc:5257 internal error with an
explicit inline-assembly A6 clobber. The final fixture instead forces LINK/UNLK
with `-fno-omit-frame-pointer`; normal product builds did not hit that error.
Earlier successful B/C fixture runs did not actively exercise A6 and are not
the final proof. Three invalid-option/macro refusals also pass in
`mac-irq-option-controls-20261116-a/report.json` under the same scratch root.

`scripts/Test-MacInterruptPreservation.py` independently replays the first and
longest complete SCC spans at both tested edge spacings in the actual revised
ROM. Four spans preserve D0-D7/A0-A6, pop exactly six stack bytes, and restore
the exact PC and complete SR read from the interrupted frame. Eighteen
deliberate checker corruptions are detected. The final receipt is
`classick-ready-production-irq-20261116-b/report.json`. Earlier receipts used
register/stack/partial-SR comparison; this one also checks saved SR/PC.
This samples four production paths, not every interrupt, C call or compiler.

### Shared queue cost without weaker admission

The canonical inbox validator now handles common READY directly, then uses
an exhaustive switch with explicit LOSS acceptance and unknown-phase refusal.
Tag, epoch, head, count and claim checks remain. There is no new admission,
state, layout, sampling rule or historical behavior claim. The new independent
metadata table exercises 4,500 phase/boundary/identity combinations and checks
all bytes on refusal. It passes against both the previous and revised code.

The hosted contract now reports 19,025 assertions including that matrix.
O0/O2 pass; four compiled bad-validator controls are detected (unknown phase,
READY claim, LOSS refusal and head bound). Exact compiler commands, sources,
hashes and results are in `inbox-metadata-controls-20261116-a/report.json`;
the driver is `C:/Repos/.task-temp/check-inbox-metadata-20261116.py`.
Thirty focused host tests pass (29 in `classick-ready-host-test-20261116-a.log`
plus `classick-ready-foreground-test-20261116-a.log`), including freestanding,
phase, loss, collection order, alignment and foreground integration checks.

Both revised Windows-native image builds have matching twins, 47,932 ROM bytes,
488 data/BSS bytes, workspace 32768..33732 and conservative nonrecursive stack
bound 4,716 bytes within the 24,320-byte reserve. Build manifests identify the
compiler, tool hashes, all 49 normalized source inputs and native opcode audits.

| Build under `C:/Repos/.task-temp/` | ROM SHA-256 | Save set |
| --- | --- | --- |
| `mac-ready-full-irq-20261116-a` | `e598255c3acc25d62d9f852b454a4d3cd17b1276133afc8ecf15a593b570d8ec` | Default full |
| `mac-ready-irq-20261116-b` | `60554b8af25260af44dd8d430ee64527686496bac7d338ca2a30db4c3ba7e710` | Experimental C ABI |

Before the queue change, option-off twins in `mac-c-abi-off-20261116-a`
reproduced retained ROM `933fe32575ad2a56e0d5ffe16f5d7bc9c327d856f363e95661f1a2489aa88172`.
The initial C-ABI-only twins in `mac-c-abi-irq-20261116-a` have ROM
`c08c4e3f96b98487681fa6627fa92744e1f4de93822c3cef0ae70db83fe9acaa`.
Those earlier manifests predate final builder/specification edits; use the
two revised builds above for current source matching. An omit-frame-pointer
experiment produced exactly the C-ABI-only image, so its flag was removed;
its manifest and all input sources are preserved in
`mac-input-omit-frame-20261116-a` and the sibling `-source` directory.
The first READY experiment omitted diagnostic counters (`mac-ready-irq-20261116-a`);
it is not the like-for-like timing comparison below.

### Measured results and continuing release blocker

All comparisons use this same BootHush executable, +32 X at cycle 100,000,000,
stop at 105,000,000, inversion 3, guard 4,096, trace 99,999,000..100,300,000.
Expected final X is 288. Every traced/plain pair matches complete JSON and
frame bytes, with complete undropped selected witness ranges.

| Variant | Max complete IPL7 cycles, edge 2,048 / 4,096 | Fast final X / omissions | Slow final X / omissions |
| --- | --- | --- | --- |
| Previous full save | 4,836 / 4,834 | 252 / 16 | 288 / 0 |
| C ABI only | 4,660 / 4,658 | 246 / 12 | 288 / 0 |
| READY validator, full save | 4,782 / 4,780 | 254 / 16 | 288 / 0 |
| READY validator, C ABI | 4,604 / 4,604 | 276 / 12 | 288 / 0 |

Reports are respectively `classick-c-abi-off-mask-20261116-a`,
`classick-c-abi-irq-mask-20261116-a`,
`classick-ready-full-irq-mask-20261116-a` and
`classick-ready-irq-mask-20261116-a`, each `report.json` under the scratch root.
`classick-ready-irq-profile-20261116-a.json` attributes the selected 4,604-cycle
span to motion 2,220, inbox 1,108, collector 716, C wrapper 330, veneer 152
and diagnostic increment 78. These are model instruction-boundary costs,
not physical timings or proof that one cost causes every omission.

The fresh 286 hard-disk diagnostic in `pc286-cpu-state-session-20261116-a`
passes eight state/full-EGA cases and two pixel-mutant controls, retaining
payload `bb0f402bbf7b9d22d7999f7afc6e44f8defdc74de4acc012af2f349818dfd336`
and disk `a01bb95b6a3840a0a5a889951d75c421bb4823b1189a8cfae1e17d7b4089350e`.
Both revised Mac images match it at eight states, all 175,104 canvas pixels,
48,896 white margin pixels and complete Mac replay at edge/guard 4,096:
`mac286-ready-full-irq-4096-20261116-a/report.json` and
`mac286-ready-irq-4096-20261116-a/report.json`.

At edge 2,048 both fail the first title checkpoint. Default full-save pointer
is `(256,195)` with 54 omissions; C ABI is `(256,211)` with 40, versus expected
`(256,75)`. Both have stage 16, fault zero and no hard losses. The matching
`2048` directories and sibling logs retain these failures. Fewer cycles or
omissions does not close the fast-input gate; C ABI remains experimental.

Reproduction uses the existing native builder with `--windows-native
--window-drawing --window-fast-collect --window-short-collect --window-loss-counts
--dcd-invert 3`, adding `--window-c-abi-irq` only for that experiment.
Use `Test-MacInterruptABI.py --tool-bin M68K_BIN --emulator EXE --output NEW`,
then the existing probe, attribution and paired runners with the exact commands
retained in their reports. No WSL, external AI service or other emulator ran.

The port independently uses the same observation API at ten real native
movement/jab calls after mouse Start; its canonical receipt at revision
`7138c9c` is `C:/Repos/Not-Super-Tilt-Bro/docs/native-call-preservation.md`.
It checks native game integration and three bad-callee controls. The eligible
transfer is the test method and tool interface, not its game implementation.

Paired-native-preview-2 remains incomplete: fast input, production 286 boot/
installation, remaining target/input qualification, physical and human evidence,
source/tool/provenance review and separately approved publication remain open.
No release candidate, release, physical behavior or playable game is asserted.

### Retained engineering snapshot

After commit `7b1b36d4b474d212ee06b4cb3330e7356278d5b1`, the existing guarded
snapshot preparer accepted all current Mac and 286 source fingerprints and
the slower paired receipt. It prepared
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261116-b` with
the default full-register-save ROM, diagnostic disk, original frames, build/
test records, checksums and conspicuous unqualified markers. The C-ABI image
remains a separate experiment. This snapshot is not a release candidate or
installer; the previously retained snapshot remains untouched. The indexed
repository guard passes 1,819 text/source files, 134 registered PNGs and two
reviewed GIFs; independent provenance review is not established by that guard.

The subsequent [input and post-interaction follow-up](input-followup-20261116.md)
records a small zero-axis cost reduction, the new BootHush later-PC stop,
port recovery-jump feedback and another passing slower paired comparison.
Fast motion and the remaining release requirements stay open.
