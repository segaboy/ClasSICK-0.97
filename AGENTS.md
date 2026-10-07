# ClasSICK 0.97 working instructions

Read `docs/clean-room/POLICY.md`, `CONTRIBUTING.md`, and
`provenance/README.md` before researching or changing this project.

- Preserve the exact human-facing name **ClasSICK 0.97**.
- Behavioral compatibility is the goal; Apple's implementation is not a source.
- Never fetch, inspect, copy, disassemble, decompile, translate, or prompt with
  Apple executable code, ROMs, leaked source, or prohibited derived material.
- Uncertain provenance makes a source ineligible until reviewed. Record source
  identifiers and document edition/page/section when supporting behavior.
- Keep historical facts, observations, design decisions, and implementations
  explicitly distinguished. Later Macintosh manuals do not prove 1984 behavior.
- Core code must be freestanding, architecture-neutral C. Keep guest addresses
  distinct from native pointers and hardware in platform/architecture layers.
- Implement from approved behavioral specifications; each compatibility change
  needs provenance, tests, and an explicit compatibility status.
- Use only independently generated, rights-cleared public fixtures. Private
  reference materials belong outside the repository and ordinary CI.
- License selection belongs to the project owner. Do not add a license without
  their explicit approval. External implementation contributions are deferred
  while licensing remains undecided.
- The founding assignment is documentation, policy, tooling, and build probes.
  It does not authorize OS subsystem implementation.
- Use `C:\Repos\ClasSICK-0.97` for source and `C:\ClasSICK` for local build/test
  output on the founding workstation. Do not depend on those paths in core code.
- Do not create specialist chats or delegate work merely because the roadmap
  suggests future roles. Create them only when the user requests them.
- Keep bootstrap status honest: build probes are not an OS boot or compatibility
  proof. Update ADRs and evidence when a claim becomes verified.
