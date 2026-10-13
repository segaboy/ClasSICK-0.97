# Heap lifecycle and dispatch: lead intake review - 2026-10-13

SRC-0135..0138. Research intake and choices for future contracts only. No new
SPEC, ADR, TEST, IMPL, executable interface or build record; no implementation,
fixture, HT case, historical probe or emulator/VM/reference run. Separate owner
approval is required before any implementation described here.

## Identity and preservation

Initial HEAD was exactly `bac81c361c9d3b92d798ff046add65fa7962cd2d`, with no tracked
changes and only the thirteen named untracked research originals. No pull, reset,
stash or clean was performed. The following identities independently match the
owner's handoff, before substantive review:

| Input in docs/research | Bytes | SHA-256 |
| --- | ---: | --- |
| heap-lifecycle-draft.md | 65282 | `79ca5051b7c447a91a083323733a8fbebf14374cc1dcbdc80c27adb7d8f76cb4` |
| heap-lifecycle-test-plan.md | 24272 | `5876485eb35cf95a55fd65e7dd1f7c6d0d70e57eab8b7b4d416046fec500481d` |
| heap-lifecycle-pe-comparison.md | 23620 | `5483b07b58d87bc3bb868b5afb08d781c88dda9d63cf8af4be213df801a5db92` |
| trap-convention-exceptions-draft.md | 11163 | `d0060ad9420b936ec6f42f84a833016d0629e354e4b38a1e5c235ef7228fc664` |
| first-application-dispatch-profiles.md | 13871 | `7c21e68ed1044bd625f836f1cf238f59f5d5256647dbf59fafe897bd15c81185` |

Byte-identical intake copies and an identity manifest for all thirteen files are
kept outside Git. The earlier callback pair, launch/segment pair, QuickDraw pair
and resource-fork pair are included. Originals remain unchanged and untracked;
neither their prose nor their local labels are silently promoted into contracts.
SRC-0136 groups the three heap files, SRC-0137 the convention note, SRC-0138 the
application profiles; SRC-0135 records this owner's intake scope and restrictions.

The first heap pair's research baseline is
`bf978462a2baf1919adee0e45b393f4eda83b114`, preceding finalized SPEC-0028. The
profile note identifies compiled `d9c0f55`; current session evidence is retained
unchanged. Claude's no-other-change/exposure assertions are attributed statements,
not a reconstructed audit of every prior concurrent action.

## Independent published-source checks

Both eligible copies were reidentified in full, including page counts:

- SRC-0071: 19,245,372 bytes, 1,284 pages, SHA-256
  `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`.
- SRC-0068: 53,888,388 bytes, 1,326 pages, SHA-256
  `924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237`,
  SHA-1 `4ed5b3c20289835996b160d7ab3a628f497f13c5`,
  MD5 `d3822276bc5f99a0e3e3beb69b609cbe`.

Book PDF pages are printed I+10, II+568, III+1003. PE mappings checked here:
assembly+98, RM+118, SM+627, MM+714, SL+776, DV+930, OS Utilities+1157.
The PE chapter footers give dates within that published edition; they are not
January 1984 evidence. pypdf extraction and thirty Poppler page-image checks are
private reading aids, not republished source material or executed test cases.

