<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0160 – Publish a complete Macintosh boot image to native RAM

`core/storage/mac-boot-image.[ch]` composes the SPEC-0119 acquisition and
metadata validation with a final 1024-byte publication for SPEC-0130. Native
failure preserves the complete prior image. Independent Mac and installed 286
probes exercise the result using their native floppy providers and the 286
banked guest interpreter. Loader, probes, harness and tests are original
GPL-3.0-or-later contributions. No external implementation or historical
calling convention was used as an implementation input. The public fixture
has its own callable entry convention.

See [paired boot-block execution](../../docs/development/boot-block-image-20261119.md) for TEST-0221 and the qualification scope.
