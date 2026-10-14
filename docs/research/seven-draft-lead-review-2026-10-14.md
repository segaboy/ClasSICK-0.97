# Seven-draft research intake - 2026-10-14

Lead self-review; human provenance review pending. Research intake only.
No stable source/specification/ADR/implementation/test identifiers are allocated.
The original temporary labels below remain research labels. An adopted proposal
is a recommendation for a future scoped contract, not implementation authority,
an executed case, a historical answer or a replacement for existing contracts.
No HQ change belongs to this intake.

## Baseline and identities

The preceding Claude/Grok qualification was completed separately: public
`c9485d4dde6f9715356100957480b6e291503787`, HQ
`2447a95` (both pushed). SPEC-0030 document revision2 remains unimplemented.
The incoming request observed public `81259d2`; the earlier MFS/window drafts
observed `33e203f`. Those are research baselines, not today's implementation
state. Intake began at c9485d4 with no tracked public changes and24 untracked
research originals. No pull, reset, stash or clean was performed.

All seven supplied sizes and complete hashes match before substantive review:

| Original in docs/research | Bytes | SHA-256 |
| --- | ---: | --- |
| later-docs-gemini-audit-draft.md | 7060 | `a42ba9eedddab4b1158b94dbc2e1400f66cd785e8f66f7abf0c9a255a9156a4c` |
| mfs-format-draft.md | 24979 | `18d70a069628c44a85b805b0575e1c2e7c2e7dfd7cbbb385c1752640e2df215f` |
| mfs-format-test-plan.md | 13254 | `22496e7c2219a88f7a0796a44ef9f3d452082a74b22c94ecb489d09dc217e420` |
| window-manager-draft.md | 28510 | `116117a6fe8119a46cf4572a220d10d848598845d3cfb14dc9549db5b3a63c7a` |
| window-manager-test-plan.md | 12958 | `e3d12b642119714cac4004a81695fdb86116af62ca271b37e043552864b81f38` |
| system-startup-obligations.md | 47721 | `920107312de8db64ccbc7fd488bec050849d45fcb1f3eb81ee8d7615e6f775ec` |
| system1-forward-map.md | 31280 | `4fa6c9043f387312afa7790372c289ccaa8a132e9e4fb2cdf96997c0336051f4` |

Byte-identical copies of all24 originals, owner request, baseline identities,
manual identities, focused page renders and final preservation results are
outside Git in the private workstation directory
`seven-draft-intake-20261014-a`. Originals remain unchanged and untracked.
No raw manual page, worker prompt/stream, research original or binary is added
to Git. Claude's reported source exposure remains an attributed statement;
these checks do not audit every action in Claude's earlier session.

## Independent source checks

Both previously eligible published manual copies were reidentified:

| Existing source | Bytes | Pages | SHA-256 |
| --- | ---: | ---: | --- |
| SRC-0071, Inside Macintosh I-III,1985 | 19245372 | 1284 | `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97` |
| SRC-0068, published Promotional Edition | 53888388 | 1326 | `924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237` |

SRC-0068 also matches SHA-1 `4ed5b3c20289835996b160d7ab3a628f497f13c5`
and MD5 `d3822276bc5f99a0e3e3beb69b609cbe`. Published edition identity is
distinct from the older revision dates/history printed within its chapters.
SRC-0070 excluded originals were not used.

Focused text and image checks cover75 distinct printed pages, with running
heads/footers checked. This is not an exhaustive verification of every locator
in the drafts or either manual:

| Group | SRC-0071 printed pages | SRC-0068 printed pages |
| --- | --- | --- |
| Boot parameters | II-9, II-120, I-246, II-212 | EM-7 |
| Traps and low memory | I-87, II-19, II-20 | PA-6, MM-15 |
| Fonts, errors and patches | II-357, I-233, II-362, I-105, I-106 | EH-3, FM-23, RM-6 |
| Edition differences | II-349, II-191, II-192, II-359, II-365, II-370 | MM-4, VR-3/4, DV-22/23, EH-4/6/13, OU-4 |
| Disk gap | II-211, III-33/34/35/36 (II-212 above) | DD-3/4 |
| Resource differences | I-118, I-119 | RM-1, RM-21/22 |
| MFS format | II-121/122/123 | FL-54/55/56 |
| Window records/conventions | I-86, I-91, I-276, I-283, I-293, I-296, I-300, I-302 | WM-13, WM-33/34/43 |
| Supplemental claims | I-277, I-294, I-298, I-341, II-126, II-81, II-55, I-114 | FL-6/7/57, RM-16 |

