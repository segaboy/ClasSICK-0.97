# TEST-0188 - Windows-built Mac and 286 model comparison

2026-11-14; SPEC-0072 / SPEC-0096 / SPEC-0106 / IMPL-0133 / SRC-0609.
Two independent Windows-native builds each produced matching ROM twins. The
current Mac ROM and 286 diagnostic matched eight scripted state and pixel
checkpoints at two modeled mouse edge spacings, with replay; a faster spacing
failed. Four invalid manifest variants were rejected. Neither emulator input
nor build reproducibility qualifies physical, human, production or release
gates. [Evidence](../../docs/development/windows-m68k-toolchain-20261114.md).
