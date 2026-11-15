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

## Reusable keyboard pointer mapping

The 286 diagnostic now calls `platform/pc286/keyboard-pointer.c`; its
[SPEC-0102](../specifications/SPEC-0102-pc286-keyboard-pointer.md) separates
the key-word mapping from BIOS delivery. The mapping keeps the existing
16-pixel arrow steps and Space toggle. The caller owns state and failure
handling. Space toggles on every delivered event; the BIOS-level fixture does
not distinguish a real release from typematic repeat and therefore does not
establish the milestone's once-per-physical-press behavior.

The independent hosted test exhaustively checks all 65,536 key words for both
held states, unknown-key atomicity, invalid state and null refusals: 131,072
key/state combinations pass. Build with the same `$cc` flags above, replacing
the two source arguments with `tests/pc286-keyboard-pointer.c` and
`platform/pc286/keyboard-pointer.c`, then run the resulting new executable.

The 21,164-byte runtime-free 286 payload SHA-256 is
`a46ae6c33f15dbb97927e71d4712b1e67080cf132f8825a951db79a3ee82550a`;
test floppy SHA-256 is
`131295a95ba9e14d481608cae4b6cbff6db0f8ca9a920f36a62e58b8fcaadf28`.
Eight natural-start 286 checkpoints, the Mac comparison and trace replay pass
in `C:/Repos/.task-temp/pc286-input-platform-session-20261114-a` and
`C:/Repos/.task-temp/mac-pc286-input-platform-20261114-a`. The EGA edge and
mutation tests also pass in `C:/Repos/.task-temp/pc286-input-platform-edges-20261114-a`.
These are scheduled emulator inputs; the production input, responsiveness,
source-matched Mac build, hardware, human and release gates remain open.

## Experimental scan-code press and release

The earlier BIOS key-word path cannot distinguish Space release from repeat.
An independent limited software decoder now handles set-1 and set-2 make,
break, prefixes, repeat suppression and explicit error refusal under
[SPEC-0103](../specifications/SPEC-0103-pc286-keyboard-scan-pointer.md).
It derives the existing pointer samples from received bytes; it is not a full
keyboard/controller driver. The host regression and an intentionally broken
repeat control are in `C:/Repos/.task-temp/pc286-keyscan-host-20261114-a`;
reproduce with `scripts/Test-PC286KeyboardScan.py --cc $cc --output` and a
fresh output directory.

The initial opt-in 286 raw fixture masked interrupts after BIOS video setup and polled
controller ports. It assumes translated set 1 and stops on reported errors;
no controller initialization, translation verification, interrupt coexistence
or recovery is implemented. BootHush's own firmware configures translation
for this model; its source and observed behavior are not physical-hardware
proof. The initial attempt `C:/Repos/.task-temp/pc286-raw-input-20261114-a`
assumed untransformed set 2 and timed out without movement, exposing this
assumption. The corrected opt-in fixture uses translated set 1; no change to
the emulator was needed.

BootHush injected set-2 make/break events, including a repeated Space make
before each break. The translated stream produced exactly the two expected
presses and releases through all eight natural-start 286 checkpoints; all
canvas and margin pixels match and Mac capture/replay also matches. The
21,822-byte payload SHA-256 is
`a9fb7b6ed4027bb39f0b4759d5adae95ec22aa397808adfa7b26cddb50e07799`;
floppy SHA-256 is
`a34add7885d48a640b2dc2b9457a59e8f215a999f581e1a5bc8f1c21befb2596`.
Receipts are `C:/Repos/.task-temp/pc286-raw-input-20261114-b` and
`C:/Repos/.task-temp/mac-pc286-raw-input-20261114-a`. The BIOS-key variant
remains separately tested in `C:/Repos/.task-temp/pc286-bios-input-20261114-a`.
Reproduce the raw fixture using the natural-start shared-session command plus
`--raw-keyboard`, then the usual Mac parity runner. These are E2-style modeled
events, not human or physical input, and the limitations in SPEC-0103 remain.

The follow-up fixture masks only IRQ1 while preserving the other PIC mask bits
and the prior interrupt-enable bit. In the normal BootHush setup the mask was
`fe` before and after; a separate test first unmasked IRQ1 and observed
`fc` become `fe`, with the interrupt-enable bit still set. All eight state and
image checkpoints pass in both cases. The test-only control is not used in
the normal image; neither run establishes physical interrupt coexistence or
restoration of the original mask on exit.

The current 21,904-byte raw diagnostic payload SHA-256 is
`c29b81963e95dc4a7fa6326ccc75a89ac3a22999f9dbb9483f14ec84e31bf8b7`,
and test floppy SHA-256 is
`120f4a2a0b7db9857daa4f40a87d91d24571c4f43d718699eea7f0c2dc13e1f8`.
The current receipts are `C:/Repos/.task-temp/pc286-raw-input-20261114-e`,
`C:/Repos/.task-temp/pc286-irq-control-20261114-a`, and
`C:/Repos/.task-temp/mac-pc286-raw-input-20261114-b`. The latter compares all
eight Mac/286 checkpoints and replays the Mac trace. The original whole-
interrupt-mask experiment remains preserved above as history.
The eight independent EGA edge checks and two mutation controls also pass on
the current source in `C:/Repos/.task-temp/pc286-raw-input-edges-20261114-b`.

The separate Not Super Tilt Bro port added mouse-start and keyboard-play hints
at commit `2563388`, with 22 modeled title/action checks and BootHush replay;
its current ROM and evidence are identified in
`C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md`. Its independent
faster-mouse timing failure remains open. The port's game-derived code is not
used as a ClasSICK implementation input, and neither result establishes
physical Mac behavior or qualifies this release.

## Bounded 286 controller configuration query

[SPEC-0104](../specifications/SPEC-0104-pc286-keyboard-config.md) describes the
new opt-in preflight. It rejects existing output without consuming it, waits
only a bounded number of polls, and accepts an enabled keyboard with controller
translation. A racing keyboard byte remains indistinguishable from the query
response: the diagnostic requires a quiescent controller and is not a
complete input driver. It restores the prior IRQ1 mask on preflight refusal.

