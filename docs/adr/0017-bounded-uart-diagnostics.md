# ADR 0017: Poll bounded UART diagnostics beside keyboard progress

Accepted scoped design 2026-10-08. SRC-0053/0054; SPEC-0013 / IMPL-0017 /
TEST-0019. Human provenance review pending.

Use the manufacturer register interface with caller-owned byte callbacks and
fixed storage. Readback/FIFO checks detect unsupported responses while explicit
machine qualification establishes mapping/clock/ownership. One startup operation
or one LSR plus at most one THR operation per turn avoids serial waits inside
keyboard service. A best-effort observer reuses the existing scene successor;
serial failure stays visible in RAM without turning a keyboard result into a
serial success. Final drain has independent finite bounds and requires TEMT.

Atomic record enqueue makes lost records countable; retained queue bytes and
versioned trace fit existing storage at offset 256. Original fixed ASCII records
need no runtime formatter. Interrupt logging, RX console, UART auto-discovery and
serial exception output would require new contracts. Native qualification and
firmware/media/edition gates remain open; no universal device claim is made.
