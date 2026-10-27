# Research study: endpoint-only phase sampling and the recorded Mac loss

Status: lead-reviewed original engineering research,2026-10-27;SRC-0509/0510.
Host reproduction executed by Codex with pinned Clang64 and GCC64. No OS change
or new compatibility contract. All desktop/emulator tests remain paused.
Baseline ebfbfb935ed560b687ffed931c0db94889ff9ce0.

## Inputs and scope

Engineering inputs used here come only from the reviewed public project
files: polled-phase.h/.c, core/input/pointer.h, SPEC-0074 and the paused
trace record mac-refresh-paused.json. SPEC-0074 itself says its PB4/PB5/DCD
normalization reuses engineering assignments, not newly verified physical pin
or January 1984 behavior. Everything below is original software and
mathematical analysis of that model. It infers no physical root cause.

## Hypothesis

The model sees only phase endpoints between calls. If more than one legal Gray
edge occurs between calls, the endpoint step can (a) agree, (b) be rejected
as a two-bit jump (whole-sample loss), (c) emit zero while full cadence would
have emitted units, or (d) emit the opposite sign. Inversion bits only flip
signs; they cannot restore information absent from the endpoints.

## Method

`tools/research/study_mac_sampling.py --repo R --compiler CC --output NEW`
(Python 3 stdlib) verifies the LF SHA-256 of the three model inputs, refuses an
existing output directory or one inside the repo, writes an original C99
adapter, compiles it with the unchanged polled-phase.c (bounded timeouts),
and writes sweep.csv and summary.json only under NEW. No network, VM,
emulator or target execution; no repo writes.

For every case two independent states are begun with the same invert and
baselined at the same raw endpoint and stable button (both BASELINED checked).
The full-cadence state is fed every intermediate legal Gray edge (forward
0,1,3,2), each step checked as SAMPLE/EMPTY with the stable button, and its
dx/dy summed. The endpoint state is fed only the final endpoint. Outputs are
cleared before each call. Any unexpected code stops the run with an error.

Enumeration: axis X/Y x start numeric phase 0..3 x direction +1/-1 x steps
0..12 x invert 0..3 x button 0/1 = 1664 cases; the other axis stays phase 0.
Columns: axis,start,direction,steps,invert,button,rc,dx,dy,full_dx,full_dy.
Units are emitted DCD units of the model, not physical edges, pixels or time.

Classes (summary.json): exact emitted-unit agreement; whole-sample loss
(endpoint rc LOST); hidden nonzero full-cadence units (endpoint 0,0, full
nonzero); opposite-sign output; other mismatch. Agreement is never proof that
no transitions were missed: a full cycle returns to the same endpoint.

## Executed examples and coverage

Five lead cases were frozen before worker output inspection and match both runs.
The model sources are unchanged and the CSV is byte-identical between compiler runs;the summaries agree apart from
compiler metadata. Exact unique Cartesian coverage and legal return codes are
checked. Both runs produce the following classifications:

| Endpoint outcome | Cases |
|---|---:|
| Exact emitted-unit agreement |256|
| Whole-sample loss |384|
| Hidden nonzero full-cadence units |704|
| Opposite-sign output |192|
| Other mismatch:all same-direction undercounts |128|
| Total |1664|

This deliberately balanced enumeration is not a probability model,field failure
rate or native test count. Both comparisons call the same original decoder;this
is an observation-loss study,not an independent proof that the decoder is correct.
The lead also checks all four trace candidates and two CLI refusals:existing
output and changed source (refused before creating output). No target runs.

The following are executed synthetic examples,not captured mouse trajectories.
X axis, invert 0, button 0, forward from phase 0 (DCD edges emit dx -1):

| steps | edges | full dx | endpoint | observed class |
|---|---|---|---|---|
| 1 | 0-1 | -1 | 0-1, dx -1 | agreement |
| 2 | 0-1-3 | -1 | 0-3 jump, LOST | whole-sample loss |
| 3 | 0-1-3-2 | -2 | 0-2 reverse PB-only, 0 | hidden nonzero |
| 4 | full cycle | -2 | no change, EMPTY | hidden nonzero |

From start phase 1 forward 3 steps (1-3-2-0): full dx -1, endpoint reads
1-0 as one reverse DCD step, dx +1: opposite sign. Steps 0 trivially agree.

## Recorded trace: missing previous PB4/PB5

The wrapper record shows current PB=AF, RR0A=RR0B=4C and loss_previous
01000000: have_phase 1, DCDa=DCDb=0, button 0. Previous PB4/PB5 were not
retained, and must not be filled from current PB. Current phases decode as
X=1 (DCD1, PB4=0), Y=3 (DCD1, PB5=1). The script baselines prior raw PB
8F/9F/AF/BF with RR0A=RR0B=0 through the API (invert 0), then steps
AF/4C/4C, reporting the actual rc/dx/dy. Both compiler runs agree:

