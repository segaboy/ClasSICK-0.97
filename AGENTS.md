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
- The owner approved GPL-3.0-or-later on 2026-10-07 for project-owned code,
  documentation and assets. Preserve that license and inbound-equals-outbound
  contribution terms; changes require explicit owner approval. Third-party inputs
  still need separate license/provenance review.
- The founding assignment covered documentation, policy, tooling, and build probes.
  On 2026-10-07 the owner authorized the project implementation chat to finalize,
  implement, test and publish SPEC-0001 bounded portable graphics surfaces. This
  supersedes the bootstrap-only restriction for that subsystem. Subsequent
  milestones remain separate; surfaces do not complete M0.1 or any boot edition.
- The owner then authorized moving on to the Windows presentation adapter and
  original visible test scene. Finalize SPEC-0002, implement and verify that scoped
  milestone, and maintain/publish its evidence using the existing project workflow.
  Arenas, normalized input, clocks and boot work remain subsequent contracts.
- The owner reaffirmed the independent clean-room Mac boot goal (real Macintosh
  and mini vMac validation), then instructed this chat to continue on 2026-10-07.
  This authorizes SPEC-0003 bounded core arenas, viewer-buffer integration, tests,
  provenance and normal progress publication. Input/clock and boot remain later
  contracts. Emulator validation does not close the physical-hardware edition gate.
- The owner then instructed this chat to move to the next step on 2026-10-07.
  This authorizes SPEC-0004 bounded normalized keyboard events, Windows/viewer
  integration, independent tests and ordinary evidence/Git wiki planning. Timing,
  broader input and boot remain subsequent work; B1 remains open.
- The owner authorized moving on to the portable clock on 2026-10-07 and named
  their installed VirtualBox for future boot tests. Finalize/implement/verify
  SPEC-0005 and hosted timed behavior, then publish evidence and Git wiki plans.
  Record VirtualBox availability without claiming a VM boot or hardware result.
  Debugger verification and native boot remain separate gates.
- The owner then said "Do it" after the proposed debugger checkpoint. This
  authorizes hosted LLDB launch/source/state/step/resume verification, necessary
  viewer launch fixes, repeatable checks, provenance/publication and Git wiki plans.
  TEST-0009 / IMPL-0007 are development evidence; B1 acceptance and native boots
  remain separate. Live OneNote still requires an explicit publication request.
- The owner authorized the remaining B1/M0.1 audit with "OK, do it", then
  explicitly requested opening the keyboard validation viewer on 2026-10-08.
  TEST-0010 / IMPL-0008 combine hosted startup/input/timing/shutdown checks;
  the operator session passed two Space presses/releases and Escape. B1/M0.1
  technical acceptance is now passed, with human provenance review separate.
  Native linkage/runtime, a second compiler and all OS boots remain later gates.
- The owner next authorized standalone linking and a second compiler with "Do it"
  on 2026-10-08. ADR-0009 / IMPL-0009 / TEST-0011 cover current-core no-runtime
  PE link fixtures and scoped GCC core conformance, including pinned portable
  tooling and normal evidence/Git wiki publication. These fixtures are never
  loaded; firmware/loader/stack/runtime beyond this profile and B2 remain separate.
- The owner then said "OK, move on" on 2026-10-08 after the proposed firmware
  handoff/device contracts. SPEC-0006 / ADR-0010 / IMPL-0010 / TEST-0012 cover
  original preboot validators/exit model and proposed PC ownership/device profile,
  hosted checks and normal evidence/Git wiki plans. Actual UEFI calls/native entry,
  drivers and boots remain later implementations. Uncertain firmware is ineligible
  until exact provenance is reviewed; do not adopt it from product/license names.
- The owner accepted the next firmware review/native-loader step with "OK, do it"
  on 2026-10-08. SPEC-0007 / ADR-0011 / IMPL-0011 / TEST-0013 cover original x64
  UEFI calls, owned-stack stop scaffold, hosted mock checks, unloaded EFI image
  audits and ordinary evidence/Git wiki publication. Firmware review is limited to
  exact publisher build/license metadata while provenance remains uncertain. Do
  not adopt/run firmware until reviewed; native exceptions/devices/B2 and all
  edition boots remain separate. Preserve real Macintosh/mini vMac goals.
- The owner said "ok, let's do it" after the first independent PC-boot milestone
  was explained on 2026-10-08. This authorizes progressing its missing startup
  components, including SPEC-0008 owned x64 exception state, original implementation,
  tests and normal evidence/Git wiki publication. Exact firmware eligibility and
  observed native execution remain prerequisites to boot claims; no provenance
  waiver, live OneNote instruction, delegation or change to Macintosh goals.
- Use `C:\Repos\ClasSICK-0.97` for source and `C:\ClasSICK` for local build/test
  output on the founding workstation. Do not depend on those paths in core code.
- Do not create specialist chats or delegate work merely because the roadmap
  suggests future roles. Create them only when the user requests them.
- Keep bootstrap status honest: build probes are not an OS boot or compatibility
  proof. Update ADRs and evidence when a claim becomes verified.
- Cross-project evidence and documentation are maintained in the owner's separate
  `C:\Repos\ClasSICK` headquarters repository. Consult its current instructions
  before cross-project changes; keep private headquarters content out of this
  public repository. Its registry preserves this project's local record namespace.
- The owner's latest HQ instruction supersedes automatic OneNote maintenance:
  ordinary progress updates publish wiki plans in the private HQ Git queue.
  Use repository mirrors without OneNote discovery, reads, writes or retries.
  Contact OneNote only on "update wiki" or a specific requested OneNote operation;
  each request authorizes one pass under the OneNote skill and HQ wiki protocol.
  Preserve personal notes and verify writes in that pass; leave unfinished scope
  queued. Never commit personal notebook IDs, URLs or reference payloads here.