| Material | Independent check and implication |
| --- | --- |
| II-28 / MM-27, PDF596/741, images | InitApplZone discards old application blocks, selects the zone and clears its grow-zone function. Both warn about code inside that zone. PE adds its own growth/master-pointer figures; none supplies our private representation. |
| II-30 / MM-29/30, PDF598/743/744 | 1985 entry/result box and PE result list inspected as images; PE entry prose extracted. Limit is an address; lowering it does not cut back the heap. 1985 lists memFullErr; PE lists only noErr. No failure predicate established. |
| II-42 / MM-43, PDF610/757, images | Locked-block HPurge note exists in 1985 and is absent from the checked PE routine page. PE silence is not a contrary behavior. |
| I-453/454/457, PDF463/464/467; SM-5, PDF632 | Both scrap diagrams inspected. Content moves out and back; no master-cell identity is drawn. I-453 requires disk scrap readback at termination; I-457 allows state zero with resident data. A NIL handle alone does not prove that no scrap must survive. |
| II-19 / MM-16, PDF587/730, images; MM-59, PDF773, image | Both figures put BufPtr above the jump table. PE summary instead describes the end of parameters. Figures also differ on QuickDraw reservation and other details. Do not choose BufPtr or H_min from this disagreement. |
| II-10/17/18/27, PDF578/585/586/595; MM-14/25, PDF728/739 | Extracted prose confirms differing zone lifetimes, the 8K default, heap-only limit, broader interrupt prohibition and startup stack globals. Current fixed-zone/owned-stack rules remain engineering contracts. |
| I-114/115, PDF124/125; RM-16/17, PDF134/135; II-188/189/190, PDF756/757/758 | I-114 image and parallel prose confirm resource-map retirement; device prose confirms pre-reset goodBye and NIL dCtlStorage. They do not define every retained system pointer or an atomic callback order. |
| I-90/91/94/95, PDF100/101/104/105; PE assembly14, PDF112; SL-9/10/11, PDF785/786/787 | Parameter table and SL-9 inspected as images; remaining convention/entry prose read. Routine conventions have exceptions; OS manager membership is different from OS trap-word format. Launch/Chain entry shapes and continuation facts reuse the separately checked launch intake too. |
| II-374..376, PDF942..944; OU-10/11, PDF1167/1168, images | Five handle utility entry boxes checked. PtrToXHand, HandAndHand and PtrAndHand list A0 outputs in 1985, A1 in PE. Do not merge the two as one historical signature. |
| II-379/380/382/383, PDF947/948/950/951, images | Date and queue entry/exit boxes confirm register conventions. Secs2Date's printed box is not a complete implementation-ready input/output span contract. |
| II-362, PDF930, image; III-222/225/226, PDF1225/1228/1229, images | SysError uses D0; its exit is exceptional. All twelve listed A9xx words and four internal-only OS words checked in the numeric tables. This is not an independent exhaustive sweep of all OS words. |
| I-91/282/288/411, PDF101/292/298/421, images; I-163/165/197/257/258/358/362, PDF173/175/207/267/268/368/372 | Stack widths, NewWindow declaration/storage, retained InitDialogs code pointer and selected callback paths checked. GetNextEvent also describes FKEY execution; desktop prose also describes DeskHook. Those paths are missing from the proposed re-entry table. |

The thirty viewed images are book PDF101,124,292,298,421,464,587,596,598,610,
930,942,943,944,947,948,950,951,1225,1228,1229 and PE632,730,741,744,757,773,
785,1167,1168. Other locator/exposure/completeness assertions remain attributed
unless explicitly checked above or in the earlier lead reviews. This is a spot
check, not certification of all citations or of the survey's negative findings.

Use only published prose, public API/format widths and interface constants. Manual
samples/equates visible beside relevant prose were not used as code or copied.
Ancillary private-overhead expressions in extraction are excluded; MM-56's private
heap-derived minimum is not reviewed or adopted. No private heap reconstruction,
SRC-0070, prohibited implementation, ROM, executable, asset or emulator source was
an input. No historical observation occurred. Human provenance review remains pending.

## HL dispositions for future contracts

Adoption means a planning choice subject to a separately approved original
contract. It neither changes SPEC-0023/0028 nor admits a new trap.

