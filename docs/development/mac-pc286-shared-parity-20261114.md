<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Mac and 286 shared-window experiment

2026-11-14. Can the existing shared window input owner and compositor run in
16-bit 286 code and produce the same seven visible states as the existing Mac
ROM? This is a software test and BootHush observation, not a physical-hardware
observation or release qualification. The [owner direction](mac-286-paired-direction-20261114.md)
and [independent implementation policy](independent-implementation-20261114.md)
govern the current work; older blanket emulator-source exclusions in the former
are superseded by the latter.

## Implementation and scope

On base commit `1dcc86c`, the original C16 fixture links the existing surface,
cursor, pointer, mouse, window, product scene and window scene/input sources.
C99-compatible static assertions and field initialization permit the existing
code to compile with Open Watcom's 16-bit compiler. The fixture uses BIOS key
events (arrows move 16 logical pixels, space toggles held state), initializes
the pointer to `(100,80)`, composes a 512x342 canvas, and presents it centered
in 640x350 EGA. The independent test-only loader now reads at most 22,528 bytes.
It is not a production loader or completed PC system.

The first dirty-presentation attempt left a stale cursor after release: the
shared scene's dirty band can be empty when only the cursor moves. The current
adapter also publishes the old and new cursor footprints. Two deliberate
single-pixel mutations, one inside the canvas and one in the margin, are
rejected by the image comparator. The loader regression covers sizes 6, 7168,
7169, 14848, 14849, 22016, 22017 and 22528, invalid sizes and an injected
third-read failure; local receipt `C:/Repos/.task-temp/pc286-three-track-20261114-b`.

The linked test executable uses four arithmetic modules from the locally
extracted Open Watcom `clibs.lib`: `i4d`, `i4m`, `i8d086`, `i8m086`. The
archive SHA-256 is `3d2d2b56c890e4ac6661cee7b85c9aa33369677c0a8ff1ed70f1a711942094f0`;
the library SHA-256 is `045469f2f05f62f8d190b0e1965abc0a1db774b6ca125672f6a1db341b870e03`.
The archive's Sybase Open Watcom Public License and exact linked modules require
source, compatibility and redistribution review before any release use; the
experimental runtime and binary are not admitted to Git or cleared for release.
The test does not measure stack high-water or prove the complete memory budget.

## Model results and reproduction

Pinned WCC `2026-10-01-Build`; host Clang SHA-256
`93303edd5791e4b60682f819267a34c349e5f52d052d067e006e76d21cc82ea4`;
BootHush commit `9628c4c`, isolated executable SHA-256
`5d4f610e98cbb8b03476f28929d6e5c0f64b941aafac3c7d5b6ef12c76c7eeb3`.
The 22,518-byte 286 payload SHA-256 is
`5303c8611edc3c48d215ff98854eb62d82a82706ad334dd75b6e36ec02ad161f`,
and test floppy SHA-256 is
`630e6d0dce14e2743b78cf8b4525f5ced1c11fbd83f7c94424b656f3b7edec0e`.
Seven separate cold runs compare state, every canvas pixel and all 48,896
white-margin pixels with a same-source hosted compositor. The disk hash is
unchanged. Source hashes, build/link commands, maps, key schedules, frame
hashes, complete guest results and mutation checks are recorded in
`C:/Repos/.task-temp/pc286-shared-session-20261114-e`.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$bh = 'C:/Repos/.task-temp/boothush-replay-series-20261114-a/boothush.exe'
& $py -B scripts/Test-BootHushPC286SharedSession.py `
  --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain `
  --runtime-library C:/Repos/.task-temp/pc286-compositor-link-20261114-a/clibs.lib `
  --host-cc C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe `
  --emulator $bh --output C:/Repos/.task-temp/NEW-286-SESSION
& $py -B scripts/Test-BootHushMac286Parity.py --emulator $bh `
  --mac-rom C:/ClasSICK/mac-candidate-anchored-20261112-a/classick-live.rom `
  --pc-results C:/Repos/.task-temp/NEW-286-SESSION --output C:/Repos/.task-temp/NEW-PAIRED-CHECK
