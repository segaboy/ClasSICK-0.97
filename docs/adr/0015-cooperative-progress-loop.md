# ADR 0015: First cooperative native loop is a single timed progress pass

- Accepted scoped original design, 2026-10-08; owner SRC-0047 continuation.
- SPEC-0011 / IMPL-0015 / TEST-0017. Builds on ADR-0013/0014.

## Context

B2 requires at least 60 seconds of visible progress from an owned time source,
then a keyboard-driven change. Keyboard, UART and interrupts are not specified yet.

## Decision

Implement one polling loop that redraws the original scene's progress segments
from PM timer seconds, with explicit stall and late-sample diagnostics. Keep it
synchronous and bounded, and make later device polling (keyboard, UART) additional
steps in the same loop rather than introducing interrupts or a scheduler now.

## Alternatives

Per-second segment cycling shows liveness but hides the overall duration; a
filling bar communicates the 60-second target. Interrupt-driven timing needs
IOAPIC/LAPIC contracts that do not exist yet. A frame-count delay would not be
an owned time source.

## Consequences

The first native run will show a static scene that fills its bar over 60 seconds,
then halts. Keyboard handling must extend this loop before any B2 attempt.
