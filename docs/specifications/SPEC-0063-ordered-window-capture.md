# SPEC-0063 - Ordered native window capture/drain helper

2026-10-22; original engineering contract following SRC-0362..0365/MG1-MG8.
Accepted implementation scope: isolated helper, hosted/native-object checks and
normal CI/Git/HQ. No Mac MMIO/ISR/framebuffer binding, target run or release.
API: apps/window-scene/capture.h. Native 68000 and x86-64 share the same C source.

The caller exclusively owns a live aligned descriptor and input-owner graph,
keeps their complete storage disjoint and provides exclusion between operations.
This sequential API is not an interrupt-safe publication scheme. No threads,
atomics, volatile-only claim, callbacks, allocation, MMIO or register recipe.
begin/restart/rebaseline/drain/stop are foreground-only. push/uncertain never
read or call the input owner. Existing owner lifetimes and alias preconditions
continue to apply; the helper retains a borrowed owner pointer until restart.

Sixteen records each hold a complete normalized cs_pointer_sample: int32 dx/dy
and buttons0..7. Every accepted record retains its order; no summing/coalescing,
button synthesis, physical pin interpretation or undocumented loss detection.
Caller-owned descriptor <=256 bytes on supported ABIs; no extra record storage.
Stack target <=512 bytes per native helper frame, checked on actual objects.
Neither bound is a complete ROM/System128K feasibility claim.

begin requires a fresh quiescent object, never reads its old fields, and a
valid READY owner. Check null, integer span overflow and direct descriptor/owner
overlap before effects; other graph disjointness/alignment are caller contracts.
A non-READY or invalid-tag owner returns STATE before descriptor writes/calls.
Initialize every metadata/last field, not unused records/padding: epoch1,
head/count/delivered/losses0,reasonSTARTUP,cause0,owner pointer,WAIT. Call existing
cs_window_input_cancel exactly once. Owner OK/CHANGED are success, returning OK
in WAIT. Failure records first raw cause/stage BEGIN_CANCEL and enters FAULT.

Every later operation requires the valid tag, nonzero epoch, recognized phase,
head<16,count<=16 and non-null owner. Null descriptor is ARGUMENT; other invalid
metadata or a zero/wrong epoch is STATE, with no effect. Invalid buttons return
ARGUMENT before descriptor reads. Production callers never mutate public state;
fields are exposed for diagnostics, not an admission/security boundary.

push in READY appends exactly one scalar record and returns OK. At capacity,
enter LOSS,reasonOVERFLOW,increment saturating losses,retain unread records/count,
return LOST;no overwrite or owner call. uncertain in READY does the same with
reasonUNCERTAIN even on an empty queue. Further producers in LOSS return LOST
without changes;WAIT returns BASELINE,FAULT OWNER,ENDED STOPPED. Diagnostic
counters never gate delivery;queue positions/committed count and epoch do.

drain in READY/empty returns EMPTY with no changes. Otherwise copy the next
record to last,advance head/decrement count BEFORE calling cs_window_input_sample
once. Owner OK/CHANGED -> saturating delivered++,DELIVERED. Failure -> FAULT,
retain first raw cause/SAMPLE and the remaining prefix;never requeue/retry.
drain in LOSS abandons the unread prefix(head/count0),then calls existing owner
cancel once:success -> WAIT,RESET;failure -> FAULT/raw LOSS_CANCEL. Repeated drain
in WAIT/FAULT/ENDED returns BASELINE/OWNER/STOPPED without calls or changes.

rebaseline is WAIT-only after cancellation; it submits zero motion and actual
current buttons once through the existing owner. It cannot fabricate a press.
Held left stays suppressed; a released observation permits the next genuine
press. Success -> READY/OK;failure -> FAULT/raw REBASELINE. Outside WAIT return
LOST/OWNER/STOPPED for those terminal/gated phases,STATE for already READY.
last stores the attempted baseline. No stale records survive recovery.

stop ends admission and abandons unread records(head/count0) before any owner
call. If owner is valid and READY,cancel once. Remain ENDED on success or failure;
return STOPPED or OWNER respectively,retaining first non-success cause and its
stage. An already faulted/non-ready owner is not called;stop still returns STOPPED.
Repeated stop is STOPPED with no changes/call. No implicit repair or detachment.

restart accepts a canonical ENDED descriptor only. At epoch UINT32_MAX return
EXHAUSTED without effects;otherwise validate the new ready/disjoint owner, then
perform begin behavior with old epoch+1. Metadata/diagnostics reset explicitly;
unused record bytes need not be cleared. Old producers must be stopped by caller;
their stale epoch tokens are refused. Calling begin again on live storage is
outside its fresh-lifetime contract, not an epoch-wrap escape hatch.

On owner failure retain the first raw non-success result/stage until explicit
restart. Successful later calls/stop do not erase it. Stage identifies the last
owner call when no failure has occurred. last identifies the last submitted
sample/baseline;cancel does not invent a sample. Owner accepts only OK/CHANGED;
all other results are failure. Full native storage safety remains a caller
precondition; no hostile arbitrary-pointer validation claim.

TEST-0075 MP1-MP12 map into the new actual TEST-0076 record when executed.
Keep synthetic queue/recovery observations distinct from true IRQ timing or
hardware loss detection. Mac M1-M7,PC GUI/human/CWP1 and all compatibility/release
gates persist. No existing boot input or published asset changes in this slice.
