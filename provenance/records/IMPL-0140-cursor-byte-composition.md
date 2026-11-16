# IMPL-0140 - Mono cursor byte-row composition

2026-11-16; SPEC-0052 byte-row amendment / ADR-0076 / TEST-0198.
Codex authored and self-reviewed original architecture-neutral C in
`core/graphics/cursor.c`: explicit row assembly, clipped byte composition,
and once-per-rectangle restore masks. No new historical claim, external
implementation source, runtime dependency or copied asset. Existing
validation/ABI and RGBA semantics remain. GPL-3.0-or-later; independent
human provenance and physical review remain open. See the
[canonical receipt](../../docs/development/cursor-byte-composition-20261116.md)
for source/build identities, measured tradeoffs and the refreshed pair.
