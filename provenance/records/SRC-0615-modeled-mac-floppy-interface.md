<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SRC-0615 — Original BootHush model interface for Mac reads

2026-11-18; scoped source admission under the owner Nov14 independent-implementation
amendment. SPEC-0114 restates current BootHush wire/device relationships as a model
contract. Source baseline5d613614dd4f17d605de75ecc3f38c9129079488:
docs/mac-floppy.md and the original tests/test_mac_floppy.py fixture.
The latter SHA-256 is109f00d96ef63575738d3255dbeeda0dc9362c1b756cb9811e074ada49f7007b.

The necessary symbol values, field relationships and externally tested model
behavior are interface facts. The new parser/capture/provider are independently
authored; the fixture decoder is not translated/copied. No MAME or other third-party
implementation body was supplied to the implementer or imported into ClasSICK.

BootHush documents a MAME-adapted BSD format component. That remains an emulator
dependency, not automatic admission of its source into this product. These tests
show compatibility with this exact model, not independently verified physical
Sony/IWM behavior, historical firmware algorithms or all disk formats.

The lead's source search surfaced a SWIM manual with a confidential marking.
It was excluded, not used as a source, sent to the implementer, or incorporated
into SPEC-0114. No facts or code from that document justify this implementation.
The own-model scope continues under the current amendment; historical hardware
source review remains separate.

The canonical implementation/test receipt is
[Mac floppy provider](../../docs/development/mac-floppy-provider-20261118.md).
Original project contributions are GPL-3.0-or-later. Human provenance review,
physical qualification and broader historical compatibility remain open.
