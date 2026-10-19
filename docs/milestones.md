# Current milestones and release tracks

2026-10-19: owner directs every substantive slice toward paired nativeMac68000/x86 PC preview releases. Current target [paired-native-preview-1](../releases/paired-native-preview-1.md),ADR-0055: finish outlines in both native scenes,then package/qualify/freeze the pair. Compiler/hosted/target/human/package/publication states are distinct; no launch/publication permission expands.

2026-10-19: SPEC-0048 separate original native68000/x86 outline helper implemented; independent pixel/state/canary tests and compiler/CI checks recorded. No existing scene/ROM/EFI input or trap change,target run or release. H1-H5 and existing gates remain open. [Evidence](development/native-outline-evidence.md).

2026-10-18: reviewed native Mac/PC boot screenshots now live in Git for research/web discovery: [image index](evidence/screenshots/README.md). SPEC-0048 defines the next paired native rectangle-outline contract; implementation and44 proposed checks remain pending.

2026-10-18: exact SPEC-0047 native68000/x86 candidates booted once each; two-port drawing and synthetic counters observed. Six static PC comparisons and normalized Mac/PC same-counter comparison pass. Sessions closed; no release. Mac progress/PC timing, Snow preference isolation and human/physical/remaining compatibility gates retain explicit limits. [Evidence](development/paired-selection-boot-trial.md).

2026-10-18: SPEC-0047 adds the shared native two-port visual workload and explicit Mac/PC candidates. Final hosted/sanitized/compiler/media checks pass; paired CI/native68000 ROM and native PC disk verification pass. Existing modes and input behavior preserved; no target run/release. [Evidence](development/paired-selection-evidence.md).

2026-10-18: SPEC-0046 implements explicit selected-port PaintRect/EraseRect through the shared native68000/x86 owner/view. Hosted/sanitized tests and CI/native68000 compile/link checks pass. Old modes/UI scenes unchanged,no target trial/release. Next is the original paired workload. [Evidence](development/selected-drawing-evidence.md).

2026-10-17: SPEC-0045 integrates actual prepared GetPort/SetPort owner transactions and original loaded-guest selection conformance. Hosted/sanitized checks and CI/native68000 compile/link verification pass (2026-10-18). Selected-port drawing/full manager remains next,not enabled here. No target trial/release. [Evidence](development/port-owner-evidence.md).

2026-10-17: SPEC-0044 / ADR-0050 finalize the prospective prepared-port ownership contract: exact-g integrity, explicit owner environment spans, one admission transaction and consumed-generation-safe retirement. All40 cases remain unexecuted. No code/binding/build/trial/release; existing native inputs unchanged. [Review](development/prepared-port-owner-lead-review.md).

2026-10-17: SPEC-0043 implements pure shared native68000/x86 drawing-port selection plans; no guest memory access or trap binding. 810 local CTests,10 width/endian object audits and8 broken-core controls pass; CI passes3013 CTests across11 profiles and six68000 objects (no target execution). Existing scenes/workloads/ROM/EFI inputs unchanged. [Checkpoint](development/native-selection-evidence.md).

2026-10-17: SPEC-0042 / ADR-0048 finalize the prepared guest GetPort/SetPort selection contract. Same native semantic core planned for68000/x86; exact A5 anchor/current-cell ownership and total-commit rules specified. No implementation, tests, registry binding, build or target run in this slice. Existing drawing candidates unchanged/unbooted; full InitGraf and selected-port drawing adapter remain prerequisites. [Lead review](development/guest-port-lead-review.md).

2026-10-17: SPEC-0041 adds a borrowed native drawing view with coordinate translation and rectangular clipping. The same68000/x86 workload exercises both while preserving every scene pixel and the independent old68k fixture.804 local tests,70 object audits and unloaded EFI controls pass; full CI and native68000 cross-build pass. New candidates are unbooted; no target trial or release.

[Drawing context checkpoint](development/drawing-context-evidence.md).

2026-10-17: One approved isolated SPEC-0040 native PC boot passes READY, direct Space plus two mouse edges/held-repeat/release, exact static-scene pixels and normal guest-second60/count3. Desktop UI delivery and human input remain unqualified. Session closed; no Mac trial, code change or release.

[Native PC mouse observation](development/pc-mouse-boot-trial.md).

2026-10-16: SPEC-0040 implements single-owner8042 keyboard/mouse startup and routing, plus an explicit native x64 PC mouse build profile. Shared button semantics and512x342 scene match the existing Mac/Windows foundation. Hosted/compiler/EFI candidate evidence only; no emulator/VM/hardware/reference trial or release. Default legacy keyboard contract/trace, UART source/test bodies, Mac ROM inputs,24 originals/copies and frozen wiki remain preserved.

