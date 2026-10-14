# Re-entry, event selection and package intake - 2026-10-14

SRC-0142..0145. Lead self-review; human provenance review pending.
Research intake only. Dispositions below are choices for future contracts, not
new specifications, dispatch records, fixtures, executed cases or implementation
permission. Published facts, deductions and engineering choices remain separate.

## Identity, baseline and preservation

Work began at `fc2e44a6c0ee407fcb35dc04ea4b644030668dd0`, with no tracked
changes and exactly seventeen untracked research originals. Claude's observed
`e6248a5f277f4acb914d23b9ae97827825bee774` is an earlier checkpoint. The two
dispatcher changes in that handoff were preserved in correction `9b4a96a7d9d72cfdc83a1eb86ae8b4938f705736`
and verified/recorded separately in `fc2e44a`. This intake changes no code.
The audit's requested research baseline `7d515a8` remains distinct from its
observed baseline; reading SPEC-0029 did not make it implementation authority.

All supplied identities match before substantive review:

| Original in docs/research | Bytes | SHA-256 |
| --- | ---: | --- |
| application-reentry-audit-draft.md | 41762 | `8614803f637b4a0695e8fa353ab7f688b07995a50442e5b3d2601d0626c915fa` |
| application-reentry-audit-draft.json | 44569 | `8e125f6ab79604f412a153e6726e578c7f6bd1f311d4ba70b43332e0421f5f79` |
| event-selection-draft.md | 24550 | `aeb4002924a623f9987de423fa397f9e2d4b3cfc51ad7f6ff7075560ecc50240` |
| package-and-glue-draft.md | 27220 | `fe3bfe19f1ae8fd34d695ff75bbb45e4a693c6b563daa51f120ece79de6acb1a` |

Byte-identical copies of all seventeen originals, plus the owner request and
previous audit manifest, are outside Git in the private workspace directory
`reentry-event-package-intake-20261014-a`. Its `identity-manifest.json` SHA-256 is
`8f8960956a1cb05cd0d2aebebc93506f6501703593e265a89ac519833178107e`.
The earlier `application-reentry-audit-20261013-a/MANIFEST.txt` matches its
reported608 bytes and SHA-256
`fb9a04b10addc8aef930fa475212d81db3190a170eb38d69ff82782fc7dfc682`;
both earlier audit copies equal the originals. Separate copies already made by
the completed Toolbox slice also remain. Nothing is reset, stashed, cleaned,
pulled, rewritten or imported from an original.

Supporting JSON has schema `claude-application-reentry-audit-draft/1`,41 routine
rows,25 paths,13 lifetime findings,14 questions,7 proposals and22 scenarios.
The9/7/1/24 grouping totals41; it is an attributed research inventory, not a
proof that any of those routines can be admitted. The JSON is not registry data.
Claude's exposure and no-other-change statements remain attributed, not audited
facts about all prior concurrent actions.

## Independent published-source checks

Both full copies were reidentified, including page counts:

- SRC-0071:19245372 bytes,1284 pages, SHA-256
  `b65fe8efe782799a333a0b59b225e2bbaf9e2e150dc949dd555571971b331d97`.
- SRC-0068:53888388 bytes,1326 pages, SHA-256
  `924902074a9798457949c46f83ac016c98b10f50258ef36c277dcd0f8278b237`,
  SHA-1 `4ed5b3c20289835996b160d7ab3a628f497f13c5`,
  MD5 `d3822276bc5f99a0e3e3beb69b609cbe`.

Book mapping: I+n10, II+n568, III+n1003. PE chapter offsets used: AL98,
EM375, WM408, MN496, DS608, PK662, MM714, SL776, OSEM792, DV930,
OU1157. Headers/footers distinguish the published editions and their chapter
dates. Neither manual proves January1984 behavior. The following is a focused
check of the requested claims, not an exhaustive survey or source attestation.

