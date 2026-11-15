# TEST-0192 - Paired engineering snapshot controls

2026-11-15; IMPL-0136. A baseline snapshot passed full output checksum
verification. Changed ROM, Mac source fingerprint, 286 disk identity and
paired-run source identity were refused before output creation; an existing
output directory and sentinel were preserved. These checks do not establish
authenticity, provenance review, target qualification or release readiness.
[Evidence](../../docs/development/mac-pc286-shared-parity-20261114.md).

The follow-up added checksummed, guest-only initial and content frames and
refused a modified frame. All seven controls passed in
`C:/Repos/.task-temp/classick-snapshot-controls-20261115-c/report.json`.
The baseline also ran both documented bounded readiness commands: Mac reached
its condition at cycle 60,936,878; 286 reached POST `b0` at cycle 4,208,555.
Checksums still matched after both runs. These are modeled readiness checks,
not evidence of physical hardware, human operation or release qualification.
An independently prepared snapshot from committed tooling is at
`C:/Repos/.task-temp/classick-paired-engineering-snapshot-20261115-c`; its
manifest records the source commit and checksums of its contents.
After the inbox refinement, all seven controls, including both modeled
readiness runs, passed again with the new Mac ROM and matching paired report
in `C:/Repos/.task-temp/classick-inbox-snapshot-controls-20261115-a`.