The standalone host test `tests/pc286-keyboard-config.c`, compiled with the
same `$cc` and `-std=c99 -O2 -Wall -Wextra -Werror -Wconversion
-Wsign-conversion`, passed all 256 configuration replies, both timeouts,
pending output, status faults and invalid calls. The BootHush raw-session
test passed eight state and full-pixel checkpoints using payload SHA-256
`9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`
(22,374 bytes) and floppy SHA-256
`ae9fc4c62417c6d426033778d7e76f66c7f9582c69945443009ab40efb1e4e07`.
Full commands, source and tool hashes, and results are retained in
`C:/Repos/.task-temp/pc286-config-input-20261114-e/report.json`.
The independent EGA edge cases and two mutation controls pass in
`C:/Repos/.task-temp/pc286-config-edges-20261114-b`. The BIOS input variant
passes in `C:/Repos/.task-temp/pc286-config-bios-20261114-a`.

Separate test-only controls declined configuration `00` and pending output,
respectively, restoring a deliberately unmasked IRQ1 from `fe` to `fc`; the
pending-output status remained set. Receipts, including commands and hashes,
are in `C:/Repos/.task-temp/pc286-config-bad-20261114-d` and
`C:/Repos/.task-temp/pc286-config-pending-20261114-d`. The test-only control
changes controller configuration; the ordinary query does not. An earlier
diagnostic incorrectly declared that an inline port read did not modify AX;
the compiler therefore reused a stale value. The corrected declaration and
rerun are recorded alongside preserved failed experiments in the local
`pc286-config-*-20261114-a` through `-c` directories.

