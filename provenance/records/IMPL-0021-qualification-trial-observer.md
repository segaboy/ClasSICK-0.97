# IMPL-0021 — Original qualification-trial observation harness

2026-10-11. SRC-0065 owner trial approval; SPEC-0015 configuration and
SPEC-0016 own-output contract; TEST-0025. OpenAI Codex authored this original
Windows host orchestration from the reviewed project contracts and reused
SRC-0061 public Oracle management interfaces (getregisters: manual 15.16.2.11).
GPL-3.0-or-later; human provenance review pending.

`scripts/Observe-QualificationTrial.ps1` requires the owner's explicit approval,
SPEC-0016 preparation and matching configuration review. It admits only the
powered-off project identity with Secure Boot off, refuses concurrent project
runs, uses fresh serial/record paths and launches the declared headless/GUI mode.
It records complete CRLF-framed own v2 lines, recognized/unknown status and actual
monotonic/UTC line-arrival brackets. It sends no keyboard input. It queries only
RIP/RSP after own READY and captures the own scene after frame output, intermediate
progress and termination. Nonzero observation commands are recorded as failures;
management failure stops the collector. It never reads guest/firmware memory,
changes native code or deletes/powers off any VM. Readback/interpretation and
operator confirmation are separate TEST-0025 evidence steps.

No third-party implementation or firmware body was reviewed or used. This is
host-side output aggregation, not new UART algorithm/specification/fixture work.
The earlier source-exposure and fresh-implementer role boundary is preserved.
Local machine identities, logs and own PNGs remain outside the public repository.
No formal B2, historical compatibility or OS-edition certification follows.