| Proposal | Disposition | Required interpretation |
| --- | --- | --- |
| HL-E1 phases | Adopt with changes | Keep preflight, staging, commit, population, ready. Preflight validates immutable launch input, every reserved/read/write span and alias, arithmetic, records/cells, rounded and zero-size allocations, future manager references and shared stack boundaries. Staging/report/native metadata must not alias guest live state, caller reports, globals, system objects or session snapshots. Freeze/revalidate inputs before the first destructive write; no concurrent guest execution. Reserve exact placement/capacity, not just a sum of logical sizes. Commit is an original logical boundary, not atomic hardware writes. No callback or ordinary fallible allocation inside commit. |
| HL-E2 precommit refusal | Adopt | Outside explicitly owned staging/report space, leave RAM, manager, directory/gateway and globals unchanged. A staging refusal may leave staging bytes unspecified. Preflight-only makes no staging write. Report storage must be prevalidated and nonaliasing; inability to write it must not change protected state. |
| HL-E3 retained fault | Adopt with changes | Population failure keeps completed postcommit effects and starts no session; no rollback. Record the first destructive boundary and actual subphase. An internal fault during commit may leave partially recreated scrap or globals, so do not promise a completed scrap recreation for every postcommit fault. Preserve staging and truthful diagnostics until a recovery contract permits retirement. Completed population must have all predictable refusals preflighted. |
| HL-E4 teardown | Adopt with changes | Host-requested teardown between sessions can reuse preflight/staging/commit without population, preserving admitted scrap. Faults never trigger automatic teardown; saved SPEC-0028 stop frame stays unchanged. Repeated teardown can recreate scrap, change its handle, clear stale globals and scrub references: reject the claim that only the epoch changes. Future goodBye/close callbacks are deferred; their guest side effects cannot satisfy blanket no-effect refusal. No Finder launch. |
| HL-E5 scrap | Adopt with changes | Stage validated resident application scrap and recreate it; re-read authoritative ScrapHandle, preserve bytes, no identity promise, new-block flags an engineering choice. State zero alone is not disk-only. Define a complete size/state/handle matrix first: NIL/empty with unavailable nonzero content or disk-backed content is refused in the initial memory-only lifecycle, not silently treated as nothing. A valid zero-size live allocation remains distinguishable from a NIL master cell. Negative/inconsistent sizes and foreign/forged cells refuse. ScrapName and any retained data pointer must remain valid outside the retired zone, or require separately contracted staging/relocation. No disk readback service is implied. Charge staging/records/cells and physical rounding. |
| HL-E6 epoch/stale aliases | Adopt | Retire application records and clear the full owned application cell band; preserve the system band. Unallocated old cells use SPEC-0023's error path until reuse. Raw guest address reuse remains indistinguishable; an epoch cannot prevent a stale numeric value aliasing a new object. Direct reads use the checked guest map and receive no payload promise. Define counter width, refuse saturation before staging, never wrap. |
| HL-E7 guest reset routines | Adopt with changes | First host-only profile explicitly refuses guest InitApplZone, SetApplBase, Launch and Chain. Defer the proposed later guest-reset branches: they conflict with the no-active-session rule and require a safe execution/lifetime contract beyond a PC/return-token check. Preserve documented below-system-end SetApplBase no-op as later evidence, not a new binding or present D0 promise. |
| HL-E8 ApplLimit | Adopt with changes | Keep SPEC-0023's even/configured-envelope checks and no cutting existing allocations. Adopt CurStackBase minus an explicit S only as an engineering formula; 8K is the later documented default. Define whether DefltStack is profile input or authoritative guest input, snapshot it and check arithmetic/ownership. Session stack intervals remain independent. Fixed HeapEnd does not become ApplLimit. SetApplLimit admission deferred; future out-of-envelope memFullErr is a chosen 1985-based extension, not a PE/January84 fact or documented failure condition. No numeric B/H_min selected. |
| HL-E9 distinct flag states | Adopt with changes | Keep resident/unloaded/purge-eligible/empty/disposed/retired distinct. Future flag-only operations preserve other bits and create no purging; resource-bit preservation and relock clearing purge are explicit engineering choices. Locked HPurge behavior uses 1985 evidence only; PE is silent. Main CODE1 rule retains LK-9's 1985 qualification. Unload requires LK-17 safety, stable validated entries and moved-block target revalidation. Absent/NIL segments remain unsupported pending a resource/purge contract. No HPurge/HNoPurge, loader, EmptyHandle or flag binding added now. |
| HL-E10 interrupt refusal | Adopt | Refuse lifecycle/selected manager operations before effects in known or unknown interrupt contexts. Use explicit call-site context, not SR mask inference or absence from Appendix B. This does not declare all returning callbacks interrupt-driven. |
| HL-E11 session separation | Adopt | Reset/teardown only while no root session or callback is active. Preserve saved session reports/stop snapshots in separately owned storage. A CPU frame snapshot is not a copy of heap/RAM; later teardown can alter the RAM it formerly described. Never claim whole-memory fault snapshots. |
| HL-E12 system references | Adopt with changes | Inventory every owned service's retained data/code reference and retire/scrub it before reuse, including A5-world/port/window/menu/hook references once admitted. Initial profile refuses unsupported registrations; no ad hoc raw pointer scan. Preserve system objects except exact contracted scrubs. Future goodBye runs before reset and can alter guest state; defer its prepare/notification/revalidation/failure order to its own contract, outside nonfallible commit. Resource file closure may change system storage too. No universal historical scrub order inferred. |
| HL-E13 discarded bytes | Adopt | No discarded-payload fill promise. Clear owned cells and overwrite only explicitly created/preserved state. Deterministic fill remains an unadopted alternative. |
| HL-E14 Finder information | Adopt with changes | Future population creates an application-zone handle and writes AppParmHandle/+16 per LK-7, with checked complete writes and capacity. Scrub old AppParmHandle and its parameter slot during reset/teardown; a failed population must not leave a stale published handle. Account for other application identity/A5/jump-table globals in a named owned-global inventory. Placement/zeroing remain engineering choices; HL-Q1/4 stay open. |
| HL-E15 Chain | Adopt | Keep unsupported; later retained-heap/resource/A5/segment lifetime requires a separate contract. No native variant selection; CODE-only scope retained. |
| HL-E16 reports | Adopt | Record operation, phase/subphase, cause, committed flag, before/after epoch, scrap size/handles, actual global values and retained fault status. Use explicit unavailable/not-reached fields, not fabricated successful values after an incomplete commit. Do not reuse historical system error IDs absent a later mapping. |

