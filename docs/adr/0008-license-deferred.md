# ADR-0008 — Owner-approved GPL-3.0-or-later licensing

- Date: 2026-10-07
- Status: accepted after explicit owner approval on 2026-10-07
- Sources: SRC-0001, SRC-0008 through SRC-0012, SRC-0015

## Context

The owner prohibited arbitrary license selection. The comparison was prepared
before any license was adopted. The owner then explicitly chose GPL-3.0-or-later
for ClasSICK 0.97's own code, documentation and assets.

## Decision

Adopt the standard GNU GPL version 3 text with an explicit version-3-or-later
grant for project-owned material. Use matching inbound-equals-outbound contribution
terms; require accurate authorship and sufficient rights. Do not require copyright
assignment or a separate CLA. Third-party tools retain their own terms, and any
distributed runtime/dependency needs separate license/provenance review.

## Alternatives

BSD/MIT are minimal permissive options; Apache adds explicit patent terms; MPL
uses file-level reciprocity; GPL requires stronger reciprocal distribution terms.
The owner chose stronger reciprocal distribution terms after reviewing scope.

## Consequences and verification

The project can now be described as GPL-3.0-or-later open-source work. The license
does not supply Apple permissions or cure prohibited derivation. Record standard
SPDX notices for our C/script sources and scope in COPYRIGHT.md. Review dependencies
before incorporating or distributing them; compilation tooling alone is separate.

## Review conditions

Any license change requires owner approval and review of contributor rights,
existing grants, scope and third-party obligations. The filename remains stable
for earlier links; this pending decision became accepted after the owner's choice.