[PC mouse checkpoint](development/pc-mouse-evidence.md).

2026-10-16: SPEC-0039 adds shared native button edges, a pure PS/2 report decoder and Windows/native-Mac scene bindings. Freestanding PC8042 startup/routing, pointer motion and full event parity remain open. No new target trial or release. OneNote and its Git wiki queue are permanently retired; existing wiki files stay frozen history.

[Native mouse evidence](development/mouse-input-evidence.md).

2026-10-16: The original native C drawing workload now calls the rectangle service directly on native68000 and x86 paths. PC boot/Windows viewer and a distinct native Mac ROM use real native results/pixels; the separate40-byte68k compatibility fixture remains. Independent pixel/failure/legacy tests, nine-profile2280 checks, dependency audits and native candidates pass. No new boot observation or release; PC mouse/event parity remains open.

[Native drawing evidence](development/native-drawing-evidence.md).

2026-10-16 owner clarification: each native Mac advancement needs matching
native x86 PC behavior; legacy68k compatibility stays a separate required lane.
Current renderer is native on both, but original native drawing-workload and
PC mouse/event parity remain open. [Paired plan](development/native-parity-plan.md).

2026-10-16: ROM0.2.0-alpha.1 is now published as an experimental prerelease.
[Receipt](../releases/rom/0.2.0-alpha.1-publication.json); System remains unreleased.

2026-10-16: the current branded drawing ROM and native-PC candidate were
captured from one isolated boot each for the owner's blog. APP/097, hollow
rectangle and six phases are visible; PC512x342 is centered in1024x768. No input
qualification. [Current boot observation](development/shared-product-scene-boot-evidence.md).

2026-10-16: shared monochrome branding and512x342 canvas now compile for the drawing ROM, native-PC and hosted viewer, with exact hosted pixel parity. [Evidence](development/shared-product-scene-evidence.md); new-candidate target boots pending.

As of 2026-10-16. This is the short current view; the [roadmap](roadmap.md)
retains founding M0.0-M1.0 criteria and dated history. Component milestones below
describe capabilities, not release numbers or complete-edition certification.

## Release state

| Component | Version | State |
| --- | --- | --- |
| ClasSICK ROM | 0.2.0-alpha.1 | Published experimental prerelease; rom-v0.2.0-alpha.1 |
| ClasSICK System software | Unassigned | Unreleased |

[Version authority, release process and compatibility matrix](../releases/README.md).
The current ROM's embedded demo is separate from a disk-based System release.

## Visible capability milestones

| Track | Capability | Current state / evidence |
| --- | --- | --- |
| ROM | Original Macintosh 128K reset, native scene and kernel input | Demonstrated in Snow; [earlier qualification](development/mac-rom-trial.md). Physical and full OS gates open |
| ROM/runtime | Load, execute and retire an original 68k application | Demonstrated in Snow; [integrated application](development/loaded-rom-app-evidence.md). Narrow prototype profile |
| ROM/presentation | Preserve the scene while updating the input counter | Demonstrated at captured points; [redraw evidence](development/loaded-app-redraw-evidence.md). Automated input delivery unqualified |
| ROM/runtime | Loaded application makes its first drawing calls through our APIs | [Selected two-call profile](development/drawing-app-evidence.md) passes hosted/compiler/CI checks; [one Snow boot](development/drawing-app-boot-trial.md) shows drawing retained as counter advances; input delivery unqualified |
| ROM/runtime | General guest port state, clipping and drawing-manager installation | Next expansion; the first rectangle profile has fixed native port state |
| ROM/System | Boot our replacement System disk on our ROM | Required; not tested; no System release exists |
| ROM | Boot owner-designated original System 1 disks | Required; not tested |
| System | Windows, menus, events, files and useful desktop workflow | Incomplete; research and isolated foundations exist |
| System | Boot our System on an identified supported original Apple ROM | Required; not tested |
| PC/portable core | Hosted core and original native-PC boot prototype | Observed; [formal B2 remains partial](development/b2-qualification-audit.md). No complete OS edition |
| Full project | Physical Macintosh/128K, cross-target parity and complete editions | Open; prototype/alpha releases do not certify these |

Future observations require exact artifact/platform identities and the owner's
fresh go-ahead. Original reference media/firmware remain separately admitted
black-box inputs. Native PC counterparts follow the paired plan; detailed ABI/loading still needs
bounded contracts. Historical R-001,
source/UART roles, human provenance and [both boot directions](development/system1-rom-boot-requirements.md)
remain in force. Milestone completion never follows from a version bump alone.