The existing Mac image SHA-256
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`
matches the 286 diagnostic at eight modeled checkpoints and its injected mouse
trace replays in `C:/Repos/.task-temp/mac-pc286-config-parity-20261114-b`.
That ROM predates the current source; a source-matched Mac build, provenance
and code review, production boot, physical and human checks, and the separate
release decision remain open. None of these emulator tests establishes physical
controller behavior or release qualification.

As a sensitivity check, `scripts/Test-PC286KeyboardConfig.py` compiles the
same hosted test against three deliberately broken variants: ignoring pending
output, waiting one poll too long, and accepting a disabled keyboard. Each
fails at its expected distinct assertion, while the unmodified baseline passes.
Compiler identity, commands and source hashes are in
`C:/Repos/.task-temp/pc286-keyboard-config-mutations-20261114-c/report.json`.
These are host-software checks, not additional physical evidence.

## Four-track diagnostic loader

The existing test-only floppy loader now reads a fourth bounded track, raising
its payload ceiling from 22,528 to 30,208 bytes; this provides room for the
experimental 22,374-byte interactive diagnostic to grow. This is not a
production loader or an installer. [SPEC-0105](../specifications/SPEC-0105-pc286-diagnostic-loader.md)
records the exact geometry and limitations.

On BootHush executable SHA-256
`5d4f610e98cbb8b03476f28929d6e5c0f64b941aafac3c7d5b6ef12c76c7eeb3`,
`scripts/Test-BootHushPC286LoaderBounds.py` generated payloads of 22,528,
22,529 and 30,208 bytes, inspected data across track boundaries and at the
end, and rejected 30,209 bytes. A separate mutated image with only 14 sectors
in the final read failed its last-sector comparison as expected. Media hashes,
commands, source hashes and observations are in
`C:/Repos/.task-temp/pc286-loader-four-track-20261114-b/report.json`.
The existing raw-keyboard interactive session still passes its eight 286
state/pixel checkpoints in
`C:/Repos/.task-temp/pc286-four-track-regression-20261114-a`. This payload is
small enough not to invoke the fourth track; the generated maximum-length
fixture exercises that read. Hardware and release requirements remain open.

The later four-track run additionally injects a separate BIOS read failure at
each track start (`0:0:2`, `0:1:1`, `1:0:1`, `1:1:1`). In all four cases
BootHush reports the requested injected failure and the loader reaches `ee`
without executing the payload. For later tracks, the last byte of the prior
track remains loaded while the first byte of the failing track stays zero;
the input image hash remains unchanged. Full commands, image identities and
observations are in `C:/Repos/.task-temp/pc286-loader-failures-20261114-b`.
The first harness attempt in sibling `-a` stopped on a test-script variable
error before failure injection and is preserved. These are modeled failure
cases, not a physical BIOS or production-loader qualification.

### Experimental payload corruption check

The original test-only loader offers an opt-in 16-bit additive sum over the
linked payload bytes, excluding sector padding. It signals `ed` on a mismatch
before the payload entry point; BIOS read failures continue to signal `ee`.
It is a diagnostic experiment, not a security or authenticity mechanism: equal
and opposite byte changes can cancel, as the negative control demonstrates.
The default loader remains unchanged. Source and tests are independently
developed here; no firmware implementation was consulted.

On BootHush SHA-256 `5d4f610e98cbb8b03476f28929d6e5c0f64b941aafac3c7d5b6ef12c76c7eeb3`,
`scripts/Test-BootHushPC286LoaderChecksum.py` passed 20 cases covering valid
payloads up to 30,208 bytes, single-byte corruption at track boundaries, ignored
sector padding, a deliberately shortened final read, an undetected balanced
corruption, and an injected failure at each read. The shortened read is a
mutated diagnostic that reports success after reading one sector too few; in
this case the sum catches the missing bytes. Commands, inputs, hashes and
responses are in `C:/Repos/.task-temp/pc286-loader-checksum-20261114-c/report.json`.
The ordinary bounds and read-failure checks were rerun in
`C:/Repos/.task-temp/pc286-loader-bounds-20261114-c/report.json`.

With `--verify-payload --natural-start --raw-keyboard`, the actual 22,374-byte
shared 286 diagnostic (payload SHA-256 `9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`)
passes its eight state and pixel checkpoints with poisoned BSS. Its checked
floppy SHA-256 is `432b0fbfb01d2f3e7369209e3f0a1aa296bd0fc646a85d9c7e76c67cef9c986d`;
the build, commands and observations are in
`C:/Repos/.task-temp/pc286-checksummed-session-20261114-a/report.json`.
That session also matches the source-matched Mac build at all eight modeled
checkpoints and in mouse-trace replay at 4,096 modeled cycles per edge:
`C:/Repos/.task-temp/mac-pc286-checked-loader-parity-20261114-b/report.json`.
The first parity invocation used an incorrect manifest path and stopped before
running a guest. These tests establish neither physical reliability nor a
production disk format, boot path, human input qualification or release.

### Whole-disk 286 diagnostic

[SPEC-0107](../specifications/SPEC-0107-pc286-hard-disk-diagnostic.md),
IMPL-0134 / TEST-0190. To investigate the hard-disk dependency, an original
generator creates a fresh whole-disk image using BootHush's documented
615-cylinder, four-head, 17-sector model. Its original boot sector reads drive
`80h`, clears declared BSS, optionally checks the same limited additive sum
as the floppy diagnostic, and jumps to the linked payload. This takes geometry
from the emulator's own documented contract; it is a model observation, not a
documented fact about a particular physical disk or BIOS. The image is not
partitioned, has no installer or filesystem, and is not a production medium.

The bounded test generated payloads of 6, 8,192, 8,193, 16,897, 25,601 and
30,208 bytes, checked selected track-boundary and BSS bytes, rejected seven
invalid input combinations, and verified unchanged media. A corrupted last
byte led to `ed`; an invalid fourth-read sector led to `ee` after earlier
bytes had loaded. A deliberately balanced two-byte change passed, exposing
the additive check's limitation. The nine modeled runs, full commands,
source/image/emulator hashes and observations are in
`C:/Repos/.task-temp/pc286-hard-disk-loader-20261114-c/report.json`.
They do not prove every loaded byte or physical disk behavior.

The same 22,374-byte original shared-window payload SHA-256
`9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`
booted from the generated 21,411,840-byte disk SHA-256
`c44a7498c35a3340cf8a7ce93c2d689cf1a920599a4e60d862b7f6acc6890d8c`.
All eight hosted state and full-pixel comparisons passed with poisoned BSS,
and the final input accounting reported 63 accepted, zero rejected and zero
pending model bytes. The disk remained unchanged. The source-matched Mac ROM
matched all eight checkpoints and replay at 4,096 modeled cycles per edge.
The floppy control also passed both the host and paired comparison; its
original diagnostic image hash stayed `432b0fbf...`.

Canonical results and commands are in
`C:/Repos/.task-temp/pc286-shared-hard-disk-20261114-b/report.json`,
`C:/Repos/.task-temp/mac-pc286-hard-disk-parity-20261114-c/report.json`, and
the corresponding `pc286-shared-floppy-control-20261114-b` and
`mac-pc286-floppy-control-parity-20261114-c` directories. BootHush executable
SHA-256 is `62daf36a8d1e034667fbd73dd09a2a6134a6a0144ea56010d327b41fd110d9da`;
its source-rebuild limitations are recorded in
`C:/Repos/boothush/docs/pc286-input-accounting-20261114.md`.
Four altered Mac manifests were refused before a guest run in
`C:/Repos/.task-temp/mac-hard-disk-admission-20261114-a`. Four further
controls changed the disk image, recorded media hash, unsupported media type,
or mislabeled the disk as floppy media; each was
refused before a paired guest run in
`C:/Repos/.task-temp/mac-hard-disk-media-admission-20261114-b/report.json`.
These are identity and admission checks, not image authentication.

BootHush subsequently added explicit fixed-disk read-failure injection. With
its twin-built executable SHA-256
`703da7b4b3883f899cff6b17d221a3afce69bcf4d04e21d07718252ed2b6c0b0`,
the diagnostic loader suite passed 13 modeled runs, including failures on
each of four read starts. Each signaled `ee` before the payload entry, retained
the last byte of any previous read, and left the failing read's first byte
untouched. The actual 22,374-byte shared-window image uses three reads; an
independent fault test injected each of those failures with the same
observations. The original disk and payload hashes remained unchanged.
The no-fault shared session again passed all eight hosted checkpoints and
matched the Mac in eight state/pixel checkpoints plus replay.

Exact commands and results are in
`C:/Repos/.task-temp/pc286-hard-disk-loader-20261114-e/report.json`,
`C:/Repos/.task-temp/pc286-hard-disk-session-failures-20261114-c/report.json`,
`C:/Repos/.task-temp/pc286-shared-hard-disk-20261114-d/report.json`, and
`C:/Repos/.task-temp/mac-pc286-hard-disk-parity-20261114-e/report.json`.
The injector itself, its independently generated tests and partial-build
limits are recorded in `C:/Repos/boothush/docs/pc286-hard-disk-fault-20261114.md`.
These are synthetic built-in firmware failures, not physical controller tests.

### Disposable-disk copy and boot experiment

[SPEC-0108](../specifications/SPEC-0108-pc286-diagnostic-disk-copy.md),
IMPL-0135 / TEST-0191. An original test guest on a newly generated floppy
wrote the 45-sector prefix of the shared-window diagnostic image to a newly
generated blank modeled fixed disk, read those bytes into a separate RAM
region, and compared them. The host saved a new disk snapshot and verified its
entire 21,411,840-byte SHA-256 was the intended
`c44a7498c35a3340cf8a7ce93c2d689cf1a920599a4e60d862b7f6acc6890d8c`.
A separate BootHush run booted that snapshot to the expected initial shared
window state. No existing user media were modified.

With newly added BootHush write-failure injection, a failed write reached `e1`
and the saved disposable disk remained blank. A failed subsequent read reached
`e2` after the completed write, and its snapshot matched the written disk:
there is no rollback. A mutation of staged sector padding passed the guest's
readback of its own input but produced a different full-disk hash, which the
host identity control rejected. Commands, generated images, hashes, guest
results and explicit limits are in
`C:/Repos/.task-temp/pc286-diagnostic-install-20261114-b/report.json`.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$bh = 'C:/Repos/.task-temp/boothush-pc-hard-fault-twins-20261114-c/a/boothush.exe'
& $py -B scripts/Test-BootHushPC286DiagnosticInstall.py --emulator $bh --session C:/Repos/.task-temp/pc286-shared-hard-disk-20261114-d --output C:/Repos/.task-temp/NEW-DIAGNOSTIC-INSTALL
```

