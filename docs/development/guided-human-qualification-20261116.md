# Guided human qualification — Mac/286 preview

Owner authorization, 2026-11-16: present the releases in BootHush, give short
test instructions, collect human feedback, and adapt the next test or fix.
This supersedes the headless-only trial restriction for these explicit owned
candidate sessions. No reference system, other emulator, physical hardware,
publication, or waiver of remaining release criteria is implied.

Canonical candidate/package identity remains in
[paired preview readiness](paired-preview-readiness-20261116.md). This record
owns the guided human-test workflow and resulting observations. Planned cases
are not results. Keep machine-generated acceptance, agent actions, and the
owner's observations separate.

## Session procedure

1. Verify the selected executable and media hashes against the admitted
   candidate before launch. Open only one native, windowed BootHush session.
2. Present a short case and its expected visible behavior. The agent may
   activate/inspect the owned window to hand it over; record those actions.
   The owner performs the test input. Do not inject test motions during handoff.
3. Retain bounded guest video, keyboard/mouse logs where supported, final
   pixels, diagnostic RAM and exact command. Default human round: five minutes;
   the owner can close early. A timeout is not itself a crash or a pass.
4. Ask for pass, specific failure, or incomplete feedback. Record the owner's
   wording and the cases actually attempted. No response remains pending.
5. Inspect the run and compare reported behavior with diagnostics/captures.
   A failed or ambiguous case gets a focused investigation and a fresh rerun;
   preserve the original failure. Advance only when the round is understood.
6. Summarize passed, failed and untested cases before any release disposition.
   Human testing does not close physical, source-review or interoperability gates.

## Planned rounds

| ID | Candidate | Human actions and observable expectations | Status |
| --- | --- | --- | --- |
| M1 | Mac | Boot scene; move in all four directions; drag the small guest window by its blank title strip; release and move away; click its patterned content twice. Pointer follows direction, only the held drag moves the window, and each content press toggles its pattern. | Failed pointer responsiveness/boundary reach; drag/release/pattern pass by owner report |
| M2 | Mac | Edges/corners, leave/reenter the viewport, rapid reversal, focus change during a held drag, return and resume. Look for stuck input, drift, missing redraw or lingering corruption. | Not run |
| P1 | 286 | Boot the same logical scene; arrows move the pointer; hold Space while arrows drag, release Space to release the button; one content tap changes the pattern. | No keys received in first session; original toggle instructions corrected below; human input remains untested |
| P2 | 286 | Held/repeated arrows, release, focus switch and return, restart with a key held; verify Space release and focus-loss release clear the held button. | Not run |
| I1 | 286 installer | Use a new disposable blank virtual disk: inspect prompt, cancel and verify unchanged media; fresh session install, retain new disk output and boot it. Never select a host/physical disk. | Not run |
| T1 | Port follow-up | Separate prototype session: mouse title, both players' supported movement/jump/attack controls, simultaneous keys and focus loss. This is not part of the ClasSICK binary pair. | Not run |

The original native preview implements one scene/window, not a general desktop
or complete System release. The 286 uses keyboard pointer control, not a mouse.
Detailed controls and limits:
`C:/Repos/boothush/docs/human-testing.md`.

## First round admission

- BootHush source: `1e002aa`; production executable SHA-256
  `f127e61e5507b50e8d109a0788344909305b6d3933cf0c2711a2feb2b810b8a8`.
- Mac candidate SHA-256:
  `baa542304ec55bcfd2fecae7c594bb8e161275e49f6c3145886f2ab2f52f75be`.
- 286 disk SHA-256:
  `6f17583422edf2eddd4e5c325020a3538b4333a0a3305ca350b99cf91e992ba6`.
- Mac settings: edge interval 2048, direction inversion 3, VBL guard 4096,
  scale 2; diagnostic inspection at `0xb00:1024`.
- Session root: `C:/Repos/boothush/out/qualification-20261116-a`.
- Initial human disposition at admission: pending. The preceding hidden-window checks are scoped
  implementation evidence, not a substitute for this round.

Subsequent feedback and evidence will be appended here; raw session artifacts
stay outside Git. The existing frozen review archive is unchanged.

