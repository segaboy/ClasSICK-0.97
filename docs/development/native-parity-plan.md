# Native Macintosh and x86 parity plan

2026-10-22,SPEC-0061:firmware-neutral [PC window session](pc-window-session-evidence.md) implemented and hosted/compiler verified. It joins shared window input/controller receipts/presentation;not boot-bound. Next:opt-in UEFI arena/trace binding. Mac M1-M7 and paired release qualification persist.

2026-10-22,SPEC-0060:isolated PC [Mono1/row presentation bridge](pc-mono-present-evidence.md) hosted/compiler verified,not boot-bound. Shared scene/core and Mac/PC release parity unchanged. Next:neutral PC window session;Mac M1-M7 remain independent prerequisites.

2026-10-22,SPEC-0059/ADR-0068: [PC window boot-binding behavior](../specifications/SPEC-0059-pc-window-boot-binding.md) finalized,32 planned TEST-0070 cases unexecuted. No binding/code/build/trial yet;first proposed implementation is the RAM Mono1/one-row PC presentation bridge. Mac M1-M7 closure and paired release criteria remain separate.

2026-10-21,SRC-0341/0342: owner adopts a [unified native x86-64 PC distribution](unified-x64-pc-plan.md):one shared compiled OS payload with BIOS/UEFI entries and growing tested hardware coverage. Current PC release remains UEFI-only;universal support is a goal,not an observed result. Paired-native-preview-2 remains the immediate checkpoint;no new code,build,trial or release in this intake.

2026-10-19 source-review continuation, SRC-0314/0315: owner clears narrow published mouse review; earlier hold resolved, declaration excluded. First pointer/input contract/inventory slice complete with Mac adapter gaps M1-M7; no code, tests, build or trial. Next recommended: bounded cursor composition contract. [Review](mac-pointer-source-review.md). Older held checkpoints below are historical. Existing gates and released capabilities remain unchanged.

2026-10-19 release2 first slice: SPEC-0051 pure native pointer/input contract for both CPUs,68 unexecuted case identifiers and separate PC input-evidence inventory. Mac hardware source lane held for owner disposition;no motion binding or pixel/GUI/human pass claimed. [Review](pointer-input-contract-review.md).

Current release/next goal2026-10-19: ROM0.3.0-alpha.1/PC0.1.0-alpha.1 published;[receipt](../../releases/paired-native-preview-1-publication.json) supersedes earlier pending-status paragraphs. Next proposed [paired-native-preview-2](../../releases/paired-native-preview-2.md) adds pointer/cursor and one draggable/content-click window to both native paths. Planning only;source sufficiency/contract first,no code or new trial.

2026-10-19 paired-native-preview-1: SPEC-0050 removes the PC normal60-second/Escape stop;38 mouse+5 Space inputs accepted through185 guest seconds,scene intact. Mac ROM byte-identical. CI3150/11 pass;human PC capture/GUI startup and release freeze remain open. [Evidence](preview-session-evidence.md).

2026-10-19 paired-native-preview-1: exact native Mac/PC boots and all175104 same-counter scene pixels match. Mac human input passes; PC human input unreliable despite21/21 synthetic presses. Release blocked on PC input/lifecycle and package/publication. [Evidence](paired-preview-boot-trial.md).

2026-10-19: owner directs every substantive slice toward paired nativeMac68000/x86 PC preview releases. Current target [paired-native-preview-1](../../releases/paired-native-preview-1.md),ADR-0055: finish outlines in both native scenes,then package/qualify/freeze the pair. Compiler/hosted/target/human/package/publication states are distinct; no launch/publication permission expands.

2026-10-19: SPEC-0048 separate original native68000/x86 outline helper implemented; independent pixel/state/canary tests and compiler/CI checks recorded. No existing scene/ROM/EFI input or trap change,target run or release. H1-H5 and existing gates remain open. [Evidence](native-outline-evidence.md).

2026-10-18: [SPEC-0048](../specifications/SPEC-0048-native-rectangle-outline.md) finalizes the next original native68000/x86 rectangle-outline contract only. No runtime/candidate changes;44 cases unexecuted,H1-H5 open. [Boot screenshots](../evidence/screenshots/README.md) are now published unchanged in Git.

2026-10-18: exact SPEC-0047 native68000/x86 candidates booted once each; two-port drawing and synthetic counters observed. Six static PC comparisons and normalized Mac/PC same-counter comparison pass. Sessions closed; no release. Mac progress/PC timing, Snow preference isolation and human/physical/remaining compatibility gates retain explicit limits. [Evidence](paired-selection-boot-trial.md).

2026-10-18: SPEC-0047 adds the shared native two-port visual workload and explicit Mac/PC candidates. Final hosted/sanitized/compiler/media checks pass; paired CI/native68000 ROM and native PC disk verification pass. Existing modes and input behavior preserved; no target run/release. [Evidence](paired-selection-evidence.md).

2026-10-18: SPEC-0046 implements explicit selected-port PaintRect/EraseRect through the shared native68000/x86 owner/view. Hosted/sanitized tests and CI/native68000 compile/link checks pass. Old modes/UI scenes unchanged,no target trial/release. Next is the original paired workload. [Evidence](selected-drawing-evidence.md).

2026-10-17: SPEC-0045 integrates actual prepared GetPort/SetPort owner transactions and original loaded-guest selection conformance. Hosted/sanitized checks and CI/native68000 compile/link verification pass (2026-10-18). Selected-port drawing/full manager remains next,not enabled here. No target trial/release. [Evidence](port-owner-evidence.md).