That snapshot is byte-identical to the independently generated disk used in
`C:/Repos/.task-temp/pc286-shared-hard-disk-20261114-e/report.json`, which
passed all eight hosted checkpoints on the same BootHush executable. The Mac
comparison and trace replay passed in
`C:/Repos/.task-temp/mac-pc286-hard-disk-parity-20261114-f/report.json`. The
full eight checkpoints were run on the independently generated identical image;
only the initial boot was run directly on the saved copy. This remains a
destructive whole-disk model experiment with no partitioning, filesystem,
existing-media protection, error recovery, physical trial or production
installer. Its modeled BIOS accepts this transfer across track boundaries;
physical firmware behavior is not established.

#### Track-bounded follow-up

On 2026-11-15 the diagnostic guest was changed to split that 45-sector copy
into three modeled tracks: 17, 17 and 11 sectors. It writes all three, then
reads them into a separate RAM region using the same boundaries and compares
all 45 sectors. The saved disk again matches SHA-256
`c44a7498c35a3340cf8a7ce93c2d689cf1a920599a4e60d862b7f6acc6890d8c`
and boots to the expected initial state.

Each of the three writes and each of the three verification reads was failed
in a separate BootHush run. For the writes, the entire saved disk matched
exactly the prefix written before the selected failure and zeros thereafter;
the first failure left it blank, while later failures left earlier tracks
written. For each verification-read failure, all preceding writes remained on
disk. A separate test-only mutation of the readback RAM reached the `e3`
mismatch stop while leaving the disk's intended contents intact. A changed
staged padding byte was also detected by the host's full-image hash check.
No rollback or real-media safety is claimed.

`C:/Repos/.task-temp/pc286-diagnostic-install-20261115-b/report.json` records
the input identities, commands, six injected failures, mismatch and successful
copy/boot. It uses BootHush SHA-256
`370f7a12a31bed9908813686a0cbd6bb197065b5e5d3e443fc48ad843e69f28d` and
the source-matched session at `C:/Repos/.task-temp/pc286-shared-hard-disk-20261114-e`.
The earlier single-transfer observations remain historical. Track splitting
reduces an identified model-specific assumption; it does not qualify real
geometry, BIOS behavior, interrupted writes, recovery or installation.

### Local engineering snapshot for review

2026-11-15; IMPL-0136 / TEST-0192. A fresh local snapshot contains the
source-matched Mac ROM, the generated 286 diagnostic disk, their build/session
records, the eight-checkpoint paired report and SHA-256 checksums. It is
explicitly labelled **unqualified**, not a release candidate or an installer.
The preparation tool validates selected source and image identities, the
emulator, the exact current parity runner, all eight modeled checkpoint
results and replay before creating a new directory. These self-reported
identities are integrity checks, not independent provenance review or proof of
physical behavior. No release has been published.

The local snapshot is
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-a`; the
control report is `C:/Repos/.task-temp/classick-snapshot-controls-20261115-a/report.json`.
The controls independently verify all output checksums; changed ROM, source
fingerprint, disk identity and parity-script identity are refused before
output creation, and a pre-existing directory is left intact. The snapshot's
`snapshot.json` records the repository base and exact preparation-script hash;
the latter was uncommitted at snapshot time. Commands and test inputs are in
the control report. Current source/tool licensing, human review, physical and
human input qualification, Mac fast input, production 286 boot and safe
installation, proper release packaging and separate publication approval are
still open.

A second snapshot was then created after committing the preparation tool:
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-b`. Its
record identifies repository commit `bf6cb92c2c2f5b621bb6050230a20406311fc483`
and the exact tool hash. The ROM and disk identities and all limitations are
the same; neither snapshot is a release candidate.

Firmware/geometry variation, boot drive selection, partitions, installation,
failure recovery, full byte coverage, human input, physical trials and release
qualification remain open.

### Scheduled-input accounting

The shared-session runner now requires the BootHush `pc_keyboard_input`
result for runs with scheduled scancodes. It checks that no byte was rejected,
the bytes needed for the observed checkpoint were accepted, and the accepted
and pending totals account for the schedule. A stop can precede an optional
trailing repeat or break, so pending input is recorded rather than assumed
delivered. Acceptance is into the emulator model; the separate guest state
and pixel assertions establish the observed behavior, not human or physical
input. The prior BootHush executable does not emit this field; use the new
interface described in `C:/Repos/boothush/docs/pc286-input-accounting-20261114.md`.

On BootHush SHA-256
`62daf36a8d1e034667fbd73dd09a2a6134a6a0144ea56010d327b41fd110d9da`,
the checked 286 payload and all eight hosted state/pixel comparisons passed;
at the final checkpoint, 63 of 63 scheduled bytes were model-accepted and
none were rejected or pending. The exact build and results are in
`C:/Repos/.task-temp/pc286-input-accounting-session-20261114-b/report.json`.
The matching Mac/286 state and pixel comparison and Mac replay passed at
4,096 modeled cycles per edge in
`C:/Repos/.task-temp/mac-pc286-input-accounting-parity-20261114-b/report.json`.
The BootHush executable was rebuilt from a fresh CLI object linked to recorded
existing core objects, not a source-complete or distribution-qualified build.
These results do not close the production boot, fast mouse, physical, human,
provenance, or release gates.

### Natural-start stack observation

The stack runner now follows the input mode and starting point in the source
session instead of always replaying the older pre-positioned BIOS sequence.
An initial attempt with the old sequence failed its final-frame comparison;
the preserved incomplete output is `C:/Repos/.task-temp/pc286-config-stack-20261114-a`.
With the current raw-input payload SHA-256 `9a443bda...`, two poison patterns
each observed a 1,056-byte touched span, with 20,168 untouched bytes between
the BSS end and the lowest observed write. Both finished the same 15-input
natural-start sequence and matched the reference image. Commands and full-gap
memory observations are in `C:/Repos/.task-temp/pc286-config-stack-20261114-c`.
The older pre-positioned BIOS mode also passes with two poison patterns in
`C:/Repos/.task-temp/pc286-bios-stack-20261114-a` (1,048 observed bytes).
Canaries can miss same-value writes; these observations do not prove maximum
stack depth, interrupt worst cases, physical performance or release readiness.