| Claim | Independent check and disposition of the evidence |
| --- | --- |
| MenuSelect/MenuKey -> SystemMenu -> accMenu | I-355/356/357/443/447 and MN-21/22, DS-11/15 corroborate the accessory path. MN-22 and I-443 images checked. Calls into driver code need the driver convention, not an ordinary Pascal-callback save rule. |
| Journaling | I-261/262 and EM-27 list GetNextEvent and EventAvail and the Control data pointer/code. PE EM-27 alone gives standard jRefNum=-2. This does not establish the complete internal event-processing order. |
| ExitToShell -> goodBye | II-59 says application heap released; II-188/189 and DV-20 say dNeedGoodBye control precedes reinitialization. The composed ExitToShell path remains an inference. Current SPEC-0029 exits a root hosted session without teardown and does not execute it. |
| AppendMenu/CalcMenuSize/MDEF | I-361/362 and MN-26: both include AppendMenu;1985 describes every menu-changing routine, PE lists four callers. The AddResMenu composition stays1985-scoped. MDEF sizing is a supported inferred chain; no exact complete caller set is established for NewMenu/GetMenu. MN-26 relevant prose was viewed as a crop excluding its illustration. |
| windowPic -> grafProcs | I-197/293 and I-296 / WM-35: picture drawing can reach the QuickDraw procedure table. CheckUpdate explicitly draws pictured windows and continues scanning. Add this inferred composed RA-P20 path to GetNextEvent; it is missing in both audit representations. |
| FindWindow/DeskHook/WDEF | I-288 / WM-27 explicitly make SystemClick the DeskHook caller after FindWindow returns inSysWindow. I-300 documents wHit; the FindWindow-to-wHit composition remains inferred. A lack of AppendixB membership never proves no callback. |
| SystemTask | II-189 and I-447 / DV-20 document dNeedTime driver control and accCursor for an active accessory. PE DS-10's accessory-only wording is narrower than its driver chapter. Preserve that difference. |
| Hook differences | I-288 / WM-27 confirm the TrackGoAway DragHook addition in1985. I-356 / MN-22 confirm MBarHook and explicit InitMenus hook reset in1985 only. Absence on the checked PE pages is not a universal negative finding. |
| SEvtEnb | I-443 says SystemEvent returns FALSE; EM-22 says GetNextEvent does not call it. These are distinct documented descriptions, not proven interchangeable side effects. |
| Grow-zone result | II-43 uses LONGINT; MM-45 uses Size, and MM-46 adds GZCritical prose. Keep literal edition variants; the spelling difference alone does not establish a different ABI width. No grow-zone callback is admitted. |
| Driver open | II-193 lists ioResult and says D0 is cleared after Open. DV-25 lists D0 with an unfinished note that it is cleared. Both allow open/close register clobber and synchronous RTS. Preserve the disagreement; no synthesized dual result. |
| Event priority | I-245 / EM-6 images confirm pending deactivate/activate, masked FIFO queue, auto-key, front-to-back updates, null. Activate/update are not queued. I-246 / EM-7/8 state initial16 and subsequent4 tick defaults and held-key/time conditions. |
| EventAvail ambiguity | I-259 / EM-22 say it behaves like GetNextEvent except queued events remain. I-442 / DS-10 say SystemEvent is called only by GetNextEvent. Journaling explicitly includes EventAvail. The contradiction and unqueued activate/auto-key bookkeeping stay open. |
| Queue-only peek | II-69/70 and OSEM-6/7 support OSEventAvail retaining the queued event. The no-Desk-Manager/alarm/Command-Shift note is1985-only. This is not a full Toolbox-stream predictor; auto-key scope remains open. |
| Command-Shift digits | I-343 says never delivered; I-258 / EM-22 narrow special effects and describe FKEY. Missing-FKEY behavior remains unknown. Do not silently choose historical consume-versus-return behavior. |
| Modifier encodings | I-252 Figure4 and I-253 masks give word bits0/7/8/9/10/11. I-265 and OSEM-11 equates show0/2/3/4/5/6. Images checked; the assembly unit/interpretation is not established. Use the word masks for the planned engineering representation; keep ES-G10 open. |
| Core versus guest event queue | Read SPEC-0004/0005: generic FULL refusal/no overwrite/no timestamps and nonhistorical monotonic time. I-245/246/249 and II-68 document oldest-event discard and posting-time event fields. A separate guest contract is needed; the existing core contracts are not defective. |
| Package words | III-218/226 images give Pack2..7=A9E9..A9EE, reserved Pack0/1=A9E7/A9E8, and InitPack/InitAllPacks=A9E5/A9E6. Reserved routine words differ from bit9's reserved trap-format flag. |
| Package selectors | I-519 / PK-25: StandardFile1..4; II-396/400 / PK-36/41: DiskInit0/2/4/6/8/10; I-504 / PK-16: International0..16 even; I-489/491 / PK-20/22: BinaryDecimal0/1. Tables agree for those four packages. II-406/407 describes two-byte FP/Elems opwords but delegates their interface elsewhere; no opword table is adopted. |
| Mixed Binary-Decimal entry | I-483/489/490/491 and PK-20/21/22: register arguments/results plus a stacked selector through A9EE. This adds one distinct mixed Toolbox word beyond the twelve surveyed exceptions, with two routines. Selector cleanup/preservation remain unresolved. |
| localRtn | I-500/503 and PK-14/15 describe executable localization in INTL1; PK-15 says the writing interface is forthcoming. A one-instruction null-routine description does not supply the general call ABI. The field is inline code, not a native function-pointer slot. |
| StringToNum spaces | I-490 and PK-21 images confirm the general versus leading-space wording difference alongside the low-nibble behavior. Keep PG-Q8 open historically; no fixture or conversion implementation is created. |
| Linked library routines | I-6/96 and AL-15 distinguish not-in-ROM/no-trap routines from trap entry. AL-15 documents interface marshalling and possible result-code global storage. If such code is linked into a guest program it can touch memory between traps; this is an interface deduction, not executable inspection. |
| Thirty globals inventory | III-227..231 images confirm the thirty named locations in package section6.2 plus SFSaveDisk. Selected routine prose corroborates direct access, particularly SysEvtMask/EventQueue, Time, CurrentA5, ROMBase, TextEdit scrap and alert state. The thirty include ANumber/DefVCBPtr mentioned alongside operations; no proof that every listed wrapper directly touches every incidental name, or that this list is exhaustive. |
| Environs/Restart/patch comparison | II-383/385 images confirm the ROMBase comparison and the word at ROMBase+8; Restart is described as restarting. Numeric Restart instructions on adjacent II-386 are excluded sample material; no offset or instruction sequence is adopted. The1985-only evidence does not prove earlier behavior. |
| MemError | II-27 attributes manufactured/stored errors to the interface. It does not locate storage. AL-15 explicitly allows a global. Reject the strong inference that no global exists anywhere; no documented low-memory MemError address is supplied or invented. |

