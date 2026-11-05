# IMPL-0111 - Experimental short Mac collector

2026-11-03; original project implementation of SPEC-0096, GPL-3.0-or-later. This opt-in experiment preserves strict loss handling and remains unqualified for physical machines. [Evidence and limits](../../docs/development/mac-fast-timing-20261103.md).

2026-11-05: unused quiet-axis phase changes no longer cancel READY samples; moving-axis, button, DCD and pending checks remain. [Correction and tests](../../docs/development/release-input-20261105.md).

2026-11-05 follow-up: SPEC-0096 explicitly supersedes strict cancellation for eligible motion-only ambiguity in SHORT only. Adds clean late-edge retry, generation-gated diagnostics and separately counted motion omission. Default strict bytes remain unchanged. Original code proposed by supervised Claude and integrated/corrected by Codex; original project inputs only, GPL-3.0-or-later. [Tests, native builds, incomplete live trial and limits](../../docs/development/release-input-20261105.md#late-edge-race-diagnostic-masking-and-motion-omission). No physical, human or release qualification.
