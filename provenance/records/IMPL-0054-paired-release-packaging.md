# IMPL-0054 - Deterministic paired release package tools

2026-10-19;ADR-0057 / TEST-0060;SRC-0301..0304.

Original Python stdlib-only scripts/Prepare-PairedRelease.py and scripts/Test-PairedRelease.py integrate separate Claude builder and independent Copilot audit artifacts with recorded lead corrections. No third-party dependency or historical implementation input. Project GPL-3.0-or-later; author Dean Howell,AI-assisted; Codex lead review.

Config admits exact original payload/source/license hashes. Builder checks inputs/docs before creating a fresh external directory,refuses existing/overlapping/aliased paths,emits deterministic component ZIPs/checksums and an external pending marker. Auditor reconstructs expected bytes from trusted config/inputs/docs rather than believing package metadata. It has no publication or execution path. Runtime/ABI/build inputs are unchanged. [Evidence](../../docs/development/paired-release-package-review.md).


2026-10-25,SRC-0454/0455:strict configurable schema2 evidence locator,legacy full-tree identity,115 local checks/35 overlapping frozen cases/two faulty controls and resealed locator-content refusals. Synthetic fixtures only,no release inputs/package/trial;worker fixture failures/lead corrections disclosed. [Receipt](../../docs/development/paired-evidence-locator.json).