Book mapping is I+10, II+568, III+1003. PE offsets: EM375, PA98, MM714,
EH1142, FM343, RM118, VR1132, DV930, OU1157, DD972, FL806, WM408.
I-86's illustrative declaration was excluded from the image's reviewed area;
routine/record declarations, equates, interface examples and adjacent
illustrations occur on some checked pages. Text extraction also encountered
the illustrative declaration. EH-4's alert artwork and I-300/WM-34's drawings
were incidental to the requested prose/figure checks. None is an asset,
fixture, algorithm or implementation input; nothing is copied into the product.
MM-4's adjacent block-organization prose and II-20's internal-structure heading
are excluded from heap design. No private heap structures are reconstructed.
No executable sample bodies, Apple binaries or emulator implementation source
were consulted. No new source uncertainty is resolved merely by an AI statement.

### Requested claims and qualifications

| Claim | Finding |
| --- | --- |
| Startup parameters before managers | II-9/120 and I-246 identify disk-supplied sizes; EM-7 names queue configuration; II-212 confirms Disk Driver system-heap allocation. A primitive read before **dependent** initialization is a sound proposed design. These pages do not require it before every manager, exclude a temporary bootstrap allocator, or establish Apple's full boot order. SK-2 remains engineering. |
| Trap reach | I-87/PA-6 expressly give15 offset bits in words. The largest nonnegative displacement is65534 bytes. A table target must also be even, within its selected mapped executable region and representable without address wrap. This constrains table entry points; upper-ROM bodies/data can be reached by independent in-range stubs. It does not force all RAM code into that reach or prove the ROM size needed. |
| Low2816 bytes | II-19/20 place globals below the system heap; MM-15 explicitly includes the table there. `$B00` is the arithmetic inference for the boundary. No exact table address or proved free contiguous1024-byte interval follows. Absence from AppendixD is not proof that an address is free. |
| Font and early error path | II-357/EH-3 require InitFonts for the documented System Error Handler; I-233/FM-23 identify ROMFont0; II-362 documents the no-alert-table drawing path. A complete original error UI needs original fonts and artwork. Before InitFonts, use a separately designed primitive diagnostic, not an assertion that SysError is already safe or resource independent. The text does not license copying the illustrated icons. |
| System-file patches | I-105/106 and RM-6 Figure3 identify the data fork as the patch location. No patch-file encoding is supplied there. Do not execute foreign data forks or assume every INIT is code; alert INIT1/2 are documented data. SQ-6 remains open. |
| Edition differences | Images corroborate16.75K/48K versus16.5K/46K; six versus five VBL tasks; different unit-table captions/content; two versus three alert tables and different startup alert lists; four PRAM bytes versus a LONGINT whose high byte is validity and lower24 bits are reserved. No edition is selected as January1984 truth. |
| Disk encoding | II-211/212 and DD-3/4 give logical geometry; III-33..36 give control/register selection and drive signals. The cited sections do not provide a complete encoded sector/IWM data-transfer contract, mark bytes/checksums or per-zone speed values. Keep this as a scoped documentation gap, not proof that no published source anywhere exists. |
| Resource differences | I-118 suppresses resPreload with SetResLoad(FALSE); RM-21 expressly still preloads. I-119 gives NIL/noErr for index<=0; RM-22 gives NIL/resNotFound outside the whole valid range. RM-1 reports INTEGER-to-LONGINT SizeResource change. That is a possible early ABI mismatch, not proof of a particular January1984 ABI. |

Additional corrections: February8,1984 is also after January1984; remove the
startup catalog/forward map's implied ST exception. XR-3's phrase about source
dates must distinguish published edition date from chapter dates and previously
reviewed older QuickDraw material. System startup opening two named drivers is
the Device Manager statement, not an exhaustive all-profile driver count when
AppleTalk is separately conditional. Caption differences leave SQ-8 open.

## Per-file disposition

