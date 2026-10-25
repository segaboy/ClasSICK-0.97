# Paused-clock candidate data mode

2026-10-25,SRC-0440/0441;existing SPEC-0072/TEST-0087 context.
The owner is using the desktop and chose independent work. The original-ROM
attempt was stopped;no timed fixture was loaded or new input result collected.
Exact owned PID1940/executable identity checked before process cleanup,with
no further GUI interaction. Recorder was not finalized and is excluded.
ROM SHA256673a2cca unchanged;original right-primary flag1 preserved.

One Copilot Sonnet5.5 low primary supplies the original candidate mode in25.94s
from4730B/one frozen-public read/zero commands,writes,tests. Lead adopts it,
reproduces an invalid-JSON acceptance defect,adds parse-constant rejection
for NaN/Infinity/-Infinity,and corrects the tuple docstring. No second worker,
requalification or runtime recreation. One reported premium request;other
workers idle,all-four balances unavailable. No measured Codex saving.

## Data-only interface

Choose exactly one origin:existing `--start N` or `--paused-recording PATH`.
The latter also requires a positive `--offset N`. A source is1..4096 bytes
of UTF-8 JSON containing nonempty [timestamp,payload] pairs. All timestamps
must be identical plain integers in0..2**63-1. Payloads have one nonempty
kind that does not start with mouse,case-insensitively;values are opaque.
This does not establish an actual emulator keyboard schema or inert GUI.
The new start is the candidate timestamp plus offset;final-event overflow,
invalid source/mode and existing outputs refuse before writing.

New receipts add anchor,input SHA256,marker count and offset. Existing
`--start` event bytes and receipt fields are preserved. Timestamp semantics
remain false;the utility never starts a guest or patches memory/ROM.
Do not interpret a synthetic marker as an observed paused clock.

Run the portable checks with:

```
python scripts/Test-SnowMouseReplay.py --output <new-directory-in-existing-parent>
```

Fourteen legacy cases passed before integration and remain unchanged.
The final49 cases cover literal outputs/receipts,source hashes,single/eight
events,output/source-alias preservation,bounded source structure/encoding/
mouse/mixed-time/nonfinite rejection,origin/offset errors and integer/input
size boundaries. The portable rerun repeats those same49 cases,not49 extra
unique tests. Windows/Linux CI now runs the portable harness. Earlier source
1ac84e1 CI40117369587 passes both jobs;new CI pending at this source record.

The live experiment still needs a qualified paused marker,a clean pre-load
RAM control,held future playback before resume and confirmed completion.
No new movement,driver defect,timing,hardware or release conclusion follows
from data-tool tests. Mac motion and PC GUI left dragging remain release
blockers.286 evidence retained,no new backend/target run. No new screenshot
or animation:63PNG/twoGIF remain. All originals,held brief,prior records/
assets/releases/tags and retired OneNote/wiki preserved.

[Machine-readable receipt](paused-clock-evidence.json).

## Next live control (planned; SRC-0443)

1. Wait for an undisturbed desktop. Reuse only a verified original ROM and
   isolated profile;recheck identity/current inputs/seal. Do not infer
   availability from a generic continuation request while the owner is busy.
2. Keep execution paused during setup. Capture an actual original marker,
   requiring identical integer timestamps and no mouse events. The tool's
   opaque synthetic markers and valid JSON do not prove a paused emulator.
3. Before loading input,read the complete known original record:MWP1 magic,
   version/size,diagnostic magic/stage/fault,services,baselines,losses,queued,
   deliveries,presses,pointer and window. Require a settled zero-loss control;
   stop on contaminated setup,do not clear history to manufacture a control.
4. Run a separate original empty-input control through the same setup/resume/
   pause route. Require at least500 further services with no loss,input or
   fault. This tests the actual control route;paused RAM equality alone cannot
   exclude queued peripheral input or prove all GUI controls inert.
5. For one single-X trial,derive a positive-offset candidate from its own
   qualified paused marker. Recheck the pre-load record;after loading,require
   unchanged paused RAM and pending playback before resume. No counter reset,
   guest-memory patch or elapsed-time-to-cycle assumption.
6. Resume once,with no extra viewport gestures. Confirm playback completion
   and collect a guarded post record through the qualified control route.
   Frontend completion is distinct from guest acceptance or physical timing.
7. Report movement only when accepted records and pointer change agree with
   the untouched other axis/button state. Preserve loss/fault evidence. A
   completed frontend replay with no accepted movement is not a motion pass.
8. Any failed guard,nonzero empty control,unqualified timestamp behavior or
   interfering input makes attribution inconclusive. Do not weaken sample
   admission,repeat blind bursts or credit a release/hardware/GUI pass.

No step above was executed by this checklist. A recorded loss remains a
separate result from a proven cause;an accepted move remains separate from
continuous dragging and paired-release qualification.


2026-10-25,SRC-0446:actual foreground access restored,but initial and reset no-replay setup records retain loss. Stop before marker/empty replay/single-X;the planned checklist is incomplete. No controlled causal motion result. Own sessions closed,ROM/preference preserved. [Actual control](pc286-bank-plan-evidence.md).