Seven proposals are adopted (E2/6/10/11/13/15/16), nine adopted with changes
(E1/3/4/5/7/8/9/12/14). Later guest reset, purge/flag/limit bindings, callback
notifications and disk scrap recovery remain deferred within those dispositions.

## HT case groups: accepted as planning input, not executed tests

All HT-A..F remain unexecuted. No fixture/generator/guest bytes were created and no
TEST ID assigned. Even a case with a documented outcome needs an admitted original
operation and a finalized contract before it becomes an executable test.

| Group | Disposition and proposal dependence |
| --- | --- |
| HT-A1..22: admission | Adopt with changes. E1/2/5/6/7/8/10/11/12 and LK profile checks govern the entire group; A3 has a documented interrupt basis too. A19..22 are deferred future guest-reset cases, not present admission. A6 equality succeeds only if exact capacities/placement and all other checks pass. Add staging/report/input aliases and unavailable disk/nonzero empty scrap as precommit refusals in a later plan. |
| HT-B1..20: lifetime | Adopt with changes. E1/4/5/6/8/12/13/14 govern the engineering outcomes. B2's complete system byte equality is valid only when no contracted scrub or close writes there. Preserve unrelated system allocations; explicitly contracted closes/scrubs may change or release service state. B8 needs a live zero-size allocation; B10/11/12 require the E5 state matrix and may refuse. B13 rejects retention of the old record, not numerical handle reuse. B18..20 deferred to resource/driver/callback contracts. |
| HT-C1..10: limits | Adopt with changes. C3/4 retain SPEC-0023 rules; proposed reset formula and admission use E8/10. C5..8 are deferred register-admission cases; C6 memFullErr is a 1985-based engineering expectation only. C7 does not free/truncate existing extents below a newly lower limit. C9 keeps SPEC-0028 stack bounds independent. Defer C10's optional extra margin until a profile adopts it; no new 128K fit assertion. |
| HT-D1..17: flags | Adopt with changes for future plans, execution deferred. E9/10 plus LK-4/9/10/17 and SPEC-0023 underpin outcomes; even D4/6/9/11/12/15 require currently absent setup/services as applicable. D6 main behavior and D9/10 locked HPurge are qualified 1985 choices. D13/14 invalid/NIL handling remains engineering mapping. D17 must not assert that a disposed cell is non-NIL: invalidity is manager metadata, not its residual bytes. No purged state is introduced by flag-only transitions. |
| HT-E1..10, E3a: stale | Adopt with changes. E5/6/12 and SPEC-0023 govern outcomes; E8's moved-handle/dangling-copy fact does not prove epoch protection. Reads through stale values may fail checked guest-map access, including odd/unmapped targets, and need not succeed or return a fixed byte. After cell/address reuse a stale value may name the new object, including recreated scrap. |
| HT-F1..15: failure/teardown | Adopt with changes. E1/2/3/4/11/16 and LK preflight choices govern every case. F4/5 are review of violated preflight invariants, not ordinary expected failures after fully reserved preparation. F3 must distinguish partial commit from population. F6/7 cannot automatically recover a corrupt manager/incomplete scrap; define recovery or explicit refusal first. F8 preserves the stop frame and completed effects at stop; it cannot promise immutable RAM forever. F11 repeated teardown preserves scrap contents but may change handles/globals/cells, beyond epoch/report. No historical error/resume/Finder assertions. |

