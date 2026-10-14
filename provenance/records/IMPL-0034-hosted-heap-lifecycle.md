# IMPL-0034 - Original hosted application-heap lifecycle

SPEC-0030 v2 / ADR-0034 / TEST-0038 / SRC-0151..0154; 2026-10-14.
Codex-assisted implementation lead, with bounded read-only Claude/Grok review.
Lead self-review; human provenance review pending. GPL-3.0-or-later,
project-owned code/tests; no third-party implementation, runtime or asset.

Original `core/memory/lifecycle.c/.h` starts from the reviewed project contracts
and existing own manager/map/session/service interfaces. Caller-owned bounded
plans, typed inventory, checked exact placement, disjoint scrap staging and
revalidation precede application retirement. System records/cells/payloads stay;
application scrap is recreated through a predetermined no-callback write recipe.
Epoch saturation, single-use plans, retained first faults, truthful publication
tags and coordinated entry refusal have explicit original semantics.

[Implementation/API review](../../docs/development/heap-lifecycle-implementation.md)
and [execution evidence](../../docs/development/heap-lifecycle-evidence.md) state
preconditions, bounded review dispositions, original tests and storage limits.
No new manual/sample/external implementation/ROM/binary/reference input;
no private heap reconstruction. Research originals and worker streams remain
outside indexed source. Claude writes no code; Grok edits no repository file.

No guest reset/launch/Scrap binding, loader, QuickDraw, MFS, system scrub,
callback closure, native application, boot integration, emulator/VM/reference
trial or UART implementation/test-body change. Full128K, physical, historical,
edition/parity/B2 criterion6 and human review gates remain open.
