# Development flow study: where ClasSICK development could speed up without weakening gates

2026-10-27;SRC-0511..0515. Lead-reviewed research proposal,not an adopted policy or implementation authorization. Desktop/VM trials and production development remain paused. Codex retains all repository writes and final verification.

## Scope and evidence

Bounded27October sample at a2fc1d7:three public receipts,not the full project.
The [measurement record](development-flow-sample.json) pins inputs and timings.

| Measure | Observed |
|---|---|
| Five external calls |490.32s total provider wait;median114.45s|
| Fourteen trials |12 durations total2306.68s;two missing;zero qualification gates completed|
| Four selected public context files |228354 working-copy bytes;AGENTS alone94979B/1042lines|
| Two successful Windows CI jobs |815s and1282s;largest step in second job834s|

Trial durations include setup/wait/cleanup and may overlap. They cannot be added
to worker waits as labor or converted to a savings ratio. Actual loaded tokens,
active Codex time and quota savings are unmeasured;two CI runs do not explain
variance. The batches yielded useful findings,including the PC startup-byte-loss
fix and Mac button/loss observations. Unqualified does not mean worthless.
Commits,lines and host-case counts are not shipped functionality.

## Process principles (broad, not predictive)

- **Small batches:** keep each change independently testable so feedback comes quickly ([DORA small batches](https://dora.dev/capabilities/working-in-small-batches/)).
- **WIP limits:** limit work in process and remove bottlenecks instead of starting more work ([DORA WIP limits](https://dora.dev/capabilities/wip-limits/)).
- **Whole-flow measurement:** measure lead time, process time and rework across the whole flow. Speeding up a stage that is not the bottleneck may not shorten delivery ([DORA value-stream visibility](https://dora.dev/capabilities/work-visibility-in-value-stream/)).

These sources do not predict a speedup for a solo, AI-assisted retrocomputer project.

## Ranked opportunities

### 1. Native input testability (highest)

**Reason.** The paired release is blocked here. The12 timed attempts out of14 total38.44 minutes;two durations are missing. This is a critical-path judgment,not a measured largest share of project labor.

**Next experiment.** Design each trial to answer exactly one question with one stimulus:

- Keep the windowed instructions visible.
- Check foreground and readiness safely before any guest input.
- Record the complete `previousPB` endpoint, which is missing from the Mac trace.
- Treat PC API return values as host facts only. They are not guest acceptance.
- Do not retry an unchanged trial unless there is a new discriminating observation or a new route.
- Keep E1, E2, E3 and E4 separate and record them separately.

**Risk.** Focused diagnostics do not replace the complete interaction sequence required on both native candidates.

### 2. Hot context size

**Reason.** `AGENTS.md` mixes policy and dated history. Four selected public files total228,354 working-copy bytes. Re-reading them can add context;actual per-call loading and quota cost are unmeasured.

**Next experiment.** Propose a compact entry point covering current policy and current state. It would link to immutable history rather than restate it.

**This study does not move or delete any policy or authorization.** Doing that is a separate change that needs owner approval.

**Risk.** A summary can drift from or weaken an authoritative policy. A rule that lives only in the history could be missed.

### 3. Coordination cost

Briefing/review/integration time is unmeasured;provider wait does not establish
Codex effort. Apply existing rules:reuse qualified workers,one coherent primary
artifact,a second only for a distinct risk,and direct lead work for small glue
where coordination costs as much. Adopt sound output instead of recreating it.
Workers still cannot edit either checkout. Keep batches independently testable;
large artifacts can shift saved generation time into lead review.

### 4. Record granularity

Use one canonical receipt per meaningful slice and short policy/status/HQ
references. Assign IDs through normal intake and preserve all source reviews,
old records and24 originals. Avoid duplicate retellings in hot context. Keep
indexes sufficient for an auditor to find the evidence. This study itself adds
one report/measurement record,prompt and source intake;no policy journal entries.

### 5. CI duration

Verify-LoadedApp.ps1 -Sanitizers took834s (13m54s),65.1% of the1282s Windows job.
Profile its internal stages first;the next-largest stages were142s and59s.
The workflow has no explicit cache directive;internal caching remains unreviewed.
The815s versus1282s job difference has no established cause (n=2).

Preserve fresh reproducible twins,mandatory compilers and audits. Independent
profile parallelism or source-identity reuse needs an authorized implementation
and failure/equivalence controls. No CI changes now. Docs-only c65e4af already
used[skip ci],so fullCI is not claimed for every document update. CI optimization
may not improve release time while input qualification remains blocked.

### 6. Work selection

Keep paired mouse blockers primary,with at most one bounded independent286 lane
while waiting. Presentation/architecture work does not close mouse gates.
Parity means equivalent behavior with platform-specific hardware bindings.
Preserve native-app,System1,physical128K and publication requirements;do not
redefine a failed pointer as releasable. Watch for the secondary lane taking
attention from the primary dependency.

## Pilot: the next three slices

After the owner resumes development,keep one primary release diagnostic active,
plus at most one bounded independent286 slice while the primary waits. No new
orchestration framework,capability audit or recurring automation is needed.

For each slice,add a few fields to its existing canonical receipt:

| Point | Record |
|---|---|
| Selection |UTC start,one targeted release criterion,hypothesis,expected distinguishing result|
| Worker return |UTC time,selected input bytes,adopted/partial/rejected output and reason|
| Integration |Lead rework and affected verification results;do not use lines as productivity|
| Checkpoint |UTC time,qualified/discriminating-negative/inconclusive outcome,CI wait and stop reason|

Targets,not predictions:one useful result and one canonical receipt per slice;
no unchanged retries;less repeated context and avoidable lead rework. Report
selection-to-evidence time separately from CI completion and idle/user waits.
The first pilot establishes a baseline;three varied tasks do not prove causality
or a percentage improvement. Shared/rounded account utilization cannot isolate
Codex savings without task-level data and controls for other activity.

Identity/safety refusal stops immediately. If a route is inconclusive,do not
repeat it unchanged:require a new observation or method. Choose another authorized
diagnostic or independent work;ask the owner only for a genuinely missing decision.

## Authority and scope

Several recommendations already exist in the protocol;consistent application
comes before more process design. Windowed instructions,concise task packets,
qualified-worker reuse and short receipt references fit existing rules once work
resumes. This research does not resume tests. Reworking instruction files,adding
CI instrumentation/decomposition or changing reuse rules would be a separately
authorized implementation slice with coverage/equivalence checks. No gate change
is proposed;all clean-room,source,native-app,System1,physical128K and release
requirements remain. Worker writes to either checkout remain prohibited.

## Review and reproducibility

Claude supplied the report from a12218-byte reviewed-public packet in36.12s,
zero tools/writes/commands/tests. Its168lines/1380words exceed the150line/1100word
request;no retry was made. Lead adopts the draft,correcting the12-of14 timing
coverage,unsupported time-share/context-loading claims,and approval/stop wording.
These recommendations are inference from a small sample,not validated savings.

The [measurement record](development-flow-sample.json) includes input hashes,
all available trial durations,worker waits,CI timestamps and source eligibility.
The largest CI entry invokes Verify-LoadedApp.ps1 -Sanitizers. A bounded literal
reference walk finds10 Verify-script names/242 lines;it misses dynamic/non-Verify
calls and is not a runtime profile or proof of redundant tests. Substage timing
is the next evidence needed before changing that834s pipeline.
