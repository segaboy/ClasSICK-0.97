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

Use [the template](template.md). Every substantive decision cites founding
requirements or approved sources, distinguishes design from historical fact,
and names how portability/compatibility will be tested.
