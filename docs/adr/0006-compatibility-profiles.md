# ADR-0006 — Historical compatibility and extensions

- Date: 2026-10-07
- Status: accepted
- Sources: SRC-0001; independent profile design

## Context

Modern screens, storage, RAM and hardware are desirable, but must not redefine
the historical observable API contract. Later manuals may describe later behavior.

## Decision

Define an exact `system1-1984` baseline only when source/version evidence permits.
Keep `classick-extended` opt-in and specify each deviation. Core capabilities are
generic; historical restrictions are applied only where required at the personality
boundary. Compatibility reports are behavior/profile/target-specific.

## Alternatives

Global 128K/display/floppy limitations constrain native portability. Silent modern
extensions invalidate historical test oracles. A generic "Mac compatible" label
hides version differences and unsupported operations.

## Consequences and verification

Profile-specific fixtures/errors and guest capabilities need explicit tests.
Independent assets may change appearance; report those deviations. Modern
documentation can guide candidate interfaces but does not establish 1984 semantics.

## Review conditions

Freeze a reference profile from eligible evidence, then refine tests. New extensions
and future personalities need separate scope decisions rather than incidental code.
