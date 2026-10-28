# Canonical records and active instructions

Owner-authorized 2026-10-28; SRC-0516. Applies prospectively. Preserve all earlier
records, stable identifiers, source reviews, contradictions and research originals.
This changes record placement and duplication, not evidence or approval requirements.

## One receipt per meaningful slice

Choose one existing or new canonical receipt in `docs/development/` for the scoped
deliverable. A readable Markdown receipt or structured JSON record is sufficient;
add a companion only when it supplies distinct data or a usable artifact. Reuse
the existing record for the same claim. New conditions, contradictions, subsequent
observations or corrections need explicit dated evidence and supersession links;
never overwrite a previous result to make it agree with a later one.

Keep these facts together, when applicable:

- Authority, exact revision/input identities, target criterion and scope limits.
- Source/spec/test/implementation IDs and claim classification; edition and rights.
- Result, artifact paths, actionable findings, actual checks and their outcomes,
  blockers, unresolved questions and decisions. Proposed cases remain unexecuted.
- Worker/model/effort, selected input identity, elapsed wait, tools actually used,
  usable artifact disposition, lead corrections and provider-reported usage.
- Selection, return and checkpoint times where known; distinguish active effort,
  provider/CI/desktop/user waits. Unknown values stay unknown. No quota equivalence
  or claimed savings without measurements that support them.

Keep verbose transcripts, private captures, account quota details and generated
build output outside Git. Public own-product media still requires the existing
[screenshot policy](screenshot-evidence.md), exact identity and rights review.

## Update records for changed facts only

| Record | Update trigger and minimum content |
|---|---|
| Canonical receipt | A new meaningful slice or changed observation; owns the detailed account |
| Provenance and required SPEC/TEST/IMPL records | New source, contract, implementation or verification claim; allocate through existing intake, reference the receipt |
| ADR | A genuine architectural decision; preserve rationale and supersession |
| Status / roadmap / release matrix | A changed milestone, next dependency or qualification state; short fact and receipt link |
| Private HQ registries and evidence | Required cross-project intake; existing IDs, immutable public revision and scope/limitations; unique private facts stay private |
| AGENTS.md | A durable rule, current authorization/pause or changed navigation; never an experiment journal |
| Orchestration protocol | A changed allocation/control rule or reusable qualification; keep per-call results in the receipt |

Do not touch every index after every task. Required registries, provenance, source
exposure, test results, compatibility status, licensing and publication review
remain mandatory when applicable. HQ may require a separate evidence ID/record;
that record indexes the public receipt instead of copying its narrative or tables.
Never transfer HQ content into public records or worker packets.

## History and lookup

Instruction snapshots are immutable, byte-identical Git blobs at the original
directory level, so their relative references retain their original base. Their
date and baseline revision identify their historical scope. The active entry
points carry current rules; an older permission or prohibition is not a new grant
or a reversal of a later explicit owner amendment. Unsuperseded scoped rules still
apply: consult the relevant source/contract/history before affected work.

Read applicable instructions once per context and when their identity changes;
use exact sections and existing qualification results. Do not routinely reload
complete histories. Keep all gates and source/role separation. The permanent
OneNote/wiki retirement includes its Git queue, mirrors and manifest.

Current implementation receipt: [workflow-compaction.json](workflow-compaction.json).
Desktop, VM and emulator testing remains paused.
