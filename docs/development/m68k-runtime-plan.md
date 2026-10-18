# 68000 application runtime plan - 2026-10-12

2026-10-18: SPEC-0047 adds the shared native two-port visual workload and explicit Mac/PC candidates. Final hosted/sanitized/compiler/media checks pass; paired CI/native68000 candidate receipt pending. Existing modes and input behavior preserved; no target run/release. [Evidence](paired-selection-evidence.md).

2026-10-18: SPEC-0046 implements explicit selected-port PaintRect/EraseRect through the shared native68000/x86 owner/view. Hosted/sanitized tests and CI/native68000 compile/link checks pass. Old modes/UI scenes unchanged,no target trial/release. Next is the original paired workload. [Evidence](selected-drawing-evidence.md).

2026-10-17: SPEC-0045 integrates actual prepared GetPort/SetPort owner transactions and original loaded-guest selection conformance. Hosted/sanitized checks and CI/native68000 compile/link verification pass (2026-10-18). Selected-port drawing/full manager remains next,not enabled here. No target trial/release. [Evidence](port-owner-evidence.md).

2026-10-17: SPEC-0044 / ADR-0050 finalize the prospective prepared-port ownership contract: exact-g integrity, explicit owner environment spans, one admission transaction and consumed-generation-safe retirement. All40 cases remain unexecuted. No code/binding/build/trial/release; existing native inputs unchanged. [Review](prepared-port-owner-lead-review.md).

2026-10-17: SPEC-0043 implements pure shared native68000/x86 drawing-port selection plans; no guest memory access or trap binding. 810 local CTests,10 width/endian object audits and8 broken-core controls pass; CI passes3013 CTests across11 profiles and six68000 objects (no target execution). Existing scenes/workloads/ROM/EFI inputs unchanged. [Checkpoint](native-selection-evidence.md).

2026-10-17: SPEC-0041 adds a borrowed native drawing view with coordinate translation and rectangular clipping. The same68000/x86 workload exercises both while preserving every scene pixel and the independent old68k fixture.804 local tests,70 object audits and unloaded EFI controls pass; full CI and native68000 cross-build pass. New candidates are unbooted; no target trial or release.

[Drawing context checkpoint](drawing-context-evidence.md).

2026-10-17: One approved isolated SPEC-0040 native PC boot passes READY, direct Space plus two mouse edges/held-repeat/release, exact static-scene pixels and normal guest-second60/count3. Desktop UI delivery and human input remain unqualified. Session closed; no Mac trial, code change or release.

[Native PC mouse observation](pc-mouse-boot-trial.md).

2026-10-16: SPEC-0040 implements single-owner8042 keyboard/mouse startup and routing, plus an explicit native x64 PC mouse build profile. Shared button semantics and512x342 scene match the existing Mac/Windows foundation. Hosted/compiler/EFI candidate evidence only; no emulator/VM/hardware/reference trial or release. Default legacy keyboard contract/trace, UART source/test bodies, Mac ROM inputs,24 originals/copies and frozen wiki remain preserved.

[PC mouse checkpoint](pc-mouse-evidence.md).

2026-10-16: SPEC-0039 adds shared native button edges, a pure PS/2 report decoder and Windows/native-Mac scene bindings. Freestanding PC8042 startup/routing, pointer motion and full event parity remain open. No new target trial or release. OneNote and its Git wiki queue are permanently retired; existing wiki files stay frozen history.

[Native mouse evidence](mouse-input-evidence.md).

2026-10-16: The original native C drawing workload now calls the rectangle service directly on native68000 and x86 paths. PC boot/Windows viewer and a distinct native Mac ROM use real native results/pixels; the separate40-byte68k compatibility fixture remains. Independent pixel/failure/legacy tests, nine-profile2280 checks, dependency audits and native candidates pass. No new boot observation or release; PC mouse/event parity remains open.

[Native drawing evidence](native-drawing-evidence.md).

Current owner correction2026-10-16: this is the68k runtime workstream, not a
gate delaying native x86 OS counterparts. [Paired native delivery](native-parity-plan.md)
advances each Mac/PC capability together under SRC-0217 / ADR-0043. Dated
native-gate statements below retain their former checkpoint scope only.

