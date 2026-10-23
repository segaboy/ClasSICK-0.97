# ADR-0080 - Checked banked guest-storage model

2026-10-23; SRC-0399..0401 / SPEC-0069, original engineering.

Use canonical logical banks and checked finite capabilities as the candidate
C16 storage boundary. One logical access fits one region but may cross banks;
never substitute a raw far pointer for a128KiB region. Preflight the whole map
before effects, retain access rights, and advance nonwrapping generations before
owner changes. Invalid new maps leave the old lifetime untouched. Bank identity
does not prove physical disjointness; native commit and far access remain
unqualified. Do not impose callbacks or new indirection on native Mac/x64 paths.

Adopt the corrected original host model as reference/test scaffolding only;
production guest APIs/enums and retained-view callers stay unchanged. P286-2 is
partial. Paired-native-preview-2 remains primary with its Mac/PC trial and source
gates; this bounded lane does not imply native286 execution or release readiness.