| Input | Disposition | Reason and boundary |
| --- | --- | --- |
| later-docs-gemini-audit-draft.md | Adopt with changes | Retain its lead-list treatment and withdraw unsupported addresses/explanations. Its original Gemini report identity, exposure history, later-document quotes and verdict counts are attributed, not independently reverified here. No later answer enters a contract. Existing RA/ES/PG dispositions stay in force. |
| mfs-format-draft.md | Adopt with changes | Useful flat-image format evidence and edition comparison. Dense12-bit packing is supported by the PE example, while nibble order, deleted-entry traversal and audit completeness remain gated. MK dispositions below control future use. |
| mfs-format-test-plan.md | Adopt with changes | Case inventory only; correct assurance, capacity/report aliasing and bitmap/output promises below. No fixture or case is generated/executed. |
| window-manager-draft.md | Adopt with changes | Useful interfaces, fields, events and edition comparison. Layout is conditional; WDEF ID splitting, ownership, transactional admission and nested updates need the qualifications below. |
| window-manager-test-plan.md | Adopt with changes | Case inventory only; fix update ordering, deferred nesting and late-failure oracles. Pixel expectations require declared original geometry and fonts. |
| system-startup-obligations.md | Adopt with changes | Adopt an obligation checklist, not its exact boot/reset order. Separate primitive early diagnostics, interrupt admission, system scrubs and hosted versus physical duties. |
| system1-forward-map.md | Adopt with changes | Useful dependency/risk map, not a replacement roadmap or authority for ROM resources, disk strategy, debugger/reference runs or names. P2 contains a disk-free/disk-read contradiction; correct it below. |

The later-document audit's source leads remain unread technical leads here.
Do not import its MemErr address, later ROMMapInsert behavior or an alleged
single modifier-bit offset. The existing eligible resource search-order evidence
and package-error review remain usable in their own scope; later-era facts still
need their own source/edition review. This intake allocates no new source IDs.

## MFS: MK-1 through MK-15

| Proposal | Disposition | Lead result for future contract |
| --- | --- | --- |
| MK-1 | Adopt | Immutable flat whole512-byte logical blocks, checked32-bit length, minimum four blocks. Physical/tag/encoded adapters are separate. No automatic assumption of400K geometry. |
| MK-2 | Adopt | Explicit big-endian scalar decoding and checked arithmetic; raw date/file-number bits, no packed native casts. Endianness is a machine/interface inference, not a format-page quote. |
| MK-3 | Adopt with changes | Header safety checks and empty-name warning are reasonable policies. Under the initial strict two-block map, n is at most640. Do not advertise4094-block compatibility while `$FFF` is reserved and MQ-8 is open; zero-allocation media acceptance is also a chosen refusal, not a documented universal invalidity. Bound directory size and map before traversal. |
| MK-4 | Adopt with changes | Initial strict map ends by byte2048. Defer the relaxed extension profile; MQ-3 and `$FFF`/4095 ambiguity need their own contract. Mark the960-byte/640-entry cap as arithmetic under that profile. |
| MK-5 | Adopt with changes | Name the packing policy explicitly; P-A may be a provisional engineering default, never an inferred historical fact from CPU endianness. No silent auto-detection/fallback. P-A/P-B examples test policy only. Dense packing has strong published-example support, not a printed nibble diagram; MQ-1/R9 remains open and real-volume extraction compatibility remains unclaimed. |
| MK-6 | Adopt with changes | Use checked logical-block regions and the PE drAlBlSt interpretation as the chosen address base. Initial admission: directory wholly before allocation area or wholly within it with every touched allocation block reserved; reject unsupported partial layouts. Never overlap startup/master-directory/map spans. Extra reservation of a partly touched allocation block is conservative policy; MQ-9 stays open. |
| MK-7 | Adopt with changes | Bound both unrounded and even-rounded entry extents, retain names/version/raw fields, stop that block at the first inactive entry and continue later blocks. This is a conservative recognized-prefix policy, not proof of complete directory enumeration after deletion. Record stop offsets/coverage; unexamined bytes must not be parsed heuristically as live entries. |
| MK-8 | Adopt with changes | Preflight the full bounded chain, rejecting free/reserved/out-of-range/cyclic links and checked physical-length mismatch. EOF equality may be accepted **with a documented-literal deviation warning**, since both manuals say physical is greater. MQ-6 remains open. `$FFF` is never a next block in this profile. |
| MK-9 | Adopt | Absent-start0 with nonzero EOF refuses that fork as a ClasSICK consistency rule; allocated empty forks remain valid. Do not recast MQ-7 as answered. |
| MK-10 | Adopt with changes | Separate header, requested-fork and recognized-directory audit scope. One visited bitmap proves repeated ownership only; naming both owners needs bounded additional ownership storage or a bounded rescan. A prefix scan cannot certify every live fork or call potentially hidden allocations definite orphans. Incomplete enumeration yields incomplete assurance, even if the parsed prefixes themselves are safe. |
| MK-11 | Adopt with changes | Whole-chain preflight before copy, checked capacity and bounded diagnostics, no allocation/recursion. Separate payload/descriptor outputs from the permitted diagnostic result; reject output/report/input/scratch overlap. Caller immutability must hold across validation/use. Iterator publication also needs full preflight if it promises no partial outputs. Failed scratch contents need an explicit contract; do not implicitly promise bitmap restoration. |
| MK-12 | Adopt with changes | Count mismatches can warn as an engineering reader policy. II-126 only records unspecified mount inconsistencies, not acceptance of every corrupt volume or this exact count set. State how next-file-number validity is assessed; unknown deletion/history cannot prove equality to max+1. Hidden prefixes prevent a complete file-count conclusion. |
| MK-13 | Adopt | Accept safe representable fork sizes allowed by the selected geometry; warn above the PE16MiB limit. This is a later-book profile choice, not period-media parity. |
| MK-14 | Adopt | Return raw flags/version/Finder words and locks; no write or guessed interpretation of unknown bits. |
| MK-15 | Adopt | Keep physical formats, tags, boot execution, writing/allocation, mounting, File Manager traps and Finder/name-policy expansion outside this slice. |