2026-10-16: One portable monochrome product scene now serves the drawing ROM, native-PC bootstrap and hosted Windows viewer. Both boot paths use the same original 68k drawing fixture, with real result/phase/pixel state. The logical canvas is512x342; PC640x480 centers it at(64,69), with exact pixel parity and white margins. The owner-supplied monitor/C mark is approximated in original black/white geometry. Both native candidates compile; no new target trial. [Shared scene](shared-product-scene-evidence.md). Native applications and older gates remain closed.

2026-10-16 drawing trial: one freshly authorized isolated Snow128K boot shows APP/097, the original loaded application’s hollow rectangle and six phases. Counter0000/0001 and later0001 captures retain exact result-panel pixels. [Observation](drawing-app-boot-trial.md). Trial closed, automated delivery unqualified and no human input credited. General guest port state/clipping remains the next expansion; no new release or other gate closes.

2026-10-16: SPEC-0036 / IMPL-0040 / TEST-0044 adds an explicit owned-code
PaintRect/EraseRect profile over a fixed native128x64 monochrome port. The
original loaded program produces a hollow rectangle in13 instructions and
explicitly retires. [Evidence](drawing-app-evidence.md). Global QuickDraw/port
state, Get/Set/direct equivalence and callbacks remain unimplemented; ordinary
trap refusal remains. Candidate compilation and actual boot observation are
separate. Released alpha assets and all historical/edition/source gates remain.

2026-10-15 redraw refinement: byte fills and counter-only updates preserve12 original frames exactly. 2127 local and2625 CI checks and original ROM twins pass. One freshly authorized Snow trial shows the complete result and counter updates without observed full-scene redraw; automated input delivery remains unqualified. [Evidence](loaded-app-redraw-evidence.md). Earlier checkpoints retain dated results.

2026-10-15: The original resource/CODE loader now runs our twelve-byte 68k test application in the native boot ROM, captures D0=97 and its 0x5A5A5A5A write, and retires the image. One owner-authorized isolated Snow Macintosh 128K cold boot displays APP/097, the full pattern and six completed phases; an automated mouse event advances the kernel counter to 0001. All 2,118 final local and 2,614 CI checks pass. This is an integrated prototype, with general SPEC-0034, desktop/QuickDraw and both System 1/ROM interoperability targets still incomplete.

[Integrated boot evidence](loaded-rom-app-evidence.md). Future emulator/VM/reference trials still require fresh owner go-ahead. Earlier checkpoints are dated history.

2026-10-15: SPEC-0034 X-1 / IMPL-0038 / TEST-0042 implements guarded single-instruction interpreter transactions. Actual opcode, extension, read, write, target, instruction and final-state checks precede staged RAM/frame publication. All A-lines refuse before either service route; strict24, bounded even A7 and S=1/T=0/I=7 apply. Existing unguarded APIs retain their semantics. X-2..X-5 and full owned-image execution remain unimplemented. No application/image/session entry, reset/boot integration, native application or VM/emulator/reference trial. All24 research originals/copies and source/UART/native/physical/full128K/parity/edition/B2/human gates and both untested System1/ROM goals remain.

[Evidence](guarded-interpreter-evidence.md). All2055 final CTests, inherited object audits, guard-test compiler twins and GNU68000 closure checks pass. Both publication CI jobs pass on source `f2f0341`; complete local/CI guard manifests match after stack-frame-row sorting; own-ROM bytes are unchanged.
Next: X-2 guarded session policy, entry/token/RTS and stop binding. Earlier checkpoints are dated history.

2026-10-15: SPEC-0034 / ADR-0038 finalizes an unimplemented controlled-image execution contract. One generation-bound root, strict guarded accesses/commit, explicit image/inventory authority, balanced return, distinct guest-fault/quarantine states and quiescent retirement precede any execution. X-1 guarded interpreter transactions are the first dependency. Claude authors the first draft; Grok checks one boundary; Codex integrates and remains the sole repository writer. Two bounded CLI calls, no truncated reads. No code/fixtures/TEST/IMPL/build, application/emulator/VM/reference run, native app or boot change. All24 originals/copies, compiled/build/edition/project pointers and both untested System1/ROM boot goals remain.

