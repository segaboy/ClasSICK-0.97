<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0152 - Startup file resource-source preparation

2026-11-18; SPEC-0122 composes existing SPEC-0119/0115/0116. Original
`core/storage/startup-files.[ch]` selects the two exact boot-header names,
validates both resource maps and publishes stable rebased source controls only
after complete success. Explicit live/exclusive workspace and session lifetime
rules accompany whole-destination failure preservation.

Original host/native probes, fixture generator and harness are qualified in
TEST-0213. Codex storage_review authored the files; mfs_volume/mfs_tests performed
the independent read-only lifetime/atomicity/near16 review. GPL-3.0-or-later,
copyright2026 Dean Howell. No external implementation or selected owner medium
was provided to the worker. Existing source format contracts are reused; no new
historical dispatch, INIT policy, executable admission or startup ABI is inferred.

The [canonical receipt](../../docs/development/startup-files-20261118.md) owns
exact identities, implementation decisions, evidence and limits. Product native
entrypoints remain unchanged; this is not a System/Finder boot claim.
