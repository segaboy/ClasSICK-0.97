# IMPL-0071 - Original banked-storage host model

SPEC-0069 / ADR-0080; SRC-0399..0401. Copilot supplies the original Python model
and contract plus one bounded corrective patch. Codex integrates the artifacts,
clarifies typed-model/native-backend boundaries, simplifies one test expression
and adds independent flat/oracle/control checks. Production guest.c/guest.h and
Mac/x64 boot paths remain unchanged. No native segmented backend or helper ABI.
[Evidence](../../docs/development/segmented-storage-evidence.md).
