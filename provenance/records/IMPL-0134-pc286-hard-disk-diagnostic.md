# IMPL-0134 - Original 286 fixed-disk diagnostic

2026-11-14; SRC-0610 / SPEC-0107. Original GPL-3.0-or-later project-owned test-media
generator derived from our own floppy diagnostic and BootHush's explicitly
modeled geometry. No Apple or Microsoft code, firmware internals, external
bootloader or protected disk image was used. It generates a disposable whole
disk and is not an installer or production boot contract. Source review,
physical BIOS behavior and release qualification remain open.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).

2026-11-15: an optional independently implemented bitwise CRC-16 was added
under SPEC-0107. It is error detection for the generated diagnostic, not an
authenticity, physical BIOS or installer claim.
An optional model-geometry check now refuses an unexpected result before the
payload read. It is derived from BootHush's documented model, not a physical
drive contract.
The optional diagnostic retry profile makes three modeled attempts and
resets between failures; its DI-preservation dependency and physical limits
are recorded in SPEC-0107 and the linked evidence.