[Contract and lead dispositions](owned-image-execution-review.md). Next: bounded X-1 guarded interpreter
transactions before any session/image execution integration. Earlier checkpoints
retain their dated scope; compiled/verified pointers and prior tests stay unchanged.

2026-10-15: SPEC-0033 / IMPL-0037 / TEST-0041 implements original host-only eager CODE staging, exclusive locked handles, A5-world data population and explicit image retirement. A first allocator OOM retains PREPARED; a later ordinary OOM retains PARTIAL with explicit cleanup and no world writes. Unexpected faults retain exact effects and block further admission. No application execution, guest binding, reset/boot integration, native application or VM/emulator/reference trial. Historical/source/native/physical/full128K/parity/edition/B2 and human-review gates remain open; both required System1/ROM boot directions remain untested.

[Implementation evidence](code-image-evidence.md). All2010 final local CTests, ten width/endian object audits and compiler twins pass;
GNU68000 sixteen audits/two complete links pass. Both publication CI jobs pass on source `99d4a02`;
the complete local/CI68000 image and prior own-ROM manifests match.
Next is the controlled execution bridge contract; no automatic application launch.
Earlier checkpoints are dated history.

2026-10-15: SPEC-0033 / ADR-0037 finalizes an original host-only eager CODE image loading/ownership/retirement contract, unimplemented. Selected bytes stage in caller-owned native storage; exclusive locked handles and loaded-table data have one owner. First allocation commits effects; ordinary later OOM retains PARTIAL for explicit cleanup, unexpected faults stay quarantined. Destination EXEC permissions and physical executable aliases refuse; execution needs a later admission bridge. Claude/Grok each read one frozen public contract/API packet and return six lead-reviewed findings, not tests or historical evidence. All24 originals/copies, compiled/interface/boot/assets/build/edition/project pointers and both untested System1/ROM boot goals remain. No implementation/fixture/TEST/IMPL/build, guest binding, app execution, native application or VM/emulator/reference run. Human provenance and source/UART/native/physical/full128K/parity/B2 gates persist; Git wiki queue only.

[Contract/review](code-image-contract-review.md). Next propose the original bounded load/cleanup implementation;
actual execution still needs its own ownership/fetch/write/re-entry bridge.
Earlier checkpoint paragraphs are dated history.

2026-10-15: SPEC-0032 / IMPL-0036 / TEST-0040 implements the original read-only hosted CODE inspection/layout checker. It checks64 entries/16 segments, literal resource-start offsets, header/group/routine bounds and explicit candidate A5/stack arithmetic without loading or executing an application. All1911 final CTests pass across nine configurations, ten width/endian object audits and matching compiler twins. GNU68000 twelve module audits/two complete relocatable links pass; manifest1440, workspace1632, layout68 bytes are partial costs, not full128K proof. Claude/Grok provide bounded read-only findings; Codex retains implementation and decisions. All24 originals/copies and prior interface/assets/boot/UART and edition records are preserved. Both required System1/ROM boot directions remain untested. No guest binding, loader/reset/boot integration, native application or VM/emulator/reference run. Human provenance and historical/physical/parity/full128K/B2 gates remain open.

[Implementation evidence](code-preflight-evidence.md). Both publication CI jobs pass on source `724a72d`;
the complete local/CI68000 manifest matches. Next propose the
executable-image ownership and launch/population contract; dynamic LoadSeg and
transitive entry lifetimes remain separate. Earlier checkpoints are dated history.

2026-10-14: SPEC-0032 / ADR-0036 finalizes data-only CODE launch preflight, unimplemented:64 entries/16 segments, literal resource-start offsets flagged historically unverified, conservative total32K segments, complete header/group/target bounds and explicit B/S/H_min world/stack arithmetic. Source warnings and snapshot applicability remain distinct from execution authority. Current full-zone stack refusal and CLOSED-only resource execution/reset gate remain; a later executable owner/continuation/population contract is required. Grok reads one frozen reviewed-public packet and returns five lead-reviewed findings, not executed tests. All24 originals/copies and compiled/interface/boot/assets/build/edition/current project pointers are preserved. Both System1/ROM boot directions remain required/unverified. No code, fixtures, TEST/IMPL/build, registry/guest binding, loader, native application or emulator/VM/reference run. Source/UART/native/128K/physical/parity/edition/B2/human gates remain; Git/HQ/wiki queue only.