## M1 result and focused investigation

Owner report: “When I moved the cursor around, it was laggy, and didn't reach
all four boundaries. I was able to move the window by clicking the title bar
and dragging. It stayed put. The pattern changed as it was supposed to as well.”
This is actual human feedback, not an author-inferred pass. M1 is failed for
pointer usability; the three described interaction functions passed this round.

The session reached its five-minute cycle limit normally. Its original video,
frame, input log and JSON remain intact. A separate `human-feedback.json`
records the owner report. There were 1165 accepted mouse samples, 8340 delivered
pin steps, no pending bridge work, 129 motion omissions, four reason-2 decoder
losses and seven FIFO overflows. Zero final pending work did not establish
complete guest movement. Host setup used only the owned window's Raise action;
the owner performed all test inputs.

Replaying the recorded motion reproduces the issue. Diagnostic derived traces
preserve all events and timestamps, change only pacing, and stop two guest
seconds after the last event. Reports:
`C:/Repos/.task-temp/human-pointer-analysis-20261116-a/report.json` and
`human-pointer-burst-20261116-a/report.json`.

| ROM / edge interval | FIFO peak / overflows | Omissions | Reason-2 losses | Final pointer |
| --- | --- | ---: | ---: | --- |
| Original / 2048 | 128 / 7 | 129 | 4 | 352,187 |
| Original / 4096 | 128 / 6 | 0 | 5 | 388,207 |
| Original / 8192 | 128 / 1 | 0 | 1 | 482,182 |
| Burst experiment / 4096 | 187 / 0 | 0 | 0 | 482,182 |

An independent sequential clamped-vector calculation ends at 482,182. A
matching endpoint alone is insufficient: the original 8192 run still loses
input. The experimental 4096 run has zero decoder/FIFO losses and omissions.
These are replay results, not a new human observation or a latency measurement.

`--window-burst-capture` extends SPEC-0109 with a 512-record FIFO in the
application workspace and a 128-record foreground batch. Normal 128/32 builds
remain selectable. Source-matched native twins use 49,500 ROM bytes, 504 native
data/BSS bytes and 11,224 workspace bytes at 32768–43992. The conservative stack
bound remains 4,924 bytes. ROM SHA-256:
`7e2b111134a7d0414129545682ccd18c06335964ccf6849195a2c1c1bc765b87`;
manifest: `C:/Repos/.task-temp/mac-human-burst-20261116-a/manifest.json`.

Both 128- and 512-record host variants pass 12,132 checks at each of O0/O2
(`mac-human-buffer-host-20261116-a` and `mac-human-burst-host-20261116-a`).
The deliberate +5000-pixel burst still reaches capacity 512, reports six
overflows and 14 total decoder losses, cancels the held drag and accepts a
fresh exact -16 move without moving the released window. This is recovery,
not lossless excess-rate handling (`mac-human-burst-recovery-20261116-a`).
All eight paired native state/pixel checkpoints and Mac replay pass at 4096;
the 286 commands were re-executed on the new BootHush build. Receipt:
`C:/Repos/.task-temp/mac-human-burst-parity-20261116-a/parity/report.json`.

BootHush separately adds optional captured raw relative motion with F12 release
to remove host-client boundary limits. Its twin executable SHA-256 is
`ab6f2d3820a326d681b9902a2a5439d043176af2bee19cfe3a72d0490fd76a6e`.
See `C:/Repos/boothush/docs/relative-mouse-20261116.md` for implementation tests.
The next round M1R1 will use this emulator, the burst ROM, and edge 4096 to test
responsiveness, all four boundaries and host capture release. Human disposition
is pending. This candidate does not replace or relabel the frozen review ZIP.

## M1R1 capture failure and M1R2 admission

Owner report: "I clicked in, but the window closed." M1R1 stopped after
61.234 seconds with `input_error`, zero completed captures, zero raw packets
and zero mouse samples. This was not the five-minute limit. Guest movement,
boundary reach and responsiveness were not tested; retain the M1 failures.
Original media, logs and guest video remain in
`C:/Repos/boothush/out/qualification-20261116-a/M1R1-mac`.

