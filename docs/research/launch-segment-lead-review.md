# Application startup and segment loading: lead intake review - 2026-10-13

SRC-0129/0130. Research intake and carry-forward decisions only; no finalized
loader specification, executable interface record, implementation or runtime test.
The resource reader was completed and published separately: compiled revision
`12208ed5591c17fbd4c031c22e138eecc69f2671`, evidence revision
`25c959d53e29932a31e71c150db343fcc5631a62`, SPEC-0024 / IMPL-0031 / TEST-0032.
Its evidence is unchanged by this intake.

## Input identity and preservation

| Untracked input in docs/research | Bytes | SHA-256 |
| --- | ---: | --- |
| launch-segment-contract-draft.md | 42399 | `b6054b8ba5b039281c2ae85b059a62eaa6e51d08a3198705ca25ed78ec789a13` |
| launch-segment-test-plan.md | 26893 | `8fcbfdb6d057c93d87deccb4fdfdc8f421a18731f1cc7b262e41412a1bae89ad` |

Both sizes and complete hashes match the owner handoff. Byte-identical intake
copies and an identity manifest are retained outside Git. Both originals remain
unchanged and untracked. The six other research originals are also preserved.
The handoff's HEAD 190263b is the research baseline, not our current implementation
or evidence revision. References to SPEC-0023/0024/0026/0027 describe that baseline;
the current reader implements the same reviewed format boundary, not resource
loading. The assertion that Claude touched no other work is attributed to the
handoff, not independently reconstructed from a concurrent working-tree history.

All LK dispositions below are choices for future contracts. Adoption here does
not authorize building a loader, generating the proposed fixtures, launching an
application or running a historical probe. No new SPEC, ADR, TEST, IMPL or build
identifier is allocated. Case lists are not executed tests.

## Independent primary-source spot checks

Existing eligible published copies were reidentified in full:

- SRC-0071: 19,245,372 bytes, 1,284 pages, SHA-256
  `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`.
- SRC-0068: 53,888,388 bytes, 1,326 pages, SHA-256
  `924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237`,
  SHA-1 `4ed5b3c20289835996b160d7ab3a628f497f13c5`.

Printed page numbers were checked. Book PDF page = printed page + 10 for I,
+ 568 for II, + 1003 for III. Promotional Edition SL-n = PDF n + 776;
the checked SL footers date the chapter 8/24/84. MM-13 = PDF 727, dated 10/9/84.
These are chapters in the published Promotional Edition, not separately retrieved
excluded prepublication drafts. Text was extracted with pypdf and nine selected
pages rendered with bundled Poppler for visual inspection. Extraction is not
treated as proof of the figures' layout.

| Requested material | Independent check and implication |
| --- | --- |
| II-60, PDF 628 | Read LoadSeg's stack-word parameter, segment lookup, conversion of all of a segment's entries and transfer to the requested routine. The prose discourages direct calls; it does not forbid them. Launch preserves desk scrap. |
| II-61, PDF 629 | Visually checked Figure 2 and the header table: eight-byte entries, two-byte routine offset, four-byte push role, two-byte trap; loaded entry has a two-byte segment number and six-byte jump. Header has two word fields, first-entry offset and count. Format facts, not an instruction encoding or an Apple algorithm. |
| II-62, PDF 630 | Visually checked all four long fields, table bytes, the all-applications offset-32 note and execution of the first entry to load CODE 1. |
| II-57/59, PDF 625/627 | Read the always-resident locked main-segment note, prohibition on unloading one's caller and outside-target-segment restriction. Unload makes a segment movable/purgeable; it does not immediately dispose of it. |
| I-89/91, PDF 99/101 | Visually checked trap bit fields and parameter widths; read auto-pop semantics, parameter order, removal of the status word and callee cleanup. Format bit 11 does not by itself determine a routine's parameter or completion signature. |
| I-94/95, PDF 104/105 | Read preservation rules: A2-A6/D3-D7 for stack calls, special dispatcher saves for register calls and the A5 requirement. These pages do not promise all-register or CCR preservation by LoadSeg. |
| II-364/365, PDF 932/933 | Read system error 15 (segment read), 25 (heap allocation), 26 (CODE 0 read) and 28 (stack/heap collision). These IDs do not define our malformed-input refusals, precedence or recovery. |
| III-226, PDF 1229 | Visually checked A9F0 LoadSeg, A9F1 UnloadSeg, A9F2 Launch, A9F3 Chain, A9F4 ExitToShell, A9F5 GetAppParms. No records added to the executable registry. |
| SL-4, PDF 780 | Visually checked Figure 1: 32-byte parameters, QuickDraw slot at +0, Finder-information handle at +16. This is a handle value, not the address of the information data block. |
| SL-11, PDF 787 | Visually checked the literal 24(SP) parameter location and continuation prose. We do not explain the discrepancy by inventing a twenty-byte save area. LQ-3 remains open. |
| SL-13, PDF 789 | Visually checked four fields, below-A5 application-globals wording and the most-applications note. Retain the disagreement with II-62, including its QuickDraw-globals wording. |

