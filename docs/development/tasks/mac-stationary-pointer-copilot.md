# Mac stationary-pointer source review prompt

Windows11,ClasSICK0.97. Fresh supervised original-code reviewer;Codex sole writer. Review ONLY the frozen public/original inputs in this packet,once. No other paths,commands,writes,web,manuals,SDK/emulator source,delegation or tests. Published interface facts already admitted in SPEC-0075;do not invent new hardware facts. Preserve all existing source/role/native-app/physical/release gates.
Actual repeated physical test with a directly connected mouse:owner sees Mac cross stationary at center while moving slowly in both axes. Own scene counter reaches0002;exact physical press count not confirmed. Candidate is unchanged separate timer ROM7de4329b10bf34fa4f03f43def49fdaf8b1c34831cc3ebf11bfb2e3bfdc86606,Relative hardware emulation,no Apple disks/ROMs,guest patches or helper input. Both sessions closed. No physical CPU/record trace was captured;do not treat this as CPU fault or an identified hardware root cause. Existing normal SCC driver remains distinct.
Task:bounded source-level review of timer controller,phase model and supplied native wiring. Identify concrete code/contract defects if demonstrable;otherwise state evidence limits. Distinguish sampling aliasing/loss possibilities from established defects. Recommend ONE next diagnostic or original candidate,with exact trigger/observables,acceptance/refusal checks and minimum other file/contract needed. Do not blindly change reload,signs,address masks or drop uncertainty checks. If a code defect is directly demonstrated in supplied code,return a small proposed patch and an independent repro case description;otherwise return no patch. Not a general architecture audit or additional emulator retry plan.
Return ONLY JSON {result,findings:[{severity,path,line,evidence,disposition}],proposed_replacements:[],next_experiment:{goal,change,observable,acceptance,refusals,missing_inputs},tests_executed:[],blockers,decisions_needed}. <=1800words/150 proposed changed lines. Do not assign stable IDs or claim tests/historical behavior. Source sections are exact;gaps stay explicit.

Frozen public code revision49b619c5da8c8fdafa9ac857f201fb0514e4f315;exact inputs:

- docs/specifications/SPEC-0075-mac-timer-poll.md
- platform/mac128k/timer-poll.c
- platform/mac128k/polled-phase.c
- platform/mac128k/window-entry.c

Full code packet and transcripts remain outside Git.
