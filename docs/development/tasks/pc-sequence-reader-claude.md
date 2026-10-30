# PC sequence reader qualification

Windows11. Baseline8eec7c3371acfa62a6fa357ce55d7429daa7e2af.
One bounded Claude Opus5.5 Low reader/test deliverable;Codex alone integrates.
Use frozen SPEC-0085 and the reviewed original proposed reader only. No HQ,
manuals,untracked originals,UART,Mac/ROM/68000 material,emulator source,web,
commands,writes,delegation or execution. Existing qualified CLI controls apply.

Return complete scripts/Read-PCInputSequence.py plus a focused independent-of-
implementation API test file tests/test_pc_input_sequence_reader.py. Combined
artifact at most400lines;handoff at most180words with tests_executed:[]. Preserve
parse_registers,unpack,replay,summarize,decode and FILE/--json CLI. SPEC-0085 owns
the exact names,typed refusal,ambiguous loss classifications and gap limits.
Reject malformed known-register lines/duplicates/missing registers and invalid
uint64 lists. Do not infer a controller/host cause from aggregate error steps.

Tests must include independently stated literal register words for compatible
overflow/unsynced/status losses,unflagged model mismatch,signed9-bit packets,
gap alignment,counts/unused data and malformed known registers. Do not derive
expectations by copying the candidate algorithm. No actual execution here;
lead will run them,contribution real-bus conformance and pre-frozen vectors.

The captured prompt lists exact paths,line counts,SHA-256 and EOF for each input.
Return JSON:files (path:string contents),handoff (result,actionable_findings,
tests_executed,blockers,decisions_needed). Worker proposals require lead review.