The Not Super Tilt Bro port also fixed its prototype damage counter wrapping;
its changed title ROM, BootHush regression and limitations are identified in
`C:/Repos/Not-Super-Tilt-Bro/docs/two-character-evidence.md` and
`C:/Repos/Not-Super-Tilt-Bro/docs/mouse-title-evidence.md`. This is port work,
not a ClasSICK implementation input or release qualification.

The port also built an opt-in mouse-title/keyboard-play ROM whose player-one
velocity step calls Windows-compiled 68000 C. Its BootHush regression,
legacy comparison, mutation control, source attribution and limits are in
`C:/Repos/Not-Super-Tilt-Bro/docs/windows-native-c-evidence.md`. This
demonstrates another bounded guest/toolchain exercise; its game-informed code
is not a ClasSICK implementation input and it closes no ClasSICK release or
physical-hardware criterion.

The port's follow-up corrected a local player-one jab movement lock and
exercised the mouse-to-keyboard handoff, both characters and pixel output on
BootHush. See `C:/Repos/Not-Super-Tilt-Bro/docs/windows-native-c-evidence.md`
for the reproduction and limits. The game-informed implementation is not a
ClasSICK source input, and this does not close a paired-release criterion.
The subsequent player-two velocity correction and modeled held-key resumption
are recorded at the same port evidence link, with the same source boundary.
The port's isolated source-derived PAL jab-chain diagnostic is recorded in
`C:/Repos/Not-Super-Tilt-Bro/docs/jab-chain-evidence.md`. It exercises the
Windows 68000 toolchain and BootHush, but contains game-derived implementation
and does not constitute a ClasSICK input or release qualification.

The port subsequently extended its opt-in title experiment to use that native
C movement component for both characters. Its modeled multi-key and rendering
observations are in the same canonical port evidence. They exercise BootHush
and the Windows 68000 compiler; the game-informed component is not admitted as
a ClasSICK implementation input and the port is not a ClasSICK release test.

On 2026-11-15 the port also exercised its dual-C title build through local
two-character combat after modeled mouse Start. The source, scheduled input,
per-frame checks and limits remain in its canonical native-C evidence above.
This supplies a separate BootHush integration scenario, not evidence of a
physical Mac, a complete game or a ClasSICK implementation source.

## Optional CRC-16 diagnostic payload check

The 286 whole-disk diagnostic's previous additive sum accepts equal and
opposite byte changes. SPEC-0107 now permits an independently written bitwise
CRC-16 with polynomial `0x1021`, initial `0xffff`, no reflection and no final
XOR. The guest checks only linked payload bytes before entry, and reports
`ed` on mismatch. The host calculation was compared with Python's separately
implemented `binascii.crc_hqx`, including `123456789 -> 29b1`; that standard
library is an oracle in the test, not linked into the 286 image. CRC has
collisions and is not authentication or protection against malicious changes.
The additive profile remains available and produces the identical earlier
disk; old artifacts were preserved.

The fixed-disk loader test passed 29 modeled cases, covering short and maximum
payloads, track boundaries, unchanged sector padding, single-byte corruption,
balanced corruption that the sum misses, and injected failures on all four
read tracks. Exact source and emulator hashes, inputs, image hashes, commands
and outcomes are in `C:/Repos/.task-temp/pc286-hard-disk-crc16-20261115-b/report.json`.
The first shared-window CRC run at `pc286-shared-crc16-20261115-a` failed:
its keys were scheduled at 6 million cycles while CRC startup was still
running, and the guest rejected a pending input stream. The corrected runner
observes a separate initial boot before choosing a start at least two million
cycles afterward. In `pc286-shared-crc16-20261115-b/report.json`, readiness
occurred at 7,520,373 and input started at 9,520,373 modeled cycles; all eight
state and pixel comparisons passed. The additive control remained ready at
4,208,555 with its unchanged 6-million-cycle schedule. These are model timing
observations, not physical performance measurements.

