<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0221 – Disk-to-RAM 68000 program on Mac and installed 286

Optimized and sanitized host checks exercise successful replacement and complete
prior-output preservation after late or malformed reads. The authored fixture
returns each of two disk-selected 32-bit values on both targets. Bad signature,
bad name and a second-sector fault each leave all destination bytes untouched.
A final cold start returns the original value. The 286 test covers installation,
whole-disk comparison, removal of installer media, ROM-equivalent native floppy
service, banked instruction execution and the full installed-payload byte check.

Run `scripts/Test-BootBlockImage.py --help` for required pinned native tooling
and external output arguments. No owner disk is an input to this test.
See [paired boot-block execution](../../docs/development/boot-block-image-20261119.md) for the exact result and limits.