The draft dependence summary's "documented-only" column is not an availability
claim. Register/CCR cases HT-C5/6/D8 use future routine-specific contracts; none
extends the existing eleven-routine adapter by implication.

## SPEC-0017 Q2 and typed convention records

**Adopt with changes: use a per-routine convention record.** Bit 11 identifies
trap format; it cannot select register-versus-stack marshalling by itself. Keep
the full historical SPEC-0017 draft unfinalized and Q2 open for its January84
question. This is a future engineering decision based on later published evidence.

The twelve register-based Toolbox-format words are:

| Routine | Word | Convention evidence |
| --- | --- | --- |
| Dequeue / Enqueue | A96E / A96F | II-383/382; A0 element, A1 queue |
| Secs2Date / Date2Secs | A9C6 / A9C7 | II-380/379; documented register boxes, full spans still require a contract |
| SysError | A9C9 | II-362; D0 input, exceptional exit |
| HandToHand | A9E1 | II-374; A0 input/output |
| PtrToXHand / PtrToHand | A9E2 / A9E3 | II-375; explicit register inputs/outputs |
| HandAndHand / PtrAndHand | A9E4 / A9EF | II-375/376; explicit register inputs/outputs |
| Launch / Chain | A9F2 / A9F3 | II-60/59, SL-10/9; A0 parameter block, transfer of application control |

Retain Claude's "no stack-based OS-format trap found" as a bounded survey result,
not a universal proof, and the twelve as a lower bound rather than a complete set.
No independent full OS/PE sweep is claimed. Package selectors, not-in-ROM
interfaces and unpublished/internal conventions stay outside the survey.

DrvrInstall A03D, DrvrRemove A03E, AddDrive A04E and RDrvrInstall A04F are
explicitly internal-only in the checked table: **defer convention; reject implicit
dispatch admission**. Unknown records refuse before effects. Shared-table ordering
corroborates existing draft reasoning; it does not recover a physical table address.

