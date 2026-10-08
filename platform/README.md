# Platform adapters

Adapters implement generic services with real hardware, firmware, or host calls.
Windows development adapters are verified through B1. `uefi/contract.c` now
validates bounded preboot data and an exit-outcome model under SPEC-0006; it makes
no firmware call or physical-memory access. Native entry and drivers remain future
work. Capabilities and ownership are explicit.
See [HAL contracts](../docs/architecture/hal.md).
