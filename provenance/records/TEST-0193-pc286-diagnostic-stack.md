# TEST-0193 - 286 diagnostic stack watermark

2026-11-15; IMPL-0134. An opt-in instrumented loader and the existing
window payload produced matching guest states and frames at eight modeled
checkpoints. The deepest changed canary byte was 1,070 bytes below the
initial stack pointer. A separate 256-byte known-depth guest and a floor
mutation exercised the measurement, and the 47-case default-loader test
passed. This is a lower bound for the observed model runs, not a maximum,
physical-memory measurement or full release budget. The test, exact input
identities, commands and limitations are in the
[canonical evidence](../../docs/development/mac-pc286-shared-parity-20261114.md#diagnostic-stack-watermark).
