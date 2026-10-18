# SPEC-0045 Integrated prepared-port execution

Status: accepted engineering implementation profile,2026-10-17. Owner "Next step"
continues SPEC-0042/0043/0044 into one native owner/coordinator, original selection
fixture and independent tests/compiler/CI. No target run,release,full QuickDraw,
SDK/global registry/callback/JSR expansion or historical claim follows.

The frozen public API is [code-select.h](../../personality/system1/code-select.h),
with constructor-owned scalar state in code-image.h. SPEC-0042 call/word/frame/
alias/order/fault rules and SPEC-0044 complete owner/lease/cleanup rules apply.
P5 is coordinator-local canonical A873/A874 with unsupported directory slots,
not global dispatch registration. P6 is truthful CODE0 allocation covering the
206-byte reservation,1..2 disjoint108-byte records and4-byte output,never auto-growth.

Frozen lead requirements (API phase corrections, normative):
1. Persist complete actual stopped frame/status/cpu/memory/steps/entered plus immutable
   first invariant cause/phase/PC/word. Retirement report reconstructs run from it.
2. Scalar owner state includes numeric owner_id and complete cs_code_layout snapshot.
   uintptr_t identities are comparisons only, not post-borrow dereferences.
3. Every resolved safe return clears ENTIRE selection workspace, including nested
   pointers. Unsafe envelope ARGUMENT preserves all outputs. Unproved quiescence
   preserves workspace and reports borrow_unresolved=1; no recovery/retire allowed.
4. Null/misaligned/short capacity/overlapping native objects, count outside1..2,
   null/misaligned active view/port are ARGUMENT, outputs/image byte-inert. Numeric
   candidate/range/guest ID defects and preclaim world/generation/budget/directory
   failures are REFUSED without claim/guest effects. All native liveness is a host
   precondition, never diagnose arbitrary dangling pointers. In-root bad guest
   parameter/frame/word/patched slot is CPU/policy; admitted trusted mutation is Q.
5. Protect cap/views/ports versus ENTIRE workspace/caller/report/image/request/map/
   lifecycle/manager/gateway/inventory controls and original/current guest backings,
   pairwise including partial aliases. Core43 checks only its passed subobjects;
   this does not prove whole-owner envelope. Reuse actual original owner checks.
6. Revalidate original cap/views against owner-made snapshots every instruction and
   every service, full table/record extents. No guest record content digest. For
   stopped retirement never dereference former views/capabilities. Guard context
   and frame stay root-local. Do not force depth0/quiescent1 on finish: driver
   controls actual synchronous depth and refuses unresolved proof.
7. Frozen test-only schedule in code-select.h: after N successful instructions,
   HEADER flips a retained pinned A5 anchor byte; BINDING changes a WS-owned view
   field (not const input); QUIESCENCE sets owner depth=1. Subsequent ACTUAL checks
   detect mismatch; Q reports first fault and preserves prior effects. QUIESCENCE
   remains unresolved, WS retained, retirement inert. kind0 clears schedule.
   No production test symbol/callback/guest injection. Existing image_test_fault
   simulates RELEASE failure after ordinary selection stop.
8. Admission canonical original checks, owner/native/candidate/range/directory
   checks, snapshot and all pinned destinations/initial frame BEFORE claim. After
   claim only total stores, RUNNING publication; no validation/lookup failure.
9. Preserve code-run.c bodies. Original execute/external/legacy native/retire must
   refuse active/stopped/retiring/quarantined selection before effects. New private
   handback invokes real original RELEASE while execution_state3/4 and select
   HANDBACK protect ownership. No temporary execution_state0. Successful empty()
   may reset only original execution state; retain consumed selection tombstone.
10. Every nonanchor protected header/table byte remains checked. Exact-g requires
    full owner identity/layout/lease match, not tag. Retirement pins WHOLE below
    and anchor before any store; zero below then4anchor, preserve stop+counts,
    original RELEASE once only, failure Q; malformed current cell okay at retire.
11. CPU/guest fixture is original legacy CODE only; OS service body same native C
    for68000/x86. No target run/release, workloads/released assets unchanged. Avoid
    variable modulus/division/runtime helper imports; power-of-two mask alignment.

Implementation and executed evidence will be recorded separately.
