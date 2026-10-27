# Research worker prompt: endpoint sampling and missing evidence

You are a supervised research worker for ClasSICK0.97 on Windows11/PowerShell.
The owner requests useful research and an interesting,accurate update for tomorrow,
while all desktop/emulator tests stay paused. Public baseline ebfbfb935ed560b687ffed931c0db94889ff9ce0.
Codex is sole repository writer/integrator/verifier. Your task is research artifact
generation,not an OS change or new behavioral contract.

All necessary original/reviewed-public input is embedded below. ZERO tools,reads,
commands,edits,web,delegation,tests or outside paths. No HQ,manuals,Apple/Microsoft
implementation,emulator source,reference media or untracked research originals.
Do not assign stable IDs or claim a proposal was executed. No actual trials.

Return ONLY one JSON object {files:[{path,content}],handoff:{result,actionable_findings,
tests_executed:[],blockers,decisions_needed}}. <=450 artifact lines total and
<=1300 words in the research note;<=160 words in handoff. No unchanged input copies.

Primary deliverable: a reproducible research study of endpoint-only phase sampling
and the exact ambiguity in our existing recorded Mac loss. Supply TWO proposed files:
tools/research/study_mac_sampling.py and docs/research/input-sampling-study.md.
The script must use Python3 stdlib only,with --repo --compiler --output arguments.
No network,VM,emulator or target execution. It may compile/run only an ORIGINAL
small C99 host adapter against the provided unchanged polled-phase.c and header;
write generated adapter,exe,CSV and summary JSON only below a new --output directory.
Reject an existing output dir;bound compiler/adapter subprocess times. Do not
modify the repo. Include original SPDX GPL-3.0-or-later/copyright2026DeanHowell
in script and generated adapter. No stability/performance/frequency claims.

Adapter: baseline two independent model states at the SAME raw endpoint/button.
For one state,feed every intermediate legal Gray edge and sum emitted dx/dy;for
the other,feed ONLY the final endpoint. Enumerate both axes X/Y,start PHASE0..3
(numeric phase,not Gray index),directions+1/-1,steps0..12,invert0..3,stable button0/1.
Expected1664 independent cases. Fixed other axisphase0. Gray forward cycle0,1,3,2.
Use begin/step public APIs,never modify their state to manufacture results. Check
both baseline return values and every full-cadence step,clear output initially,
report failures honestly and stop. Record CSV columns
axis,start,direction,steps,invert,button,rc,dx,dy,full_dx,full_dy.
Return-code names are explicit in the supplied header. Distinguish full-cadence
emitted DCD units from physical edges/pixels and from elapsed time.
Use a separate JSON summary for counts/classifications and the four trace candidates.
Never present endpoint agreement as proof that no physical transitions were missed.
Useful classes: exact emitted-unit agreement,whole-sample loss,hidden nonzero
full-cadence units,opposite-sign output,other mismatch. Direction inversion cannot
restore information absent from the endpoints. No physical root cause inferred.

Separately enumerate the recorded trace's missing previous PB4/PB5 assignments.
Current PB=AF,RR0A=RR0B=4C;previous have_phase=1,DCDa=DCDb=0,button0 (loss_previous
01000000). Prior PB4/PB5 are NOT retained;do not fill them from current PB.
Use prior raw PB8F/9F/AF/BF with RR0A=RR0B=0 and baseline through the API,then
current AF/4C/4C. Report each actual model rc,dx,dy;explain these four logical
possibilities are not probabilities or proof of which axis moved. Do not confuse
polled-model LOST3 with the timer wrapper's recorded loss_reason6 Gray jump.
The missing previous PB endpoint justifies a discriminating future observation,
not a sampling-rate,sign,latch or historical behavior fix.

The note should give the hypothesis,method,small expected examples clearly labeled
EXPECTED UNTIL CODEx EXECUTES,source/edition limitations and concrete next evidence.
No invented counts apart from the1664-enumeration cardinality;lead will insert actual
results after execution. Explain why stable current snapshots alone do not settle
coherence relative to the earlier accepted endpoint. Tie to the PC evidence without
conflating cases:Windows accepted12 guarded API mouse events while six own exports
stayed identical;the later physical attempt yielded a changed frame but was NOT a
completed human test because fullscreen hid the instructions. No same-packet claim.
All release E1/E2/E3/E4 gates remain. Keep primary historical/interface facts separate
from this original software/mathematical analysis. A fresh engineering experiment
can illuminate a blind spot without proving a native hardware cause.

Additional verified project measurements for a compact sidebar (not a size forecast):
Mac refresh ROM file65536B;existing native audit reports23772 usedROM bytes (360B
staticRAM,964B workspace,4992B conservative nonrecursive stack bound;DO NOT add
RAM categories or call stack bound measured). PC EFI file184320B;FAT boot disk
file67108864B. These are different metrics;180KiB EFI and64MiB container do not
imply64MiB of OS code. Current scene artifacts are not a completed System1 volume,
Mac-application compatibility proof or a286 install footprint. Lead will verify
all hashes/measurements independently. The shipped analysis must not need these
native binaries to run. No need to generate a chart;lead will render it from actualCSV.

Input hashes and reviewed-public code were frozen outside Git;see the research receipt.
