# ADR-0008 — License selection reserved to the owner

- Date: 2026-10-07
- Status: pending owner decision
- Sources: SRC-0001, SRC-0008 through SRC-0012, SRC-0015

## Context

The owner expressly prohibited arbitrary license selection. The repository is
public, but no general open-source grant follows from visibility alone.

## Decision

Commit a comparison and explicit pending status, not a license. Defer external
implementation/asset merging until inbound/outbound terms are approved. The
owner-authorized founding documentation and tooling probes can be published.

## Alternatives

BSD/MIT are minimal permissive options; Apache adds explicit patent terms; MPL
uses file-level reciprocity; GPL requires stronger reciprocal distribution terms.
The owner chooses after reviewing the implications and license scope.

## Consequences and verification

Do not label the project open source while undecided. No license file or standard
SPDX license notice may be adopted without approval. Tool notices are separate;
future distributed helper/runtime dependencies need their own audit.

## Review conditions

After explicit owner approval record exact license/version and scope, add standard
text/accurate notices, update policy/README/ADR, and establish inbound terms.
