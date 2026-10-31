# TEST-0122 - 286 capability and borrow qualification

2026-10-30; SRC-0574 / SPEC-0087 / IMPL-0098. Three host suites pass in four Windows compiler/sanitizer profiles; a separate literal oracle detects three specifically introduced scratch faults. Four pinned 80286/USE16 unlinked objects have no unresolved imports; two near-data profiles fail at the required guard. [Evidence](../../docs/development/pc286-span-cap-evidence.md). No target execution, hardware or release qualification.
