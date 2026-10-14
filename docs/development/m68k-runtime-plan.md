# 68000 application runtime plan - 2026-10-12

Owner-governed plan, SRC-0099 / ADR-0026. This incorporates Claude's returned
Memory Manager, application-loading and native-application research without
treating research proposals as finalized historical specifications.

## Typed Toolbox checkpoint - 2026-10-14

SRC-0139-0141 / SPEC-0029 / ADR-0033 supplies the bounded hosted prerequisite2
slice: explicit per-routine conventions, checked stack arguments and result/
staged writes, returning/procedure auto-pop, and actual root-only ExitToShell.
Register Toolbox exceptions and LoadSeg continuations are recognized/refused;
installed Pascal patches, direct veneers, retained pointers and manager callbacks
remain later contracts. [Evidence](toolbox-evidence.md) records initial1614,
final72 affected plus1542 unchanged retained checks; ten final audits/twins,
matching complete fourteen-audit/two-link68000 manifests,30-byte/eight-step
encoding and successful CI1614 plus384 bootstrap. Partial frames388/376 do not
prove cumulative stack or128K fit. Six EFI/two own ROM profiles and all seventeen
untracked research originals are preserved. Four concurrent arrivals stay
unreviewed for a separate intake; Claude plans remain unexecuted.

Next proposed prerequisite3 contract is heap lifecycle/reset: owned reference and
global inventory, scrap-state matrix, capacity/preflight/staging, explicit commit
and retained fault snapshots, teardown/zone epochs and stale aliases. It builds
on SPEC-0023 allocation, SPEC-0028 sessions/fault snapshots, and this dispatch
boundary. No loader/reset implementation follows automatically from this report.
Numeric BufPtr/H_min and historical HL/LQ/AL questions remain unselected/open.
All owner/source/role/native/VM/physical/128K/edition gates persist.

## Heap lifecycle and dispatch intake - 2026-10-13

SRC-0135..0138: [one lead review](../research/heap-dispatch-lead-review.md)
records the five verified identities and all sixteen HL dispositions, HT-A..F
dependencies/corrections, convention exceptions and application profiles.
All thirteen originals remain unchanged/untracked with private copies/manifest.
Seven HL proposals adopted, nine adopted with changes for future contracts;
HL-Q1..20 remain open, including Q18. SetApplLimit memFullErr and locked HPurge
carry 1985-only qualifications. Per-routine convention records are required;
three A0/A1 edition differences and unknown register/CCR rules remain explicit.
Profiles need FKEY/DeskHook and transitive re-entry accounting, complete aliases/
ownership and retained-pointer lifetime. No case execution, fixture, registry
import, code, new build or implementation authority.

SPEC-0028 already supplies prerequisite1. Next propose prerequisite2, typed
Toolbox-format arguments and returning/continuation/no-return completion, with
checked entry/result/auto-pop/control frames, per-routine signatures and shared
call/session budgets. Loader continuation can remain reserved/refused until its
own context exists; no-return inside returning callbacks remains unsupported
without a reviewed unwind contract. Prerequisite3 heap-reset/lifetime follows
with owned global/reference inventory, scrap-state matrix, staging/capacity and
commit/recovery semantics. Numeric BufPtr/H_min remain unselected. Separate
owner approval is needed before either implementation; all gates persist.
Earlier checkpoint paragraphs retain their dated scope.

## Launch/segment research intake - 2026-10-13

After the reader checkpoint was completed and recorded separately, SRC-0129/0130
intakes Claude's startup/segment pair. [Lead review](../research/launch-segment-lead-review.md)
records all LK-1..20 dispositions and selected text/image checks against both
published editions. Eight originals remain unchanged/untracked. No fixture/code
or case/probe execution; AL-2 and LQ-1..15 remain open, LO-1..8 unauthorized.
Next proposed contract: bounded session execution with an owned exit transition,
followed by typed Toolbox stack dispatch (continuation/no-return included),
heap-reset/lifetime rules and focused confirmation of the existing four-byte
push instruction coverage before loader fixtures. Numeric BufPtr/H_min remain
unselected. This intake authorizes none of those implementations. Reader evidence,
native-application/source/role/VM and edition gates stay unchanged. Earlier
foundation paragraphs are dated checkpoints, including their pre-intake status.

## Current foundation