Forty page-image checks were actually viewed: book PDF255,262,263,269,275,298,
306,371,452,453,499,500,501,510,529,611,757,951,953,1221,1229,1230..1234;
PE113,381,397,518,522 (prose crop),618,676,678,683,687,698,759,803,955.
Other checks above use selected prose/table extraction or the previously reviewed
calling evidence. Rendered-but-unviewed pages are not counted as checks.

Exposure record: some full-page reading-aid output included adjacent manual
usage/assembly illustrations (notably I-96, II-27/386, PE MN-26, MM-46 and
OU-10, plus short calls alongside relevant prose). Those passages were not used
as evidence, copied into this review or used to write specifications/code.
No numeric Restart target or grow-zone algorithm is extracted from them.
This is not a claim of zero visual exposure. No Apple/Microsoft implementation,
ROM/binary/resource/disk image, disassembly, emulator source, SRC-0070 or private
heap reconstruction was retrieved. No code changes or guest/reference/scenario
executions follow this intake; ordinary documentation guards are separate.

## RA-E dispositions: future closed-world refusal profile

| Proposal | Disposition | Decision and reason |
| --- | --- | --- |
| RA-E1 | Adopt with changes | Records must identify convention, completion, lifetime, context, transitive re-entry and effects, with provenance/unknown markers. The supplied RA sets are incomplete and may contain shorthand. Include native callees routed through guest-patchable entries. Unknown/unproved closure refuses; no automatic JSON import or admission of the9/7/1 groups. |
| RA-E2 | Adopt with changes | Plan a conservative initial profile with no guest callbacks: nonowned/unknown definitions, hooks, driver/accessory or resource paths, grow-zone and picture/custom-procedure paths refuse before effects. Validate live owned identities, not only mapped addresses. Require synchronized native input/time too. Resume remains stored-only; faults use SPEC-0028, not ResumeProc. This does not bind41 managers to SPEC-0029 pure handlers. |
| RA-E3 | Adopt | Keep asynchronous guest-code installation unsupported, including VBL and completion/driver interrupt code. Owned platform input is distinct but must synchronize with future manager admission/commit. |
| RA-E4 | Adopt with changes | Select ES-O1 conservative refusal first. Do not use EventAvail or OSEventAvail as a proven full-stream predictor. Refuse when an unsupported path might run or its enabling state cannot be safely determined; a failure after dequeue/drawing is not pre-effect refusal. ES-O2 remains deferred. |
| RA-E5 | Adopt with changes | Future launch/reset must inventory typed globals, ownership and reset/preserve/rebind/refuse actions. Stage scrap and system references before retirement and expose coherent state before guest execution resumes. Do not blindly zero the RA-L5 or package inventories. Epoch/mapping/identity checks cannot identify every stale same-address raw pointer; define reuse/alias limits explicitly. |
| RA-E6 | Defer | The proposed order is useful dependency input, not an approved sequence. Zero parameters do not settle return/register/A5/cleanup rules. Each path needs its own complete call-site and effect/lifetime contract before admission, shared budgets and approved implementation. |
| RA-E7 | Adopt | Returning callbacks still cannot request no-return completion. SPEC-0028/0029 remain unchanged; generalized unwind, restart and resume transitions are deferred. |

