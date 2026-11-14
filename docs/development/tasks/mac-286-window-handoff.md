<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# Mac/286 paired preview: scoped implementation handoff

This is a proposed implementation handoff, not permission to bypass the project's clean-room rules or evidence gates. The current owner direction is [Mac/286 paired direction](../mac-286-paired-direction-20261114.md), SRC-0608. An implementer must meet the project exposure rules and receive only permitted inputs. Do not assume a fresh process or prompt alone provides isolation; assess the actual access controls and document the chosen role and exposure boundary before implementation. Do not read emulator implementation, game-derived source, Apple/Microsoft implementation, ROM/System media, excluded drafts, held UART bodies, untracked research originals, HQ contents or protected assets. Only the documented BootHush interface and original guest images may be used for authorized headless black-box tests.

## Target

Implement the existing interactive window goal on an original native 286 executable for the documented PC/AT 5170-class, 640 KiB, EGA configuration and preserve the paired Mac path. The PC logical canvas is 512x342, unscaled and centered at (64,4) in 640x350. Provide original cursor movement, title-strip drag/release and content toggling through a specified 286 input path. A keyboard-operated pointer is the existing candidate; define its mapping and release semantics against admissible interfaces before relying on it. Do not treat the separate BootHush geometric diagnostic as a ClasSICK implementation or as physical evidence.

## Start from reviewed project material

- [286 feasibility and staged work](../pc286-feasibility.md), especially the actual profile, C16 and segmentation constraints.
- [286 capability execution](../pc286-capability-boot-20261113.md), [current 286 observations](../boothush-paired-20261114.md), and the existing original `probes/pc286/` contracts and tests. These are diagnostics, not a production backend or system boot.
- [Shared native window contract](../../specifications/SPEC-0053-native-window-interaction.md), [window composition](../../specifications/SPEC-0054-native-window-composition.md) and the applicable current original scene/input specifications and provenance. Verify current revisions before use.
- Existing released Mac candidate identity and [qualification checklist](../release2-qualification-checklist.md), while accounting for the owner's target replacement. Do not silently reclassify old x64 results as 286 evidence.

Determine the smallest reviewed 286 boot/display/input vertical slice that uses the existing original shared semantics. Establish a source-eligible BIOS/EGA and input contract before new bindings; implement with the pinned 16-bit toolchain and independently authored fixtures. Prove map/segment and stack bounds, legal CPU instruction profile, helper/runtime closure and media identity. Use BootHush for bounded headless tests, including guest-observed input and full logical-pixel comparisons. Record all unsupported cases and any emulator gaps, then iterate to the actual candidate. Preserve existing worktrees, outputs, historical records and media.

## Release gates

The paired milestone remains incomplete until both exact candidates meet the applicable E1–E3 state/pixel/input criteria, the prior failed human observations are resolved or separately dispositioned, applicable physical/polarity and review gates are addressed, and source, licenses, setup instructions, checksums and package reviews match the final images. Report E4 separately. The owner must separately authorize publication. Neither diagnostic boot, synthetic input nor matching screenshots alone prove full hardware or operating-system compatibility.