[Contract/review](code-preflight-contract-review.md). Earlier checkpoint paragraphs are dated history.

2026-10-14: SPEC-0031 / IMPL-0035 / TEST-0039 implements the original hosted resource-residency service: one immutable fork,16 locked application copies and32 distinct native lease tickets. Checked ownership, saturating IDs, source/epoch lifetime, explicit eviction/close and retained partial-fault masks precede any future CODE loader. All1803 initial local CTests pass across nine configurations;90 final affected checks pass with1713 unaffected retained, ten final width/endian object audits and matching compiler twins. GNU68000 twelve module audits/two complete relocatable links pass. Native service3314, request52, report246, ticket36 bytes; source bytes/resident extents and existing controls are additional costs, not full128K or cumulative-stack proof. Claude/Grok read one frozen public/original packet each and supply lead-reviewed findings, without execution or historical authority. All24 research originals/copies, prior interface/assets/boot/UART inputs and edition gates are preserved. No guest Resource Manager binding, loader/execution, native application or VM/emulator/reference trial. Human provenance review and historical/physical/parity/full128K/B2 gates remain open.

[Implementation evidence](resource-residency-evidence.md). Both publication CI jobs pass on compiled source `b94e2ff`;
the complete local/CI68000 manifest matches.
Next propose a bounded CODE-only launch/segment contract using these foundations;
dynamic LoadSeg and transitive executable-resource ownership remain separate.
Earlier checkpoint paragraphs are dated history.

## Hosted resource residency contract - 2026-10-14

2026-10-14: [SPEC-0031](../specifications/SPEC-0031-hosted-resource-residency.md)
/ ADR-0035 finalizes a hosted Resource Manager residency foundation: one immutable
source,16 locked application copies,32 explicit tickets, release/eviction/close
lifetimes, actual partial-fault diagnostics and FUTURE/REFUSE composition with
heap cleanup. [Lead review](resource-residency-contract-review.md)
records bounded Claude/Grok findings and final dispositions. Contract only,
unimplemented; no new tests/fixtures/code/bindings/build or execution. All24
research originals/copies and prior verified inputs are preserved. Next propose
its original hosted implementation before CODE loading. Existing historical,
native/VM/UART/physical/128K/edition/B2/human-review gates remain separate.
Earlier checkpoint text below is dated history.

## Hosted heap-lifecycle implementation - 2026-10-14

2026-10-14: SPEC-0030 v2 / IMPL-0034 / TEST-0038 implements the bounded hosted
application-heap lifecycle. Application allocations retire, persistent system
state and admitted resident scrap survive, single-use plans revalidate, epochs
saturate and partial faults retain diagnostics and block coordinated entry.
[Evidence](heap-lifecycle-evidence.md) records1713 initial local
checks,99 final affected checks with1614 unaffected retained, ten object audits,
compiler twins and standalone68000 closure/storage. Both publication CI jobs pass on corrected source `89e3bbb`;
complete local/CI68000 manifests match, and prior ROM/media build identities are preserved.
Claude/Grok provide bounded read-only reviews; Codex retains decisions. All24
untracked research originals/copies and prior boot/interface/assets inputs are
preserved. No guest reset/loader/boot binding, new VM/reference trial or native
application. Next is Resource Manager service/lifetime design before CODE loading.
Historical/full128K/physical/edition/B2 and human review gates remain separate.
Earlier checkpoint text below is dated history.

## Claude/Grok orchestration - 2026-10-14

SRC-0149/0150 / [protocol](agent-orchestration.md): owner authorizes Codex-led
bounded assignments and relevant reviewed public-contract/original-code
transfers to Claude/Anthropic and Grok/xAI. Other input/task/execution gates
remain. Claude's first Read-only run returns two reviewed issues/eight unexecuted
cases; SPEC-0030 v2 clarifies teardown scrap and explicit controller transitions.
No implementation/fixture/TEST/IMPL/build or historical closure. Next remains
the separately scoped hosted heap implementation; Codex writes, Claude checks
failure oracles, Grok checks requirement coverage within approved scope.