RA-P1..25 and RA-L1..13 are adopted with the evidence qualifications above as
research inventories only. No path is a newly executable callback signature.
SystemClick's JSON row abbreviates the DragWindow/TrackGoAway chains and omits
their full WDEF/picture closure, including RA-P20; never treat it as a complete
flat set. SelectWindow's markdown note that activate events are queued is wrong.
Its JSON row does not contain that note. Neither original is changed.

RA-S1..22 remain unexecuted review descriptions. Future use depends on the RA-E
choices and the corresponding manager contracts. S4 also needs the missing-FKEY
choice; S5/10/11 require admitted callbacks, not v1 pure handlers; S15 requires
teardown/goodBye beyond current root exit. S18 cannot expect a numeric owned-
identity/epoch check to detect every reused raw address. No scenario, fixture,
harness or expected-output implementation was generated from this inventory.

## ES-O dispositions and event profile

| Option | Disposition | Decision and reason |
| --- | --- | --- |
| ES-O1 | Adopt with changes | Choose conservative refusal for first admission. Avoid its circular predicates based on what the next event or update scan would do: absent a proven evaluator, refuse on any reachable/unknown unsupported FKEY, accessory, journaling or picture-processing state. Alarm, disk mount/ejection and other native effects also need effect contracts; native implementation alone does not prove callback-free operation. No resource/queue/clock/window mutation during refusal preflight. |
| ES-O2 | Defer | A future single evaluate/admit/commit operation must use one time/input/manager snapshot, bounded work and complete effect closure. It needs guest queue/time, resource search, Window/QuickDraw and File Manager contracts plus a defined engineering order. Reject independent peek-then-dequeue equivalence as a historical or current guarantee. |
| ES-O3 | Adopt with changes | Select(b): EventAvail shares conservative refusal conditions with GetNextEvent. In a future admitted owned-only path, choose that queued and activate state and repeat timing remain unchanged by a peek. This pending-activate choice is engineering, not proof of historical behavior. Refuse journaling rather than silently omitting its documented call. Do not adopt(a) as an unrestricted historical replacement. |
| ES-O4 | Adopt with changes | Choose guest auto-key synthesis using KeyThresh/KeyRepThresh and guest ticks, not host repeat timing. Advance repeat bookkeeping only on committed retrieval in the planned profile; peeks remain stable. Held-key selection, masking, release, tick wrap/epoch and missed input need the later contract. Historical ES-G4 stays open; no current keyboard/clock change. |
| ES-O5 | Adopt with changes | Plan a separate guest queue over SPEC-0004/0005: configurable bounded capacity/default20, oldest discard, posting-time fields, SysEvtMask posting filters and FlushEvents stop result. Explicitly define conversion to guest ticks and live low-memory views. Queue-header/direct-walk support needs its public ABI contract or explicit refusal, not a private heap reconstruction or silent metadata fiction. Core FULL behavior remains unchanged. |
| ES-O6 | Adopt | Plan the16-bit modifiers from Figure4/Pascal word masks (bits0/7/8/9/10/11); retain the assembly-equate discrepancy. No inferred equate unit or January1984 claim. |

