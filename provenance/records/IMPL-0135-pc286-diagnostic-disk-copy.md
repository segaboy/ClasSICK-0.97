# IMPL-0135 - Original disposable-disk copy test

2026-11-14; SRC-0610 / SPEC-0108. Original GPL-3.0-or-later project-owned
test code emits a 286 BIOS-call guest and generates fresh disposable media.
It uses the model's documented interface, own loaders and own payload only;
no third-party installer, Apple or Microsoft implementation, firmware source
or protected image was used. This is not a production installer or disk format.
Human provenance/code review and physical qualification remain open.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).

The independently written opt-in write retry preserves the attempt counter,
reissues the modeled request after reset and stops after three failures.
Its contract and limits remain SPEC-0108; it is still a disposable test guest.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#disposable-copy-write-retry).
