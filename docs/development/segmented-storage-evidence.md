# Checked segmented storage -2026-10-23

SPEC-0069 / ADR-0080 / IMPL-0071 / TEST-0084; SRC-0399..0401.
Partial P286-2: original logical bank/capability contract plus an executable host
model. A128KiB region spans multiple banks while retaining24-bit big-endian,
alignment, permission and validate-before-effect rules. Capabilities retain
rights and finite generation/lifetime; invalid map creation has no epoch effect.
Actual native far access, physical alias ownership and infallible commit or
staging are explicit prerequisites. Mac/x64 production paths are unchanged.

Two targeted CTests pass: model assertions and independent checks.38 frozen
checks were prepared before reading the worker;1,596 flat read/write cases and
three deliberately faulty controls were added afterward. The initial model
failed its self-test and9/38 independent checks. One bounded correction fixes
those defects; lead simplifies a test expression and tightens contract limits.
The private frozen JSON is preserved; its public copy differs only by Git LF.
The first public hash check used the private CRLF hash and was corrected.

Grok timed out at240.78s without an artifact; no same-provider retry. Copilot
Sonnet5.5 low returns404 lines (300 cap missed), then21 edits/91 replacement
lines at57.97s after its123.75s primary. Primary made an extra input-directory
listing; no additional file content or writes. Correction used only its prompt.
No worker executed tests. Reported premium-request cost2/nanoAIU31,133,040,000
are task counters, not remaining balance, paid entitlement or Codex savings.
All-four remaining allowances are unverified. No renewed tool qualification.

Native CI pending; earlier native/WCC evidence is reused, not newly counted.
No target trial, production backend, loader, release or new screenshot.
Paired Mac/x64 release remains primary. Mac binding review is held under
POLICY81 after excluded assembly-style constants in a cached published summary;
no function bodies, worker transfer or affected implementation. Owner disposition
and the exact PC GUI-trial go-ahead are pending. Other work is unaffected.
[Contract](../specifications/SPEC-0069-checked-segmented-storage.md),
[machine receipt](segmented-storage-evidence.json).
