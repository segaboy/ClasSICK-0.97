# ADR-0067 - One native x86-64 PC payload across firmware families

Accepted2026-10-21;SRC-0341/0342.

Adopt one PC release/build pipeline with an identical native x86-64 OS payload
loaded through BIOS or UEFI entries. Firmware-specific startup stubs and media
wrappers are part of that distribution. Hardware detection/adapters and tested
support profiles advance the owner's broad "any64-bit PC" goal;that phrase is
not an existing universal-compatibility result.

Keep paired-native-preview-2 as the immediate Mac/PC checkpoint. The common
handoff,legacy interfaces,payload format,ABI,CPU baseline,media layout and
hardware coverage need later reviewed contracts and qualification. This decision
does not implement a loader or change current runtime/trial/release scope.
Use existing original portable C/native x86 work and retain legacy68k application
compatibility. Display target,source boundaries and all other gates persist.

[Roadmap](../development/unified-x64-pc-plan.md).