2026-10-17: SPEC-0044 / ADR-0050 finalize the prospective prepared-port ownership contract: exact-g integrity, explicit owner environment spans, one admission transaction and consumed-generation-safe retirement. All40 cases remain unexecuted. No code/binding/build/trial/release; existing native inputs unchanged. [Review](prepared-port-owner-lead-review.md).

2026-10-17: SPEC-0043 implements pure shared native68000/x86 drawing-port selection plans; no guest memory access or trap binding. 810 local CTests,10 width/endian object audits and8 broken-core controls pass; CI passes3013 CTests across11 profiles and six68000 objects (no target execution). Existing scenes/workloads/ROM/EFI inputs unchanged. [Checkpoint](native-selection-evidence.md).

2026-10-17: SPEC-0041 adds a borrowed native drawing view with coordinate translation and rectangular clipping. The same68000/x86 workload exercises both while preserving every scene pixel and the independent old68k fixture.804 local tests,70 object audits and unloaded EFI controls pass; full CI and native68000 cross-build pass. New candidates are unbooted; no target trial or release.

[Drawing context checkpoint](drawing-context-evidence.md).

Current owner authority2026-10-16: SRC-0217/0218 and
[ADR-0043](../adr/0043-coupled-native-mac-pc-parity.md).
The former blanket wait for a complete Mac experience no longer governs native
PC advancement or original native parity workloads. This began as a scope correction. SPEC-0038 now implements the paired native
drawing workload; target boots and broader parity remain separate evidence.

## Workstreams and completion

1. Native Macintosh: compile the portable OS/managers for68000 and supply the
   Mac hardware/ROM/ABI boundaries needed for the complete Mac experience.
2. Native PC: compile the same semantic implementation forx86 and supply PC
   hardware/ABI boundaries for each matching advancement. The OS executes
   natively; it must not depend on interpreting68k code to supply OS features.
3. PC legacy applications: interpret original-compatible68k application code
   through documented guest boundaries into the same native managers. Maintain
   this alongside the native PC path; it is not the PC OS implementation.

Each feature record identifies the native Mac path, native PC path, applicable
legacy68k path, expected behavior and tested gaps. Record compilation, hosted
conformance, actual target observations and physical/historical qualification
separately. An interpreted fixture or equal screenshot alone does not establish
native workload/API/input parity. A shared milestone stays incomplete if one
native side is missing. Exceptions need an explicit owner decision.

Use one portable C implementation for shared behavior and original workloads
where practical; compile it for both CPUs. Assembly is not required for ordinary
portable features. Native application ABI/SDK/container work is developed when
its bounded feature needs it, with reviewed contracts; no whole-Mac-completion
prerequisite and no implied claim of an already working general native loader.
Existing transparent-variant and licensing requirements remain unchanged.

## Current state and gaps

| Capability | Macintosh path | PC path | Disposition |
| --- | --- | --- | --- |
| Branding/layout renderer | Native68000 C | Native x86 C | Shared512x342 renderer; hosted exact-pixel parity and current boot captures recorded |
| Fixed-port PaintRect/EraseRect semantics | Native C manager | Native C manager | Selected bounded API profile; not general QuickDraw |
| Existing original40-byte application fixture | Guarded68k interpreter prototype | Guarded68k interpreter prototype | Compatibility/runtime coverage; not native application-execution parity |
| Original native drawing workload using those APIs | Published ROM0.3.0-alpha.1 | Published native PC0.1.0-alpha.1;hosted binding also exists | Both exact images observed;shared outline/counter scene and normalized pixel parity recorded |
| Native coordinate view,rectangular clip and outline | Same original view/selection/outline core in native68000 workload | Same core in native x86 workload | SPEC-0041/0045..0049 foundations are used by the released pair;general guest graphics ABI remains incomplete |
| Scene input | Shared native button core;Snow human2 clicks pass | Shared core;native continuous preview38 mouse+5 Space injected through guest185 | Mac motion/cursor and full event parity absent from released UI;PC human capture/GUI unqualified,synthetic success is scoped |
| Pointer/cursor and one interactive window | Shared native core compiler-verified;Mac motion/boot adapter pending | Shared core and Windows hosted preview verified;PC controller delivery verified;window boot binding pending | Paired-native-preview-2 in progress;hosted/compiler evidence does not qualify native motion boots |
| Full windows/menus/events/files/System disk experience | Incomplete | Incomplete | Develop each advancement on both native sides |

The renderer and original drawing workload execute as native C on both targets.
The [published pair](../../releases/paired-native-preview-1-publication.json)
records exact images and their bounded evidence. The separate guarded40-byte68k
fixture still supplies compatibility/runtime coverage. Mac human button input
passed;native PC synthetic input passed but ordinary human capture/focus and GUI
startup remain unqualified. No general external native application ABI/loader is claimed.

SPEC-0039 supplies shared button semantics, native hosted bindings and pure report
decoding. SPEC-0040 now implements single-owner8042 startup/routing and scene integration
in an explicit native x64 profile. One approved native PC boot now passes direct
Space and mouse API edge/held/release checks with static pixels preserved. The
session is closed. Full pointer motion/events, human input, the current native
Mac profile boot and broader target qualification remain open; synthetic device
events cannot close them.
Actual target runs need fresh owner go-ahead. Keep24 originals/copies, UART
bodies, releases, previous results and both System1/ROM requirements intact.

2026-10-17: SPEC-0042 / ADR-0048 finalize the prepared guest GetPort/SetPort selection contract. Same native semantic core planned for68000/x86; exact A5 anchor/current-cell ownership and total-commit rules specified. No implementation, tests, registry binding, build or target run in this slice. Existing drawing candidates unchanged/unbooted; full InitGraf and selected-port drawing adapter remain prerequisites. [Lead review](guest-port-lead-review.md).
