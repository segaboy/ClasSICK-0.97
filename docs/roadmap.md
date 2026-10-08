# Milestone roadmap

Status: dated 2026-10-07; dependency-driven gates, not calendar promises.
Milestone numbers retain the founding brief's labels; they are not a strict
implementation sequence. Early MFS/resource research feeds the complete M0.4 gate.

| Milestone | Deliverable | Evidence required to close |
| --- | --- | --- |
| M0.0 Project bootstrap | Public repo, policies, provenance, architecture, ADRs, toolchain/setup, build probes | Public commits, guard pass, ordinary Windows setup pass, two fresh native probe builds/tests; owner's license decision documented |
| M0.1 Portable hosted core | Native Windows core and abstract graphics/input/memory | B1 gate; headless surface/arena/event contracts; no hardware imports in core; debug workflow exercised |
| M0.2 Native x86-64 boot | Native UEFI loader/kernel with owned framebuffer/timing/keyboard | B2 gate; image/link provenance; repeated post-ExitBootServices progress/input; named VM/PC |
| M0.3 Macintosh 128K boot | Native 68000 core and independently created ROM/startup | Published hardware/ISA specs, measured RAM/ROM budget, cold start/input, no Apple ROM execution |
| M0.4 System 1 environment | Graphics/windows/menus/events/resources/files and useful simple desktop | Profile-specific manager tests and original scene/workflow evidence; required M0.5 fork/MFS foundation |
| M0.5 MFS compatibility | Generic-device MFS and data/resource forks | Synthetic format/error/fuzz tests plus authorized safe interoperability evidence; mount read-only before writes |
| M0.6 68000 application runtime | Optional contained interpreter and application-to-Toolbox boundary | Synthetic ISA/trap/loader tests, then one named legally supplied original application's safe behavior report |
| M0.7 Expanded compatibility | Named January-1984-era application corpus and error coverage | Published matrix with supported operations, failures, profiles, source revision; cross-target parity |
| M1.0 Portable System 1-compatible OS | Native x86-64 and 68000 boots, ARM64 native core execution, measured compatibility | Charter release gates, frozen corpus/coverage criteria, audited provenance, selected license, reproducible release artifacts |

The owner approved **GPL-3.0-or-later** during bootstrap for project-owned code,
documentation and assets. Contributions/releases still require clean provenance
and third-party runtime/dependency license review. Tool licenses do not become
the project license. Record any future license change through owner approval/ADR.

## Immediate dependency order after bootstrap

1. Completed the headless [SPEC-0001 bounded surfaces](specifications/SPEC-0001-surfaces.md)
   with caller-supplied storage and original tests; see TEST-0004. M0.1 remains open.
2. [SPEC-0002 Windows presentation](specifications/SPEC-0002-windows-presentation.md)
   and original scenes are verified under TEST-0005. [SPEC-0003 bounded arenas](specifications/SPEC-0003-arenas.md)
   and viewer-buffer integration now pass TEST-0006. [SPEC-0004 keyboard input](specifications/SPEC-0004-input.md)
   and shared synthetic/Windows viewer input pass TEST-0007. [SPEC-0005 clocks](specifications/SPEC-0005-clock.md)
   and hosted timed behavior pass TEST-0008. The x64 debug workflow now passes
   TEST-0009. Next audit remaining B1/M0.1 hosted-start acceptance.
3. Prove freestanding linkage/runtime ownership and a second compiler/core build.
4. Prepare/review UEFI VM tooling and device contracts; the owner named installed
   VirtualBox (observed 7.2.16r174877) for native-PC tests. No VM/startup/firmware
   validation is claimed. Implement the B2 dependency
   chain in [the boot graph](architecture/boot.md).
5. In parallel only when explicitly staffed, pursue historical-source eligibility,
   Macintosh hardware budget, resource-fork/MFS specs, and trap ABI contracts.
   Those investigations must not import protected implementation material.
   The owner named mini vMac for independent Mac firmware/OS image validation;
   review its startup/configuration before testing and retain a physical Mac gate.
6. Build compatibility managers from reviewed versioned specifications. Stage
   application translation after memory/trap/loader boundaries are measurable.

## Scope controls

Do not start with pixel-for-pixel Finder work, introduce later System versions, or
make a JIT prerequisite for useful behavior. Every milestone publishes achieved,
partial, unverified, and blocked-by-research items separately. Do not replace a
native boot with a hosted screenshot or replace 128K support with an unrecorded
larger-memory requirement.

See [status](status.md), [research backlog](research/questions.md),
[testing methodology](testing/methodology.md), and [specialist roles](governance.md).
