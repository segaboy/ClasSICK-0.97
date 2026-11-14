# IMPL-0126 - Diagnostic 286 BSS initialization

2026-11-14; IMPL-0123 / IMPL-0125. Original AI-assisted Codex diagnostic
loader can clear a bounded uninitialized-data interval after BIOS reads;
test-only poisoning checks the assumption explicitly. GPL-3.0-or-later;
human code and provenance review remain open. Not a production loader.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