```

The Mac ROM, built earlier from `0f8d85df`, has SHA-256
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`.
With injected Mac window messages and scheduled 286 keyboard input, the
following observed states match, including all 175,104 normalized pixels:

| Checkpoint | Pointer | Window origin | Pattern | Held | Presses |
|---|---|---|---:|---:|---:|
| initial | 100,80 | 80,70 | 0 | 0 | 0 |
| pressed | 100,80 | 80,70 | 0 | 1 | 1 |
| moved | 132,80 | 112,70 | 0 | 1 | 1 |
| released | 132,64 | 112,54 | 0 | 0 | 1 |
| post-release | 148,64 | 112,54 | 0 | 0 | 1 |
| content | 148,112 | 112,54 | 1 | 1 | 2 |
| content-release | 148,112 | 112,54 | 1 | 0 | 2 |

The Mac reported no loss/omission/fault, 12 mouse samples, 540 modeled steps
and no pending work. Its recorded trace SHA-256 is
`01e3b01a4ead7a8ce2a9b167f1990309be463416b5ecb9156ff2500b68b8cbc9`;
replay reproduced all seven frame bytes, sampled memory and CPU boundaries.
The complete commands, reports, hashes, trace and captures are in
`C:/Repos/.task-temp/mac-pc286-parity-20261114-c`. BootHush's independently
tested replay-capture facility is recorded in
`C:/Repos/boothush/docs/mouse-replay-series-20261114.md`. The port independently
used it for title input, see `C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md`;
game source is not an implementation input here.

The original Mac image and experimental 286 payload are different builds, not
a source-matched candidate pair. The hosted reference shares implementation
and is not by itself independent correctness evidence. The sampled sequence does
not cover all bounds, cancellation, rollover or motion speeds. The separate
host CTest run passed 9/9 scene, cursor and window tests in
`C:/Repos/.task-temp/classick-host-20261114-c`.

Production 286 boot/presentation, runtime admission, full budgets, input and
performance qualification, matching candidate builds, E1-E4 dispositions,
human/physical trials, provenance review, packaging and separate publication
approval remain open. Do not treat this experiment as a completed milestone.

## Narrow surface arithmetic follow-up

After the preceding recorded run, surface validation selects `uint32_t` as its
wide intermediate only when both `size_t` and `ptrdiff_t` maxima fit in it;
larger hosts retain `uintmax_t`. Positive dimensions are int32, and the RGBA
multiplication is preceded by the existing `SIZE_MAX / 4` guard. This removes
the two 64-bit runtime modules from the current 286 link; `i4d` and `i4m`
remain, and the same runtime review remains open. The new 21,084-byte payload
SHA-256 is `8a33cfde760dd130807688bc92edb80e6c64aa7f1af222023f89f2923d555122`,
and floppy SHA-256 is
`050cef45dffecb66bf8e531d62a09132dbead2b8030d64c5cecadfb97ded20d7`.
The same seven state/image/margin checks and replay passed; receipts are
`C:/Repos/.task-temp/pc286-shared-session-20261114-f` and
`C:/Repos/.task-temp/mac-pc286-parity-20261114-d`. A separate static-compositor
comparison is `C:/Repos/.task-temp/pc286-shared-compositor-20261114-b`.

The real C16 `cs_surface_init` additionally passed 13 boundary cases in
BootHush, including row-size, stride, storage and `PTRDIFF_MAX` boundaries.
An intentionally incorrect expectation reached its failure marker, proving
the fixture detects a mismatch. Exact commands, inputs, image hashes and
results are in `C:/Repos/.task-temp/pc286-surface-boundaries-20261114-a`;
reproduce with `scripts/Test-BootHushPC286SurfaceValidate.py` using the same
tool root, runtime and BootHush paths as above and a new output directory.
The 16 relevant hosted CTest cases (surface, numeric model, compositor, cursor
and window) also passed in `C:/Repos/.task-temp/classick-host-20261114-c`.
The numeric model is independent but does not by itself cover every C16 input;
the physical and release limits above are unchanged.

## Original arithmetic helpers, no runtime library