The CRC disk SHA-256 is
`e802001caeba5a184abde2902536375354c644dfaa273ef66fbf575f475df7f5`;
the unchanged linked payload is `9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`.
The Mac and CRC disk passed all eight paired full-pixel checkpoints and replay
at 4,096 modeled cycles per mouse edge in
`C:/Repos/.task-temp/mac-pc286-crc16-parity-20261115-a/report.json`. The
disposable disk-copy run in `C:/Repos/.task-temp/pc286-diagnostic-install-crc16-20261115-a/report.json`
also passed copy, boot, track read/write failure, mismatch and altered-input
controls. The entire copied disk matched the generated source. Nine snapshot
controls, including rejecting disabled or conflicting checksum modes, passed
in `C:/Repos/.task-temp/classick-crc-snapshot-controls-20261115-a/report.json`.
Neither the disk nor that copy workflow is a production loader, safe physical
installer, physical-hardware result or release candidate.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$bh = 'C:/Repos/.task-temp/boothush-pc-hard-fault-twins-20261114-c/a/boothush.exe'
& $py -B scripts/Test-BootHushPC286HardDiskLoader.py --emulator $bh --output C:/Repos/.task-temp/NEW-CRC-LOADER-TEST
& $py -B scripts/Test-BootHushPC286SharedSession.py --tool-root C:/ClasSICK/pc286-toolchain-20261023-a/toolchain --assembler C:/Repos/.task-temp/pc286-independent-arith-20261114-a/wasm.exe --host-cc C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe --emulator $bh --output C:/Repos/.task-temp/NEW-CRC-SESSION --natural-start --raw-keyboard --verify-crc16 --hard-disk
```

### Model geometry preflight

The optional fixed-disk diagnostic now queries and compares the built-in
BootHush model's documented 615-cylinder, four-head, 17-sector, one-drive
geometry before payload reads. It reports `ef` on a query failure or mismatch.
It still selects drive `80h` and assumes this exact model; it is neither
general disk discovery nor a physical-disk protection mechanism.

Thirty-three loader cases passed in
`C:/Repos/.task-temp/pc286-hard-disk-geometry-20261115-a/report.json`.
Controls deliberately changed the expected cylinder or head result or queried
an invalid drive; each stopped at `ef` before the payload was loaded. The
full shared session with both CRC and preflight passed all eight states and
images, starting its scheduled keys after a separate readiness observation;
its exact command and tool/source identities are in
`C:/Repos/.task-temp/pc286-shared-geometry-crc-20261115-a/report.json`.
The generated disk SHA-256 is
`39139fed7c6f0274e785ac38c8ee8660f1525a6d540f4af710d2434f63af37af`.
The paired model sequence and replay passed at 4,096 cycles per mouse edge in
`C:/Repos/.task-temp/mac-pc286-geometry-crc-parity-20261115-a/report.json`.
The fresh-disk copy, boot and injected error controls passed in
`C:/Repos/.task-temp/pc286-diagnostic-install-geometry-20261115-a/report.json`;
nine snapshot controls passed in
`C:/Repos/.task-temp/classick-geometry-snapshot-controls-20261115-a/report.json`.
Add `--verify-geometry` to the shared-session reproduction above. None of
these results changes the physical, installer or release gates.

### Bounded modeled read recovery

The diagnostic fixed-disk loader optionally retries a failed payload read up
to three times, resetting the built-in disk shim between attempts. It stops at
`ee` after the third failure or a failed reset. This initial version relied on
the model preserving DI; the later counter-preservation test is recorded below.
It is not a reviewed physical BIOS contract or a production installer.

BootHush added a separately tested one-shot read-failure injector, recorded in
`C:/Repos/boothush/docs/pc286-hard-disk-fault-20261114.md`. With its active
twin executable SHA-256
`ef7b5d9cf89fd0cc0eb1a816b5e489ffc34931349370a178c0579267dcc2ff2a`,
44 generated loader cases passed in
`C:/Repos/.task-temp/pc286-retry-loader-final-20261115-b/report.json`. These
include transient and persistent faults at each of four track starts and a
deliberately failed reset, plus retry with the additive or no payload check.
At each track a single injected failure recovered;
three persistent failures reached `ee`. The full shared-window image likewise
passed three transient recovery and three persistent refusal cases in
`C:/Repos/.task-temp/pc286-retry-final-faults-20261115-a/report.json`.

The CRC/geometry/retry disk SHA-256 is
`a580ce5642035b4c013979e22807989adf42b0f297ce33e83204ec77b7354230`;
its unchanged payload SHA-256 is
`9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`.
The session, Mac/286 full-pixel parity and replay, generated-disk copy and
fault tests, and nine snapshot controls passed in the respective
`pc286-retry-final-session-20261115-a`,
`mac-pc286-retry-final-parity-20261115-a`, `pc286-retry-final-copy-20261115-a`
and `classick-retry-final-snapshot-controls-20261115-a` directories under
`C:/Repos/.task-temp`. Each report records exact commands and identities.
To reproduce the session, add `--retry-reads` to the CRC/geometry command
above and use the new BootHush twin. The separate game title and combat tests
also passed on that twin. Neither emulator observation proves physical disk
recovery, human interaction, a safe installer or a completed release.

### Retry counter preservation

The first retry diagnostic relied on the model preserving DI across disk
calls. The follow-up saves and restores its attempt counter around each read
and reset; the POP does not change the returned carry condition. This removes
that specific assumption, but does not qualify physical firmware, stack or
other register behavior.

Forty-seven loader cases passed in
`C:/Repos/.task-temp/pc286-retry-di-preservation-20261115-a/report.json`.
For a test-only guest, the generated call wrapper set DI to zero after each
modeled firmware call using a flag-preserving move. With save/restore, a
transient failure recovered and a persistent failure stopped after exactly
three attempts. Removing the save/restore in the test-only control reached
the bounded cycle limit after 3,999 injected failures, demonstrating that
the control can expose a lost counter. This is a software mutation test,
not an observation that any physical BIOS changes DI.

The updated diagnostic disk SHA-256 is
`a1404d32f6acd81452bf6f4991789aea279a7caa955646913fb673e44c6b527b`.
The complete 286 session and its three transient/three persistent failures,
all eight paired state/pixel checks and replay, generated-disk copy controls,
and nine snapshot controls passed in the respective
`pc286-preserved-di-session-20261115-a`,
`pc286-preserved-di-session-faults-20261115-a`,
`mac-pc286-preserved-di-parity-20261115-a`, `pc286-preserved-di-copy-20261115-a`
and `classick-preserved-di-snapshot-controls-20261115-a` directories under
`C:/Repos/.task-temp`. Reports contain the exact commands, source and image
identities. The emulator remains the active twin `ef7b5d9c...`. None of these
observations changes the release, physical or safe-installation gates.

A potentially useful later IBM-published BIOS interface reference has been
identified as an unadmitted research lead in
`docs/research/pc286-bios-interface-lead-20261115.md`. Its 1987 publication
does not establish 1984 firmware behavior and informed none of this model code.

### Rebuilt active BootHush profiles

Two fresh active-profile BootHush builds produced the same executable SHA-256
`678f374a8adad6a32ea31f8da69ccf15c75f6568a7d679f56bbb73952e93890b`.
The build, exact tools, sources and retained prebuilt x64 media archive are
recorded in `C:/Repos/boothush/docs/active-profile-build-20261115.md`. It
rebuilds Mac, 286 and NES components but is not a source-complete distribution
or provenance qualification; no paused x64 source was compiled.

On this executable the CRC and geometry diagnostic passed 33 loader cases
(`C:/Repos/.task-temp/pc286-active-twin-loader-20261115-a/report.json`),
fresh-disk copy and injected failures (`pc286-active-twin-copy-20261115-a`),
the eight-state session (`pc286-active-twin-session-20261115-a`), and all
eight paired frames, state checks and replay
(`mac-pc286-active-twin-parity-20261115-a`). Nine snapshot controls passed
in `classick-active-twin-snapshot-controls-20261115-a`; these paths are under
`C:/Repos/.task-temp`. BootHush's independent 25-case 286 suite and the port's
title and combat checks also passed on this executable. This is emulator and
software evidence; fast input, production boot and installer, source review,
physical/human qualification and release decisions remain open.

The unqualified engineering snapshot now includes checksummed guest-only
initial and content PBM frames from the paired run, plus bounded headless
readiness commands. Seven integrity and preservation controls passed, and the
packaged Mac and 286 images each reached their modeled readiness condition;
their files still matched their checksums after execution. The commands and
observations are in `C:/Repos/.task-temp/classick-snapshot-controls-20261115-c/report.json`
and TEST-0192. This is not a release candidate or physical qualification.

### Diagnostic stack watermark

To investigate the missing 286 memory budget, an optional diagnostic loader
fills offsets `c020` through `fffd` of the payload segment with `a5` before
entering the same 22,374-byte window payload. The payload's linked BSS ends at
`ad16`; the instrumented disk SHA-256 is
`23bef093117439c32a1391401b89d7143f6ac55344a2ecc05d97ed2356b2330b`.
The payload SHA-256 and BootHush executable are the preserved-DI identities
above. Each of the eight natural-start, raw-keyboard checkpoints reached the
same diagnostic state and pixel-identical EGA frame as the earlier session;
the changed-byte watermark extended at least 1,070 bytes below initial SP
`fffe`. The canary floor remained unchanged. A separate tiny guest with 128
pushes marked the expected 256-byte depth, and a synthetic floor change was
detected. The ordinary loader's 47-case regression also passed.

This is a changed-byte lower bound for these modeled runs, not a measured
maximum: writes matching `a5`, skipped addresses, paths and interrupts not
observed by the test can escape detection. It does not establish the full
640 KiB budget, actual physical BIOS use, a production stack or a release
gate. The exact commands and input identities, all eight logs, pixel hashes,
known-depth control and limitations are in
`C:/Repos/.task-temp/pc286-stack-canary-20261115-c/report.json`; the loader
regression is `C:/Repos/.task-temp/pc286-stack-loader-regression-20261115-b/report.json`.
Both are generated artifacts outside Git.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$bh = 'C:/Repos/.task-temp/boothush-transient-active-twins-20261115-b/a/boothush-active.exe'
& $py -B scripts/Test-BootHushPC286StackCanary.py --session C:/Repos/.task-temp/pc286-preserved-di-session-20261115-a --emulator $bh --output C:/Repos/.task-temp/NEW-STACK-CANARY
& $py -B scripts/Test-BootHushPC286HardDiskLoader.py --emulator $bh --output C:/Repos/.task-temp/NEW-LOADER-REGRESSION
```

