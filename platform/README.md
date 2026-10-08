# Platform adapters

Adapters implement generic services with real hardware, firmware, or host calls.
Windows development adapters are verified through B1. `uefi/contract.c` now
validates bounded preboot data and an exit-outcome model under SPEC-0006; it makes
no firmware call or physical-memory access. Original native entry and scoped
framebuffer/PM timer/keyboard/UART drivers have hosted/unloaded evidence; actual
native execution remains future work. Capabilities and ownership are explicit.
See [HAL contracts](../docs/architecture/hal.md).
