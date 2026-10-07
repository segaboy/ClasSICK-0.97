# IMPL-0001 — Founding documentation and development probes

- Date: 2026-10-07. Owner: project owner; authoring role: Project HQ.
- Paths: founding policies/docs/ADRs, source/provenance templates, PowerShell setup,
  guard/inventory/verification scripts, CMake presets/build rules, CI workflow,
  and `tools/toolchain-smoke/` probes.
- Inputs: SRC-0001 and listed public tool/interface/license references in the
  source catalog. No Apple implementation, ROM, disassembly, decompilation,
  proprietary application, copied artwork or private reference-system payload.
- Specifications: founding requirements; B0 acceptance in the boot document.
  SPEC-0001 is a **proposed future surface contract**, not implemented here.
- Tests: TEST-0001 native smoke via CTest; TEST-0002 two fresh executable hash
  comparison; TEST-0003 index/payload/links/JSON/credential-signature guard.
- Design: independently authored project documents, scripts and minimal
  `puts`/compile-width probes. No operating-system subsystem implemented.
- Third-party content: no vendored source or assets. Downloaded development tools
  stay ignored/outside public commits. Hosted probe links their supplied Windows
  startup/runtime; those dependencies are separate from freestanding OS plans.
- AI assistance: Codex GPT-6-family agent authored/reviewed bootstrap from the
  owner's brief and eligible public interfaces. Allowed inputs are enumerated
  above. No prohibited code was supplied as a prompt or research source. AI
  provenance is a disclosure, not proof; repeatable tooling tests and owner review
  are the concrete review mechanisms.
- Review: Project HQ self-review for scope, policy, links, source boundaries and
  actual build results. No separate human or independently staffed-team review
  is claimed. Owner can inspect the public commit/diff before subsequent work.
- License: GPL-3.0-or-later, explicitly approved by the owner on 2026-10-07 after
  the comparison was prepared. Scope: our code, documentation and assets. Upstream
  tools retain their own notices. No probe binary is committed or released publicly.
- Evidence/limitations: [bootstrap verification](../../docs/development/bootstrap-evidence.md).
  No historical compatibility, debugger/sanitizer, native boot, m68k or ARM64
  execution claim follows from bootstrap.