Additional checks: SL-7/9/12 (PDF 783/785/788) for unload warnings,
GetAppParms/UnloadSeg signatures and the parallel entry/header description;
SL-12 was also visually checked. II-17/20 (PDF 585/588) and MM-13 (PDF 727)
confirm the default 8K stack allotment and parameter-area facts. Earlier reviewed
ISA rules in SPEC-0022 are reused, not newly verified against a Motorola page.
All other draft locators and its full-page/exposure attestations remain attributed;
this is a spot check, not certification of every cited page.

Only published prose, API widths, constants and documented interchange-format
facts are adopted. Sample instructions and ancillary equates visible in extraction
are not used, transcribed or copied into contracts, fixtures or replacement code.
No Apple/Microsoft implementation, executable, ROM, extracted resource, asset,
emulator source or excluded SRC-0070 material was an input. Neither manual proves
January 1984 behavior; the edition disagreements remain disagreements.

## LK dispositions for future contracts

| Proposal | Disposition | Lead decision and required qualification |
| --- | --- | --- |
| LK-1: CODE 0 decoding | Adopt with changes | Big-endian unsigned32 decoding is an engineering interpretation of the documented widths. Signedness remains LQ-1. Check every add/subtract/multiply, complete source span and strict24 logical target; do not truncate a long into CurJTOffset or a native pointer. |
| LK-2: CODE 0 acceptance | Adopt with changes | Keep nonempty eight-byte-entry tables, even sizes/offsets, offset at least32 and no more than7FFE, complete resource spans and above >= offset + length as a bounded engineering subset. The signed-word-safe ceiling is our restriction, not a documented limit. Small below-A5, including zero, does not waive later InitGraf span checks. Noncanonical slack/offsets/trailing bytes may be flagged engineering extensions. If slack is accepted, report the actual table end separately: it is below B, so B must not also be described as that table's exact end. Arithmetic and complete stack/parameter/heap nonoverlap must be preflighted. |
| LK-3: routine-offset base | Adopt | Default to the literal resource-start reading, counting the four-byte header; normal routine offsets must be at least4. Expose one named, data-selected policy and explicitly report unverified parity. Keep the alternative reading documented. AL-2/LQ-2 remain open; Fixture D proves only that a selected policy is honored. |
| LK-4: LoadSeg mechanics | Adopt with changes | Return-address minus8, six-byte consumption and transfer to entry+2 are an original continuation model inferred from the public calling and format facts. They are not recovered Apple mechanics or an explanation of24(SP). Validate actual trap PC/return, stack spans, table-relative alignment, positive segment ID, header range and every affected target before commit. Loader metadata alone cannot certify guest bytes that were altered after launch; define consistency/refusal rules in the future contract. Separate read/format failure from allocation failure rather than treating every failure as documented ID15. |
| LK-5: restore unloaded entries | Adopt with changes | Retain an immutable original table as an original strategy, with checked capacity and an explicit RAM charge. Account for per-entry/segment state too. It grants no final128K fit claim. Mutated guest entries need defined diagnostics, not silent reliance on a stale mirror. |
| LK-6: entry state | Adopt with changes | Use A5=CurrentA5; zero other ordinary registers; SR2000; A7=CurStackBase-4 with an owned exit token; PC=first entry+2 as an explicit hosted engineering state. This state is at the first jump-table instruction, not a historical promise about MAIN. The push may change CCR before LoadSeg. Preflight the complete writable stack and exit token, enforce budgets, and require a valid owned exit transition rather than treating any jump to a veneer address as success. LQ-5/7 stay open. |
| LK-7: application parameters | Adopt with changes | For the selected cross-edition engineering profile, clear the32 bytes, write the Finder-information handle at+16 and AppParmHandle, and leave the+0 QuickDraw slot zero until its own initialization. Full span/handle lifetime checks precede writes. +16 is documented by SL-4, not established as a universal1985 or January1984 requirement. A reserved field is not proof of equivalent behavior. |
| LK-8: zeroing and exit | Adopt with changes | Zero the declared below-A5 span as our deterministic choice; LQ-6 remains open. ExitToShell or a valid owned return ends the hosted session with distinct statuses, without a Finder launch. Define successful teardown separately from fault snapshots and retained completed effects. A heap reset must preserve defined system/scrap state and invalidate retired application handles; do not promise nothing resident across all outcomes or wipe all heaps. LQ-7 remains open. |
| LK-9: main segment | Adopt | Keep CODE1 resident and locked. Main-segment UnloadSeg is a no-op with an engineering diagnostic. The resident/locked fact is documented by II-57; exact erroneous-call behavior remains LQ-9. |
| LK-10: purging | Defer | Unlock/relock is a useful future plan, but the current manager has no reviewed purgeable-state service or launch/reset lifecycle. Define flag ownership, reset lifetime and moved-handle revalidation before admitting this operation. Do not imply purging exists. |
| LK-11: first scope | Adopt | Host-supplied immutable resource fork and host-only launch. Guest Launch/Chain explicitly unsupported. Target GetAppParms, LoadSeg, safely bounded UnloadSeg and ExitToShell only after their prerequisites; no support is added by this intake. This is a hosted CODE-only profile, not Finder/MFS/Resource Manager or general application compatibility. |
| LK-12: LoadSeg register saves | Adopt with changes | Preserve D0-D7/A0-A6 and CCR as an engineering superset, measured at the trap entry after the unloaded-entry push. PC/A7 change by the explicit continuation; no other control state is silently changed. Do not promise preservation of the caller's CCR from before the push, or infer Apple's all-register preservation from I-94/95. LQ-4 remains open. |
| LK-13: oversized segments | Adopt with changes | First executable-loader profile uses the documented32K segment ceiling. The format reader's ability to expose larger resources does not establish executable-segment support. Oversized segment execution is deferred to a separately named extension; LQ-11 stays open. |
| LK-14: interrupt context | Adopt with changes | Refuse the selected moving/lifecycle operations before effects in a known interrupt context. Use explicit call-site/context records, not the SR mask, and do not classify every completion callback as an interrupt automatically. Unsupported/unknown contexts are refused. GetAppParms' absence from Appendix B alone is not a complete transitive memory-safety contract. |
| LK-15: launch-space profile | Adopt with changes | Keep explicit B/BufPtr, stack allotment and H_min parameters; 8K is the documented default. No numeric B or H_min is selected now. Derive the future values from the owned RAM map, heap metadata/required allocations, reserved buffers and stack budget; check Z+H_min <= ApplLimit with overflow and overlap guards. Symbolic cases or invented B are not physical128K proof. LQ-8 remains open. |
| LK-16: direct LoadSeg | Adopt | Refuse a call without the admitted unloaded-entry continuation, including the unsupported auto-pop shape, in the first profile. Both manuals discourage direct calls but do not prohibit them. This is an explicit engineering limitation; LQ-14 stays open. |
| LK-17: unload caller undetected | Reject | Do not adopt normal completion while knowingly leaving a return path into movable code unguarded. No raw stack scan is required by this decision. Until a reviewed execution/lifetime model can establish safety, refuse or defer unsafe/unknown unloads with explicit diagnostics. The documented application warning is not proof that silent acceptance is safe in our bounded runtime; LQ-15 remains open. |
| LK-18: GetAppParms writes | Adopt with changes | Write only length+name bytes, the reference-number word and handle long. Preflight all destinations and define alias/overlap and fault behavior before any write. Name capacity and the source globals belong to the selected profile. This narrow deterministic write policy is not measured historical behavior. |
| LK-19: auto-pop | Defer | I-89 confirms the semantic purpose for returning Toolbox calls. Precise return-frame validation, parameter cleanup and failure restoration belong to the future stack dispatcher, not a blanket after-routine pop rule. LoadSeg auto-pop stays unsupported under LK-16; exit is explicitly no-return. No auto-pop implementation is approved here. |
| LK-20: first entry CODE1 | Adopt with changes | Require the first admitted jump-table entry to designate CODE1, per II-62. Validate during launch preflight where possible; do not wait until a partially initialized application has run merely to detect a bad first entry. Label malformed-entry refusal and any associated26 diagnostic as ClasSICK policy; ID26 documents a failed CODE0 read, not this new condition. |

