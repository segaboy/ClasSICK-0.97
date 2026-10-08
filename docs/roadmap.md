# Milestone roadmap

Current component checkpoint: SPEC-0014 original GPT/FAT32 boot media packages
the audited loader reproducibly and passes independent checks; it has not been
read by firmware. Next are exact firmware eligibility and machine qualification,
then observed B2 cold starts. Macintosh replacement startup and
mini vMac validation remain later independent milestones.

Status: dated 2026-10-08; dependency-driven gates, not calendar promises.
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
   with caller-supplied storage and original tests; see TEST-0004.
2. [SPEC-0002 Windows presentation](specifications/SPEC-0002-windows-presentation.md)
   and original scenes are verified under TEST-0005. [SPEC-0003 bounded arenas](specifications/SPEC-0003-arenas.md)
   and viewer-buffer integration now pass TEST-0006. [SPEC-0004 keyboard input](specifications/SPEC-0004-input.md)
   and shared synthetic/Windows viewer input pass TEST-0007. [SPEC-0005 clocks](specifications/SPEC-0005-clock.md)
   and hosted timed behavior pass TEST-0008. The x64 debug workflow now passes
   TEST-0009. TEST-0010 and the owner-operated keyboard session now complete
   [B1/M0.1 technical hosted-start acceptance](development/hosted-start-audit.md).
3. Completed the scoped [current-core link and second-compiler checkpoint](development/core-link-evidence.md):
   Clang/LLD and GCC/GNU ld complete no-runtime links at two widths/optimizations,
   matching twins and failure controls. This establishes the current profile's
   closure; future helper dependencies and actual native execution need review.
4. [SPEC-0006 preboot data/exit-model checks](specifications/SPEC-0006-uefi-handoff.md)
   pass with original hosted fixtures. The proposed VirtualBox
   [PC ownership/device profile](development/native-pc-profile.md) now has an
   original [SPEC-0007 loader/stop scaffold](development/uefi-loader-evidence.md)
   with hosted calls and unloaded EFI inspection. The actual x64 producer is
   identified; selected firmware provenance remains unresolved before launch.
   [Owned exception tables/scaffold](development/x64-exception-evidence.md) now
   pass hosted byte tests and unloaded install/vector/fault-path audits.
   Formatted boot media, actual exception delivery, reviewed device leaves and real
   handoff/stack execution remain required. B2 is unverified; follow
   [the boot graph](architecture/boot.md).
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