BootHush's separate agent-operated diagnostic reproduced startup failure and
identified `ClipCursor`, Win32 error 5 (access denied), before capture could
complete. The [BootHush receipt](https://github.com/segaboy/boothush/blob/3f8afe06ddd6d876c8b7796d707d3611b25fbbc4/docs/relative-mouse-20261116.md)
records the diagnostic session, API-contract sources and automated tests.
An attempted second agent UI inspection timed out; its untouched session
reached the time limit and is not a capture pass.

M1R2 uses explicit unconfined relative input: the host pointer remains visible
and free, click inside starts input, F12 stops it, and focus loss releases it.
No confinement API or desktop permission change is used. The enlarged-FIFO
ROM and edge-4096 timing remain unchanged. BootHush twin executable SHA-256:
`5d86fb1bec512cf8ea965437004ce33914dcb00c591e994f3497740745e8c4bc`.
Build/validation receipts:
`C:/Repos/.task-temp/boothush-relative-free-20261116-a`.
M1R2 human result is pending; the frozen archive and release gates are unchanged.

## M1R2 result and sensitivity investigation

Owner report: "OK, I did all the things. I was able to touch all four corners
with the mouse, and the windows never closed. However, was not relative. I had
to move my local cursor VERY far to reach each corner."
Corner reach and absence of unexpected closure passed by owner report. Mouse
travel/perceived relative behavior remains unsatisfactory. Lag improvement was
not explicitly reported and is not inferred from this response.

The session reached its five-minute limit normally with unchanged inputs:
1210 raw packets, two starts/two releases, 1055 scaled samples, 4704 pin steps,
zero pending input, zero omissions, zero decoder/FIFO losses, FIFO peak 120,
and zero guest fault. These counters support complete guest consumption, not
acceptable sensitivity. Original session and separate feedback/analysis are
in `C:/Repos/boothush/out/qualification-20261116-a/M1R2-mac`.
`corners.gif` is a 26-second excerpt of the guest video beginning at second
100; `derived-media.json` records the conversion and hashes. No desktop pixels
were captured and the excerpt is not a substitute for the owner report.

BootHush adds explicit linear gain (default unchanged at 100 percent), initial/
final settings in JSON and F9/F10 adjustment. Its native tests cover signed
fractional carry, limits, repeats, reset on gain change, host-only shortcuts,
focus handling and signed-range rejection. The CLI retains exact scaled-vector
replay at 100 and 400 percent. Source-matched executable twins:
`fe262bc58ae58cb1d9b22ae5f409b71693e6ebf0fd305e4846692a5873588de8`.
Build/validation: `C:/Repos/.task-temp/boothush-mouse-gain-20261116-a`.

Stress traces multiply the accepted M1R2 vectors while preserving all event
times and button states. They are controlled rate tests, not raw-packet replay
or physical latency measurements. At edge 4096, the existing 512-entry FIFO
overflows at 2x and 4x. A 1024-entry candidate passes 2x (peak 637) but still
overflows at 4x. All endpoints happen to match the independent clamped-vector
oracle, proving why endpoint agreement cannot stand in for loss accounting.
Failed controls are retained in `human-gain-stress-20261116-a/report.json`
and `human-gain-stress-1024-20261116-a/report.json` under `C:/Repos/.task-temp`.

The selected 2048-entry candidate passes the 1x/2x/4x traces: FIFO peaks
120/637/1404 respectively, zero overflows, zero decoder losses or omissions,
zero pending input and zero faults, with all three oracle endpoints matching.
Receipt: `C:/Repos/.task-temp/human-gain-stress-2048-20261116-a/report.json`.
This does not establish lossless handling of every possible rate or latency.

The 128/512/1024/2048 FIFO contracts pass at O0 and O2. Both enlarged native
candidates retain eight paired state/pixel checkpoints and exact Mac replay;
the selected candidate still cancels deliberate excessive-burst overflow,
releases the drag and accepts fresh exact movement. Selected receipts:
`mac-human-gain-parity-2048-20261116-a/parity/report.json` and
`mac-human-gain-recovery-2048-20261116-a/report.json` under `C:/Repos/.task-temp`.

The selected native ROM twins use 49,500 ROM bytes, 504 native data/BSS bytes,
41,944 workspace bytes at 32768-74712 and the unchanged conservative 4,924-byte
stack bound. The linker checks screen/workspace separation. ROM SHA-256:
`e6c0163867fe969075ccb4ef2dd9030830e1e9df9e5764476eda117027be1370`;
manifest: `C:/Repos/.task-temp/mac-human-gain-2048-20261116-a/manifest.json`.

M1R3 is admitted for an owner sensitivity test at 400 percent, edge 4096,
inversion 3, VBL guard 4096 and display scale 2, with the emulator/ROM above.
The Windows pointer remains visibly unconfined; this is not absolute alignment
between the two pointers. F9/F10 allow bounded adjustment, F12 stops input.
Human comfort, lag and continued interaction correctness remain pending.
Frozen release files are unchanged.

## M1R3 result and 300-percent retest

Owner report: "Slightly too fast... Does this have anything to do with how far
the emulator is zoomed in?" The session used 400 percent throughout (zero
runtime gain changes), with 1803 raw packets, 1802 scaled samples, 11264 pin
steps, zero pending input, zero omissions/losses/overflows/faults and FIFO
peak 283. It reached the five-minute limit normally with unchanged inputs.
Sensitivity remains slightly too fast; other case results are not inferred
from this short response. Evidence and separate feedback:
`C:/Repos/boothush/out/qualification-20261116-a/M1R3-mac`.

