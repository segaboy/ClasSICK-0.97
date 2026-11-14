# TEST-0180 - Natural initial Mac/286 cursor position

2026-11-14; IMPL-0123 / IMPL-0127. An opt-in unpositioned 286 diagnostic and
the earlier Mac ROM both show the shared `(256,171)` pointer and matching
initial scene in BootHush. Their sources are not a frozen release pair; no
physical, human or release qualification is established. A subsequent bounded
continuous injected-input sequence matches eight checkpoints, with slower
spacing than an earlier failed checkpoint.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
