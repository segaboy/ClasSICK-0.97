# Mac motion: bounded controller/latch audit

Windows11;public baseline5a9a0e0. Codex sole repository writer/integrator.
One Copilot Sonnet5.5 low primary,read-only frozen packet. No commands,other
reads,writes,web,MCP,delegation or tests. No manuals,research originals,HQ,
Apple/MS implementation,ROM/media,emulator source or manual sample code.

Task: inspect the original controller/binding against its OWN contract and
actual trace. Return one useful bounded defect analysis plus a ready-to-run
original C counterexample ONLY if there is a supported concrete code defect.
Do not invent a fix to justify code. If no defect is established,identify
one discriminating black-box observation and the exact fields needed. Do not
suggest repeating blind GUI bursts or weakening stability/loss checks.

Actual latest Mac native b3cb3fe: foreground resumes,fault0,two baselines/two
losses,services5918,VBL5916/SCC2,zero delivered records/presses,pointer256,171,
window80,70. First loss UNSTABLE2,entry READY3,RR3=9,
snapshots AF4444 /9F4444 /AF4C4C,previous01000000.
Snapshot pack PB<<16|RR0A<<8|RR0B. Master loss-quiesce and fresh baseline
rearm are already implemented/tested. Tiny native three-byte by-value ABI
fault already fixed by pointer-only samples. GUI mouse mode is RelativeHw.
Latest PC follow-up packet3 repeats dx255/dy-255/right-button2 during two
one-pixel/reverse gestures plus recapture. This does not prove a Mac cause.
Do not duplicate first-loss diagnostics or already-completed recovery work.

Reviewed interface facts (lead paraphrases,not manual pages):
- SCC control register pointer shared across channels,returns to0 after a
  complete selected control access. Native scc_read ALREADY skips selector for
  RR0; do not recommend removing a write that does not exist.
- Native whole-chip owner,non-nesting producer,IPL7 veneers;16NOP byte guards
  retained. Data byte widths/read-even/write-odd rules preserved.
- Per-channel external/status source latching is not an event queue;DCD can
  latch transitions. RR0 may contain live/latched conditions. Exact chip/model
  and physical polarity are not proven. Do not assert an exact unstated latch
  reset algorithm or emulate the full chip from these limited facts.
- PB3 low=left-down;PB4/PB5 are second X/Y phases;DCD A/B first phases.
- RR3A bits8/1 are A/B external pending;RR0 DCD bit3;WR0 external reset0x10,
  highest-IUS0x38. This last command warns unsupported in observed Snow;no
  fallback or emulator-source inspection is authorized.

Review scope: separate supported bug,contract limitation,host-injection
hypothesis and unverified hardware question. Check IO ordering,loss recovery,
publishing and whether trace actually supports a change. Keep finite ISR,
ordered records,all strict phase/pending/stability checks,existing source/
native-app/physical/128K/B2/System1/release gates. No historical OS algorithm
or foreign implementation reconstructed.

Return JSON ONLY {findings:[{id,kind,evidence,impact,proposed_action}],
counterexample:{path,content}|null,next_observation:{action,fields,stop_rule},
handoff:{result,tests_executed:[],blockers:[],decisions_needed:[]}}.
<=1000 words excluding optional <=140-line C artifact. A counterexample must
use existing public APIs and literal expected results from the contract;
proposed tests are not executed. No replacement implementation on speculation.