The next experiment removes the third-party runtime from this particular
286 window build. `arch/i286/watcom-arithmetic.asm` independently implements
the compiler-emitted 32-bit multiply and signed/unsigned divide helpers using
the observed register convention. Unsigned division uses two 16-bit divides
when the divisor fits, and a restoring algorithm otherwise. Zero division and
signed `INT32_MIN / -1` are outside the C operations' defined domain. This is
not a claim that the compiler's entire runtime ABI is implemented.

`scripts/Test-BootHushPC286Arithmetic.py` builds a guest without a runtime
library and checks 259 directed and deterministic pseudorandom operand pairs
against independently computed Python integer products, quotients and
remainders, including negative and boundary operands. The deliberate wrong
expectation fails as intended. The current assembler, extracted as a tool
from the previously identified archive without running its installer, is
`2026-10-01-Build`, SHA-256
`9e779dda024c6cee24943f917b0f761ce862951caec0a4c0fbadbc760895c0cb`.
The complete original helper source SHA-256 is
`2e58dc63e94fc66c57b4af1173cc2876e8e7f00d5cd97164e2c8ec4865574d60`;
the oracle receipt is `C:/Repos/.task-temp/pc286-arithmetic-test-20261114-c`.
No third-party runtime implementation source was consulted or incorporated.

The full 286 window fixture links with `nodefaultlibs`, the original helpers,
and no runtime library; the link map lists no `clibs.lib` modules. Its
21,066-byte payload SHA-256 is
`a7e808a78eb2f413806cb71e868b1ffe6316891304245b95a8f47d4dfe82db1b`,
its floppy SHA-256 is
`65e79893261b25602e63019c542423fed535d8042a398a4831258a4238b5c1ac`.
All seven same-source 286 comparisons, all seven Mac/286 checkpoints and the
Mac replay pass. Receipts are `C:/Repos/.task-temp/pc286-shared-session-20261114-h`
and `C:/Repos/.task-temp/mac-pc286-parity-20261114-e`. The 13 C16 surface
boundary checks also pass without a runtime library in
`C:/Repos/.task-temp/pc286-surface-boundaries-20261114-b`.

The helper's initial unoptimized division raised the modeled initial-render
checkpoint from 4,836,929 to 6,526,306 cycles; the 16-bit-divisor fast path
reduced it to 4,915,331. These are model cycle counts for these particular
test images and inputs, not a real-time usability qualification. The helper,
compiler output, platform bindings, memory/stack budget, production boot and
all other applicable milestone gates still require their own review and
evidence. The earlier runtime-library licensing hold applies to those earlier
library-linked experiments, not to this runtime-free test payload.

To reproduce, substitute `--assembler C:/Repos/.task-temp/pc286-independent-arith-20261114-a/wasm.exe`
for `--runtime-library` in the shared-session command above. Use the same
`--tool-root`, `--assembler`, `--emulator` and a fresh `--output` for
`scripts/Test-BootHushPC286Arithmetic.py` and
`scripts/Test-BootHushPC286SurfaceValidate.py`.

## Explicit uninitialized-data startup

The prior diagnostic loader did not explicitly clear the linked BSS and could
therefore accidentally rely on BootHush's initial RAM state. Its new optional
bounded startup clear starts after the initialized payload and ends at the
linked BSS end. It leaves at least 4,094 bytes below the initial stack pointer;
that bound is a guard, not a measured stack requirement or a complete memory
budget. A test-only option first poisons this range before loading. The window
runner reads the actual BSS bounds from its link map and now hashes the loader
and relevant test scripts along with the sources.

The unchanged 21,066-byte original-helper payload has SHA-256
`a7e808a78eb2f413806cb71e868b1ffe6316891304245b95a8f47d4dfe82db1b`.
Its poisoned-then-cleared test floppy SHA-256 is
`949545f33c73fc54773fb4f4d15228ed384cb86be1712d8298dab0565ac88ae9`;
its BSS starts at offset 21,066 and spans 21,936 bytes. Seven hosted/286
comparisons and seven Mac/286 comparisons with replay pass; exact receipts
are `C:/Repos/.task-temp/pc286-shared-session-20261114-j` and
`C:/Repos/.task-temp/mac-pc286-parity-20261114-f`.