Adopt: LK-3/9/11/16. Adopt with changes: LK-1/2/4/5/6/7/8/12/13/14/15/18/20.
Defer: LK-10/19. Reject: LK-17's undetected normal-completion proposal.
The rejected proposal does not close the historical question or prohibit future
UnloadSeg support under a reviewed safety contract.

## Open questions and test-plan corrections

LQ-1 through LQ-15 all remain open, including AL-2. The review selects future
engineering behavior, not historical answers. LO-1 through LO-8 are not authorized;
no emulator, VM or reference-system run is authorized. No fixture, CODE resource,
generator or test was created from the plan, and no cases/probes were executed.

Before adopting any cases into a future test record:

- Reconcile LK-6's initial jump-entry SR with LK-12's trap-entry preservation.
  E5/C5 cannot prove preservation of flags from before the push.
- Preserve D1/D2 as policy discriminators only. G21 is not evidence of AL-2.
- Replace oversized-segment acceptance G18 with the selected strict-profile
  refusal; a later extension needs its own boundary cases.
- U12's unguarded unloading is not an accepted runtime expectation. A case
  asserting nothing about later return does not establish a safe continuation.
- Reconcile R2/R3's retained fault state with X1/X2 teardown, including when
  inspection ends and explicit disposal begins. Cleanup is not rollback.