BootHush computes guest pixels as raw counts times gain divided by actual
integer display scale. Rendering multiplies guest pixels by that same scale,
so zoom does not multiply on-screen speed (apart from pixel quantization).
Larger zoom enlarges the whole canvas, so crossing it requires farther travel
at the same on-screen speed. This differs from Windows pointer acceleration
and does not imply alignment between the two visible pointers.

M1R4 keeps the same BootHush and 2048-record ROM identities, edge 4096,
inversion 3, VBL guard 4096 and 2x zoom; only initial sensitivity becomes
300 percent (25 percent slower than M1R3). This is a configuration retest,
not a new build or a human pass. Comfort and precise interaction remain pending.

## M1R4 recording and axis investigation

Owner report: "Horizontal movement feels faster than vertical movement in the
emulator... Maybe you should just record a video of the emulator while I move
the mouse..." After requested horizontal/vertical sweeps and a circle/square,
the owner reported "ok, done." This confirms completion of the requested
recording, not acceptance of sensitivity or a new interaction pass.

M1R4 closed normally after 223.860 wall seconds, with unchanged inputs and
300-percent gain throughout. It retained 2700 raw packets, two starts/releases,
2698 scaled samples and 12862 pin steps. Pending input, decoder losses,
omissions, FIFO overflows and guest faults were all zero; FIFO peak was 40.
The 512x342 guest video contains 1119 frames at 5 fps without truncation.
Original evidence remains in
`C:/Repos/boothush/out/qualification-20261116-a/M1R4-mac`.
No agent test motions were injected during the human session.

The actual recording excerpt `M1R4-axis-analysis/axis-comparison.mp4` covers
guest seconds 201-221; its manifest records source/output hashes and FFmpeg
conversion. The contact sheet was visually inspected. Some sweeps reach the
left/top limits, so their endpoint displacement cannot measure unconstrained
physical hand travel. The recording contains guest pixels, not the Windows
pointer or the user's hand. The trace records accepted scaled vectors, not
raw device counts. A 5-fps recording cannot resolve a 20-ms delay.

Headless replay of the complete recorded trace exactly matches final guest
RAM and pixels. All 31 checked recording frames, sampled from 201.8 through
219.8 seconds, also exactly match replay. This validates the replay against
the captured guest behavior; it does not qualify host-device handling.
Reproduction, commands and comparisons: `M1R4-axis-analysis/video-analysis.json`
under the same qualification root.