PtrToXHand, HandAndHand and PtrAndHand have **A0 output in 1985 versus A1 in PE**.
Keep separate evidence variants; **defer executable profile selection**, rather
than synthesizing both-output compatibility. Ordinary register preservation and
CCR applicability for register-based Toolbox words remain unresolved. OS bit8's
A0 policy must not be applied to Toolbox trap-number bits. An explicit output
must not be erased by a blanket saved-register mask. A convention record needs
input/output and preservation masks, CCR policy, flag interpretation, completion
kind, context and applicability, beyond just a `register`/`stack` tag. No changes
to the thirteen executable registry entries or existing patch signatures.

## First-application profiles

**Adopt with changes as review input for prerequisite 2.** Forty stack records
supplement the existing 41-routine catalog; FlushEvents remains register-based.
No automatic registry import or manager binding. Specific declarations and widths
derive the entry offsets; they are not measured historical dispatcher frames.

- Frame arithmetic checked independently for representative mixed-width calls:
  NewWindow parameters occupy26 bytes, from refCon at entry+4 to wStorage at+26;
  result is entry+30 (four bytes). GetNextEvent mask+8/output-pointer+4/result+10
  and GetItem menu+10/item+8/string-output+4 are consistent with I-91. A returned
  SP advances by4 plus parameter bytes, leaving result space for the caller.
  Future contracts must distinguish normalized routine entry from the A-line
  exception's status/return frame, direct veneers, patches and auto-pop. No
  undocumented save area is inferred from PE LoadSeg's24(SP).
- Validate complete aligned frame/return/result regions with checked arithmetic
  and read/write permissions. Decode bool16's high-byte bit0 and char16's low
  byte explicitly. A result slot reserved by the caller has no initial-value
  promise. Unknown stack CCR remains SPEC-0017 Q10. Auto-pop needs its own exact
  successful-return cleanup/refusal contract, not a generic additional pop.
- Pointer spans are minimum obligations, not an exhaustive manager memory map:
  Rect8, EventRecord16, VAR WindowPtr4, counted string1+length are useful. GetItem
  must have a safe caller capacity or a precomputed exact output before effects;
  checking the old string length is insufficient. Manager handles/pointers need
  typed ownership, size/state and sentinel rules (for example NIL wStorage versus
  special `behind` values), not just arbitrary mapped bytes. Define aliases
  between frame/results/input/output, globals, scratch and owned control tokens.
- InitGraf's206-byte reservation and (A5) slot remain qualified as in the prior
  QuickDraw review; no private globals layout recovered. SetPort retaining a
  grafPtr also requires validation of the live port and later accesses, not only
  the four-byte global write. WindowRecord size and exact InitGraf writes remain
  open. Copied title/name and resume-pointer non-dereference are labeled inferences
  until a routine contract establishes them. Kept data/code pointers need owners,
  epochs, movement rules and revalidation at use, tied to E6/12.
- Re-entry table is **incomplete**. In addition to the listed accessory, DragHook,
  WDEF/MDEF/grafProcs/grow-zone paths, I-258 permits custom FKEY resource execution
  during GetNextEvent; I-282 and I-288 describe DeskHook drawing/click routes.
  Window/desktop operations and SystemClick must account for those call sites.
  MDEF sizing/drawing and manager/resource/allocation paths need transitive
  closure. A moves-memory label alone is not a complete callback predicate or
  proof that all such routines necessarily invoke grow-zone. Conditional refusal
  before effects is useful only if every enabled unsupported path is detected.
  A false-negative callback list must not silently execute or ignore guest code.
- Each actual call site needs its own signature, A5/context, pointer lifetimes
  and shared-budget completion/error rules; proc0 support is not blanket support
  for WDEF, grow-zone, driver or interrupt callbacks. ExitToShell is no-return;
  the other 39 stack entries return normally as listed (39 returning, one exit).
  LoadSeg continuation belongs to a separate loader contract. An application
  jump-table call may load its target; an ordinary Toolbox trap does not inherently
  pass through the application's jump table. Reject that wording in section4.

## Questions carried forward

Every HL-Q1..20 remains open, including Q18 despite the comparison's proposed
closure. Its selected comparisons are now evidence annotations, not historical
answers. No question is closed by an engineering disposition.

