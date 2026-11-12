# BootHush Mac mouse investigation — 2026-11-12

The owner selected BootHush for continued Mac-port development. These runs used
only the original ClasSICK candidate and generated input, with no Apple media,
reference system, or emulator implementation source. The local BootHush Windows
executable SHA-256 was
`fbb27c63797e8fea00a3928d7a053fa9258a6fb147a6176912f2e595889d3205`.
The local BootHush checkout reported revision `3702053bbbcd65840bc004dc6d66df78d73d339c`;
that alone does not prove which source produced the executable. The 65,536-byte
ClasSICK ROM SHA-256 was
`faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c`, built
from `0f8d85df5b6fe21508c22594e0dc20923d4eebe6`. A fresh isolated twin build
reproduced that ROM identity. The owner's runbook SHA-256 is
`54dc93e90597c5db161fb65a9f2b1dec18eae38275dce8072005ea474299c333`.

## Automated and live observations

With the specified fixture environment variable set, BootHush's `mac_live` CTest
reported corners, two drags, release movement and a content click accepted:
zero hard losses, zero omissions, 2,279 merges. The test injects window messages;
it is not a physical mouse test. The exact output and workspace remain at
`C:/ClasSICK/boothush-mouse-20261112-a/mac-live-ctest.txt`.

In one live BootHush window, Windows automation sent two primary clicks and
movement. The candidate finished at pointer `(141,71)`, window `(80,70)`,
press count 2, zero hard losses and omissions, and 284 merges. BootHush reported
six samples, 394 delivered steps and no pending steps. The [guest-only frame](../evidence/screenshots/2026-11-12/boothush-live-mouse.png)
was converted losslessly from its PBM capture; the decoded RGB pixels match.
The subsequent attempted drag delivered motion but the recorded trace contained
no third primary press or release. The available automation drag action could not
select a mouse button on this swapped-button Windows setup. This is an
automation-input limitation, not proof of a BootHush drag defect. No live
primary-held drag or human/physical result is claimed.

Replaying that six-sample recording ended at the same cycle and produced
byte-identical 1,024-byte diagnostic memory and the same framebuffer SHA-256,
`e3096d795e6e670a9a475f1010ce08f6675f6da96d9fbb4c517d384c459a0add`.
The mouse recording SHA-256 is
`d637e1758008a2ad4a7412ebd0fbd7ddd9f1efcb095e93c0aadc82d9ce2a99e3`.
An earlier three-sample live movement/click and its replay also matched cycle,
diagnostic memory and frame. These are automated host-window observations, not
proof of physical input, historical accuracy or release qualification.

## Reproducible timing failure

The original diagnostic probe [Probe-BootHushMacMouse.py](../../scripts/Probe-BootHushMacMouse.py)
starts this exact ROM, schedules `(+32,0)` at guest cycle 60,000,000 and
inspects its own MWP1/MLC1/MMD1/MCM1 records at cycle 65,000,000. It verifies
the ROM identity supplied by the caller, the diagnostic tags and all recorded
input steps, and refuses to overwrite the output. It returns 1 when any case
misses its pointer, loss or omission criteria; the current candidate does so.

```powershell
$py = 'C:\Users\segab\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py scripts/Probe-BootHushMacMouse.py `
  --emulator 'C:\Repos\boothush\out\windows\boothush.exe' `
  --rom 'C:\ClasSICK\mac-candidate-anchored-20261112-a\classick-live.rom' `
  --rom-sha256 faf37c6f96f8eb2ee6cf7a1e58a8b1b9538055d2fc89493c7419eeea816f6c8c `
  --output 'C:\ClasSICK\boothush-mouse-20261112-a\new-report.json'
```

| Edge interval, VBL guard (guest cycles) | Final pointer | Omitted passes | Result |
| --- | --- | --- | --- |
| 8,192 / 4,096 | `(288,171)` | 0 | pass |
| 4,096 / 4,096 | `(288,171)` | 0 | pass |
| 2,048 / 4,096 | `(255,171)` | 29 | fail: requested right, moved left |
| 8,192 / 0 | `(287,171)` | 2 | fail: one pixel short |

All four reported 64 delivered steps, zero pending and zero hard losses. A
12-case timing matrix, with four starting cycles and edge intervals of 4,096,
3,072 and 2,048, reproduced the leftward final position in every 2,048-cycle
case; one 3,072-cycle case ended one pixel short. These are emulator timing
experiments, not measurements of real hardware or proof of emulator fidelity.

The `--mouse-witness-range 59998000:60040000` capture for the 2,048-cycle
case recorded 128 rows and zero dropped rows. At cycle 60,006,236 the X phase
was 2 when its SCC interrupt became pending. The next phase changed to 0 at
60,008,286, before the guest VIA read at 60,009,664. This demonstrates an
input phase change while the earlier interrupt was pending in this simulation;
it does not alone establish the cause of every lost or reversed movement.
The corresponding 4,096-cycle trace had 70 rows and zero dropped. Raw JSON,
matrix, replay and witness files remain in the private workspace above.

Two scratch-only diagnostic ROMs reduced or stopped foreground trace copying;
the latter's direct owned-session pointer was `(257,171)` at 2,048 cycles.
This did not restore the requested movement, and disabling trace publication
made the normal diagnostics stale. Neither experiment was integrated or
qualified as a fix. The Mac mouse timing blocker remains open.

## Feedback for BootHush developers

1. `boothush.exe --help` exits 2 and prints JSON `{"status":"error",...,
   "error":"expected pairs of options and values"}`. A normal standalone help
   option listing the documented CLI and limits would improve discovery. The
   README documents the options; this did not block testing.
2. The documented live runner caps `--timeout-ms` at 60,000. A live session
   therefore ends at most one minute after guest execution starts, including
   boot time. This interrupted one attempted drag before the action began.
   A longer configurable live duration or controlled pause/resume would make
   multi-step input capture easier. Existing `--cancel-event` is useful for
   ending a bounded run, but does not extend that cap.
3. Default mouse pacing and VBL guard allow the documented fixture test to
   pass, while the explicit timing cases above reveal lost or reversed guest
   movement. The controls and witness already enable reproduction. Please
   document the provenance and intended fidelity of the default edge timing,
   VBL guard and SCC latch behavior, including whether they model physical
   hardware timing or are input-delivery conveniences. No emulator defect is
   asserted from a comparison against our own ROM alone.

`ctest` was not initially on the default shell PATH; the existing ClasSICK
development environment supplied it. This was a local setup issue, not an
emulator deficiency. The Mac input fix, normal held-drag validation and Mac
release qualification remain incomplete.