The independent loader regression checks every byte of a 21,994-byte poisoned
range after clearing and a deliberately mutated clear that leaves it at `a5`;
both distinguishable outcomes, eight payload sizes, an injected third-read
failure and seven invalid arguments pass. The input images remain unchanged.
See `C:/Repos/.task-temp/pc286-three-track-20261114-c` and run
`scripts/Test-BootHushPC286ThreeTrack.py` with the recorded executable and a
new output directory. This improves one diagnostic's startup contract; it does
not establish general BIOS behavior, stack high-water, physical operation or
a production loader.

### Modeled stack observation

The test-only loader can additionally fill the available near-data/stack gap
with a chosen byte before switching to its new stack. The bounded eleven-key
window sequence was run twice with different fill bytes (`5a` and `a5`) and
the whole gap was inspected in two cold runs per fill. Both reached the same
final frame as the uninstrumented sequence. The earliest changed offset was
`0xfbde`, 1,056 bytes below initial SP `0xfffe`; 21,476 bytes between the BSS
end `0xa7fa` and that offset remained at the fill value in each run. Both runs
stopped at modeled cycle 56,238,947. This is an observed touched span for
these inputs and emulator firmware, not a worst-case stack bound: writes equal
to the fill byte can be invisible, untested paths may use more, and physical
interrupt timing is not represented. It does not close the memory-budget gate.

The images, full ranges, exact commands, hashes and results are retained in
`C:/Repos/.task-temp/pc286-stack-window-20261114-a`; run
`scripts/Test-BootHushPC286Stack.py --emulator $bh --pc-results`
`C:/Repos/.task-temp/pc286-shared-session-20261114-k --output`
`C:/Repos/.task-temp/NEW-STACK-TEST`. The associated loader regression is
`C:/Repos/.task-temp/pc286-three-track-20261114-d`, with nine invalid inputs.
The matching Mac replay receipt is
`C:/Repos/.task-temp/mac-pc286-parity-20261114-g`.

### Input-rate characterization

The same eleven 286 key presses, each with its corresponding break sequence,
were scheduled with starts 5,000,000, 1,000,000, 500,000 and 250,000 modeled
cycles apart. Every run reached the same final state and all final pixels;
the respective stop cycles were 56,238,947; 18,472,973; 17,713,107; and
17,713,107. At the shortest period the last make was due at 8,500,000 cycles,
so the final response followed roughly 9.2 million modeled cycles later. This
indicates a modeled backlog in this scenario, not an established physical
latency or usable-input-rate threshold; intermediate responses and physical
keyboard rollover were not qualified. Exact schedules, JSON, images and hashes
are in `C:/Repos/.task-temp/pc286-input-rate-20261114-a`. Reproduce with
`scripts/Test-BootHushPC286InputRate.py` using the same executable and
`pc286-shared-session-20261114-k` plus a fresh output directory.

### EGA copy measurement and improvement

Optional diagnostic POST markers isolate the first 16-pixel drag into shared
input/scene work and EGA publication. The instrumented pre-change fixture
spent 1,271,624 modeled cycles in the shared sample and 833,782 in publication
(`C:/Repos/.task-temp/pc286-timing-20261114-a`). Its source and image identity,
build options, matching seven checkpoint frames and schedules are in
`C:/Repos/.task-temp/pc286-shared-timing-20261114-a`. Intervals include marker
overhead and are BootHush estimates, not physical measurements.

The EGA adapter now computes the source and destination row addresses once per
row instead of repeating multiplications for every byte. The same instrumented
drag measured 1,271,624 shared-sample cycles and 400,139 publication cycles,
about 52% less publication work, in `C:/Repos/.task-temp/pc286-timing-20261114-c`.
Seven instrumented state and full-pixel checks still pass in
`C:/Repos/.task-temp/pc286-shared-timing-20261114-c`. An intermediate version
(`-b`) measured 398,591 cycles but was revised to avoid computing a source
pointer beyond the array after the last row.