MFS case dependencies are H->MK1/2/3/6/14, P->MK4/5/6,
D->MK7/10/12 and existing filename choices, C->MK6/8/9,
A->MK6/10/11/12, R->MK11/13. Every fixture depends on an explicit MK-5
policy. No synthetic result would settle real nibble order.

Required case corrections before any future fixture authoring:

- D5/D6 deliberately hide C. They cannot coexist with an unconditional
  complete-volume audit or a definitive orphan/count result for hidden entries.
  Add incomplete coverage to A-series expectations when a prefix is skipped.
- P2's relaxed acceptance is deferred. Odd n needs a bounded final partial pair,
  checked remaining bytes and a defined padding-warning rule; no third-byte read
  when only two bytes exist.
- C14 succeeds with a policy-deviation warning, not a silent historical claim.
- A2/A3 identify both forks only with a separately bounded ownership strategy;
  the stated one-bit visited storage alone cannot retain each owner identity.
- Observables' all-output preservation conflicts with R1's writable report.
  Specify diagnostic exceptions and scratch behavior, plus all alias checks.
  Capacity/output refusal must precede publication, with fixed error precedence.
- Four baseline fixtures and H1..16/P1..6/D1..12/C1..16/A1..8/R1..5 are
  descriptions only. MO-1..4 remain unauthorized; no fixture bytes, generator,
  mutation sweep, runtime case or reference observation was produced here.

MQ-1..12 remain open, including the packing, map extent, deleted entries,
EOF equality, absent-fork values and4095 questions. Existing filename decisions
remain in force; no Str63 restriction or speculative case-fold table is adopted.

## Window Manager: WK-1 through WK-16

