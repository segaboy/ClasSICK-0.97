# ADR-0024 - Original A-line instruction boundary

Accepted 2026-10-12, SRC-0092-0094 / SPEC-0021.

Use a checked portable instruction/value boundary for the two adopted services.
An original twelve-byte fixture is executed by a small PC interpreter and the
actual 68000 exception bridge. Share bytes and service semantics, not CPU entry
machinery. The interpreter supports MOVEQ/NOP/RTS/Axxx and immediate supervisor
service completion only. Symbolic tokens and arbitrary patches retain explicit
limits. User/trace/interrupt/callback/general CPU and historical OS work remain.
This makes instruction advancement/register/stack behavior independently testable
without importing an emulator, driver or Apple implementation.
