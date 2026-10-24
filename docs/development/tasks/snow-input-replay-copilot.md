# Original bounded Snow replay fixture generator

Windows11,public baseline5db64b0. Codex sole writer/integrator/verifier.
Copilot Sonnet5.5 low,one frozen packet only. No commands,other reads,writes,
tests,web,MCP or delegation. No manuals,HQ,untracked research,Apple/MS code,
ROM/media,emulator source or manual samples. Return proposed artifacts only.

Deliver scripts/New-SnowMouseReplay.py,original standard-library Python,
<=110 lines. Its output is original input DATA for the installed Snow1.5
recording/replay interface,not implementation code or guest patches.

Lead independently recorded original project input through Tools>Record input:
JSON array of [timestamp,{"MouseUpdateRelative":{"relx":integer,"rely":integer,
"btn":true/false/null}}]. Observed button-down and up each have relx0/rely0,
timestamps differ500000;later GUI navigation records18354/32767 with btnnull.
No timestamp unit/origin or replay-reset behavior is yet qualified. Do not
infer seconds,physical motion or stable emulator API from this observation.

CLI requires --output PATH,--start N,--gap N,--case single-x|slow-x.
single-x emits one event(+1,0,null);slow-x emits eight such events separated
by gap. Keep no button or other-axis events. Require start>=0,gap>0,all values
plain integers (argparse type=int),last timestamp<=2**63-1;refuse BEFORE writing
on invalid values or existing output. Never overwrite. Parent must exist.
Write exact compact JSON UTF-8 plus newline,then a concise JSON stdout receipt
with case,start,gap,event_count,last_timestamp,SHA256,bytes,original_data=true,
timestamp_semantics_verified=false,tests_executed=[]. No output sidecar.
No ROM/session/GUI/VM/file-opening execution,installed-tool inspection or imports
of emulator components. No inferred historical/performance/release result.

Return JSON ONLY {files:[{path,content}],handoff:{result,tests_executed:[],
blockers:[],decisions_needed:[]}}. Codex will review,integrate,run independent
literal-output/refusal checks and decide whether the native replay is useful.
