# ADR-0034 - Prepare heap retirement before destructive writes

Accepted engineering decision, 2026-10-14; SRC-0146..0148 / SPEC-0030.
Contract only, unimplemented; lead self-review, human provenance review pending.

Reset must retain system allocations, preserve admitted resident scrap and leave
session stop reports truthful. Reusing ordinary manager initialization would
clear both cell bands. A rollback promise cannot cover callbacks, disk recovery
or an internal failure halfway through retirement.

Choose an inactive host coordinator with exact capacity/placement preflight,
disjoint staging, full revalidation and a named first destructive boundary.
Before it, refusal leaves protected state unchanged. After it, an unexpected
fault retains completed changes, truthful partial-state diagnostics and staging;
the coordinator refuses further use until a separately contracted recovery.
Neither current ExitToShell nor a fault triggers teardown automatically.

Keep the original bounded manager's system state intact. The initial profile
admits no guest patches, callbacks, system-storage scrubs, disk scrap recovery,
Finder/A5/segment population or guest reset/limit entry points. Classify unknown
references as refusal rather than scan RAM. A 32-bit saturating epoch tracks
native ownership but cannot distinguish stale raw guest addresses after reuse.
Explicit host stack reserve/minimum capacity inputs avoid guessing historical
DefltStack/BufPtr/H_min values. New application scrap flags are a declared original
choice; system scrap remains unchanged. A future application consumer must meet
continuous global/reference validity before guest code can run.

Grok's bounded document cross-check is review input, not contract authority or
historical evidence. Codex retains design and adoption decisions. No original
research file is modified; no fixtures/tests/code or new runtime binding is
created by this decision. Existing own ROM/EFI images and historical gates persist.

2026-10-14 clarification (SRC-0149/0150): Claude's read-only review exposed
misleading overview wording and missing explicit controller transitions. Both
reset and teardown preserve admitted scrap; phase labels do not omit that work.
SPEC-0030 v2 defines cancel/refusal/success/FAULTED plan lifetime and state.
No implementation or historical behavior changes; eight cases stay unexecuted.