The uninstrumented 21,088-byte payload SHA-256 is
`6a6b14dc8991918c045cef0a1f3bc0a5fd95ca85adde37034ec761f5d2263c79`,
and test floppy SHA-256 is
`4974fbea22221ccb26ea2af90a13aec4a8bc847ee8beb958e3caa7ce703df845`.
Seven same-source 286 and seven Mac/286 comparisons plus replay pass in
`C:/Repos/.task-temp/pc286-shared-session-20261114-m` and
`C:/Repos/.task-temp/mac-pc286-parity-20261114-i`. The two-fill stack test
again observes a 1,056-byte touched span in
`C:/Repos/.task-temp/pc286-stack-window-20261114-c`.
The four input-rate cases all reach the same final state and pixels in
`C:/Repos/.task-temp/pc286-input-rate-20261114-c`; the fastest sequence
finishes at cycle 15,523,853 instead of 17,713,107 before this change. A
backlog remains. No physical speed, worst-case stack, user-acceptance or
production-release claim follows.

Reproduce by running `Test-BootHushPC286SharedSession.py` with the original
assembler as above and `--timing-markers`, then run
`scripts/Test-BootHushPC286Timing.py` with that result directory, `$bh` and a
fresh output directory. Repeat without the flag for the uninstrumented checks.

The actual optimized adapter was separately compiled into eight original
boundary fixtures: full canvas, all four corners, negative clipping, a
byte-crossing rectangle and an empty rectangle. For each, an independent
Python oracle checks all 224,000 EGA pixels including the white margins and
byte-aligned update extent; one changed canvas pixel and one changed margin
pixel are each rejected. Source, tool, image and frame identities, commands
and results are in `C:/Repos/.task-temp/pc286-present-edges-20261114-b`.
Use `scripts/Test-BootHushPC286PresentEdges.py` with the same pinned tool root,
assembler and emulator, `--pc-results` pointing to the uninstrumented session
above, and a new output directory. These are emulator observations, not an
exhaustive geometry proof or a physical display test.

## Natural initial position

The preceding seven-state 286 fixture deliberately pre-positioned the pointer
to `(100,80)` in code, so it did not prove that the initial visible position
matched the Mac. An opt-in `--natural-start` build now leaves the shared
owner's initial `(256,171)` position untouched. In separate BootHush runs it
matches the existing Mac ROM's pointer, `(80,70)` window, zero presses/pattern,
all 175,104 canvas pixels and all 48,896 white-margin pixels. The hosted
reference also matches. Mac stop was cycle 100,000,002; 286 POST `b0` was cycle
3,935,711. The 21,034-byte 286 payload SHA-256 is
`4b9c6465265bb8f2c31557fb1f30449fdb6160b81783862c30cfe03047186332`,
test floppy SHA-256
`27ebcf1e36f3a35417af0b75d33590dcce4f44815e823b5b6acd80ee4c3e6f16`,
and Mac PBM SHA-256
`2c952f820d085441c34e5e8d4b63852c4c223dab52002a8b9ad01d7e4b24361b`.
Receipts and exact commands are `C:/Repos/.task-temp/pc286-natural-start-20261114-a`
and `C:/Repos/.task-temp/mac-pc286-natural-start-20261114-a`.

The pre-positioned seven-state sequence was rerun separately against the same
current sources in `C:/Repos/.task-temp/pc286-shared-session-20261114-n` and
`C:/Repos/.task-temp/mac-pc286-parity-20261114-j`. These are two distinct 286
builds; the subsequent continuous test below addresses the intervening
scripted input path. To repeat, add `--natural-start` to
the shared-session build and use its output with
`scripts/Test-BootHushMac286NaturalStart.py`; use fresh output directories.

The Mac image is still from the earlier source revision. On this workstation
`wsl.exe --list --quiet` returned `WSL/E_ACCESSDENIED` in the current execution
environment; the existing Mac build script expects a Linux m68k cross-toolchain.
This check does not establish that no alternate permitted build environment is
available. A fresh source-matched Mac build, broader paired input, production
286 binding, remaining qualification and separate publication approval are
still required.

### Continuous modeled interaction from initial state

The natural-start fixture was extended to accept a 15-key sequence from the
center position: move to the title, hold, drag right and up, release, move to
content, press and release. Eight 286 cold runs check successive prefixes of
the same sequence; the last executes all fifteen keys in one boot. The actual
Mac ROM receives corresponding injected window messages in one run. Pointer,
window, button, counter, pattern, all 175,104 canvas pixels and all 48,896
286 margin pixels match at all eight checkpoints. The recorded Mac mouse
trace replays with identical capture cycles, memory and images. The 286
fixture is still diagnostic and the Mac ROM is from an earlier revision.

