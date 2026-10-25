# Retained guest-view inventory - first reviewed slice

2026-10-25,SRC-0442/0443. Lead adopts the scoped inventory and arithmetic
bounds as engineering evidence;all C16 API/backend changes remain proposals.

First slice only: `guest.h`, `m68k-call.h/.c`, `toolbox.h/.c` at baseline 3e35bbd. Other modules (`guest.c`, `m68k-bus`, `m68k-session`, trap gateway) are UNREVIEWED, not absent. This review does not adopt a production API or claim complete coverage. It proposes no runtime/API/ABI change and no benchmark, compiler, emulator or hardware run. Retained resource/source/cache views, patch routes, drawing state, the full instruction bus and the physical backend are out of scope.

A native handler call here is distinct from guest execution. Its no-mutation,
no-reentry/no-retention contract is in toolbox.h:21-22,not SPEC-0069.
Validation and read/write function calls are not automatically execution
callouts. A view variable may remain in scope without a post-call dereference.
Map stability alone does not qualify segmented chunk-pointer lifetime.

## Inventory (14 rows, see JSON)
1. **core/memory/guest.h:14.** Caller-owned raw backing pointers. Their borrow must remain truthful/stable;active runtimes add immutability requirements. Proposed C16 bank descriptors still need ownership/admission;identifiers do not encode qualified far pointers.
2. **core/memory/guest.h:17,22-23.** Flat view API and map/buffer lifetime. SPEC-0069 requires a separate C16 surface for logical cross-bank spans. The32767 chunk policy is distinct from actual Toolbox spans,which are at most256 bytes. Keep native flat paths.
3. **m68k-call.c:13 (`interval`).** The local view from cs_guest_window is validation-only,never dereferenced;no explicit execution callout appears here. Proposed C16 logical range preflight should avoid creating an unused borrow.
4. **m68k-call.c:70 (A5 window).** Validation-only. `a5` stays in scope across the step loop (lines 84-99) but is never dereferenced. C16: logical range check.
5. **m68k-call.c:71 and 79 (push).** Acquired at 71 and written at 79. Between them only local state and `copy_frame` run, so it is consumed before the callout. C16: per-operation 4-byte logical write after preflight. Preflight must precede entry;commit must be infallible or transaction-capable,not merely buffered.
6. **m68k-call.c:90-91.** Bus-step/trusted executor callout. Local a5/push variables remain in scope,but this function performs no post-call view dereference. Proposed C16 temporary chunk borrows must finish before guest execution. Borrowed map/options/config remain immutable (m68k-call.h:19,23-24);bus internals are unreviewed.
7. **toolbox.c:33 (`records_valid`).** Validation-only stack window,rechecked at140;no direct view dereference. Proposed C16 logical range preflight. guest.c internals are unreviewed.
8. **toolbox.c:185,234,244,246.** Frame view spans the handler and is written afterward. Checked extent<=42 bytes:normal6+32+4;autopop6+32+0+4 because result must be NONE. Actual frame writes<=10 bytes. These are arithmetic bounds. Proposed logical preflight/commit must preserve infallibility;buffering42 bytes alone is not a transaction guarantee.
9. **toolbox.c:185,196,246.** Exit frame spans cs_m68k_session_request_exit. Valid exit records have no arguments/result/autopop,so extent/writes are6 bytes. Session internals remain unreviewed. Preflight must precede the accepted exit request;no new fallible operation may follow it.
10. **toolbox.c:221, 230, 234.** Read spans (`access&1`). `input.span[i]=views[i].bytes` is read by the handler during the invocation only. Each span is <=256 bytes (argument_valid line 22; STRING is a length byte plus up to 255). The sum over `CS_TB_ARGUMENTS` is <=2048 bytes (arithmetic bound). The trusted handler contract forbids mutation/reentry/retention. C16: native keeps the borrow. For C16 choose an owned bounded copy per span, or change the handler ABI (deferred choice).
11. **toolbox.c:221, 239.** Write spans (`access&2`). Handler output is staged in `reply.output` (<=`CS_TB_OUTPUT_BYTES`=16) and copied to `views[i].writable` at 239. Preflight covers executable overlap (222-227), argument aliasing (228-229) and stack aliasing (220). C16: keep `reply.output` as the owned staging copy, then per-operation logical writes or a capability. Commit must be infallible, with no partial write.
12. **toolbox.h:19,21-23.** Handler raw pointer ABI is invocation-only. A span<=256 can still cross a bank boundary;size alone does not qualify a pointer or concurrent borrowed spans. C16 owned-copy/accessor/ABI choices remain deferred;native fast paths stay unchanged.
13. **toolbox.c:174;toolbox.h:17.** kept metadata refuses before this dispatch acquires its frame/argument views. Earlier records_valid already acquired a validation-only stack window. No retained-handler support or zero-memory-access claim follows.
14. **Value reads, not borrows.** Examples are m68k-call.c:29,69 and toolbox.c:123,144,189,210,217. They copy values through `cs_guest_read`. C16: stay per-operation logical reads of width 1/2/4.

## Local vs post-call spans
- **Validation-only or consumed-before-call**: rows 3, 4, 5, 7.
- **Read by a handler**: row 10.
- **Written after a handler or external call**: rows 8, 9, 11.

## Recommended first prerequisite
A small contract for a *preflighted bounded logical span commit*:
- It covers whole-map and whole-span preflight, then a commit that cannot fail.
- Start with validation-only logical range checks and a preflighted4-byte call-stack commit.
- Later cover frame extent42/frame writes10,replies16/combined Toolbox writes26,read spans256/input total2048. These are arithmetic bounds,not measured budgets.
- A staged byte buffer alone does not make a fallible backend commit atomic.
- It follows SPEC-0069 sections 3 and 5 (validate before effect, checked capability).
- Logical addresses and lengths stay 32-bit with 24-bit address limits.
- Adjacent regions are not merged.
- Native 68000/x64 keep the flat fast path. No per-byte callbacks and no wholesale staging.

## Blockers before qualified native bank access
- No qualified bank-offset to far-pointer conversion.
- No infallible-commit guarantee, so staging is needed unless one is shown.
- No physical-alias proof or general C alignment guarantee.
- Map immutability does not define a segmented chunk-pointer lifetime.
- The handler span-pointer ABI is undecided.
- `guest.c`, `m68k-bus`, `m68k-session` and the trap gateway are unreviewed.

## Decisions needed
- Whether C16 span handlers get an owned copy or a changed ABI.
- Bounds may inform later design;no stack/RAM fit or performance conclusion is adopted.