| Proposal | Disposition | Lead result for future contract |
| --- | --- | --- |
| WK-1 | Adopt with changes | Conditional P+48 layout only. P=108 gives156 bytes, P=106 gives154; QK1/2/WQ1 remain unresolved. A future checked graphics contract must select the edition/layout before offsets enter guest bindings. |
| WK-2 | Adopt | Store255 for visible/hilited TRUE and read those flags as nonzero, supported by I-276/277 and WM-13's explicit ORD advice. Do not generalize that advice to every Pascal Boolean or template word. |
| WK-3 | Adopt with changes | Initial owned WDEF-only admission is reasonable. Split procID into resource ID in its upper12 bits and variation in lower4; IDs0..4 use resource0,16..23 use resource1, not WDEF resources with each full procID. Guest-visible handle/variation representation and native binding identity need a contract; never store a native function pointer as a guest handle or ignore an overriding guest resource. Refuse unsupported WDEF/patch paths before effects. |
| WK-4 | Adopt with changes | Refuse hooks only on an operation that would execute them; FindWindow may return inSysWindow without executing DeskHook. Validate full transitive closure, lists/globals/handle identities and lifetimes, including data fields guests can change. Current bindings support none of these manager paths. |
| WK-5 | Adopt | ActiveFlag-only is a named1985 engineering profile. Preserve the withdrawn PE changeFlag evidence and WQ11; no January1984 conclusion. |
| WK-6 | Adopt with changes | NIL/consistent ownership on admitted creation failures; preflight/stage and clean up before linking. Define resource-cache effects and temporary allocations explicitly: allocation-before-linking alone does not prove every heap/cache byte unchanged. Distinguish unsupported-path refusal from an admitted manager NIL result. |
| WK-7 | Adopt | Checked18 fixed bytes plus title length byte and declared characters; minimum19+length. Bit8 interpretation is a documented-parameter inference; other bits warn under the chosen profile, not an observed format rule. |
| WK-8 | Adopt with changes | Prefer explicitly labeled1985 choices; make one profile, not an accidental edition mix. WM-34's `$8000`/minus32768 wording does not prove a full32-bit result of `$FFFF8000`; retain ambiguity rather than claim the test's PE long is established. Complete BeginUpdate, frame, title and pattern dependencies remain separate. |
| WK-9 | Adopt with changes | Front-to-back CheckUpdate is documented. Before-effects refusal needs preflight of the entire path through earlier pictured windows and QuickDraw bottlenecks; otherwise later faults must retain already completed effects. A picture is guest data even when only original drawing routines execute it. Bound scan/work and do not infer historical complete event ordering. |
| WK-10 | Adopt with changes | Pending CurDeactive/CurActivate, deactivate-first delivery and clearing suppression are useful. Separate pending slots from any materialized queue and define coalescing/close/repeated-selection policy; WQ8/10 remain open. Do not claim clearing removes an already delivered event. |
| WK-11 | Adopt with changes | Shared bounded input/call/session budgets and release tracking, with transitive hooks admitted or refused. A late budget/input failure cannot generally restore prior drawing, consumed input or guest effects; use truthful retained-effect diagnostics, unless a future explicitly staged operation proves atomicity. |
| WK-12 | Adopt with changes | bounds=limits/slop and a copied structure outline are provisional original choices. Transform all DragGrayRgn arguments into the current port's local coordinates; DragWindow's global bounds are not directly interchangeable. WQ5 stays open. Preserve originals and define temporary outline cleanup. |
| WK-13 | Adopt with changes |512x342, rounded16/16 minus20-row menu bar can be a named original geometry policy; WQ3 stays open. Pattern16 comes through the reviewed resource-search layer, with bounded8-byte data, not arbitrary guest executables. Original fallback/pixel metrics require asset and graphics contracts. |
| WK-14 | Defer | I-293 documents caller-managed assembly nesting with SaveVisRgn, not the proposed overwrite/ownership/failure semantics for an invalid nested call. A second BeginUpdate needs a reviewed lifetime/region-save protocol or an explicit pre-effect refusal; do not bless silent overwrite/leak behavior. |
| WK-15 | Adopt with changes | Checked nonrelocatable record/WMgrPort allocation in the application heap is supported. Validate caller storage, aliasing, ownership, lifetime, all regions/title/controls/pictures and system-held pointers against reset. SPEC-0023's bounded32-object profile is not a capacity proof for a complete window manager. |
| WK-16 | Adopt with changes | Standard hit+2 mapping is inferred from two documented enums, with explicit supported range. Prioritize menu/system cases correctly; reject invalid custom hit values. whichWindow=NIL for wNoHit is a chosen policy under WQ4, not a manual guarantee. |

Window case dependencies: R->WK1/2/3/15, T/N->WK3/6/7/15,
U->WK8/9/14 plus region/event contracts, A->WK5/10,
F/G->WK3/4/16 plus list/global validity, K/D->WK4/11/12,
I/P->WK4/8/13/15 plus fonts/menu/resource dependencies. The 1985 behavior
choices do not settle WQ-1..12 or any QuickDraw question.

Corrections before future fixture authoring:

- U6 using W3's B,A,C order returns B's update before reaching pictured A.
  To exercise picture-then-event, explicitly reorder to A,B,C or make B not
  need updating. Reset other pending activate events in the setup when testing
  update delivery, and explicitly call BeginUpdate/EndUpdate between U4 events.
- U9's silent second-save overwrite is deferred with WK-14. Valid saved/restored
  assembly nesting and invalid unmatched/nested calls need distinct oracles.
- K5's entire pre-call-state restoration is not proved by a bounded loop.
  Specify consumed-input/drawing/retained effects and temporary-highlight cleanup;
  K1..3's exact toggle counts also require an explicit original loop policy.
