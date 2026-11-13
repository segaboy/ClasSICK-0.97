# TEST-0164 - 286 capability boot diagnostic

2026-11-13; SRC-0574 / SPEC-0087 / IMPL-0117. Two independent pinned C16
builds and BootHush runs matched. The baseline cross-bank capability case
reached `A7`; three fixture wrong-expectation controls reached `D5`, `D8` and
`DD`. Full loaded payload and four banks matched in each stop, and media was
unchanged. Near-data compilation refused the four-byte pointer assertion.
The two matching final runs included new-owner invalidation and exhaustion. These are
bounded emulator/model results, not a full OS, physical hardware or release
qualification. [Detailed evidence](../../docs/development/pc286-capability-boot-20261113.md).
