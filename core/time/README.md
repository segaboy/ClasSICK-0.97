# Portable time

[SPEC-0005](../../docs/specifications/SPEC-0005-clock.md) defines canonical
seconds/nanoseconds, checked arithmetic, monotonic observations and deterministic
advancement. Constant-work uint32_t core; no host calls, floating point or 64-bit
helpers. Caller owns clock domains/epochs. No calendar, scheduler or TickCount claim.
