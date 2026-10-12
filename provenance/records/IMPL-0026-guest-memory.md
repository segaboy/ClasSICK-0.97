# IMPL-0026 - Original checked guest memory execution

2026-10-12; SPEC-0022 / ADR-0025 / SRC-0096-0098 / TEST-0030.
Codex-assisted original design and implementation; lead self-review, human
provenance review pending. Permitted inputs are the published Motorola prose/
encoding tables and existing original project contracts. No manual sample code,
Apple executable/source/assets, third-party CPU/emulator implementation, or
uncertain discovery material is used. No new runtime dependency. UART bodies
remain outside this role. Claude's two concurrent research drafts are untouched.

Original caller-owned region descriptors, checked big-endian accesses, temporary
CPU value frame and deferred writes. Original 36-byte data/control fixture and
independent condition masks/boundary tests. SPEC-0021 interface remains unchanged.
Native execution and historical allocation/OS compatibility are separate claims.
