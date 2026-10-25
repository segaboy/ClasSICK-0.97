# ADR-0057 - Paired preview package freeze for approval

Accepted2026-10-19,SRC-0301..0304. Preparation is authorized; publication is not.

Freeze unchanged native68000 and native x86-64 candidates as separate ROM0.3.0-alpha.1 and PC0.1.0-alpha.1 experimental prereleases with matching scene capability. Use platform-specific offline instructions,complete corresponding source at the compiled revision,GPL text,inner/outer checksums and deterministic ZIPs. One Claude primary artifact and distinct-risk Copilot independent auditor; Codex alone integrates/verifies.

Keep compiled-source,qualification-evidence and eventual tag/metadata revisions distinct. Version strings live in filenames/notes,never patched into already qualified image bytes. Admission records and published assets are immutable; a separate post-approval receipt records actual publication. Current published pointers remain unchanged.

PC synthetic input success cannot override prior human misses or preboot GUI failure. Explicit owner acceptance is needed to publish a limited experimental preview. All other historical,physical,128K,B2,native-app,Linux and both System1/ROM gates remain. No new runtime code,build,target run,tag or release. [Proposal](../../releases/paired-native-preview-1-approval.md).


2026-10-25,SRC-0454/0455:future config schema2 requires an explicit validated ASCII docs/...json evidence_path;SOURCE.txt pins it to evidence_revision. Schema1 outputs remain byte-identical;review.json stays schema1 and adds path only for schema2. Auditor validates/reconstructs independently. No evidence existence/qualification or new release readiness inferred. [Review](../development/paired-evidence-locator.md).
