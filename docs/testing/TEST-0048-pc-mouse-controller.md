# TEST-0048 - Controller sequence, isolation and native integration

SPEC-0040 / ADR-0046 / IMPL-0044; SRC-0233..0237;2026-10-16.
Authored full keyboard/AUX startup checks per-turn I/O bounds and unchanged
output-port machine bits. Independent tests isolate AUX phases using explicitly
primed qualified keyboard state: literal I/O trace, wrong-source replies/floods,
elapsed/poll bounds, retry D4 pairs/caps,64/65-byte drain, OBF/IBF, sticky failures,
clock regression, prefix interleaving, payload decoding, recovery and saturation.
Lead controls reject forged READY/ACTIVE states before I/O. This priming is not
credited as complete keyboard startup; the separate full fixture covers that.

Native x64 hosted integration uses the real selected profile, original ACPI/
port fixtures and shared scene: early gates, failed AUX startup before drawing,
two mouse presses/held repeats/releases, one Space and Escape, v2 trace112-byte
guards, row/outer guards and independent bounds around the centered counter cell.
Legacy keyboard/UART and previous shared-button/decoder tests remain independent.
Compiler/import/width/endian checks include expected unsupported native-ABI
rejections. Own EFI twins/corruptions/omissions remain unloaded; no real ports,
human input or target observations are credited. [Results](../development/pc-mouse-evidence.md).
