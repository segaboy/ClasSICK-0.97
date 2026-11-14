# IMPL-0124 - Narrow-host surface validation arithmetic

2026-11-14; SPEC-0001 / IMPL-0123. AI-assisted Codex change uses uint32_t for
the existing validated surface arithmetic when size_t and ptrdiff_t fit; wider
hosts retain uintmax_t. Original GPL-3.0-or-later code; human review remains
open. Two experimental 286 runtime modules remain unreviewed for distribution.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