With BootHush's default 8,192-cycle edge setting, the successful run is
`C:/Repos/.task-temp/mac-pc286-natural-parity-20261114-d`; its trace SHA-256
is `11a57a8a24dc166ebb39209a3f317be243685f64a463813f9cb5ec9f4eada495`.
Equivalent runs using 16,384 and 32,768 modeled cycles per edge are in
`mac-pc286-natural-parity-20261114-e` and `-c`. Their shared 286 results are
`C:/Repos/.task-temp/pc286-natural-session-20261114-b`.

The earlier `mac-pc286-natural-parity-20261114-a` used closer event/checkpoint
spacing: at the drag checkpoint the Mac pointer/window were `(272,73)` /
`(96,68)` rather than `(272,59)` / `(96,54)`, despite zero reported loss and
pending steps. Its next capture at 144 million cycles did show the expected
position, window and released state, and its later content state was correct;
the intermediate checkpoint still failed. The subsequent 32,768-cycle attempt `-b` sampled the first
96-pixel movement before its scheduled steps could finish, so it is invalid
as a settled comparison. The passing runs allow more time between events and
checkpoints. These results do not prove a particular cause or a physical speed
threshold; they leave responsiveness and real input qualification open.

The 286 source now defaults to the natural shared position; only the explicit
`CS_P286_PREPOSITION_TEST` build option selects the older aligned fixture.
Both variants were rebuilt and checked. The current natural run and Mac replay
are `C:/Repos/.task-temp/pc286-natural-session-20261114-c` and
`C:/Repos/.task-temp/mac-pc286-natural-parity-20261114-f`; their payload and
media hashes are unchanged from the natural test above. The pre-positioned
regression is `C:/Repos/.task-temp/pc286-shared-session-20261114-o`.
The eight EGA edge checks and two pixel-mutation controls were repeated on
the current source in `C:/Repos/.task-temp/pc286-present-edges-20261114-c`.

## Reusable 286 EGA adapter

The original EGA byte-publication code now lives in
`platform/pc286/ega-mono.c`, under the explicitly bounded
[SPEC-0101](../specifications/SPEC-0101-pc286-ega-mono-present.md), and the
diagnostic calls that shared platform implementation. Its caller still sets
the mode and supplies valid, nonoverlapping buffers. This advances the actual
platform binding; it is not a production boot or hardware qualification.

Thirteen hosted rectangle cases compare all 28,000 output bytes against an
independent intersection oracle, check guards and null refusals. Compile and
run from the repository with the recorded local Clang:

```powershell
$cc = 'C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe'
& $cc -std=c99 -O2 -Wall -Wextra -Werror -Wconversion -Wsign-conversion `
  tests/pc286-ega-mono.c platform/pc286/ega-mono.c -o C:/Repos/.task-temp/NEW-ega-test.exe
& C:/Repos/.task-temp/NEW-ega-test.exe
```

Eight 286 display-edge fixtures with two mutation controls pass in
`C:/Repos/.task-temp/pc286-platform-edges-20261114-b`. The runtime-free
21,084-byte natural-start payload SHA-256 is
`4c2fd6cdf15bef2422faf9fd07e818187964069ff937654b493cf123af8678ec`;
its test floppy SHA-256 is
`2bf9b93d36c473e8224cff6a9fbcbd1862b64d76d83909a470b28affbd71ad50`.
All eight hosted/286 checkpoints and the matching Mac replay pass in
`C:/Repos/.task-temp/pc286-platform-session-20261114-b` and
`C:/Repos/.task-temp/mac-pc286-platform-parity-20261114-b`. The separate
instrumented pre-positioned drag measures 373,535 modeled publication cycles
and 1,271,624 shared-sample cycles in
`C:/Repos/.task-temp/pc286-platform-timing-measure-20261114-b`; it includes
marker overhead and is not a physical timing result. Provenance, source-matched
Mac builds, remaining input cases, stack worst case, human/physical reviews,
production loader and release gates remain open.