- N/T no-change cases must distinguish protected externally visible state,
  resource-cache/allocator temporary effects and actual result/report writes.
  N3 caller storage and DisposeWindow require ownership refusal cases.
- P3's PE32-bit oracle is unverified. F10 is consistent with WK-4 only because
  FindWindow does not execute DeskHook; SystemClick would be separately refused.
- R5's raw four-byte curvature storage needs an explicit checked representation;
  the prose saying diameters are stored there does not print their byte packing.
  Add list cycles, bad globals, alias/overlap, corrupted handles, stale epochs,
  picture preflight, budget and capacity cases in a future contract's own plan.
- R1..5/T1..5/N1..8/U1..12/A1..8/F1..11/G1..2/K1..5/D1..5/I1..5/P1..5
  remain descriptions; WO-1..6 unauthorized. No window, resource, input script,
  region oracle, pixel golden or executed case was generated by this intake.

## Startup catalog: SK-1 through SK-12

| Proposal | Disposition | Lead result for future contract |
| --- | --- | --- |
| SK-1 | Adopt with changes | One obligation/coverage record with tier applicability and chosen versus documented fields, not an assertion both tiers initialize identical physical hardware. Distinguish boot-once state from each-launch state; record unsupported rows explicitly. |
| SK-2 | Adopt with changes | Dependency ordering, not this unqualified total sequence. Boot parameters precede dependent final manager allocation; a primitive read is one original strategy. Hold dependent interrupts/tasks until their queues/driver/error prerequisites exist. Before fonts/resources, use a primitive owned fallback; no SysError prerequisite claim. An authorized ROM-resource path would need its own parameter/bootstrap branch. |
| SK-3 | Defer | Reserve the documented low-memory boundary as planning input, but no table address/free range is selected. Consolidate vectors, all documented spans, runtime reservations and unknown ranges first. The forward map's keep-unknown-zero policy conflicts with using an undocumented gap as free. |
| SK-4 | Adopt with changes | Validate nonnegative even offset<=65534, checked base addition, actual ROM/RAM executable bounds and table identity. In-range stubs can reach larger original code bodies. This intake adds no linker check or target placement. |
| SK-5 | Adopt with changes | A signed/versioned original startup-data format is reasonable only for ClasSICK media; validate size/bounds/field budgets and refuse foreign boot content without execution. All-zero and unknown nonzero are profile policies, not complete detection of historical bootability. Hosted and any ROM-resource branch need explicit equivalent parameters. |
| SK-6 | Adopt with changes |1985 PRAM layout/validity and a named preference-first fallback policy, retaining edition differences/SQ9. Define raw20-byte host representation, persistence/error/lifetime rules separately; no host file is created here and no original hardware write is authorized. Defaults are selected source values, not copied assets. |
| SK-7 | Adopt with changes | Keep the six-task list as a candidate1985 profile and the five-task evidence. A conditional keyboard task means an explicit profile variant, not both fixed-six and optional-six claims. Task enablement, interrupt/register/A5, frequency and no-memory-call rules need contracts; current hosted lifecycle refuses interrupt context. |
| SK-8 | Adopt with changes | Original alert data/fonts/artwork only, with owner wording/names undecided. Choose the DSAT->user switch by readiness, not a brittle step number. Programmer INIT1 deferral is a first-profile limitation, not historical absence. Validate tables and callback/Restart paths before binding. |
| SK-9 | Adopt with changes | Original shell as FinderName target can follow the documented interface. No default name, filesystem search/launch/reset semantics or error41 implementation is selected here; owner OD4 and loader/resource/file contracts come first. |
| SK-10 | Adopt with changes | Share a reviewed coordinator, not the unsafe literal step list. Validate/prepare all ownership/reference work before destructive retirement; unknown system scrubs currently refuse under SPEC-0030. goodBye callbacks must finish before entering its inactive commit, with mutations revalidated. Resource-map cleanup and loader actions need an explicit transaction/lifetime boundary; do not dereference retired handles. Chain's retained heap needs a distinct contract, not a flag casually bypassing retirement. Existing root ExitToShell is a session stop, not this coordinator. |
| SK-11 | Adopt with changes | Empty accessory units until opened is a possible minimal profile, pending Desk Manager ownership/driver contracts and original accessory decisions. Preserve DV-23's startup caption; do not call this historical evidence or claim other unit entries exist because a figure names them. |
| SK-12 | Adopt with changes | Per-launch package reset is a candidate lifecycle choice, not proven startup frequency. Require reviewed package ownership/selector ABI and scrubbed references; a reset may not silently clear persistent system package handles or use unsupported callbacks. Current package execution remains refused. |

