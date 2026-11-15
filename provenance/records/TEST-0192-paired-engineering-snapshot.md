# TEST-0192 - Paired engineering snapshot controls

2026-11-15; IMPL-0136. A baseline snapshot passed full output checksum
verification. Changed ROM, Mac source fingerprint, 286 disk identity and
paired-run source identity were refused before output creation; an existing
output directory and sentinel were preserved. These checks do not establish
authenticity, provenance review, target qualification or release readiness.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).
