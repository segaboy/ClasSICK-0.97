# Provenance ledger

Every major compatibility subsystem must trace behavior through:

**eligible source -> independently written specification -> test -> original implementation**.

Use stable IDs: `SRC-####`, `SPEC-####`, `EXP-####`, `TEST-####`, `IMPL-####`, and
`ADR-####`. IDs are never reassigned. Put the source catalog in `sources.md`,
subsystem records in `records/`, and templates in `templates/`. Source access is
not blanket approval for every section or every version of Macintosh behavior.

Classify assertions as **documented fact**, **reference observation**, **design
decision**, or **unverified assumption**. Specifications carry a profile and
evidence strength. Source records state eligibility, version applicability, and
limitations. Observation records state exact reference version, probe revision,
input, output, repeat count, timing conditions, and safe-export policy. Do not
publish private device identifiers or protected screenshots/binaries.

Implementation records identify paths, specification IDs, tests, original design,
author/reviewer, review date, AI assistance and permitted input boundaries,
third-party dependencies/assets, license state, and unresolved questions.

An original generic core algorithm may use an independently authored design
contract rather than a historical Macintosh source. Do not manufacture reference
claims for such code. Synthetic fixtures must identify their independent generator
and purpose; they do not establish reference compatibility.

Tool dependencies have separate source/version/hash records. Toolchain notices do
not constitute the project license. Any runtime/library actually distributed with
a future product needs an explicit third-party license and provenance inventory.

No external implementation, copied assets, or uncertain-origin runtime enters this
repository until reviewed. Project-owned contributions use the owner's approved
GPL-3.0-or-later terms; third-party license compatibility needs separate review.
See [policy](../docs/clean-room/POLICY.md).