- Do not assert whole-state atomicity after a reset without a reservation/commit
  design. Finder-information allocation, scrap preservation and stale handles
  require a heap-reset contract. Allocation errors need their own classification.
- W1/Q3 must identify B as the chosen upper application boundary when slack
  leaves the actual table end below it. Keep this noncanonical behavior labeled.
- LK-20/G20 preflight timing and LK-19's returning frame need revised case
  expectations when those future contracts are finalized.

## Runtime prerequisites and recommended order

1. **Bounded session execution first.** SPEC-0026 currently completes only a
   returning call at its owned RTS sentinel. An application needs a separately
   owned session, bounded instruction/depth budgets and stack/control checks,
   explicit running/returned/exited/fault/refusal results, and a valid exit veneer
   transition. Define caller restoration and retained effects. Entry need not
   return as an ordinary callback; execution must still be bounded. Finalize
   this contract next, without loading resources or resetting heaps yet.
2. **Typed Toolbox-format stack dispatch.** Add separately checked parameter
   widths/locations, callee cleanup, return-frame/auto-pop rules and returning,
   continuation and no-return completion kinds. LoadSeg resumes the admitted
   application continuation; ExitToShell ends the session. Launch/Chain remain
   refused. A Toolbox bit pattern cannot be routed through the existing OS
   register-only signatures, and continuation patches are not ordinary RTS calls.
3. **Heap reset and launch-space lifecycle.** Current SPEC-0023 fixed zones and
   ApplLimit are insufficient. Define preflight/commit, metadata reservation,
   application-handle invalidation, system/scrap preservation, limit updates,
   successful teardown versus retained fault state and future purgeable flags.
   B/H_min values follow this contract and an owned RAM/stack budget.
4. **Confirm the four-byte push coverage before a loader fixture.** The natural
   original candidate, immediate-word MOVE to predecrement A7, is already in
   SPEC-0022's declared instruction/addressing subset; its word push consumes
   two stack bytes plus a two-byte immediate extension in the instruction.
   A focused future CPU check must confirm actual A7 bounds, extension fetch,
   CCR and failure behavior. This is a verification dependency, not evidence
   from an executed unloaded entry and not permission to copy manual samples.

These prerequisites can be planned independently of the unresolved offset base.
Only after their contracts and separate authorized checks should a CODE-only
loader adapter copy reader spans into guest memory. Resource Manager search
chains, MFS/Finder, managers, general CPU coverage and all application/boot
qualification remain later work.

Codex-assisted lead self-review; human provenance review remains pending.
Native-application owner gate stays closed. Existing compiled-reader/CI evidence,
boot fingerprints, UART role boundary, historical/physical/full128K/parity/three
edition and B2 criterion6 gates are unchanged. No delegation, new chats,
automation or OneNote; ordinary Git/HQ/wiki intake records only.