RA-Q3 is narrowed by the documented priority rule but its internal ordering is
still open. Preserve every ES-G1..11: order, EventAvail effects/pending activate,
auto-key bookkeeping, multiple activation changes, conditional picture scans,
nonqueued fields, missing FKEY, journal playback, equate interpretation and
intercepted output. The engineering choices above do not answer them historically.

## PG-O dispositions, library state and packages

| Proposal | Disposition | Decision and reason |
| --- | --- | --- |
| PG-O1 | Adopt with changes | Plan a separate(package, selector/opword) descriptor with register/stack inputs, checked selector ownership/cleanup, results/preservation/CCR, completion and re-entry/lifetime/effects. Unknown selectors and reserved Pack0/1 refuse before effects by future policy. Current trap records have no mixed selector ABI; no interface registry import or package binding. |
| PG-O2 | Adopt with changes | Binary-Decimal is first package research candidate, not ready implementation: selector cleanup, preservation, pointer/length/alias/error cases still need a contract. International needs original rights-cleared data and localRtn handling; StandardFile/DiskInit need dialogs/files/events/driver effects. FP/Elems stay unsupported without eligible interface evidence. No source discovery or numerics import occurs here. |
| PG-O3 | Adopt with changes | Own native international comparisons may implement an original no-localization policy. Do not encode a native function pointer in INTL1's inline code field. Guest/document localRtn requires pre-effect refusal until a complete call ABI/resource-lifetime contract exists. Its RTS description proves neither A5 nor preservation/cleanup; StandardFile hooks are additional out-of41-set dependencies, not new admitted RA rows. |
| PG-O4 | Adopt with changes | Choose no fabricated system PACK resource for unsupported/native-only packages: future lookup reports absence under its Resource Manager contract. Reject a nonexecutable placeholder that could be mistaken for callable code merely to make HUnlock/HNoPurge succeed. This does not override future normal lookup of separately admitted original application resources or silently change existing resource-reader results. No PACK bank/handle is created now. |
| PG-O5 | Adopt with changes | Adopt a continuous guest-visible state/lifetime obligation for the named low-memory inventory and SFSaveDisk, plus A5/manager records and hooks. Define each supported value's width, owner, mutability, epoch and validity before any guest instruction, not just at trap entry. Future ROMBase must coherently name an original owned region and an explicitly identified ClasSICK +8 word; no Apple version is asserted. Choose explicit unsupported Restart execution until a separately reviewed entry/outcome contract; no numeric target from sample code or guessed callback unwind. |
| PG-O6 | Adopt with changes | Require future admitted address profiles to keep original builtins>=the coherent guest ROMBase, and classify admitted RAM patches consistently with the published comparison. SPEC-0020 already places builtin tokens in its logical ROM window but permits a disjoint RAM window without requiring it below that base. No continuous guest ROMBase view is bound. Therefore baseline detection is partially aligned, not a proved global invariant; stricter admission/coherence is future work, with current profiles unchanged. |
| PG-O7 | Adopt with changes | Preserve correct documented D0/result/CCR behavior for register calls and leave unspecified glue-managed error storage opaque. II-27 does not locate it; AL-15 permits globals. Do not invent a low-memory MemError global, but do not claim no library/global storage can exist. This does not alter SPEC-0023's own diagnostic state. |

The package inventory concerns new words/selectors outside the41-routine audit,
and low-memory direct access spans later managers. It is not a list of currently
supported guest fields. The thirty named globals are: DoubleTime, CaretTime,
TEScrpHandle, TEScrpLength, DlgFont, ACount, ANumber, ScrHRes, ScrVRes, ApplLimit,
SysZone, ApplZone, GZRootHnd, MemTop, AppParmHandle, SysEvtMask, EventQueue,
FSQHdr, VCBQHdr, DefVCBPtr, DrvQHdr, PrintErr, CurPitch, SdVolume, VBLQueue,
Time, SysParam, ROMBase, CurrentA5 and UTableBase. SFSaveDisk is additional.
Some are addresses/queue headers/handles, others scalar values. Their public
locations do not supply private heap implementations, ownership or release rules.
Callbacks installed in TextEdit record fields are a separate record-lifetime class.

