# Milestone roadmap

2026-10-14: SPEC-0029 / ADR-0033 / IMPL-0033 / TEST-0037 implements the
bounded typed Toolbox stack dispatcher, staged writes, checked normal/procedure
auto-pop and root-only hosted ExitToShell. [Evidence](development/toolbox-evidence.md)
records initial1614 plus72 final affected checks, ten object audits/twins, complete
matching fourteen-audit/two-link68000 manifests and the30-byte/eight-step probe.
CI passes corrected-source1614 plus384 bootstrap checks; six EFI/two own ROM
profiles and seventeen untracked research originals are preserved. Explicit
register/continuation refusals, Pascal patches/direct veneers/kept pointers/manager
callbacks remain deferred. No loader, heap reset, manager/boot/native application
or new VM/emulator/reference run. All source/role/native/edition gates remain;
human provenance review pending. Next propose heap lifecycle/reset before launch.
Earlier entries below retain their dated scope.

2026-10-12 current: SPEC-0023 bounded pointer/handle allocation, size, resize,
lock and copy services pass [hosted/compiler verification](development/memory-manager-evidence.md):
1,245 local checks and both CI jobs, with an exactly matching 68000 manifest.
Thirteen guest interface contracts are registered. The 32-object/fixed-zone
engineering profile and register adapter still need typed guest trap binding
and explicit data-address normalization; no new boot entry or native trial.
The [68000 runtime plan](development/m68k-runtime-plan.md), SRC-0099 / ADR-0026,
records the completed registry/Memory Manager subset and orders JSR/JMP/nested guest calls,
typed dispatch and CODE-only launch selection. X86 application/native test/loader
work waits until the owner explicitly confirms the complete 68k experience;
own native OS/core/tools are unaffected. Future emulator/VM runs need his
go-ahead. Prior entries below remain dated history; no full-edition gate closes.

2026-10-12: the original 128K ROM demonstrator has passed two Snow timed cold
starts, own image/TRAP diagnostics and owner mouse input. See
[the trial](development/mac-rom-trial.md). This advances the Macintosh startup
lane; full M0.3, physical hardware, budgets and System 1 services remain open.
The next runtime lane is the independent trap/service gateway; SPEC-0017 remains
a later-manual draft requiring scoped finalization, not historical conformance.
The earlier checkpoint below is retained history; formal PC B2 is unchanged.

Current checkpoint: TEST-0025 observes the SPEC-0016 candidate in three no-input
cold starts and one live keyboard run. All pass the measured controller/self-test
and bit-20 precondition and complete guest second 60. Own-stack placement is now
observed. [Qualification evidence](development/qualification-vm-evidence.md).
Formal B2 remains partial on unchanged criterion 6's firmware image hash.
Claude's next lane is the System 1 source foundation and proposed M0.3 feasibility
documents; those do not follow automatically from this VM result. Macintosh replacement
startup, 128K feasibility and real Mac/mini vMac remain independent milestones.

Status: dated 2026-10-11; dependency-driven gates, not calendar promises.
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
   identified; selected firmware provenance remains uncertified. SRC-0059 later
   cleared only black-box test use, and SRC-0060 authorized SPEC-0015's trial.
   [Owned exception tables/scaffold](development/x64-exception-evidence.md) now
   pass hosted byte tests and unloaded install/vector/fault-path audits.
   Boot media and observed native scene/timing/input are now recorded. Actual
   exception delivery and remaining machine-state/identity qualification remain
   open; SPEC-0016's measurements await a VM trial. Formal B2 is partial; follow
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
