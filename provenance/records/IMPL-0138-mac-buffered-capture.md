# IMPL-0138 - Buffered Mac input acquisition

2026-11-16; SRC-0608 / SPEC-0091 / SPEC-0109. Original packed observation,
bounded interrupt FIFO and serialized foreground decoder, with explicit loss
and fresh-baseline recovery. Full-register veneers retained. See the
[canonical receipt](../../docs/development/paired-preview-readiness-20261116.md).
Self-reviewed implementation; no historical/physical or release claim.

2026-11-16 human-test follow-up: optional SPEC-0109 burst profile uses a
512-record application-workspace FIFO and 128-record foreground decode batch.
Original project code and the owner's own-candidate motion trace are the inputs;
no external guest implementation is used. AI-assisted implementation and
author self-review; independent review remains pending. See the
[guided human receipt](../../docs/development/guided-human-qualification-20261116.md).

## Sensitivity follow-up, 2026-11-16

Explicit power-of-two burst capacities 512/1024/2048 retain the existing
default, 128-record batch and overflow/rebaseline algorithm. The selected
2048-entry workspace candidate follows failed 512/1024 faster-input controls.
Implementation, memory bounds and identities are in the
[guided human receipt](../../docs/development/guided-human-qualification-20261116.md).
