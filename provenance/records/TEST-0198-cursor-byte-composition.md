# TEST-0198 - Cursor byte-row equivalence and native qualification

2026-11-16; IMPL-0140 / SPEC-0052. Codex independently expressed the added
9792 width/hotspot/alignment/clipping scenarios using the existing per-pixel
oracle. O0/O2 and O2 ASan/UBSan pass; nine focused host checks pass. Native
Mac twins, 286 runtime closure, 16 media cases, two eight-state paired
comparisons, 14 boundary checkpoints, overflow recovery, four recorded-input
replays and 72 latency cases pass in BootHush. All 36 candidate timing
comparisons improve; remaining axis/phase variation is not a human pass.
See the [canonical receipt](../../docs/development/cursor-byte-composition-20261116.md)
for exact conditions, identities, captures and limitations.
