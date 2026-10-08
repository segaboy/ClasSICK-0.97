# Normalized keyboard queue

Original freestanding C11 FIFO over caller-owned typed records. See
[SPEC-0004](../../docs/specifications/SPEC-0004-input.md) for lifetime,
non-aliasing, error precedence, source identity and reset-epoch rules.
The first contract supports Space/Escape press/release/repeat only, with bounded
sequence numbering. No timestamps, OS imports, allocation or historical Event
Manager compatibility. Windows mapping belongs in the platform layer.
