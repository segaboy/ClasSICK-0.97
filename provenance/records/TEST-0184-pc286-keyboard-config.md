# TEST-0184 - Bounded keyboard configuration checks

2026-11-14; SPEC-0104 / IMPL-0131. Hosted checks cover all 256 reply bytes,
timeouts, pending output, status faults and invalid calls. A BootHush 286
diagnostic checks normal translation and controlled refusal of disabled
translation and pending output, including IRQ mask restoration. Emulator
observations do not qualify physical controllers, concurrent input or release.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
