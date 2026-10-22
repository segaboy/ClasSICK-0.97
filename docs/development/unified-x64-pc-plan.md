# Unified x86-64 PC distribution goal

Accepted owner goal2026-10-21;SRC-0341/0342;ADR-0067.
Status:roadmap only. Current PC release is UEFI-only;BIOS support and universal
physical-PC compatibility have not been implemented or demonstrated.

## Owner requirement and engineering interpretation

> I think we should do that. What I want is a single 64-bit build that will work on any 64-bit PC.

Target one PC release/build pipeline and one canonical native x86-64 OS payload.
Bundle firmware entry loaders for legacy BIOS and UEFI so users do not select
different OS builds for those firmware families. Both loaders must hand off to
the exact same compiled payload,with its identity verified. Startup stubs may
use the execution modes required by their firmware;the running OS stays 64-bit.
Aim for one primary bootable image. Any necessary media-format wrappers must
come from the same release and retain that identical payload.

"Any 64-bit PC" is the owner's coverage goal. Supported-machine claims will
identify tested firmware/CPU/device profiles and known gaps. Boot-path coverage
alone does not establish usable display,input or storage on every machine.
Firmware permission to boot the OS,CPU/firmware bitness combinations and device
interfaces need explicit assessment. Do not silently assume x86-64 CPU implies
one firmware ABI,or promise support for untested/locked/proprietary configurations.

## Architecture and work order

| Area | Planned work | Current position |
| --- | --- | --- |
| Shared payload | One compiled native 64-bit OS;firmware-independent handoff for memory,display,CPU/stack and device ownership | Current PC entry/handoff is UEFI-specific;shared BIOS/UEFI payload is future work |
| Firmware entries/media | Reviewed BIOS and UEFI loaders,original media layout,validated loading and transition to common payload | UEFI-only original images exist;no BIOS loader/media layout admitted here |
| Hardware coverage | Explicit CPU baseline/detection,display,timing,input and storage adapters;safe unsupported-profile reports | Current qualified profiles have framebuffer,ACPI timing and PS/2 requirements |
| Evidence | Same payload hash and normalized behavior from each entry,then separately scoped machine/VM qualification | Existing source-pinned UEFI evidence is retained;no new boot/hardware result |

First inventory the existing UEFI dependencies and freeze the common-payload
handoff/ownership contract. Review eligible published BIOS/media/CPU interfaces
before implementing startup. Choose payload format,ABI,CPU minimum and media
layout through that contract;this goal chooses none of their low-level details.
Then integrate the entries and extend hardware drivers in bounded releases.
USB input,additional storage/display paths and timing alternatives are coverage
work,not features supplied automatically by a BIOS loader. Audit firmware-bit-
ness combinations before choosing any additional bootstrap entries.

## Relationship to the current project

The paired-native-preview-2 Mac/PC interactive-window milestone remains the
immediate checkpoint. BIOS/unified-media work is a longer-term PC target and
is not newly required for its provisional ROM 0.4.0-alpha.1/PC 0.2.0-alpha.1 pair.
Continue shared native68000/x86 feature parity and PC legacy68k application
compatibility;the external native-application gate stays closed.
All PC paths keep the original monochrome512x342 logical canvas,unscaled and
centered in supported larger framebuffers. Preserve released assets/tags and
old evidence;do not relabel existing UEFI observations as BIOS coverage.

This intake changes goals/records only: no code,fixtures,feature tests,BIOS
builds,device/source recipes,target trials or release publication. Future implementation
starts from reviewed contracts/source prerequisites. Fresh exact-image trial
and publication authority,clean-room/role/UART and all compatibility gates
continue unchanged. No new worker call is needed for this small decision.

[Current parity](native-parity-plan.md),[display target](display-target.md),
[current release goal](../../releases/paired-native-preview-2.md),
[machine-readable scope](unified-x64-pc-plan.json).