SQ-1..17 remain open. Numeric BufPtr/H_min, ROM capacity, startup table location,
default file/FCB budgets and January1984 behavior are not decided. AC-1..7 are
unexecuted acceptance descriptions; separate boot initialization from per-launch
reset in AC-1, account for retained-fault outcomes in AC-2 and primitive fallback
readiness in AC-6. SO-1..3 remain unauthorized.

## Forward map, XD options and XR ranking

The Resource Manager is a substantive prerequisite to historically documented
CODE segment loading: II-55 expressly uses a Resource Manager-loaded locked
handle, and I-114/RM-16 require resource-map lifecycle work. SPEC-0024 is a
reader only. A future narrow caller-supplied original resource service could be
defined as an engineering loader profile, but would not constitute full Resource
Manager or named-application compatibility. No loader work is authorized here.

P1 is a useful dependency sketch after SPEC-0030, not proof that a complete
Resource Manager must be implemented before every independently scoped loader
validator. Its exit target involving menus exceeds the present tier-1 window
target and needs tier-2 selection. P2's supposedly disk-free steps1..9 include
SK-2's primitive disk read and Disk Driver startup: replace with explicitly
different disk and ROM-resource bootstrap branches if the owner chooses one.
A ROM resource map alone provides no MFS volume, shell CODE or System-file data
fork; define their providers explicitly. P3's application/asset/CPU/source
constraints and all three edition gates remain, rather than being closed by Snow.

| Option | Disposition | Lead result |
| --- | --- | --- |
| XD-1 | Defer | Plausible interim original ROM-resource provider, but OD1 is undecided. Need ROM/RAM budget, mutable guest map copies, read-only persistence behavior, explicit owned resources/CODE/shell providers and identity. No boot or compatibility permission follows. |
| XD-2 | Adopt with changes | Access-based code placement is a design principle. Only table-installed entry points need the reach cap; not every system allocation/driver/VBL body. Package resSysHeap attributes and lifetimes decide RAM cost; do not assert rarely used code always lives only in the application heap. No fixed ROM capacity/layout is selected. |
| XD-3 | Defer | Published-source screen, interim synthetic/ROM branch and independently authorized hardware study are alternatives for OD1. Research can identify candidates, but no uncertain article technical content or IWM implementation is approved. Deferring a physical path does not satisfy it. |
| XD-4 | Defer | Publisher documents debugging features; any run/use still needs owner approval. Own-image tools could inspect original code/state only, with manually specified documented addresses. Do not treat the emulator's built-in global catalog or ROM expectations as eligible historical source evidence. |

