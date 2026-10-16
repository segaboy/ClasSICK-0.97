# Current milestones and release tracks

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
