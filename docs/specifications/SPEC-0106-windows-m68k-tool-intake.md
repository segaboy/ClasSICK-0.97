<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# SPEC-0106 - Experimental Windows 68000 toolchain intake

2026-11-14. The owner directs the project away from WSL. This contract permits
experiments using an explicitly selected Windows-hosted m68k ELF compiler and
associated GNU-style tools; it does not admit any particular distribution for
release or replace toolchain provenance and license review.

For an explicit bin directory and m68k prefix, the intake requires GCC, nm,
objcopy and objdump Windows PE executables, reports their host machine types
and SHA-256 hashes, and requires the compiler to identify an m68k ELF target.
The window-ROM builder inspects these executables again after its normal twin
build, and records their identities in the build manifest. The native option
does not substitute a Linux sysroot. Existing link, image, stack and output
checks remain required. Querying a target and hashing four frontends do not
establish archive provenance, executable trust, all subordinate program/header
identities, compiler correctness, runtime licensing or release qualification.

The selected experimental toolchain must additionally compile, link and pass
the project's own source-matched image audits and bounded BootHush tests.
Distribution, runtime modules and notices need independent review. See the
[canonical receipt](../development/windows-m68k-toolchain-20261114.md).
