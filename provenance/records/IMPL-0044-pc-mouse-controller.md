# IMPL-0044 - PC mouse-controller routing and native profile

2026-10-16; SRC-0233..0237 / SPEC-0040 / ADR-0046 / TEST-0048.
Claude supplies original bus code and exact existing-keyboard/native binding
patches. Copilot supplies independent adversarial tests without implementation.
Codex integrates/corrects and owns full-startup/native fixture, compiler wiring,
source decisions and actual verification. Default legacy keyboard behavior,
UART bodies, existing Mac/Windows core/decoder and all protected records remain.
[Actual results](../../docs/development/pc-mouse-evidence.md).
