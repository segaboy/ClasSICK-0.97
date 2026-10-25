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
