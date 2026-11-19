<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
# SRC-0622 - Published original ROM responsibility boundaries

Reviewed 2026-11-19. Scoped use of published Apple Inside Macintosh I-III (1985),
already admitted as SRC-0071, plus IV (1986) introduction xii to distinguish the
64K-ROM family from its successor. Sources, edition/page locators, findings,
uncertainties and applicability appear in the
[canonical audit](../../docs/development/original-rom-parity-20261119.md).

I-10 supplies the family-level ROM/RAM map; I-106 the System resource/patch
division; I-344 and I-483 resource-backed definitions/packages; II-383 the
patch/direct-call interface; III-42 the hardware startup sequence. These are
published-contract facts, not tests of our implementation or authentication of
the owner's disk versions. Later RAM-software behavior is not promoted to the
January1984 corpus. Exact ROM-revision applicability and routine-level coverage
remain open.

Public primary-document passages were retrieved through web search/open; the
large separate PDF downloads exceeded retrieval limits. The published combined
edition supplied the cited passage text. No full-document hash was obtained in
this audit. Earlier source hashes retain their own exact input identities.
Only narrow paraphrased facts are used; no manual implementation sample, Apple
ROM/System body, disassembly, copied asset or confidential draft is a source.
This supports requirements/architecture only, not new runtime code.