After committing the watermark tooling, the default, non-instrumented 286
session was rebuilt against the updated loader source. Its disk and payload
hashes remained the preserved-DI identities above. Eight modeled paired
states, all normalized pixels and Mac replay matched at explicitly selected
4,096-cycle edge and VBL guard settings. Nine snapshot controls and both
bounded readiness checks passed. The initial parity run omitted the explicit
guard option; although its model checks passed, the snapshot identity guard
correctly refused that incomplete declaration. The corrected run and
controls are `C:/Repos/.task-temp/mac-pc286-watermark-source-parity-20261115-b`
and `C:/Repos/.task-temp/classick-watermark-snapshot-controls-20261115-b`;
the refused run and output remain in their `-a` siblings. The corresponding
unqualified snapshot is `C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-j`,
with source commit and checksums in its manifest. It is not a release candidate.

### Raw 286 input bursts and model queue

The current CRC/geometry/retry fixed-disk session's 15 input groups and 63
scheduled set-2 bytes were retimed to investigate the raw input path while
the guest redraws. At 5,000,000, 1,000,000, 500,000, 250,000 and 125,000
modeled cycles between groups, all eight selected state and complete-frame
checkpoints matched, and all 63 scheduled bytes were accepted. At the
125,000 setting the final stop was 7,043,672 modeled cycles after the last
scheduled byte. These are cold-run checkpoint comparisons and a bounded
backlog observation, not a claim about every intermediate instant.

At 62,500, 31,250 and 10,000 cycles per group, intra-group byte spacing was
scaled down to preserve order. BootHush stopped with `machine_fault` on one
rejected byte, respectively after accepting 57, 47 and 42; 5, 15 and 20
later bytes were still scheduled. The newer emulator's side-effect-free
controller snapshot showed its output queue at 32/32 in each of those runs;
the five slower final stops showed 0/32. Its queue is a model constraint, not
a physical controller measurement. The model accepted translation prefixes
without necessarily producing output bytes. These results do not prove real
input speed, key rollover, human responsiveness, or a production driver.