Latest execution checkpoint, 2026-10-13: SPEC-0028 / ADR-0032 provides the
owned bounded session and root-step exit boundary. Complete caller restoration,
separate guest-stop diagnostics, retained effects, shared budgets/depth and
explicit exit admission pass. [Evidence](guest-sessions-evidence.md) records
initial1542 plus63 final affected checks (1479 unaffected retained), ten final
object audits/twins, matching twelve-module-audit/two-link68000 manifests and
18-byte/seven-step original encoding. CI passes final1542 plus368 bootstrap checks.
Session/report/request124/98/16 and closure frames228/252 are partial costs only.
No actual ExitToShell/Toolbox stack/completion binding, loader, heap reset or boot
entry. Eight earlier research originals remain unchanged; concurrent heap/convention
drafts stay unreviewed for separate intake. Next contract: typed Toolbox stack
arguments and returning/continuation/no-return completion; lifetime/reset follows
Claude's separately reviewed research. All source/native/VM/role/edition gates.



Latest format checkpoint, 2026-10-13: SPEC-0024 / IMPL-0031 now implements the
bounded read-only resource-fork reader. Exact type/ID lookup, structural-first
bounds, duplicate refusal, opaque references and output preservation pass original
fixtures and a delivered independent model. [Evidence](resource-reader-evidence.md)
records 1,479 local CTests in nine profiles, 1,022,976 mutation comparisons, ten
extra object audits and complete matching local/CI 68000 manifests. Both CI jobs
pass the final-source matrix plus 354 bootstrap checks. Descriptor costs 124 bytes
and frame bounds 216/192 are partial, not full 128K proof. No resources are loaded,
MFS decoded or application started. Eight research drafts, including Claude's
new launch/segment pair, remain untracked; the new pair has not been reviewed.
Next is lead review of that handoff and remaining CODE-only launch prerequisites.
All native-application, boot/VM, source/role and edition gates persist. Earlier
checkpoint paragraphs retain their original dated scope.

Latest checkpoint, 2026-10-13: SPEC-0027 / ADR-0031 supplies opt-in typed
register trap patch execution for thirteen selected OS signatures. All eleven
memory patches chain to saved originals with active allocation flags; nested
patches share ancestor budgets. Register saves and pre-entry refusals are distinct
from retained completed guest effects on late faults. The legacy atomic path
remains available. [Evidence](guest-patches-evidence.md) records the initial1416
matrix,54 final affected reruns, ten final object audits, matching compiler twins
and complete matching68000 local/CI manifest. Both CI jobs pass the complete
final-source1416 plus340 bootstrap checks. Existing boot images and all six
research originals are unchanged. No boot binding, VM/emulator/reference trial,
native application/SDK, QuickDraw or full callback/OS claim. Earlier paragraphs
below retain their dated checkpoint scope. Next is implementing finalized
SPEC-0024's bounded resource-fork reader for CODE-only loading prerequisites.

2026-10-13: SPEC-0026 / ADR-0030 now supplies canonical checked JMP/JSR,
explicit stack intervals and the original ordinary proc0 call service. It bounds
RTS completion, nesting and ancestor instruction budgets, restores caller
controls, and makes retained guest effects on late failure explicit. Its
20-byte original fixture returns in six instructions. Parameter/result
marshalling, actual manager callbacks and guest-patch routing remain later
contracts. [Evidence](guest-calls-evidence.md) records1362 local CTests,63 final
affected reruns, both successful CI jobs and an exactly matching68000 manifest.
The [QuickDraw intake](../research/quickdraw-lead-review.md)
preserves layout/edition/clipping ambiguities as future graphics work; no
graphics implementation or new boot/VM/native application permission inferred.

SPEC-0020 supplies selected Get/SetTrapAddress services; SPEC-0021 supplies an
original bounded A-line path and native Mac exception return. SPEC-0022 supplies
checked data regions, selected MOVE/MOVEA and byte/word branches, BSR/RTS,
transactional failures and a new original twelve-step program. That program is
hosted/core verification, not an x86 application or a new boot-image trial.
SPEC-0023 now supplies eleven bounded pointer/handle/size/lock/copy services,
checked globals and a separate register adapter. Published interface review and
the original 32-object/fixed-zone engineering contract are finalized; historical
uncertainties remain explicit. [Memory Manager evidence](memory-manager-evidence.md)
records 1,245 local checks and both successful CI jobs with an exactly matching
68000 manifest. The registry contains thirteen guest contracts. SPEC-0025 now
binds the eleven memory routines to actual A-line execution, with typed logical
identities, strict control/stack accesses and opt-in DATA24 data resolution.
Its original 82-byte program covers all eleven routines in 35 steps/15 traps.
[Guest-trap evidence](guest-memory-traps-evidence.md) records1299 local checks,
54 final affected reruns, both successful CI jobs and matching68000 manifests.
SPEC-0026 now supplies ordinary proc0 guest execution; other reviewed callback
signatures and actual guest patch dispatch remain next. Most CPU
instructions, purging/growth/callbacks and application loading are unimplemented.
[Lead review](../research/memory-manager-lead-review.md) distinguishes the final
contract from Claude's preserved research proposals.

