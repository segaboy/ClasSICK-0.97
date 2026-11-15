# IMPL-0137 - Window cursor art validation

2026-11-15. Original shared-scene code computes the existing per-byte
validity mask directly rather than iterating through its bits. The public
scene contract and refusal behavior are unchanged. Windows host and native
Mac/286 modeled results are recorded in the [canonical evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#cursor-art-validation-optimization).
