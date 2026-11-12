# IMPL-0115 - Original BootHush Mac mouse timing probe

2026-11-12; SRC-0601 / SPEC-0096 / TEST-0162. Lead-authored
`scripts/Probe-BootHushMacMouse.py` calls the user-selected BootHush executable
as a black box with an identity-checked original ROM and original synthetic
motion. It checks the candidate's diagnostic tags and reports controlled
timing cases without editing the ROM or emulator. GPL-3.0-or-later; no emulator
implementation source, historical code, reference media or third-party code
was used. [Results and limits](../../docs/development/boothush-mac-mouse-20261112.md).