Controlled equal-axis tests use identical accepted vectors, edge 4096,
inversion 3, VBL guard 4096 and the unchanged emulator/2048-record ROM.
All eight cases (single 64-pixel bursts and 64 one-pixel events at about
125 Hz, each in four directions) move exactly 64 pixels on the requested
axis with zero loss, omission, pending input, overflow or fault.
First diagnostic completion of the bursts takes 135.1-137.1 modeled ms.
For the streams, completion after the last input takes 62.3-65.4 ms for X
and 85.4-86.2 ms for Y. These are guest diagnostic writes after composition,
not physical device-to-screen latency. Receipts:
`C:/Repos/.task-temp/equal-axis-20261116-a/report.json` and
`completion-timing.json` in that directory.

Thirty-two further stream runs vary the start across eight offsets spanning
one VBL period. All complete exactly without loss. In 15 of the 16 paired
direction/timing comparisons, Y settles 16.5-23.9 modeled ms later than X;
one comparison reverses the difference by 4.6 ms. This supports a small,
timing-dependent response difference in these workloads, not different
movement gain or proof of the cause of the owner's perception. Receipt:
`C:/Repos/.task-temp/axis-phase-20261116-a/report.json`.

No code, sensitivity or zoom was changed during this investigation. Mac
responsiveness remains open. Further attribution should separate quadrature
delivery, foreground consumption and cursor composition before changing
axis gains. P1 can independently qualify the 286 keyboard path and shared
scene interactions; it cannot close this Mac issue. Frozen release files
and the unqualified release disposition remain unchanged.

## P1 286 admission

The first 286 owner round uses the unchanged preview hard disk
`6f17583422edf2eddd4e5c325020a3538b4333a0a3305ca350b99cf91e992ba6`,
the previously validated BootHush executable
`fe262bc58ae58cb1d9b22ae5f409b71693e6ebf0fd305e4846692a5873588de8`,
and the original diagnostic font
`0d2224d4908bd4856e001b1e7aeb2409b92ebd785d5691d41db6253759ea0517`.
The native window is scale 2, limited to five minutes with complete guest
video at 5 fps, accepted keyboard log and final diagnostics at `0x60000:17`.
The launcher verifies input hashes before opening; mounted disk changes are
not saved. No scheduled guest inputs are supplied.

Prepared plan and resulting session directory:
`C:/Repos/boothush/out/qualification-20261116-a/P1-pc286`.
The owner is to check arrow-key movement, Space-toggle drag/release and
pattern changes. Human feedback is pending. Mac M1/M2 and subsequent 286
cases remain open; this admission is not a passing result.

## P1 outcome and control-instruction correction

P1 reached its 300-second limit normally with zero accepted/logged keys,
zero pending keys, ready POST B0 and the initial pointer/window/pattern state.
Inputs were unchanged. Its 1500-frame capture reached the configured video
limit (`truncated: true`); original logs and media remain in the session
directory above. The owner subsequently requested more release progress,
without reporting test actions or results. This is an idle boot observation,
not a human input pass. The owned window was confirmed closed.

The handoff instructions incorrectly described Space as a toggle. Inspection
of the admitted raw-keyboard payload and `platform/pc286/keyboard-set2.c`
confirms that Space make holds and Space break releases; repeated makes are
ignored. The older BIOS-keyboard diagnostic used a toggle. The P1 handoff and
its original plan are preserved as the erroneous instructions, not silently
rewritten as a valid test. Subsequent P1/P2 rounds must **hold Space while
moving with arrows, then release Space**; one tap in content makes one click.
The release TRYING document already described the correct raw-keyboard
behavior. BootHush's general human guide is corrected to distinguish images.

## M1R5 owner result, 2026-11-17

The refreshed cursor-byte ROM from the frozen review package was tested at
2x scale, relative-free input, 300-percent gain and edge spacing 2048.
ROM SHA-256: `5d48f7877386e30f8e45e0456f528e93c1990aa7dd5b7ef37b40ddd7cf1de5a0`.
BootHush SHA-256: `fe262bc58ae58cb1d9b22ae5f409b71693e6ebf0fd305e4846692a5873588de8`.
The owner confirmed the window was visible; the desktop helper was unavailable.
The agent launched the guarded session and injected no guest input or activation.