Gemini's [MFS research intake review](../research/mfs-gemini-lead-review.md)
checks the supplied proposals against published format diagrams. It finds
reversed lock bits, invalid fixture geometry and incomplete directory/empty-fork/
name rules. Test categories are useful; no parser contract or fixture is finalized,
and 12-bit packing remains unverified. Filename-comparison research is a separate
follow-up. The 2026-10-13 [filename review](../research/mfs-filenames-lead-review.md)
(SRC-0109) corrects the report's case flag and version/diacritic claims. Future
reader choices preserve counted raw names and versions, identify ASCII-only
comparison as incomplete, and refuse an ambiguous selected identity. No MFS API
or decoder is finalized. At that intake guest-trap integration was the next
dependency; SPEC-0025 now supplies that bounded built-in layer.

2026-10-13: [Claude's resource-fork review](../research/resourcefork-lead-review.md)
adopts SPEC-0024 / ADR-0028 and settles L-1..12 for a tolerant bounded reader.
TEST-0032 is a conformance plan; no parser or audit has executed. The optional
full audit remains deferred with explicit scratch/work requirements. The
[asset register](../compatibility/assets.md) tracks future original content;
its [source review](../research/asset-replacement-lead-review.md) does not create
new legal approval gates. Existing compiled Memory Manager/native results and
the implementation order remain unchanged.

2026-10-13: SPEC-0025 / ADR-0029 connects the existing memory register adapter
to a personality-owned signature-checked directory and original interpreted
heap program. Get/Set patches remain observable; saved original identities have
a value-frame direct path. Guest patches still return needs-execution. All
failure paths preflight exception scratch, including low-memory and heap/BlockMove
overlap; guest RAM, manager and CPU state are preserved on contract failures.
No boot-entry binding or VM/emulator run. Claude's concurrent callback catalog
remains untracked research for separate source/signature review, not a dependency
of this built-in slice. Complete callback and user/Toolbox execution remain open.

## Work allowed now, in dependency order

2026-10-13 research intake: the [callback/application lead review](../research/callback-application-lead-review.md)
(SRC-0113/0114/0115) settles K-1..7 and FA-1..5 as planning choices. Step 3
uses call-site signatures, specific register exceptions and explicit A5 context;
bounds/alignment, owned return, completion shape and depth/budget limits are
mandatory. Unknown cleanup, interrupt entries and nonreturning entries are
deferred. Step 4 assigns guest identities to supported defaults, without deriving
native pointers from guest data. Step 1 imports no unreviewed catalog JSON;
Appendix B omissions are not transitive safety guarantees. Steps 5/6 respect
declared CODE0 sizes while checking the actual InitGraf span and (A5) slot;
private QuickDraw state is original, and the first candidate workload is the
23-routine window/drawing/mouse/quit tier, followed by menus/original accessories.
The 512 by 342 guest screen is a planned engineering profile, with larger guest
screens explicitly extended. Baseline UX claims remain a checklist and R-001
stays open. This research intake changes no code and authorizes no new slice.

1. **Routine and record interface registry.** The two trap-address services and
   eleven phase-1 Memory Manager routines are now recorded after source/contract
   review, with thirteen records and eight rejection controls passing. Expand
   through further reviewed contracts. Per routine record trap word/number, parameter
   type/width/location, value/object/code class, procedure signature, result,
   result-code globals, preservation/stack cleanup, memory-movement status and
   profile availability. For guest records, record verified field widths,
   offsets/byte order and source locator. Unknown introduction release stays
   unknown; first documented edition is a separate field. Independently check
   68000 marshalling/guest parsing from the registry. No native headers/SDK.
2. **Original bounded Memory Manager.** SPEC-0023 finalizes pointer/handle/cell/
   zone representation over SPEC-0022, invalid/zero/resize/error behavior,
   checked guest globals and allocation-only SYS/CLEAR routing. Live-block
   nonoverlap, foreign values, locks, prefix preservation, failed-call atomicity
   and copy overlap pass original tests. Raw stale aliases remain indistinguishable
   after address reuse. SPEC-0025 now connects the separate adapter through typed
   built-in traps and opt-in data-only address normalization; the original guest
   fixture dereferences locked cells while preserving their flag-bearing values.
   Current native manager metadata is 672 bytes and guest cell bands
   256; final linked 128K budgets remain open. No Apple heap/header/list or
   placement algorithm adopted. The binding adds 20 bytes on 68000. Purging/growth/
   callbacks and actual guest patch execution need later contracts.
3. **JSR/JMP and call-into-guest service.** Review published original-68000
   control-address modes; implement checked targets/pushes separately from data
   access. Then run a guest address under an explicit signature until return to
   an owned sentinel. Each nested call has its own budget and bounded depth;
   preserve/restore A5/A7 exactly on the specified return/error paths. Tests
   cover nesting, early failure, forged/bad return, stack exhaustion, budget
   exhaustion and incompatible signatures. This serves 68000 procedures and
   patches; it introduces no native application entries.
   SPEC-0026 supplies the first bounded ordinary no-parameter/no-result service
   and canonical control modes. Further call-site shapes still need verified
   marshalling, return and side-effect contracts; no blanket callback support.
4. **Typed dispatch.** Specify built-in identity and guest-code entries with
   consistent Get/Set/direct-call logical addresses. Route an installed guest
   patch through the bounded service, preserving per-routine calling rules.
   Reserve unspecified extension space; no third native kind, native marker or
   native-entry registry is populated. Test saved original-address chaining,
   unsupported types, independent directories and pre-entry refusal without partial
   writes; record completed guest effects retained after entry.
   SPEC-0027 now supplies this selected register patch profile, including eleven
   saved-original memory veneers. Get/Set direct veneers, arbitrary direct guest
   calls, Pascal shapes and broader typed dispatch remain explicit later work.
5. **CODE-only launch selection and format prerequisites.** Make selection a
   separate stage with one permitted outcome, 68000 CODE, plus explicit refusal
   diagnostics. First implement checked read-only resource/MFS contracts using
   wholly original generators and fixtures. Verify CODE0 sizes, A5-world/stack
   arithmetic and segment/jump-table requirements from eligible published
   sections; keep unresolved offset/layout/entry-state questions explicit.
   Launch/LoadSeg/UnloadSeg/ExitToShell, Toolbox stack marshalling and heap-reset
   behavior each need their own spec/tests. No native descriptor or image loader.
   SPEC-0024 now supplies the read-only resource-fork prerequisite; it is not
   Resource Manager loading or an application loader. MFS packing remains an
   unresolved source question. Claude's launch/segment drafts require lead
   review before new startup/segment contracts or execution.
6. **Useful original 68000 application, then complete experience.** Execute an
   independently generated resource-fork application through managers and
   lifecycle services, expand the missing ISA/QuickDraw/events/windows/menus/
   file contracts, then use a named legally supplied original application only
   under an approved local protocol. Record results and unsupported operations
   per exact profile. Optional debug interrupt-context checking uses verified
   routine metadata; a flag alone cannot infer every interrupt context.

Each step follows specifications, original code/tests, provenance and evidence;
this plan is not authority to skip those gates. New VM/emulator execution waits
for the owner's go-ahead and exact image/platform admission.

## Closed native-application gate

No x86 application/native test program or native image loader is built, loaded
or run until the owner explicitly confirms the 68k experience complete. Own
native OS/core/tools and hosted contract tests remain permitted. No native
converter/packager, descriptor/type code, entry registry/marker, SDK/header or
chosen x86 application ABI implementation is prepared meanwhile. D3/D5-D7 and
ISA-tag/native format choices remain proposals; later-era orientation sources
must be reviewed/registered before any specification relies on them.

When the 68k experience is near completion, propose measurable acceptance
criteria for the owner's approval, including the supported manager/API matrix,
M0.6/M0.7 results, his chosen application's real-Mac-equivalent workflow,
document/file results and known differences. Only his confirmation opens the
gate; neither a passing build nor a milestone automatically does so.

Future authorized variants use the ordinary transparent experience: one file/
identity, no choice prompt, matching results, shared guest-visible state,
working patches and an intact 68000 path elsewhere (T-1 through T-6). Existing
GPL-3.0-or-later terms remain unchanged; the project does not decide others'
licences. Do not infer January1984 application support from later published
examples. Full OS/physical/parity/three-edition gates persist.
