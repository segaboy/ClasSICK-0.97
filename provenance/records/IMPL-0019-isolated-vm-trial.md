# IMPL-0019 — Original isolated-VM preparation and observation

2026-10-08. SPEC-0015 / ADR-0019 / TEST-0022; owner SRC-0060 and public
management interface SRC-0061/reused SRC-0035. OpenAI Codex (GPT-6) resumed
leadership from the owner's Claude handoff. Human provenance review pending.
GPL-3.0-or-later; no legally certified clean-room staffing claim.

Original `scripts/Prepare-VMTrial.ps1` validates approved raw/payload identities
and complete packaging evidence before creating a fresh trial root. It records
exact CLI argv/exits/output, uses newly generated machine/medium UUIDs only,
requires a new project name, checks raw-to-VDI-to-raw identity, and configures
the declared isolated machine through public interfaces. The existing inventory
must differ by only the one new project identity. It never starts a VM or deletes
anything; retained configuration must be reviewed before a separate start.

Only public manual/CLI contracts and project-owned metadata were implementation
inputs. No external tool/firmware/device body or protected payload was consulted.
No native/core/driver/fixture change, and no new UART algorithm review by the
coordinating lead. The earlier UART fresh-implementer boundary is preserved.
Generated disk/container/configuration/logs stay outside the source repository.

Script/configuration self-review is separate from machine/device conformance.
The trial and any failures are recorded in TEST-0022 and the evidence snapshot.
The final helper records empty output explicitly and admits only the exact
missing-PK refusal with public SecureBoot=off. Actual preparation used retained
manual resumption; the final helper was not replayed on another new VM. Local
observers aggregated own serial, monotonic host timing and safe scene captures;
the reserved-Input harness failure and corrected successor are retained.
B2, edition boots/parity, physical hardware and independent Mac/mini vMac remain
open until their own observed gates pass. No OneNote or further delegation.
