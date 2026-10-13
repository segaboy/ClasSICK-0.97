# Architecture decision records

An ADR records why a decision exists, its alternatives, consequences, evidence,
and conditions for review. Use four-digit sequential IDs; never reuse an ID or
erase a superseded decision. A changed decision gets a new ADR linking both ways.

Statuses: **proposed**, **accepted**, **selected for bootstrap**, **superseded**,
or **pending owner decision**. Accepted architecture remains revisable with
evidence; foundational project constraints cannot be silently relaxed.

| ID | Decision | Status |
| --- | --- | --- |
| [ADR-0001](0001-native-portable-layers.md) | Native portable layers, System 1 personality, optional guest execution | Accepted |
| [ADR-0002](0002-address-and-format-boundaries.md) | Separate guest/native addresses and explicit external representations | Accepted |
| [ADR-0003](0003-language.md) | Conservative freestanding C11 with minimal architecture assembly | Selected for bootstrap |
| [ADR-0004](0004-windows-toolchain.md) | Portable LLVM-MinGW, CMake/Ninja/CTest, ordinary PowerShell setup | Selected for bootstrap |
| [ADR-0005](0005-clean-room-provenance.md) | Specification/evidence trace and clean-room publication gates | Accepted |
| [ADR-0006](0006-compatibility-profiles.md) | Historical behavior and opt-in extensions use separate contracts | Accepted |
| [ADR-0007](0007-boot-and-target-order.md) | Hosted contracts first, measurable native boot, 128K feasibility gate | Accepted |
| [ADR-0008](0008-license-deferred.md) | Owner-approved GPL-3.0-or-later for project code/docs/assets | Accepted |
| [ADR-0009](0009-core-link-and-second-compiler.md) | Current-core complete no-runtime links and scoped GCC conformance | Accepted for development verification |
| [ADR-0010](0010-uefi-preboot-contracts.md) | Original preboot data/exit model; proposed native-PC ownership/device profile | Accepted contract; firmware/profile needs review |
| [ADR-0011](0011-original-uefi-loader.md) | Original x64 loader calls and owned stop scaffold; inspected EFI packaging | Accepted for development verification |
| [ADR-0012](0012-owned-x64-exceptions.md) | Owned x64 descriptor bytes, emergency stacks and terminal first-fault capture | Accepted for development verification |
| [ADR-0013](0013-native-framebuffer-presentation.md) | Gated native framebuffer presenter, bounded band staging and original boot scene | Accepted for development verification |
| [ADR-0014](0014-acpi-pm-timer.md) | ACPI PM timer via pre-exit RSDP capture and map-checked table walk | Accepted for development verification |
| [ADR-0015](0015-cooperative-progress-loop.md) | Single cooperative timed progress loop before keyboard/UART | Accepted for development verification |
| [ADR-0016](0016-polling-ps2-keyboard.md) | Bounded untranslated set-2 polling keyboard and interactive successor loop | Accepted for development verification |
| [ADR-0017](0017-bounded-uart-diagnostics.md) | Bounded polling UART and retained native keyboard diagnostics | Accepted for development verification |
| [ADR-0018](0018-fixed-profile-boot-media.md) | Original fixed-profile GPT/FAT32 boot media | Accepted for development verification |
| [ADR-0019](0019-isolated-black-box-vm-trial.md) | Isolated own-image VM configuration and retained cold-start observations | Accepted scoped trial; B2 qualification partial |
| [ADR-0021](0021-mac128k-original-rom.md) | Original 68000 ROM, own helpers and isolated Snow 128K trial | Accepted scoped development/trial; hardware and OS gates open |
| [ADR-0022](0022-native-fault-observation.md) | Original terminal record exports and separate UD/GP native fixtures | Accepted scoped development/trial; B2 firmware and OS gates unchanged |
| [ADR-0023](0023-bounded-shared-service-directory.md) | Bounded portable address services before the full historical/CPU gateway | Accepted engineering profile; historical/edition gates unchanged |
| [ADR-0024](0024-original-aline-execution.md) | Bounded original program and native A-line return bridge | Accepted scoped profile; complete CPU/historical gateway open |
| [ADR-0025](0025-checked-guest-memory.md) | Checked mapped data and bounded MOVE/branch/subroutine instructions before allocation policy | Accepted engineering profile; native integration and allocation services separate |
| [ADR-0026](0026-m68k-first-application-runtime.md) | Shared application memory, one routine implementation and CODE-only selection under the owner's 68k-first gate | Accepted 68000 foundations; native applications gated |

Use [the template](template.md). Every substantive decision cites founding
requirements or approved sources, distinguishes design from historical fact,
and names how portability/compatibility will be tested.
