# TEST-0175 - Poisoned 286 startup memory

2026-11-14; IMPL-0126. BootHush checks all 21,994 bytes of a poisoned BSS
interval, a deliberately broken clear, loader bounds and media failures. The
shared-window fixture and seven Mac/286 replay comparisons also pass after
poisoning and clearing its linked BSS. Physical boot and stack budget remain
open. [Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