Publisher documentation was reopened read-only on2026-10-14, without following
source/code/ROM download links. [Mini vMac hardware documentation](https://www.gryphel.com/c/minivmac/hardware.html)
describes patched disk access and low-memory mouse injection for its documented
Macintosh Plus model; that is not a fresh executable qualification for the128K
candidate. [Snow introduction](https://docs.snowemu.com/) describes hardware
emulation. Its [published user manual](https://docs.snowemu.com/print) documents
ROM-validation control, sector/bitstream/flux inputs, the relative hardware
mouse, and debugging UI. The general hardware statement has an explicit
absolute-mouse exception. Publisher claims do not prove actual exact-build
behavior, correctness, provenance of its catalog, or independent test-oracle
authority. The existing executable clearance is not renewed by this intake.

| Risk | Disposition of ranking | Lead qualification |
| --- | --- | --- |
| XR-1 | Adopt with changes | Critical for **physical floppy/disk boot**, not every possible ROM-resource demo. This is the checked manuals' protocol gap, not a universal absence claim. |
| XR-2 | Adopt with changes | High qualification dependence. Say Snow is the currently qualified own-ROM demonstration path, not proof no other emulator can help or that all disk/mouse use on it is qualified. Relative hardware mode remains required for such future trials. |
| XR-3 | Adopt with changes | High historical-evidence gap. Fix the ST date exception and publication/chapter-date conflation. R-001/TD-003 remain open. |
| XR-4 | Adopt with changes | High application risk. Missing AppendixD entries mean unknown ownership, not permission to reuse or force all those bytes to zero across sessions. Reconcile with SK-3 before placement. |
| XR-5 | Adopt | High layout/asset risk. Original fonts are mandatory under existing policy; metrics target remains OD2. |
| XR-6 | Adopt with changes | Medium-high entry-placement/capacity risk. Reach arithmetic is confirmed; budgets and whole-image placement remain unproved. |
| XR-7 | Adopt with changes | Medium-high for numeric applications. Say a suitable numerics reference is **not yet reviewed/admitted**, not categorically ineligible. IEEE754 alone does not provide the Macintosh package ABI or every SANE operation. |
| XR-8 | Adopt with changes | Medium for a minimal original shell; potentially high for named Finder/desktop fidelity. Application-side documentation does not exhaust user-visible Finder behavior. |
| XR-9 | Adopt with changes | Medium corpus risk, with no named titles or prevalence verified here. Use owned original fixtures first; later corpus exclusions require explicit scope and lawful inputs. |
| XR-10 | Adopt with changes | Medium across ABI profiles. Preserve the current checked24-bit guest-address policy; do not turn flag masking into unchecked native host pointer arithmetic or erase distinct metadata. |
| XR-11 | Adopt with changes | Medium timing risk; physical disk/mouse/interrupt behavior may make it high. A tick-only solution does not qualify physical device timing. No cycle-accuracy scope is waived by this ranking. |
| XR-12 | Adopt with changes | Low-medium for the initial UI target, higher for printing parity. An original image printer is a future option, not an adopted backend. |
| XR-13 | Adopt with changes | Low-medium initially; originals and driver/lifetime/callback contracts are still needed. Figure names are not an authorization to reproduce assets or establish January1984 contents. |
| XR-14 | Defer | Scope/severity depends on OD5 and eligible release evidence; do not decide inclusion/exclusion by a1985 chapter date alone. |
| XR-15 | Adopt with changes | Medium for synthetic contract conformance, high for historical compatibility conclusions. An emulator checks execution on its model, not undocumented Apple behavior. The finding about copy protection should refer to XR9, not XR15. |

Risk ranks are provisional engineering priorities. All XR questions remain open;
none is historical certification. Existing low-memory, edition, source, role,
native-application, physical/128K and B2 criterion6 gates are unchanged.

## Owner decisions remain pending

These seven items are listed for the owner, not treated as decisions or permission:

1. **OD-1:** disk strategy XD-3a/b/c, and whether original ROM-resident system
   resources are an interim Snow path. No default is silently selected.
2. **OD-2:** original font assets and the desired documented metric compatibility.
   Original authorship is already required; the metric target is still pending.
3. **OD-3:** seek an eligible Apple Numerics Manual/package-interface source,
   consider IEEE754's limited coverage, or defer numerics. No candidate admitted.
4. **OD-4:** shell/System-file names, original startup-alert wording and original
   desk-accessory names, consistent with the existing asset-replacement process.
5. **OD-5:** AppleTalk inclusion in the0.97 profile; release evidence separate.
6. **OD-6:** future Snow debugger use on specified ClasSICK-only images, under
   the existing emulator-run gate. Prior screenshots authorize no new run.
7. **OD-7:** separately scoped black-box observations on owner hardware,
   including SO-1..3, or decline. No protected binary/export permission implied.

## Next step and closeout

For implementation, the existing order remains: separately authorize the
bounded hosted SPEC-0030 v2 implementation, with no boot/loader/manager expansion.
Then define the Resource Manager service/lifetime seam before CODE-only loading,
and retain all launch/LQ/AL and re-entry questions. Resource-map references,
package/driver ownership and callbacks are important future reset prerequisites;
their absence currently means deterministic refusal, not hidden cleanup.

The most useful next research is XQ-1, Resource Manager runtime contract and case
plan, followed by XQ-2, font/visual resource formats and original asset budgets.
This is a recommendation, not a new assignment. The two Gemini source-screening
prompts are reusable research input only; they were not sent and their unread
technical candidates are not adopted. Any later assignment must be scoped and
presented as a complete copyable prompt under the owner's presentation rule.

No implementation, specification, fixture, runtime test, build, interface registry
import, emulator/VM/reference run, HQ record, stable ID, OneNote, automation,
new chat or delegated research is created by this intake. Existing originals,
contracts/code, build/edition records and compiled inputs are preserved. Only
this lead review is committed; documentation/identity/preservation checks are
not execution of either research test plan. All41 MQ/WQ/SQ questions and15 XR
risks remain open, plus OD1..7 and all prior historical/compatibility questions.
