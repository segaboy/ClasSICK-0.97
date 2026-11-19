<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# TEST-0203 — Modeled native Mac floppy reads

2026-11-18; SPEC-0114/IMPL-0144. Eight original-fixture scenarios pass in
C:/Repos/.task-temp/mac-floppy-provider-20261118-e/report.json.
Native reads, zone/seek/tag/canary checks, late checksum-failure preservation,
bounded absent-media handling, mapping/checksum mutations and fresh recovery.
Source/spec/helper identities pinned before and verified after all builds/runs.
[Canonical receipt](../../docs/development/mac-floppy-provider-20261118.md).
No selected disk, System boot or physical-hardware result in this suite.

## Startup checksum recovery, 2026-11-19

`scripts/Test-StartupChecksumRetry.py` compares a pinned single-attempt ROM
and the current startup-files ROM on original generated media. Six BootHush
runs cover healthy, one-shot and persistent checksums with direct guest
diagnostics from the native Macintosh and complete startup report checks.
[Qualification receipt](../../docs/development/startup-checksum-retry-20261119.md)