## Static cross-check with the completed dispatcher

Read current SPEC-0020/0027/0029 and the original Toolbox admission/step path.
This is a static review, not a new executed conformance case.

- Unknown Toolbox records refuse. A record explicitly marked register convention
  refuses before argument access. SPEC-0027 still admits selected OS signatures
  only. None of these slices supplies package services or selector marshalling.
- Pack2..7 and reserved Pack0/1 have no installed package records in the completed
  profile and remain unsupported. **There is no hardcoded package blacklist in
  the generic trusted table.** A future owner-supplied incorrect stack record can
  misdescribe such a word; this intake does not claim that every possible table
  mechanically prevents that. PG-O1 is a future admission-policy obligation.
- Register-plus-stacked-selector is unimplemented, not an ordinary stack ABI.
  Existing register descriptors require no declared stack parameters; marking
  one REGISTER permits refusal, not decoding the selector. Leave mixed package
  entries unregistered/refused until a separate contract exists.
- The A0(1985)/A1(PE) exit disagreements for PtrToXHand, HandAndHand and
  PtrAndHand remain, rechecked in II-375/376 / OU-10/11. No variant is selected
  or combined. The package's13-word lower bound is distinct from the unchanged
  thirteen OS/register interface records in the existing registry.
- Root-only hosted ExitToShell remains the completed engineering behavior;
  historical Finder launch/goodBye/heap teardown is not newly bound.

## Cross-draft follow-ups and remaining work

Record these here; apply none to the originals or catalogs silently:

| Proposed follow-up | Disposition |
| --- | --- |
| Event note section10: add GetNextEvent's RA-P20, correct caller-missing/priority/result descriptions | Adopt with changes as review corrections. TRUE/FALSE distinguishes application handling from null/intercepted cases; intercepted record details remain ES-G11. |
| Add EventAvail and ModalDialog/Alert rows | Adopt with changes as a future wider inventory, outside the41-routine set. Their effect/re-entry paths carry EventAvail ambiguity and dialog dependencies, not unconditional literal-I-259 admission. |
| Audit section4: menu/journal/goodBye/windowPic/driver/DeskHook additions | Adopt with changes with the evidence tags above and complete SystemClick transitive closure. Correct the SelectWindow queued-activate note. No callback/general-register rules are silently broadened. |
| Package note section9: exception and callback catalogs | Adopt with changes for later review: one mixed A9EE word/two routines; selectors, StandardFile hooks and localRtn require distinct call-site ABIs. This is not a registry import or an extension of the41-row audit today. |
| Mark low-memory and ROM obligations in existing research | Adopt with changes here as an inventory/coherence requirement, keeping MemError location and sample-only Restart offset excluded. Existing low-memory research originals/catalogs are not modified by this intake. |

All RA-Q1..14, ES-G1..11 and PG-Q1..9 remain historically open (34 labels),
along with prior CQ/HL/LQ/AL and edition questions. In particular callback/A5/
cleanup, resource search/overrides/lifetimes, allocation/re-entry, event order,
selector cleanup/register preservation, missing PACK/localRtn/opwords, spaces,
Restart behavior and January1984 applicability are not settled. Numeric BufPtr,
H_min, new ROM version/entry values and final128K fit remain unselected/unproved.

Next runtime prerequisite remains the proposed heap lifecycle/reset contract:
typed reference/global inventory and continuous validity, staged scrap/system
references, capacity/preflight, explicit commit/retained-fault rules and teardown/
epochs/alias limits, using the existing allocation/session/dispatch contracts.
This intake supplies research constraints only; implementation needs separate
owner continuation. Guest events, packages, managers, loader and generalized
callbacks follow their own prerequisites. No additional Claude assignment is
needed to draft that next contract; the documented gaps remain recorded.

No new SPEC/ADR/TEST/IMPL/build, fixture, scenario execution, interface registry,
code change, native application/SDK, emulator/VM/reference run, delegation, chat,
automation or OneNote. Completed dispatcher evidence and all source/UART/owner/
native/physical/parity/128K/three-edition/B2criterion6 gates retain their scope.