| prior PB | prior X,Y | observed model result |
|---|---|---|
| 8F | 0,0 | LOST (Y 0-3 jump) |
| 9F | 2,0 | LOST (X 2-1 jump) |
| AF | 0,2 | SAMPLE dx -1, dy -1 |
| BF | 2,2 | LOST (X 2-1 jump) |

These are representative previous PB bytes for the four missing-bit assignments;
other ignored PB/RR0 bits are not reconstructed. They are logical possibilities,
not probabilities. Three assignments can produce the recorded Gray loss;the
trace cannot distinguish which one applied or which axis caused it. The native
manifest confirms invert0 as the build setting,not a verified physical direction.
Polled-model return LOST3 is wrapped as timer loss_reason6 for the same Gray
rejection path (timer-poll.c:219-220 and timer-poll.h:30). These are two layers
of reporting,not different mechanisms or proof of an actual intermediate path.

Stable current snapshots (loss_snap0 = loss_snap1 = AF4C4C) show only that
the recorded readouts match. They do not prove atomicity,continuity or that all
intervening transitions were observed. A jump is defined relative to the earlier accepted endpoint, whose PB half is missing,
so current stability cannot settle whether the earlier endpoint was coherent
or how many edges intervened. The trace is paused and is not a clean
pre-input control or causal attribution of the first loss.

This gap justifies a discriminating future observation (retain the full
previous PB/RR0 endpoint with each loss), not a sampling-rate, sign, latch or
historical behavior fix.

## Relation to PC evidence (separate cases)

On PC, Windows accepted 12 guarded API mouse events while six of our own
exports stayed identical. A later physical attempt yielded a changed frame,
but it was not a completed human test: fullscreen hid the instructions. No
same-packet claim is made, and neither case is evidence about the Mac
polled model. A fresh engineering experiment can illuminate a blind spot
without proving a native hardware cause. Release gates E1/E2/E3/E4 remain.

## Limitations

Software model only; SPEC-0074 assignments are engineering, not verified
hardware or edition behavior. No SCC settings, cadence, latency or live-level
sampler is modeled. Host compiler (gcc/clang-style flags) only; no M68000 or
MSVC qualification. No stability, performance or frequency claim.

## Next evidence

1. Retain the executed CSV and source pins;do not repeat unchanged trials.
2. Proposed instrumentation: record the full previous PB and RR0A/RR0B with
   every loss, plus call index, so future losses are discriminable.
3. A controlled single-axis observation once tests are unpaused.

## Sidebar: verified project measurements (different metrics)

Not a size forecast. Mac refresh ROM file 65536 B; the native audit reports
23772 used ROM bytes, 360 B static RAM, 964 B workspace, and a 4992 B
conservative nonrecursive stack bound (a bound, not a measurement). PC EFI
file 184320 B; FAT boot disk file 67108864 B. A 180 KiB EFI and a 64 MiB
container do not imply 64 MiB of OS code. Current scene artifacts are not a
completed System 1 volume, Mac-application compatibility proof or a 286
install footprint. The lead verifies all figures; this study does not need
those binaries.


## Reproduction and shareable material

Use Python3 and a trusted GCC/Clang-compatible host compiler. Run from PowerShell:

```powershell
python -B tools/research/study_mac_sampling.py --repo C:/Repos/ClasSICK-0.97 --compiler .tools/llvm-mingw/llvm-mingw-20260908-ucrt-x86_64/bin/clang.exe --output C:/ClasSICK/new-study-output
```

The utility checks three exact original source hashes after CRLF→LF normalization;
it intentionally refuses changed inputs. Use a new outside-repo output directory.
It never launches an emulator or writes source files. The recorded results use
the pinned LLVM-MinGW and w64devkit compilers already installed for the project.

[Executed CSV](data/input-sampling-2026-10-27/sweep.json),
[summary and original source hashes](data/input-sampling-2026-10-27/summary.json),
[measured artifact identities](data/input-sampling-2026-10-27/artifact-sizes.json),
[original diagram and SVG export instructions](figures/endpoint-sampling.md),
[Mermaid diagram source](figures/endpoint-sampling.md).

The existing native audit reports a23772-byte used-ROM span (23.21484375KiB)
in the64KiB file,an arithmetic remainder of41764 bytes;this is not an allocator
guarantee for future features. That remainder is not a RAM budget or a promise
that a completed System1 fits. The180KiB EFI file includes its own file overhead;
the64MiB disk is a boot container. These categories cannot be compared as if each
were used OS code. Keep these measured baselines for future286 budgeting;no286
installed size or performance is measured here. [Post fact pack for28October](../development/2026-10-28-update-research-brief.md).