BootHush executable SHA-256
`e84e214750c69907f6c5052397243661392810d1f1b14e357d6387a29008885d`;
input disk SHA-256
`a1404d32f6acd81452bf6f4991789aea279a7caa955646913fb673e44c6b527b`.
The complete schedule, commands, output snapshots and input/source identities
are in `C:/Repos/.task-temp/pc286-raw-queue-rate-20261115-a/report.json`.
The earlier executable's corresponding characterization is
`C:/Repos/.task-temp/pc286-raw-input-rate-20261115-d/report.json`;
status, stop cycle, final state, frame and input accounting matched for every
rate. The newer executable also passed the eight modeled Mac/286 parity
checks and replay in `C:/Repos/.task-temp/mac-pc286-queue-parity-20261115-a`.
The BootHush snapshot contract and its independent device tests are documented
in `C:/Repos/boothush/docs/pc286-input-accounting-20261114.md`.
Nine package controls and two bounded readiness checks also passed on the
new executable in `C:/Repos/.task-temp/classick-queue-snapshot-controls-20261115-a`;
the matching, still unqualified engineering snapshot is
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-k`.

```powershell
$py = 'C:/Users/segab/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
& $py -B scripts/Test-BootHushPC286RawInputRate.py --emulator C:/Repos/.task-temp/boothush-pc-queue-active-20261115-a/a/boothush-active.exe --pc-results C:/Repos/.task-temp/pc286-queue-session-20261115-a --output C:/Repos/.task-temp/NEW-RAW-INPUT-RATE
```

### Unadopted covered-cursor publication experiment

An opt-in diagnostic experiment skipped the old or new cursor's EGA copy
only when the scene's dirty rectangle contained its whole un-clipped
footprint. A same-source build without that option served as control. Both
passed the eight normal state and full-pixel checkpoints. Across the five
non-overflow burst periods above, the final stop relative to the last input
changed by +367, -3,957, -4,331, -4,172 and -2,444 modeled cycles; the
three faster cases still stopped with a full model queue and the same accepted
counts. The change did not meaningfully resolve the multi-million-cycle
backlog in this scenario and was not adopted. It is not a physical
performance result or a general proof of safe cursor clipping.

Control payload SHA-256
`9a443bdaf9a753be3c41bca3a4b5d72ce60d0e517a3f672a486f2d4b585c2c82`;
experimental payload SHA-256
`6c137707056d098e038e24102882429dc3c637de1d8a09000bbf8842d3ac13a0`.
The builds, eight rate runs, exact commands and hashes are retained under
`C:/Repos/.task-temp/pc286-covered-{control,experiment}*20261115-a`.
The experimental source copies remain in
`C:/Repos/.task-temp/pc286-covered-source-20261115-a`; their hashes are in
the associated session reports. Current working source retains the baseline.

### User-requested 286 display capture

On 2026-11-15 the owner asked to see the current 286 boot. The unqualified
snapshot `classick-paired-engineering-snapshot-20261115-k` fixed-disk image,
SHA-256 `a1404d32f6acd81452bf6f4991789aea279a7caa955646913fb673e44c6b527b`,
booted in BootHush executable SHA-256
`e84e214750c69907f6c5052397243661392810d1f1b14e357d6387a29008885d`.
The previously tested natural-start keyboard schedule produced a window drag,
release and content change; all 63 scheduled bytes were accepted, the final
state matched and the final 640x350 EGA frame hash was
`4df2e3d7049a2e6d1d066d158c61866a2334c412a4fd793e3dca7dd6919db0cc`.

The guest-only recording contains 67 frames at five frames per modeled
second (13.4 seconds); it includes boot and scripted interaction, without
desktop capture or audio. GIF SHA-256
`cf600ccfea3663677310c140629cc98d0bee1493d11e9ed0619c6dd0a07d7739`,
MP4 SHA-256 `07f4bc54e5f3c94d7ab60531121659d0dc3bcc1b65bd577d40ede6c06a858419`,
and lossless final PNG SHA-256
`84975e953cdddd9c825705fb698c48db375fdf13886c0984bc698a4732ba24dd`.
These generated assets, the original Y4M and PPM, full run and conversion
inputs remain outside Git under `C:/Repos/.task-temp/classick-286-show-20261115-a`.
The GIF scales nearest-neighbor and the MP4 is lossy; the PNG derives from
the final PPM. FFmpeg SHA-256 is
`851aa8ea5366b5af33c0681cc292d662829c7088e9ac96f6e9b37030e6835ba4`.
The complete BootHush command, guest result and recorded executable/media
identities are in `capture-report.json` in that directory. This is a modeled
diagnostic demonstration, not a human-input, physical or release test.

### Cursor art validation optimization

The shared scene's existing validation checked each bit individually to form
the allowed mask for every cursor-art byte. It now calculates the equivalent
mask once per byte, retaining bounds, image-subset-of-mask and refusal rules.
The implementation uses an unsigned shift bounded to 0..7, including on the
16-bit 286 compiler. It is original project code, not a change to the artwork
or to Macintosh compatibility claims.

An independently expressed per-bit reference checked 196,608 single-bit
mask/image combinations across all widths and heights 1..16, plus valid mixed
patterns, invalid hotspots, dimensions and null bits. The old and optimized
validators passed the single-bit suite; the optimized suite also passed with
Windows Clang AddressSanitizer/UndefinedBehaviorSanitizer. A control removing
the top allowed bit failed at width 1, height 1. The original scene reference
test passed 10,040 complete frames and 40,520 checks. An initial link attempt
using section garbage collection failed on unresolved symbols; linking the
actual scene dependencies succeeded. A first sanitizer launch lacked the
runtime DLL on PATH; the corrected launch passed. Test programs and commands
are reproducible with:

```powershell
$cc = 'C:/ClasSICK/bootstrap-clean-source-verified/.tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe'
$env:PATH = (Split-Path $cc) + ';' + $env:PATH
& $cc -std=c11 -O2 -Wall -Wextra -Werror tests/window-scene-art-exhaustive.c core/graphics/surface.c core/graphics/cursor.c apps/product-scene/scene.c -o C:/Repos/.task-temp/NEW-ART-TEST.exe
& C:/Repos/.task-temp/NEW-ART-TEST.exe
```

The rebuilt Mac ROM SHA-256 is
`32d7a5e8c897aae3bdd02ac2dc551c2eca23578bd3e942a2a99d869dbd6ad944`;
the 286 payload and diagnostic disk SHA-256 are
`bb0f402bbf7b9d22d7999f7afc6e44f8defdc74de4acc012af2f349818dfd336` and
`a01bb95b6a3840a0a5a889951d75c421bb4823b1189a8cfae1e17d7b4089350e`.
With BootHush SHA-256 `e84e214750c69907f6c5052397243661392810d1f1b14e357d6387a29008885d`,
all eight 286 states, all pixels, eight Mac/286 comparisons and Mac replay
passed at 4,096 modeled cycles per edge. At 2,048, the Mac title checkpoint
still failed with pointer `(256,189)` and 70 omissions. The exact builds,
input schedules, frames and source/tool identities are in the
`mac-art-mask-20261115-a`, `pc286-art-mask-session-20261115-a`,
`mac-pc286-art-mask-parity-20261115-a` and `mac-art-mask-fast-20261115-a`
directories under `C:/Repos/.task-temp`.

At the fastest passing 125,000-cycle input-group period, the observed final
stop after the last scheduled byte fell from 7,043,672 to 6,862,428 modeled
cycles; the same three faster burst settings still filled the emulator queue.
The full rate report is `C:/Repos/.task-temp/pc286-art-mask-rate-20261115-a`.
These are model-specific results, not physical performance or release
qualification. The production loader, fast Mac input, human/physical tests
and other paired-release criteria remain open.

The exhaustive test is also registered as `window-scene.art-exhaustive`;
that test and `window-scene.reference` passed in
`C:/Repos/.task-temp/classick-art-cmake-20261115-a`. Nine snapshot controls
and both bounded readiness checks passed in
`C:/Repos/.task-temp/classick-art-mask-snapshot-controls-20261115-a`, and
the corresponding unqualified engineering snapshot is
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-l`.