| Question | Still-open subject |
| --- | --- |
| HL-Q1 | Exact historical reset global writes |
| HL-Q2 | ApplLimit initializer/reference point and DefltStack/MinStack enforcement |
| HL-Q3 | Scrap handle identity, flags, size bounds, disk and exit behavior |
| HL-Q4 | Finder-information placement and timing |
| HL-Q5 | Chain resource/segment/A5 lifetime |
| HL-Q6 | Every other retained system reference |
| HL-Q7 | Historical notification/closure/reset order |
| HL-Q8 | Post-release launch failure/recovery |
| HL-Q9 | SetApplLimit error predicate and direct-store effects |
| HL-Q10 | SetApplBase growth/globals/boundary behavior |
| HL-Q11 | Purged/reallocated/resident-reloaded handle flags |
| HL-Q12 | Segment resource-bit changes |
| HL-Q13 | Live register/stack pointer safety during reset |
| HL-Q14 | Historical stale references |
| HL-Q15 | Meaning of original zone size at Launch |
| HL-Q16 | Discarded payload clearing |
| HL-Q17 | Growth/stack margin interaction |
| HL-Q18 | Full edition comparison/applicability; selected differences retained |
| HL-Q19 | January 1984 applicability |
| HL-Q20 | BufPtr text/figure discrepancy |

AL-2/4/5, LQ-1..15, SPEC-0017 Q2/7/9/10 and prior callback/QuickDraw questions
retain their prior status. The convention note's preservation/CCR/output choices,
the profile's record sizes/retention/re-entry completeness and future lifecycle's
ownership, recovery, global inventory and exact RAM costs require explicit contracts.
No numeric BufPtr/H_min, private-overhead/default-master count or final128K fit
is selected. LO-1..8 and other historical/emulator/VM probes remain unauthorized.

## Recommended next contract and prerequisites

Runtime prerequisite1, the bounded session executor, is already completed and
recorded as SPEC-0028; its root-only exit does not bind ExitToShell. **Next propose
prerequisite2: typed Toolbox-format argument dispatch with separate returning,
continuation and no-return completion contracts.** Obtain the owner's separate
approval before finalizing/implementing that slice. It precedes prerequisite3,
the application-zone reset/lifetime contract reviewed here, and later CODE-only
launch/segment loading. Existing four-byte push coverage has original session
evidence; its use in an unloaded entry still needs loader-specific validation.

Prerequisite2 builds on checked data/control/stack maps (SPEC-0022), logical typed
identities and original chaining (SPEC-0025/0027), bounded call-site execution and
shared budgets (SPEC-0026), and root-session stop/exit/fault ownership (SPEC-0028).
It needs a reviewed per-routine convention, normalized frame/result/auto-pop
model, alias/permission preflight, owned return/control targets, register/CCR
policy, and retained-effects/report rules. A no-return request inside a returning
callback remains explicitly unsupported unless a separately reviewed unwind
contract admits it. Loader-specific continuation may be reserved/refused until its
own validated loader context exists; no fake LoadSeg binding is needed to specify
the completion shape. Broad manager callbacks remain deferred.

Kept-pointer handling can first be contracted as explicit unsupported/limited
lifetime in this slice; it cannot promise application-reset safety before
prerequisite3 exists. Heap reset then needs the owned global/reference inventory,
staging budget, complete scrap-state matrix, epoch saturation, exact preparation/
commit/recovery semantics, and the fixed-zone launch-space account before any
population. Neither slice requires a new historical run for this intake.

All existing published-only, UART-role, native-application, owner-run, historical,
physical/128K/parity/three-edition and B2 criterion6 gates remain unchanged. HQ
content stays private. Git/HQ/wiki records only; no delegation/chats/automation/
OneNote publication. Existing compiled reader/session and native evidence stays
unchanged; this record adds research decisions, not compatibility certification.
