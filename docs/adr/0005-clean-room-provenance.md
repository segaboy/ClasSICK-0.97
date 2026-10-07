# ADR-0005 — Specification and provenance publication gate

- Date: 2026-10-07
- Status: accepted
- Sources: SRC-0001; binding clean-room policy

## Context

The public repository must never contain excluded payloads, even temporarily.
Behavioral compatibility cannot justify reconstructed Apple implementations.

## Decision

Establish policy before implementation. Trace eligible source to independent
specification, test and original implementation. Distinguish documented facts,
observations, design choices and assumptions. Keep reference material outside the
checkout/CI and record AI inputs/exposure. Use index/history review, source guards
and maintainer provenance review before publication/merge.

## Alternatives

Unattributed implementation and retrospective provenance are not auditable.
Public availability is not permission or clean-room eligibility. Automatic binary
and secret checks cannot replace human provenance/rights review.

## Consequences and verification

Research takes longer but yields reusable behavioral evidence. One contributor
can maintain conceptual separation; do not claim independently staffed legal
clean-room certification. In uncertain-origin incidents stop affected derivation
and use the documented contamination response without republishing the material.

## Review conditions

Improve automation and independent review with staffing. The exclusion of Apple
executable/ROM and prohibited derived code is non-negotiable.