The owner reports that the Windows pointer is still faster, horizontal movement
feels faster than vertical, and relative movement fails the intended behavior.
Corners were approximately reachable; dragging and pattern changes worked.
This is a **failed human responsiveness result**, not a four-corner precision
pass. Release/focus behavior was not explicitly reported for this round.

The completed 300.109-second session accepted 2483 mouse samples and delivered
11058 pin steps; pending input, guest losses, faults and capture FIFO overflow
were zero. It nevertheless recorded **27 guest motion omissions**, with FIFO
peak 185. Final pointer is (328,196), window (231,110), pattern 0. The trace
contains scaled guest vectors, not raw mouse counts or Windows-pointer travel.
Its 5-fps, 1500-frame guest capture reaches the configured recording cap; the
report correctly marks it truncated. PNG and MP4 conversions succeeded.

Original session artifacts and the separate attributed feedback receipt are in
`C:/Repos/Release-Review/ClasSICK-Mac286-Cursor-20261116/M1R5-mac/`.
Original hashed files, including the incomplete observations template, remain
unchanged. Session JSON SHA-256:
`31e1145e75a290b90488d299df0c795c68290d5478858d461f5a398e67723725`.
Mouse trace SHA-256:
`081d544555ab3be94b66ff308a5b5a719fe329f809dfec1bf0cd054a661dbf37`.
The final guest PNG was visually inspected after completion; it shows the moved
window and original scene. That single frame cannot verify movement quality.

Headless replay reproduces all 27 omissions at spacing 2048. Changing only trace
spacing to 4096 yields zero omissions and FIFO peak 87. Both final frames match
the original: an equal final endpoint can conceal intermediate missing motion.
The slower pin timing is a diagnostic control, not a demonstrated sensitivity
or latency fix. Receipt:
`C:/Repos/.task-temp/m1r5-analysis-20261117-a/report.json`.

The previous M1R4 and synthetic timing results remain scoped observations. This
new owner trace exposes a condition those checks did not cover. The current
candidate remains unqualified; neither the frozen archive nor any image was
rewritten. No ClasSICK/game implementation or compatibility contract changed.

## M1R6 diagnostic admission, 2026-11-17

BootHush now offers an optional log of raw counts, scaled guest vectors and
sampled Windows-pointer displacement. Its
[immutable diagnostic receipt](https://github.com/segaboy/boothush/blob/c4a4cd5ab659d8310f176d89eeb84916c89f9329/docs/pointer-observation-20261117.md)
owns the format, host-sampling limitations, original implementation and tests.
That is a local commit; remote availability is not asserted. It changes no
ClasSICK code, host mouse setting or relative-axis gain calculation.

Executable SHA-256:
`9dc2f28425bafd10114cfde9ab5c439d61234873854cfa1c211cdcfb26bda82d`.
Identical build twins, four hidden C++ checks and 47 Python cases pass (three
optional fixture cases skipped). Replaying the actual M1R5 trace with the new
executable produces the same 2048/4096 results and exact final pixels as before.
Receipt: `C:/Repos/.task-temp/m1r5-pointer-build-20261117-b/report.json`.

The proposed M1R6 uses the same packaged ROM, 2x scale and 300-percent gain.
Edge spacing changes to 4096 to isolate the loss-free replay control; this may
alter delivery latency and does not establish acceptable mouse feel. The
five-minute plan records guest video, scaled replay input and the new pointer
log. The intended actions are short horizontal/vertical sweeps, each slowly
and then more quickly, with pauses and no sensitivity adjustment or edge hits.
This can discriminate total displacement from delayed delivery; host packet
coalescing prevents interpreting individual log rows as exact OS acceleration.

Prepared plan:
`C:/Repos/Release-Review/ClasSICK-Mac286-Cursor-20261116/M1R6-pointer-20261117/launch.ps1`.
The guarded P1R2 plan in `P1R2-pc286-20261117/` refreshes the 286 launcher identity
for the unchanged packaged disk/font. Previous plans remain preserved and
refuse the changed launcher hash. These are admissions, not executed human
passes; launch/handoff and subsequent observations belong in the local workflow
and a dated follow-up to this receipt. Release qualification remains blocked.
