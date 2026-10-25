# Original paused-clock candidate input mode

Windows11,public baseline1ac84e1. Codex sole writer/integrator/verifier.
One Copilot Sonnet5.5 low primary,one frozen packet only. No commands,other
reads,writes,web,MCP,delegation or tests. No manuals,HQ,untracked research,
Apple/MS code/media,emulator source or sample code. Return proposed code only.

Extend the supplied original scripts/New-SnowMouseReplay.py with a bounded
paused-recording candidate mode. Preserve all existing explicit --start output
bytes,receipt fields and refusal behavior. This is input DATA tooling only.
No ROM,driver,guest patch,GUI/session or emulator execution is authorized.

CLI: exactly one of --start N or --paused-recording PATH is required.
--offset N is required and positive only for paused-recording;refuse it with
--start. Keep required --gap,--case,--output unchanged. Existing output must
never be overwritten;all validation precedes writing.

Read at most4097 bytes of the recording and require1..4096 UTF-8 JSON bytes.
The recording must be a nonempty array of two-item arrays [timestamp,payload].
Every timestamp must have Python type int (not bool),0..2**63-1,and all must
be identical. Each payload must be a one-key object with a nonempty string
kind whose case-insensitive prefix is NOT mouse. Payload values are opaque
original marker data:do not claim an emulator keyboard schema or execute them.
Reject empty/malformed/mixed-time/mouse recordings,IO/UTF-8/JSON errors and
arithmetic overflow cleanly,before output. Candidate start=T+offset;the existing
last-event timestamp bound still applies. Offset/gap units are unverified.

Only in this new mode,add receipt fields paused_clock_anchor (T),
paused_recording_sha256 (exact input bytes),paused_marker_count and offset.
Keep timestamp_semantics_verified=false and tests_executed=[] always.
These are candidate data-consistency checks,NOT proof that the CPU was paused,
GUI controls were inert,live input was isolated,or an emulator consumed input.
No actual paused clock recording is qualified yet:the owner is using the
desktop,so the target trial was stopped and native validation is deferred.

Return JSON ONLY {files:[{path,content}],handoff:{result,tests_executed:[],
blockers:[],decisions_needed:[]}}. One complete original file,<=150 lines,
no unrelated formatting churn. Codex will run frozen legacy and independent
candidate/refusal controls. No second worker or speculative runtime fix.
