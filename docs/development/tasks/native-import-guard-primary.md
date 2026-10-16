# agy qualification prompt

Primary qualification: read packet.txt once (1..132, EOF known). Deliver substantial usable artifacts through captured output only: (1) the complete original Python guard, max180 lines; (2) concise replacement docs/development/agent-orchestration.md, max110 lines; (3) exact minimal integration edits for removing unused CMake token and wiring guard+independent tests before builds in CI. Return a JSON object with files:[{path,content}] and handoff:{result,actionable_findings,tests_executed,blockers,decisions_needed}. Tests_executed must be empty. Include GPL-3.0-or-later and Copyright(C)2026 Dean Howell in Python. Do not create test artifact: a distinct worker owns independent tests. Stop after final deliverable; no reads beyond packet, tools besides reading, edits or commands. Plan mode permits proposed code, not application. CLI timeout180s. Keep handoff under180 words. All contract/boundaries are in packet. No historical facts inferred.

Input: reviewed-public snapshot `0d0bce1286fc2536e068c0347ae1e3e3dff36293`; frozen packet identity in qualification receipt. Four exact source excerpts and the stated guard API; no entire repository/history or private inputs.

[Shared contract and exact inputs](native-import-contract.md).
