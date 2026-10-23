# Mac first-loss observation assignment

Windows11; paired-native-preview-2. Frozen baseline cfaeb7a3b4ba325e03993d8671f016e9d07be0a0; runtime source0ce4c34e. Codex remains sole writer/integrator. One Copilot Sonnet5.5 low primary, prior qualification reused. No second worker or automatic retry.

Use only the supplied frozen original public files. No other paths, commands, writes, web, manuals, research originals, HQ, Apple/Microsoft/emulator implementation, delegation or execution. Return proposed artifacts; tests_executed=[] is required.

Actual own-ROM black-box result: startup passes; foreground ACTIVE, four baselines, three loss/rebaseline cycles, peak queued0, delivered0, unchanged pointer256,171. This does not identify the failing sample-admission condition. Do not guess a fix or relax stability/pending/loss rules.

Deliver a bounded first-loss record and targeted original tests. It is observation only: no extra hardware reads/writes, allocation, counter meaning changes, admission-policy changes or callback calls. Store the first actual transition to loss for the lifetime of a prepared motion object; retain it through discard/rebaseline, later losses and stop. A fresh successful prepare clears it. WAIT instability and already-LOSS service do not create a record. Hard faults are separate.

Append a 28-byte `cs_mac_motion_loss` structure to the motion object: uint32_t reason, rr3, snapshots[3], previous, inbox_phase. Reason values NONE=0, NO_PHASE=1, UNSTABLE=2, PENDING=3, OVERFLOW=4. Use the existing decision precedence: missing phase before unstable before pending mismatch; overflow only when push returns LOST. Pack each already-read snapshot as PB<<16|RR0A<<8|RR0B; pack previous as have_phase<<24|last_raw_a<<16|last_raw_b<<8|last_buttons. Record inbox phase at entry, before changing it. No added hardware transactions. Commit reason last; readers still use existing interrupt exclusion, not an atomic ABI. Stay within motion<=192 and combined motion/trace<=320 bytes.

Append this same record after the existing100-byte native MWP1 trace prefix, copy it fieldwise under the existing observe exclusion, and bump trace version to2/bytes128. Keep the separate diagnostic64-byte prefix unchanged. Never introduce structure assignment into a volatile target if that adds an unexpected runtime helper.

Return exact unique replacements in motion.h, motion.c, window-entry.c and tests/mac-motion.c. Include useful tests for all four reasons, exact raw snapshot order and pre-loss fields/phase, no record in WAIT/already-LOSS/fault, first-record retention through recovery/stop, fresh reset, and unchanged service IO transaction count. Do not duplicate the driver to generate expected values. Keep existing tests. Output JSON only: {replacements:[{path,old,new}],handoff:{result,actionable_findings:[],tests_executed:[],blockers:[],decisions_needed:[]}}. Prefer <=260 changed code lines. No history/historical claims or proposed test counted as run.
