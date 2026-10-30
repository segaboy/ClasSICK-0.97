# TEST-0115 - 286 checked-access qualification

2026-10-30; SRC-0566 / SPEC-0086 / IMPL-0096. Four host profiles pass the three author suites; three introduced defects are detected. The pinned 16-bit compiler generates four unlinked 80286/USE16 objects, refuses two near-data models at the guard, and rejects seven conflicting options before output. Previous compiler profiles pass with matching listings apart from generated module paths. [Evidence and limits](../../docs/development/pc286-span-access-evidence.md). No target execution, hardware or release qualification.
