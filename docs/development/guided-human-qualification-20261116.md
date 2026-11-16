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
| P1 | 286 | Boot the same logical scene; arrows move the pointer; tap Space to hold, arrows drag, tap Space again to release; content press/release changes the pattern. | Not run |
| P2 | 286 | Held/repeated arrows, release, focus switch and return, restart with a key held; distinguish keyboard release from the guest's Space button toggle. | Not run |
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
complete. The [BootHush receipt](../../../boothush/docs/relative-mouse-20261116.md)
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
