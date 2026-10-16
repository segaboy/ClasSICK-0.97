# copilot qualification prompt

Independent test qualification: read packet.txt once (1..131, EOF known). Return one usable original unittest artifact tests/scene-import-contracts.py (max200 lines) for the specified audit API, with original tiny synthetic fixtures and at least8 independent boundary/mutation tests. You do not see the primary implementation. Also identify at most3 concrete requirements gaps in the stated guard design, without inventing runtime scope. Return a JSON object with files:[{path,content}] and handoff:{result,actionable_findings,tests_executed,blockers,decisions_needed}; tests_executed must be empty. Include GPL-3.0-or-later and Copyright(C)2026 Dean Howell. Only view packet; no edits, commands, tools beyond view, URL/web/MCP/subagents/skills, account/config/credential access or extra files. Do not change account or purchase credits. Stop at final artifact, handoff under180 words. All contract/boundaries are in packet.

Input: reviewed-public snapshot `0d0bce1286fc2536e068c0347ae1e3e3dff36293`; frozen packet identity in qualification receipt. Four exact source excerpts and the stated guard API; no entire repository/history or private inputs.

[Shared contract and exact inputs](native-import-contract.md).
