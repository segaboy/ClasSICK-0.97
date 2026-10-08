# Research questions and technical risks

Status: open unless explicitly closed with source/spec/evidence IDs. Severity
reflects potential scope impact, not confirmed incompatibility.

| ID | Question / weak assumption to test | Evidence and next step | Gate |
| --- | --- | --- | --- |
| R-001 | Which exact January-1984 System, Finder, ROM and hardware define the profile, and which primary source confirms internal 0.97? | Eligible edition-specific documentation and authorized version-identification observation; do not inspect binary code | Historical spec freeze |
| R-002 | Are original Macintosh 128K RAM/ROM budgets feasible for selected native features? | Independent link maps, stack/heap/bitmap budgets, hardware manual constraints; define optional configurations | M0.3 feasibility |
| R-003 | Can startup and ROM replacement initialize the exact Macintosh hardware independently? | Published hardware schematics/specs; overlay/reset/video/input/storage/interrupt experiment plan | M0.3 |
| R-004 | Which trap encodings, calling conventions, callbacks, low-memory globals and patching behaviors are required? | 1984-applicable API documentation and original black-box probes | Trap manager / M0.6 |
| R-005 | How do memory handles, purge/lock/move rules, exhaustion and error results behave in the exact profile? | Edition-specific contracts plus authorized state/side-effect observations | Memory Manager |
| R-006 | Which QuickDraw pixel/region/text corner cases differ by version? | Versioned published interfaces; original geometric probes; independent font metric strategy | QuickDraw |
| R-007 | What is the precise MFS layout, allocation/write behavior and corruption response? | Eligible format/hardware documentation; synthetic generator before interoperability measurements | M0.5 |
| R-008 | Which resource-fork, CODE segment, jump-table and loader rules apply in 1984? | Approved file-format and application ABI specs; synthetic application probes | Resource/loader / M0.6 |
| R-009 | What original 68000 instruction/exception/alignment/address-mask behavior is required? | Eligible published Motorola ISA manual; original conformance tests; no Apple ROM inputs | CPU runtime and m68k native |
| R-010 | What PC machine, firmware and drivers make B2 repeatable? | Review QEMU/firmware packages and licenses, UEFI entry/link/runtime, timing and keyboard contracts | M0.2 |
| R-011 | Which standalone Windows-hosted m68k compiler/linker and helper runtime are auditable and reproducible? | Evaluate GNU m68k cross-build/packaging and native instruction output; no assumption LLVM-MinGW solves m68k | M0.3 |
| R-012 (partially resolved 2026-10-07) | Do selected LLDB and ASan/UBSan work in the pinned hosted package? | TEST-0009 x64 source/state/step/resume passes; x64 sanitizer controls/current conformance pass. WOW64 debugger trial failed; other sanitizer/debug targets remain open | Developer tooling |
| R-013 | Which ARM64 host/board/runner and toolchain will prove runtime portability? | Select a concrete runner; compile and execute same core contracts | M1.0 portability |
| R-014 | Which real applications form a lawful, representative 1984 corpus? | Owner-authorized local possession/use and safe workflow reports; no public binary uploads | M0.6/M0.7/release scope |
| R-015 (closed 2026-10-07) | What project license does the owner approve? | Owner selected GPL-3.0-or-later for own code/docs/assets after reviewing options; dependency license audits remain necessary | ADR-0008 accepted |
| R-016 | Can historical appearance/layout remain useful with independently authored assets? | Define original font metrics/artwork and explicit appearance deviations | UI compatibility |
| R-017 | Which programs depend on ROM entry points, direct hardware, timing or undocumented behavior? | Black-box symptom/probe catalog without internal code inspection | Application matrix |

## Initial debt register

- **TD-001, HQ, before M0.3:** 128K feasibility and exact hardware startup remain
  unproven; architectural optionality is not a size proof.
- **TD-002, tooling owner, before core portability claims:** bootstrap tests only
  one native hosted compiler/architecture; add an alternate compiler and m68k/ARM64
  runtime checks without assuming they are already installed.
- **TD-003, specification owner, before historical behavior implementation:** later
  public Apple manuals are discoverable but no 1984 contract has yet been approved.
- **TD-004, maintainer, before broad contributions:** review enforceable PR protection
  settings and dependency licenses. CODEOWNERS alone does not enforce review.

Reference/specification work never resolves these questions by reconstructing
Apple instructions or importing a convenient implementation of uncertain origin.
