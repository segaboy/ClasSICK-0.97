# SPEC-0066 - Foreground capture/drawing coordinator

2026-10-23;SRC-0390/0391/0393,ADR-0078;IMPL-0068/TEST-0080.
Original production-use software boundary joining SPEC-0065 inbox and dedicated
SPEC-0063 capture helper. No CPU mask,ISR,SCC/VIA program or boot binding.
The [C header](../../apps/window-scene/foreground.h) defines the exact owned API.

## Preconditions and scheduling

One CPU,single non-nesting producer;serialized foreground. Borrowed coordinator,
inbox,capture,owner graph and hook storage are live,aligned,disjoint,stable and
exclusively owned as specified. Fresh begin does not inspect old descriptor bytes.
Producer stays quiescent through begin/stop/restart and before storage rebinding.
Hooks are infallible,exclude every producer entry and provide compiler/publication
ordering. enter returns an opaque32-bit token;leave restores it exactly. Foreground
otherwise permits capture. No hook or owner re-entry. Real primitives are unadmitted.

Every foreground inbox read/mutation occurs inside the short section. No capture,
owner,render or direct leaf call may occur there. Helper metadata is foreground-only.
Producer uses only the inbox API. The coordinator has no allocation,drain loop,
mutable global,MMIO or atomics. One native descriptor is bounded by128 bytes;
one private by-value sample is copied field by field while excluded. It adds to,
and does not replace,the inbox/helper/owner/workspace and total stack budgets.

## Operations

| Operation | Required ordering and effects |
| --- | --- |
| begin | Null/callback/ready-owner preflight has no effects. Initialize fresh coordinator;unmasked capture begin/cancel;excluded inbox begin. Failure closes/faults initialized inbox,retains raw cause/stage and returns FAULT. Success is ACTIVE/WAIT at epoch1;no hardware enable follows |
| step | ACTIVE only;at most one claim/delivery or one loss recovery. Preflight dedicated helper valid/sameepoch/count0/legalhead and WAIT or READY. Under exclusion enforce inbox/helper phase correspondence before accepting a new record. EMPTY or WAIT returns EMPTY without owner call |
| baseline | Claim/copy under exclusion;unmasked helper rebaseline once;excluded completion. Later producer samples may stage during PRIMING. Return BASELINED even if a newer loss remains pending |
| ordinary sample | Claim/copy once;one unmasked helper push then one drain;excluded completion. No coalescing,retry or queue replay. Return DELIVERED for that claimed record even if a newer loss is pending |
| loss | Excluded inbox discard first;keep LOSS sticky through unmasked cancellation. Helper READY requires uncertain+drain RESET;helper WAIT needs no duplicate cancel. Only successful cancellation permits excluded resume,whose existing result is CS_INBOX_BASELINE. Return RESET |
| stop | At quiescent boundary,excluded inbox stop/discard before unmasked capture stop. Coordinator ENDED even on failure;repeat stop returns STOPPED with no hook/helper/owner call |
| restart | ENDED,quiescent producer,ready owner. Excluded preflight both valid ENDED empty/unclaimed descriptors,same nonzero epoch and no-wrap exhaustion before any effect. Unmasked helper restart/cancel precedes excluded inbox restart to matching epoch. Success clears old diagnostics and enters WAIT |

Claim is acceptance. Only one accepted delivery may finish during newer loss;
the next call must recover loss before accepting a later sample. Stop is not an
interrupt callback. Detectable overflow/uncertainty is sticky;this software does
not guarantee detection of hidden physical edges or zero motion from silence.

First raw owner/helper failure and stage freeze. Owner raw cause is taken from
the helper on CS_CAPTURE_OWNER;other failing helper/inbox results remain raw.
Failure takes precedence over concurrent LOSS and closes admission. Unread fault
prefix remains until explicit stop. Failed state permits only stop's existing
safe cancellation path;no automatic repair. ENDED repeat stop never replays it.

Restart invalid/exhausted preflight leaves all descriptors unchanged(no owner
call). After a valid restart attempt begins,old coordinator diagnostics clear.
If helper restart fails,the inbox stays ENDED and descriptor epochs may differ;
coordinator remains ENDED with the new raw fault. If later inbox restart fails,
keep admission closed and retain the actual helper state. No rollback or retry;
a fresh quiescent lifetime is required for that mismatch. Exact CPU-level
quiescence and ordering remain caller obligations,not properties proved here.

## Verification and release scope

TEST-0080 distinguishes frozen API-only oracle,real-owner clamp/drag/release
behavior,synthetic producer interleavings,compiled defect controls and compiler
size/import/frame facts. Hosted models do not prove instruction ordering,CPU
mask restore,actual ISR latency or full128K fit. Mac M1-M7 and both source/hardware
binding gates remain;the shared native software is not installed in ROM/EFI by
this spec. Existing PC GUI/human/CWP1 and all compatibility/release gates persist.
