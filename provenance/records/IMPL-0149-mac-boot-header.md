<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
<!-- Copyright (C) 2026 Dean Howell. -->
# IMPL-0149 - Data-only Macintosh startup header

2026-11-18; SPEC-0119 / SRC-0617 / TEST-0210.
Paths: core/storage/mac-boot-header.[ch]. Original exact-area decoder and atomic
two-block acquisition with copied counted names, raw numeric fields and complete
failed-output preservation. Entry/code bytes are never decoded into output.
No startup dispatch, allocator, guest address, retained pointer or source import.
Codex authored from the reviewed field contract and existing block-service API.
[Canonical receipt](../../docs/development/mac-boot-header-20261118.md) records
actual verification, retained earlier failure, costs and scope. GPL-3.0-or-later;
coordinating integration and independent human provenance review remain separate.