## Hosted heap-lifecycle contract - 2026-10-14

SRC-0146..0148 / [SPEC-0030](../specifications/SPEC-0030-hosted-heap-lifecycle.md)
/ ADR-0034 finalizes prerequisite3's engineering contract, not its implementation.
Inactive closed-world ownership, exact placement/capacity, disjoint scrap staging,
commit/fault/teardown separation, persistent system allocations and saturating
native epochs are explicit. Guest resets/limit binding, installed patches,
unsupported globals/references, system scrubs, disk scrap and Finder/A5/segment
population refuse or remain later contracts. Six existing manager views are
supported; the31-name inventory is future continuous-validity coverage, not31
bound fields. Saved CPU snapshots are not heap/RAM copies; raw address reuse
remains indistinguishable. Grok supplied a bounded18-row review, accepted with
lead qualifications; all22 originals/copies remain unchanged/untracked.

[Contract review](heap-lifecycle-contract-review.md) records the decisions and
usage. No TEST/IMPL/build or executed case, code/registry/boot change or historical
closure. Next propose the separately scoped original hosted implementation of
this contract, followed by CODE-only loader-specific validation. Native/VM/source/
UART/physical/full128K/edition/B2/human-review gates remain separate.

Owner-governed plan, SRC-0099 / ADR-0026. This incorporates Claude's returned
Memory Manager, application-loading and native-application research without
treating research proposals as finalized historical specifications.

## Re-entry, event and package intake - 2026-10-14

SRC-0142..0145: [one lead review](../research/reentry-event-package-lead-review.md)
records four matched identities,17 preserved originals, forty selected published
image checks and all20 RA/ES/PG dispositions. Closed-world refusal comes first;
EventAvail uses conservative(b), guest auto-key synthesis and a separate guest
queue are planned, selective evaluation deferred. Packages need a separate mixed
selector ABI; no fake system PACK handle. Direct library memory/ROM obligations
are continuous, and MemError storage is unspecified. Existing dispatcher admission
refuses absent/register records but has no hardcoded package blacklist. Correct
queued-activate wording and complete picture/SystemClick closure in future records.
All34 RA/ES/PG historical labels and earlier questions remain open; RA-S1..22 are
unexecuted. No original is edited, registry imported, spec/code/fixture/build made
or new execution/implementation gate opened. Completed SPEC-0029 evidence stays
unchanged. Next remains the separate heap-lifecycle/reset contract, now with typed
reference/global lifetime and continuous coherence obligations before guest code.
Owner continuation precedes implementation; all standing gates persist.

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

## Current native parity rule; earlier blanket gate superseded

SRC-0217 / [ADR-0043](../adr/0043-coupled-native-mac-pc-parity.md) clarifies
that a complete68k experience is the Mac goal; matching native x86 PC behavior
must advance with it. No whole-Mac-completion prerequisite delays native PC
counterparts or original native parity workloads. This plan remains the68k
compatibility/runtime lane, coupled to the [native parity plan](native-parity-plan.md).

Native ABI/SDK/container/loader details are not automatically implemented or
selected by this clarification. Specify each needed boundary and validate it
within its paired feature slice. Preserve all legacy instruction/guest-state
and source/edition constraints. Actual target trials still require fresh go-ahead.

Future authorized variants use the ordinary transparent experience: one file/
identity, no choice prompt, matching results, shared guest-visible state,
working patches and an intact 68000 path elsewhere (T-1 through T-6). Existing
GPL-3.0-or-later terms remain unchanged; the project does not decide others'
licences. Do not infer January1984 application support from later published
examples. Full OS/physical/parity/three-edition gates persist.

2026-10-17: SPEC-0042 / ADR-0048 finalize the prepared guest GetPort/SetPort selection contract. Same native semantic core planned for68000/x86; exact A5 anchor/current-cell ownership and total-commit rules specified. No implementation, tests, registry binding, build or target run in this slice. Existing drawing candidates unchanged/unbooted; full InitGraf and selected-port drawing adapter remain prerequisites. [Lead review](guest-port-lead-review.md).
