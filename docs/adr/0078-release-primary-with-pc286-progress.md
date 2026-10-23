# ADR-0078 - Release primary with bounded286 progress

2026-10-23;SRC-0390..0393. Owner returns paired Mac/x64 release work to the
main track while requiring progress on the286. Adopt the bounded
[two-lane plan](../development/release-and-pc286-plan.md). No new quota target,
release version/hardware gate or automatic fan-out to every provider.

Release software:SPEC-0066 is the original foreground coordinator joining the
reviewed inbox/helper with short abstract exclusion and unmasked owner calls.
Hardware primitives and boot binding remain separate. Native performance and
shared semantics persist. Side work:SPEC-0067 admits a C16-only language/math/
helper policy,not an implementation,ABI selection or global C11 downgrade.
First helper fault/epoch/no-partial-commit/ownership behavior remains mandatory.
All existing source,native-app,trial,edition,physical/128K,B2 and release gates
persist. No OneNote/wiki,extra chat/subagent or automation.
